# 🔧 FINAL RESULTS: METRICS, INTERPRETATION, AND FIGURE MAPPING

---

# 🔹 5.2 Simulation & Haptic Rendering Validation (Motors OFF)

⚠️ Penetration is NOT a performance metric here — it is the input to the force model.

---

## **5.2.1 Single-Depth Wall Contact**

| Metric | Value | Figure | File |
|---|---:|---|---|
| mean force magnitude (contact) | 11.4567 N | Fig. 8a | `SingleDepthHold.csv` |
| force std (hold region) | 0.2993 N | Fig. 8a | same |
| force coefficient of variation | 0.0261 | Fig. 8a | same |

### Interpretation
The force response remained stable during sustained contact, with a low coefficient of variation (0.026), indicating minimal fluctuation under constant penetration. Force was only present when penetration occurred, confirming correct contact detection.

---

## **5.2.2 Force–Penetration Characterisation**

| Metric                      |      Value | Figure  | File                  |
| --------------------------- | ---------: | ------- | --------------------- |
| effective stiffness (K_eff) | 489.57 N/m | Fig. 8b | `MultiDepthHolds.csv` |
| fit R²                      |     0.9973 | Fig. 8b | same                  |

### Interpretation
The force–penetration relationship is highly linear (R² = 0.997), with an effective stiffness closely matching the configured virtual coupling stiffness. This confirms that the simulation accurately implements the intended spring-based force model.

---

## **5.2.3 Free-Space Validation**

| Metric | Value | Figure | File |
|---|---:|---|---|
| force magnitude (no contact) | ~0 N | Fig. 8c | `E1_NoContact_simulation_validation_log.csv` |

### Interpretation
No force was generated in free space, confirming correct separation between contact and non-contact states.

---

## **5.2.4 Deep-Penetration Saturation**

| Metric | Value | Figure | File |
|---|---:|---|---|
| max commanded force | 31 N | Fig. 8d | `simulation_validation_log.csv` |

### Interpretation
Force output saturates at approximately 31 N, confirming correct implementation of host-side force limiting under extreme penetration.

---

# 🔹 5.3 Safety Validation

---

## **5.3.1 Torque Saturation**

| Metric | Value | Figure | File |
|---|---:|---|---|
| host saturation rate | 31.104 % | Fig. 9a | `skew_saturation_device_state_log.csv` |
| device saturation rate | 0.000 % | Fig. 9a | same |
| max raw torque | 9.348 N·m | Fig. 9a | same |
| max clamped torque (host) | 7.662 N·m | Fig. 9a | same |
| max applied torque (device) | 6.500 N·m | Fig. 9a | same |

### Interpretation
Host commands exceeded actuator limits and were correctly clamped. The device enforced the 6.5 N·m torque cap without exceeding limits, confirming safe bounded operation.

---

## **5.3.2 Rate Limiting (Torque Slew)**

| Metric | Value | Figure | File |
|---|---:|---|---|
| median slew rate | 0.000 N·m/s | Fig. 9b | `skew_saturation_device_state_log.csv` |
| 95th percentile | 55.000 N·m/s | Fig. 9b | same |
| 99th percentile | 55.005 N·m/s | Fig. 9b | same |
| max slew rate | 55.010 N·m/s | Fig. 9b | same |
| firmware limit | 55.000 N·m/s | Fig. 9b | — |

### Interpretation
Measured slew rates match the firmware limit almost exactly, demonstrating correct implementation of torque rate limiting. Most samples show zero slew, indicating limiting occurs only during transitions.

---

## **5.3.3 Watchdog Behaviour**

| Metric | Value | Figure | File |
|---|---:|---|---|
| watchdog event count | 259 | Fig. 10a | `watchdog_test_device_state_log.csv` |
| watchdog active rate | 6.810 % | Fig. 10a | same |
| total active time | 0.529 s | Fig. 10a | same |
| mean duration | 0.0020 s | Fig. 10a | `watchdog_test_device_timing.csv` |
| max duration | 0.311 s | Fig. 10a | same |

### Interpretation
The watchdog reliably detects communication loss and removes torque. Most activations occur at millisecond timescales, while longer durations correspond to intentional communication interruption.

---

## **5.3.4 System Consistency**

| Metric | Value | Table X | File |
|---|---:|---|---|
| mean torque magnitude error | 0.1343 N·m | Table X | `device_state_log.csv` |

### Interpretation
Torque error remains small relative to operating magnitudes, indicating good agreement between host commands and device execution.

---

# 🔹 5.4 Hardware Interaction Validation (Motors ON)

---

## **5.4.1 Repeated Contact**

| Metric | Value | File |
|---|---:|---|
| mean penetration | 0.0342 m | `Motors_Contact_Move_simulation_validation_log.csv` |
| max penetration | 0.0517 m | same |
| effective stiffness | 490.99 N/m | same |
| fit R² | 0.9979 | same |

### Interpretation
Penetration remains bounded (~0.05 m), confirming stable contact. The effective stiffness matches the model, indicating consistent force rendering under closed-loop interaction.

---

## **5.4.2 Constrained Motion**

| Metric | Value | File |
|---|---:|---|
| mean penetration | 0.0246 m | `constrained_simulation_validation_log.csv` |
| max penetration | 0.0512 m | same |
| median |Fn|/|Ft| | 8.279 | same |
| 95th percentile ratio | 169.351 | same |
| mean force | 15.08 N | same |
| force CV | 0.445 | same |

### Interpretation
The system enforces constraints normal to the surface while allowing tangential motion. High |Fn|/|Ft| confirms correct force direction, while bounded penetration demonstrates stability. Variability reflects expected compliance due to latency.

---

# 📊 REQUIRED FIGURES

| Figure   | Description                    | Required Content                          | File Name (YOU FILL)                          |                                    |     |           |     |
| -------- | ------------------------------ | ----------------------------------------- | --------------------------------------------- | ---------------------------------- | --- | --------- | --- |
| Fig. 8a  | Single-depth contact           | penetration vs time + force vs time       | SingleDepth_Force_Vs_Time.png                 | SingleDepth_pen_Vs_Time.png        |     |           |     |
| Fig. 8b  | Force vs penetration           | scatter + linear fit (K_eff, R²)          | MultiDepth_Force_Vs_Pen.png                   | 489.57 N/m R = 0.9973              |     |           |     |
| Fig. 8c  | Free-space validation          | force vs time (~0 N)                      | SingleDepth_Force_Vs_Time.png                 | SingleDepth_pen_Vs_Time.png        |     |           |     |
| Fig. 8d  | Force saturation               | force vs penetration (plateau)            | Saturated_Force_Vs_Pen.png                    | Could use for Force vs penetration |     |           |     |
| Fig. 9a  | Torque saturation              | raw vs clamped vs applied torque          | Raw_Vs_Clamped.png                            |                                    |     |           |     |
| Fig. 9b  | Slew rate                      | slew vs time or histogram + 55 N·m/s line | SlewHisto.png                                 |                                    |     |           |     |
| Fig. 10a | Watchdog behaviour             | torque vs time + watchdog flag            | Device Safety Validation - Watchdog Focus.png |                                    |     |           |     |
| Fig. 10b | Watchdog zoom (optional)       | zoomed torque drop                        | in the above one                              |                                    |     |           |     |
| Fig. 11a | Hardware contact               | penetration vs time (motors ON)           | MotorON_Pen_vs_Tine.png                       |                                    |     |           |     |
| Fig. 11b | Constrained motion             | Trajectory                                | ConstrainedMotionTraj.png                     |                                    |     |           |     |
| Fig. 11c | Force decomposition (optional) |                                           | Fn                                            | /                                  | Ft  | histogram |     |

---

# 🧠 FINAL TAKEAWAY

This dataset demonstrates:

- accurate force model implementation  
- correct contact logic  
- robust safety mechanisms  
- stable but compliant physical interaction  

👉 This is **complete quantitative validation of the system**