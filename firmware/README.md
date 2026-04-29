Firmware for the 2-DOF haptic device
=====================================

Purpose
-------
This folder contains the embedded firmware used on the device's MCU (STM32 Nucleo-G431RB). It implements:
- Device state packet transmission (joint angles, MCU timestamp, sequence number)
- Torque command reception and execution (watchdog, torque slew-rate limiter)
- Low-level encoder and motor driver integration (AS5048 joint encoders, AS5600 motor/rotor encoder, SimpleFOC drivers)

Files
-----
- `firmware.ino` – main firmware sketch/entry (Arduino-style)
- `DeviceComms.cpp` / `DeviceComms.h` – packet packing/parsing and serial I/O
- `Packets.h` – packet formats and constants
- `README.md` – this file

Dependencies
------------
- STM32 Arduino core (for Nucleo G4 series) or equivalent toolchain
- SimpleFOC / SimpleFOC-Mini drivers used for FOC-based torque control
- (Optional) PlatformIO or Arduino IDE for building and flashing

Quick Start — Arduino IDE
-------------------------
1. Open `firmware/firmware.ino` in Arduino IDE.
2. Install the STM32 Arduino core and any required libraries (SimpleFOC, Wire/SPI as needed).
3. Select board: **Nucleo G431RB** (or the closest STM32G4 target) and the correct upload method.
4. Connect the Nucleo board via USB (note COM port) and click Upload.

Quick Start — PlatformIO (VS Code)
----------------------------------
1. Install PlatformIO extension in VS Code.
2. Open the `firmware/` folder as a project.
3. Create a `platformio.ini` with an appropriate board (e.g., `nucleo_g431rb`) and framework `arduino` or `stm32cube`.
4. Run `PlatformIO: Upload` or use the `platformio run -t upload` command.

Baud & Serial
-------------
- Default baud rate used by host and firmware: `460800` (USB virtual COM).
- Packet framing: two-byte header (0xAA 0x55), payload, checksum. See `Packets.h` for exact field layout.

Protocol Summary
----------------
- Device -> Host (State packet): [header][seq][mcu_ts][q1][q2][checksum]
- Host -> Device (Torque cmd): [header][cmd_seq][ref_seq][tau1][tau2][checksum]
(See `Packets.h` and `DeviceComms.*` for precise byte layouts and endianness.)

Pinout / Wiring
---------------
- Joint encoders: AS5048 (SPI) mounted on joint axes
- Motor rotor encoder: AS5600 (I2C) on motor shaft
- Motor drivers: SimpleFOC Mini (FW-driven) – see schematics in docs or `firmware` comments for pin assignments

Pin Assignments (as used in `firmware/firmware.ino`)
--------------------------------------------------
Below are the MCU pin names used in the current firmware. Check `firmware/firmware.ino` if you change wiring or port the code.

- USB Serial: `Serial` at 460800 baud
- I2C (AS5600 motor rotor sensor): `SDA = PB9`, `SCL = PB8`
- Magnetic SPI sensors (AS5048 / SPI):
	- `joint1_sensor` (AS5048) CS: `PA1` (other SPI pins use the hardware SPI peripheral)
	- `motor2_sensor` (AS5048/alt) CS: `PA0`
 - Hardware SPI (STM32 hardware pins typically used by the Arduino core):
	 - `SCK` = `PA5`
	 - `MISO` = `PA6`
	 - `MOSI` = `PA7`
- Motor 1 driver (3PWM): PWM pins `PA8`, `PA9`, `PA10`; enable `PB5` (driver1 = BLDCDriver3PWM(PA8, PA9, PA10, PB5))
- Motor 2 driver (3PWM): PWM pins `PB4`, `PC7`, `PB0`; enable `PB3` (driver2 = BLDCDriver3PWM(PB4, PC7, PB0, PB3))

Notes:
- The `MagneticSensorSPI` constructor in the code also configures SPI mode/clock; CS pins above are the chip-selects used in the project. If you port to different pins, update the constructor calls in `firmware.ino`.
- PWM pin assignments correspond to the STM32 timer channels used by the Nucleo-G431RB Arduino core. Verify with your board variant if using a different MCU.

Safety & Watchdog
-----------------
- A communication watchdog enforces a timeout: if no valid host torque command is received within the timeout window, commanded torques are set to zero.
- A torque slew-rate limiter prevents sudden large torque steps.
- There is no hardware current sensing; use conservative torque limits when testing.

Troubleshooting
---------------
- If upload fails: check selected board, COM port, and drivers for the ST-Link/USB interface.
- If motor behaves erratically: verify encoder wiring (SPI/I2C), correct supply voltages, and motor driver configuration.
- To inspect serial traffic, use a serial terminal at `460800` baud or run the host `serial_tests` tool.

Contributing / Extending
------------------------
- Add a `platformio.ini` if you want reproducible PlatformIO builds.
- Add documented pin macros in `firmware/` headers to make porting easier.

Credits & License
-----------------
See the project root `LICENSE` (if present) for licensing. Acknowledge use of SimpleFOC and STM32 Arduino core per their respective licenses.
