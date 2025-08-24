# **Budget Proposal**

## Microcontroller

* Arduino £10-20

  | Pros                           | Cons                                |
  | ------------------------------ | ----------------------------------- |
  | -Easy to Use                   | -Limited Performace                 |
  | -Works with simple FOC         | -No Built in wireless communication |
  |                                | -Requires External Motor Driver     |

* STM32 Nucleo £20-30
* ESP32 £10-15

  | Pros                             | Cons                            |
  | -------------------------------- | -------------------------------- |
  | -Built in Wireless Communication | -Limited Performace             |
  | -Works with simpleFOC            | -Requires External Motor Driver |

* ODrive £150

  | Pros                 | Cons       |
  | -------------------- | ---------- |
  | -Acts as Motor Driver | -Expensive |
  | -Supports FOC        |            |
  | -Well documented API |            |

## Motor Driver

* **TMC6300** £3–12

  | Pros                                 | Cons                                  |
  | ------------------------------------ | ------------------------------------- |
  | -Very cheap, compact                 | -Low power (only small gimbal motors) |
  | -Proven in SmartKnob project         | -Requires MCU to run FOC              |
  | -Breakout boards available (£8–12)   | -Single-axis only                     |

* **SimpleFOC Shield** £30–70

  | Pros                               | Cons                             |
  | ---------------------------------- | -------------------------------- |
  | -Works directly with Arduino/STM32 | -Lower current capability        |
  | -Great for learning FOC            | -Scaling beyond 2 axes is tricky |
  | -Open-source, large community      | -Requires careful tuning         |

* **VESC 4/6** £100–150

  | Pros                                   | Cons                               |
  | -------------------------------------- | ---------------------------------- |
  | -Mature open-source ecosystem          | -Higher cost than SimpleFOC        |
  | -Powerful FOC implementation           | -Bigger than needed for gimbals    |
  | -Tuning & monitoring via VESC Tool GUI | -Single-axis per board (Expensive) |
  | -Supports encoders, sensors, CAN, etc. |                                    |

* **ODrive S1** £150

  | Pros                                | Cons                           |
  | ----------------------------------- | ------------------------------ |
  | -Dual-axis driver (2 motors)        | -More expensive than SimpleFOC |
  | -Robust FOC + torque/position modes | -Overkill for small motors     |
  | -Easy PC/ROS/Isaac Sim integration  | -Larger footprint              |
  | -Well-documented Python API         |                                |

* **Moteus R4.11** £200

  | Pros                             | Cons                      |
  | -------------------------------- | ------------------------- |
  | -Research-grade servo controller | -Very expensive per motor |
  | -Encoder interface built-in      | -Less beginner-friendly   |
  | -CAN-FD networking support       |                           |
  | -Proven in robotics research     |                           |
  |                                  |                           |

## Motor

* **T-Motor GB2208 / GB4106 / GB4108 (Gimbal BLDC)** £25–70

  | Pros                        | Cons                                |
  | --------------------------- | ----------------------------------- |
  | -Cheap and widely available | -Low torque (~0.1–0.5 N·m cont.)    |
  | -Very smooth, backdrivable  | -Can’t render stiff walls/textures  |
  |                             | -Requires external encoder          |

* **Hoverboard Hub Motor (modded BLDC)** ~£50

  | Pros                                  | Cons                                |
  | ------------------------------------- | ----------------------------------- |
  | -Very cheap for high torque (~5 N·m)  | -Bulky and heavy                    |
  | -Easily available second-hand         | -Not designed for precision haptics |
  | -Already has Hall sensors             | -High inertia, poor backdrivability |

* **Maxon EC-i / EC-flat series BLDC** £200–500+

  | Pros                               | Cons                       |
  | ---------------------------------- | -------------------------- |
  | -High quality, very smooth torque  | -Expensive                 |
  | -Used in research/industry haptics | -Requires encoder add-on   |
  | -Low cogging, reliable             | -Long lead times sometimes |

* **Direct-Drive Torque Motor (Kollmorgen KBM, Teknic, etc.)** £500–1,500

  | Pros                                                | Cons                      |
  | --------------------------------------------------- | ------------------------- |
  | -“Gold standard” for haptics                        | -Very expensive           |
  | -High torque (5–20+ N·m cont.)                      | -Large size / power needs |
  | -Excellent backdrivability & fidelity               | -Overkill for project     |
  | -Paired with optical encoders → surgical precision  |                           |
  |                                                     |                           |

## Encoder

* **AS5600 (Magnetic, I²C)** £5–8

  | Pros                              | Cons                           |
  | --------------------------------- | ------------------------------ |
  | -Very cheap, small form factor    | -Lower resolution (12-bit)     |
  | -Easy I²C interface               | -Sensitive to magnet placement |
  | -Good for proof-of-concept builds | -Limited speed performance     |

* **AS5048A (Magnetic, SPI/PWM)** £15–25

  | Pros                                     | Cons                                       |
  | ---------------------------------------- | ------------------------------------------ |
  | -Higher resolution (14-bit)              | -Slightly more expensive                   |
  | -SPI or PWM output → flexible            | -Still less accurate than optical encoders |
  | -Widely used in robotics and DIY haptics | -Needs careful magnet alignment            |

* **MT6701 (Magnetic, Absolute)** £15–20

  | Pros                                 | Cons                             |
  | ------------------------------------ | -------------------------------- |
  | -Compact, absolute angle measurement | -Requires custom PCB or breakout |
  | -Used in SmartKnob project (proven)  | -Availability can vary           |
  | -14-bit resolution, low cost         |                                  |

* **Optical Incremental Encoder (CUI, US Digital, etc.)** £50–200

  | Pros                                     | Cons                            |
  | ---------------------------------------- | ------------------------------- |
  | -Very high resolution (up to 20k+ CPR)   | -Expensive compared to magnetic |
  | -Industry standard for precision haptics | -Bulky vs magnetic ICs          |
  | -Stable, not magnet placement sensitive  | -Requires quadrature interface  |

* **High-end Absolute Optical Encoder (Renishaw, Heidenhain)** £300–1,000+

  | Pros                                        | Cons                          |
  | ------------------------------------------- | ----------------------------- |
  | -Ultra-high precision (16–24 bit absolute)  | -Extremely expensive          |
  | -Standard in surgical/industrial haptics    | -Overkill for student project |
  | -Robust, accurate, no magnet needed         | -Larger size, integration effort |
  |                                             |                               |

## Budget Options

### Option A — Ultra‑Budget 2‑DOF (ESP32 + TMC6300s) ~£160–£220

MCU: ESP32 (£15)

Drivers: 2× TMC6300 breakouts (2× £10 = £20)

Motors: 2× T‑Motor GB2208 (2× £30 = £60) (or GB4106 if found at similar price)

Encoders: 2× AS5600 (2× £8 = £16)

Other: PSU (24 V/3‑5 A) + wiring + 3D prints/frame (£50–£90)

| Pros | Cons |
| ---- | ---- |
| Cheapest 2‑DOF haptics; proven parts; very light & backdrivable | Low torque; careful FOC tuning; single‑axis drivers = more wiring |

### Option B — Mid‑Range 2‑DOF (SimpleFOC) ~£300–£420

MCU: Teensy 4.1 (£35) (or STM32 Nucleo £25)

Drivers: 2× SimpleFOC shields / MOSFET stages (2× £50 = £100)

Motors: 2× T‑Motor GB4108 (2× £60 = £120)

Encoders: 2× AS5048A (2× £20 = £40) (or MT6701 £15–20 ea)

Other: PSU (24 V/5 A) + wiring + frame (£80–£120)

| Pros | Cons |
| ---- | ---- |
| Solid 2‑DOF with smoother torque & higher resolution;  | More tuning/time; two power stages; MCU handles all encoder I/O |

### Option C — Polished Dual‑Axis 2‑DOF (ODrive S1) ~£390–£510

Controller/Driver: ODrive S1 (dual‑axis) (£150)

Motors: 2× T‑Motor GB4108 (2× £60 = £120)

Encoders: 2× AS5048A or MT6701 (2× £20 = £40)

Host: PC (USB) or small MCU for high‑rate loop (optional £15–35)

Other: PSU (24 V/5–10 A) + wiring + frame (£80–£120)

| Pros | Cons |
| ---- | ---- |
| Easiest robust path; dual‑axis on one board; torque/velocity/position modes; great PC/ROS/Isaac Sim workflow | Highest cost of the three; ODrive is oversized for tiny gimbals |

### Option D — 2× VESC (alternative high) ~£420–£560

Drivers: 2× VESC 4/6 (2× £120 = £240)

Motors/Encoders/Other: same as Option C (≈ £180–£320 total)

| Pros | Cons |
| ---- | ---- |
| Mature tooling (VESC Tool), encoder inputs on board, per‑axis isolation | Two separate boards (space/cabling), pricier than ODrive for 2‑DOF |

### Notes

* PSU cost can be avoided if a bench lab supply is available; otherwise budget for a dedicated 24 V unit.
* Higher-end motor drivers improve current control, allowing torque to be applied more precisely (stiffer, more accurate haptics).
* A capstan (or belt) drive could increase effective torque with minimal impact on feel if low friction/compliance, but adds mechanical complexity.
