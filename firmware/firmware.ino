#include <SimpleFOC.h>
#include <Wire.h>
#include "DeviceComms.h"

constexpr bool ENABLE_MOTORS = false;

// ===================== Motors / Drivers =====================

// motor 1
BLDCMotor motor1 = BLDCMotor(7);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(PA8, PA9, PA10, PB5);

// motor 2
BLDCMotor motor2 = BLDCMotor(7);
// TODO: choose correct PWM pins for motor2
BLDCDriver3PWM driver2 = BLDCDriver3PWM(PB4, PC7, PB0, PB3);

// ===================== Sensors =====================

// Motor 1 sensor (I2C AS5600)
MagneticSensorI2C motor1_sensor = MagneticSensorI2C(AS5600_I2C);

// SPI sensors
MagneticSensorSPI joint1_sensor = MagneticSensorSPI(PA1, 14, 0x3FFF, 1000000);
MagneticSensorSPI motor2_sensor = MagneticSensorSPI(PA0, 14, 0x3FFF, 1000000);

// ===================== Comms =====================

DeviceComms device;

// ===================== State =====================

float joint_angle0 = 0.0f;   // from joint1_sensor
float joint_angle1 = 0.0f;   // from motor2_sensor

float joint0_zero_offset = 0.0f;
float joint1_zero_offset = 0.0f;

float applied_torque1 = 0.0f;
float applied_torque2 = 0.0f;
bool watchdog_active = false;
bool torque_saturated1 = false;
bool torque_saturated2 = false;

uint32_t state_seq = 0;

// ===================== Helpers =====================
float joint_angle1_filt = 0.0f;
bool joint1_filter_init = false;

float joint_angle0_filt = 0.0f;
bool joint0_filter_init = false;


float lowPass(float previous, float current, float alpha) {
  return previous + alpha * (current - previous);
}

float clampSymmetric(float x, float limit) {
  if (x > limit)  return limit;
  if (x < -limit) return -limit;
  return x;
}

float getCommandedTorqueWithTimeout(DeviceComms* dc,
                                    int index,
                                    unsigned long now_us,
                                    unsigned long timeout_us,
                                    float torque_cap) {
  float cmd = 0.0f;

  if ((unsigned long)(now_us - dc->last_command_us) < timeout_us) {
    cmd = dc->commanded_torque[index];
  }

  return clampSymmetric(cmd, torque_cap);
}

void updateSlewLimitedTorque(float& applied_torque,
                             float target_torque,
                             unsigned long now_us,
                             unsigned long& last_update_us,
                             float slew_rate,
                             float torque_cap,
                             bool& saturated) {
  float dt = (last_update_us == 0) ? 0.0f : (now_us - last_update_us) * 1e-6f;
  last_update_us = now_us;
  saturated = false;

  if (dt > 0.0f) {
    float max_step = slew_rate * dt;
    float error = target_torque - applied_torque;

    if (error > max_step)  error = max_step;
    if (error < -max_step) error = -max_step;

    if (error != target_torque - applied_torque) {
      //saturated = true;
    }

    applied_torque += error;
  }

  float clipped = clampSymmetric(applied_torque, torque_cap);
  if (clipped != applied_torque) {
    saturated = true;
  }

  applied_torque = clipped;
}

// ===================== Setup =====================

void setup() {
  Serial.begin(460800);
  delay(800);

  // IMPORTANT:
  // Do not use SimpleFOCDebug on same Serial as binary comms.
  // It will corrupt packet traffic.
  // /SimpleFOCDebug::enable(&Serial);

  DeviceComms_Init(&device, Serial);

  // ---------- I2C ----------
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  Wire.begin();

  // ---------- Sensors ----------
  motor1_sensor.init(&Wire);
  motor1.linkSensor(&motor1_sensor);

  joint1_sensor.init();

  motor2_sensor.init();
  motor2.linkSensor(&motor2_sensor);

  // ---------- Initial angle calibration ----------
  joint1_sensor.update();
  motor2_sensor.update();

  float raw0 = joint1_sensor.getAngle();
  float raw1 = motor2_sensor.getAngle();

  // Want startup pose to read:
  // joint0 = 90 deg = pi/2
  // joint1 = 0 deg
  joint0_zero_offset = raw0 - PI / 2.0f;
  joint1_zero_offset = raw1;

  if (ENABLE_MOTORS) {
    // ---------- Driver 1 ----------
    driver1.voltage_power_supply = 12.0f;
    driver1.voltage_limit = 12.0f;
    driver1.pwm_frequency = 20000;

    // ---------- Driver 2 ----------
    driver2.voltage_power_supply = 12.0f;
    driver2.voltage_limit = 8.0f;
    driver2.pwm_frequency = 20000;

    // ---------- Init drivers ----------
  if (!driver1.init()) {
    while (1);
  }
  Serial.println("driver1 ok");

  if (!driver2.init()) {
    while (1);
  }
  Serial.println("driver2 ok");

    // ---------- Link drivers ----------
    motor1.linkDriver(&driver1);
    motor2.linkDriver(&driver2);

    // ---------- Motor config ----------
    motor1.controller = MotionControlType::torque;
    motor1.torque_controller = TorqueControlType::voltage;
    motor1.voltage_limit = 12.0f;
    motor1.voltage_sensor_align = 12.0f;

    motor2.controller = MotionControlType::torque;
    motor2.torque_controller = TorqueControlType::voltage;
    motor2.voltage_limit = 6.0f;
    motor2.voltage_sensor_align = 6.0f;

    // ---------- Init motors ----------
    if (!motor1.init()) {
      while (1);
    }

    if (!motor2.init()) {
      while (1);
    }

    if (!motor1.initFOC()) {
      Serial.println("Motor 1 FOC Fail");
      while (1);
    }

    if (!motor2.initFOC()) {
      Serial.println("Motor 2 FOC Fail");
      //while (1);
    }

    Serial.println("Starting with motors enabled");
  } else {
    Serial.println("Starting in sensor-only mode");
  }
}

// ===================== Loop =====================

void loop() {
  static unsigned long last_comms_us = 0;
  static unsigned long last_torque_update1_us = 0;
  static unsigned long last_torque_update2_us = 0;

  const unsigned long comms_period_us = 1000;   // 1 kHz
  const unsigned long cmd_timeout_us  = 15000;  // 15 ms

  const float torque_slew_rate = 55.0f; // units per second //65 felt ok 200 okish 
  const float torque_cap = 6.5f;

  unsigned long now = micros();

  // FOC update for both motors
  if (ENABLE_MOTORS) {
    motor1.loopFOC();
    motor2.loopFOC();
  }

  // Receive newest command packets
  DeviceComms_Receive(&device);

  // NOTE: Inverted semantics: watchdog_active==1 now indicates a TIMEOUT (no
  // recent command) so that a "1" represents a safety fault (out-of-date
  // command). This matches the safety-validation analysis expectations.
  watchdog_active = (unsigned long)(now - device.last_command_us) >= cmd_timeout_us;

  // Get target torques regardless of motor enable so the packet always reflects
  // the commanded/applied safety state.
  float target_torque1 = getCommandedTorqueWithTimeout(&device, 0, now, cmd_timeout_us, torque_cap);
  float target_torque2 = getCommandedTorqueWithTimeout(&device, 1, now, cmd_timeout_us, torque_cap);

  updateSlewLimitedTorque(applied_torque1, target_torque1, now, last_torque_update1_us, torque_slew_rate, torque_cap, torque_saturated1);
  updateSlewLimitedTorque(applied_torque2, target_torque2, now, last_torque_update2_us, torque_slew_rate, torque_cap, torque_saturated2);

  device.applied_torque[0] = applied_torque1;
  device.applied_torque[1] = applied_torque2;
  device.watchdog_active = watchdog_active ? 1 : 0;
  device.saturation_active[0] = torque_saturated1 ? 1 : 0;
  device.saturation_active[1] = torque_saturated2 ? 1 : 0;

  // Only do torque path if motors are enabled
  if (ENABLE_MOTORS) {
    // Apply torques
    motor1.move(applied_torque1);
    motor2.move(applied_torque2);
  }

  // Stream state at fixed 1 kHz
  if ((unsigned long)(now - last_comms_us) >= comms_period_us) {
    last_comms_us += comms_period_us;

    // joint_angle0 from joint1_sensor
    // joint_angle1 from motor2_sensor
    joint1_sensor.update();
    motor2_sensor.update();
  float raw0 = joint1_sensor.getAngle();
  float raw1 = motor2_sensor.getAngle();

  float joint_angle0_raw = raw0 - joint0_zero_offset;
  float joint_angle1_raw = raw1 - joint1_zero_offset;

  // Low-pass filter for joints
  const float joint0_alpha = 0.15f;  // lower = smoother, higher = faster response
  const float joint1_alpha = 0.1f;  // lower = smoother, higher = faster response

  if (!joint0_filter_init) {
    joint_angle0_filt = joint_angle0_raw;
    joint0_filter_init = true;
  } else {
    joint_angle0_filt = lowPass(joint_angle0_filt, joint_angle0_raw, joint0_alpha);
  }

  joint_angle0 = joint_angle0_filt;

  if (!joint1_filter_init) {
    joint_angle1_filt = joint_angle1_raw;
    joint1_filter_init = true;
  } else {
    joint_angle1_filt = lowPass(joint_angle1_filt, joint_angle1_raw, joint1_alpha);
  }

  joint_angle1 = joint_angle1_filt;
    DeviceComms_SetState(&device, state_seq, now, joint_angle0, joint_angle1);
    DeviceComms_SendState(&device);

    state_seq++;
  }
}