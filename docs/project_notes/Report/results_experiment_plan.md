# Results Section Experiment Plan

Purpose: define the minimum experiment set needed to support the report's results chapter with quantitative evidence for simulation validity, hardware behavior, and safety mechanisms.

## 0) Report-to-Plan Match and Current Data Coverage

Goal of this section: avoid redoing everything by proving that each experiment already written in the report has at least one supporting dataset, then only topping up the weak spots.

Legend:
- `Covered` = at least one dataset already exists and can produce at least one metric/figure.
- `Top-up` = some data exists, but one focused extra run is recommended to strengthen the claim.

| Results chapter experiment (current report)        | Matched plan experiment(s) in this note | Existing data source(s) in repo                                                                                                                                                                             | Coverage status | Minimum metric you can extract now                                           |
| -------------------------------------------------- | --------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------- | ---------------------------------------------------------------------------- |
| Packet integrity and requested-rate tracking       | E8                                      | `Test Data/rate_100hz.csv`, `Test Data/rate_250hz.csv`, `Test Data/rate_500hz.csv`, `Test Data/rate_1000hz.csv`, `Test Data/rate_1250hz.csv`, `Test Data/rate_2000hz.csv`, `Test Data/rate_summary.csv`     | Covered         | dropped packet fraction vs rate; measured vs requested rate                  |
| Round-trip latency (1/2/10 ms ping intervals)      | E8                                      | `Test Data/rtt_1ms.csv`, `Test Data/rtt_2ms.csv`, `Test Data/rtt_10ms.csv`, `Test Data/rtt_summary.csv`                                                                                                     | Covered         | mean/p95/p99 RTT and timeout count                                           |
| End-to-end host-side timing                        | E8                                      | `build/Debug/device_timing.csv`, `Test Data/device_timing.csv`, `Test Data/device_timing_summary.csv`, `Test Data/End_to_End.m`                                                                             | Covered         | parse-to-tx latency mean/p95/max; sequence gap stats                         |
| Simulation virtual wall response                   | E2, E3                                  | `build/Debug/simulation_validation_log.csv`, `Test Data/simulation_validation_log.csv`, `Test Data/simulation_validation_analysis.m`, `Test Data/MultiDepthHolds.csv`, `Test Data/SingleDepthHold.csv`      | Covered         | force vs penetration slope; penetration depth distribution                   |
| Simulation constrained-motion behaviour            | E4                                      | `Test Data/SingleDepthHold.csv`, `Test Data/MultiDepthHolds.csv`, `Test Data/Motors_Contact_Move_simulation_validation_log.csv`                                                                             | Covered         | normal-to-tangential force ratio (median and p95) during contact             |
| Safety: torque saturation                          | E6                                      | `build/Debug/device_timing.csv`, `build/Debug/device_state_log.csv`, `Test Data/device_safety_validation_analysis.m`, `Test Data/MultiDepth_device_timing.csv`, `Test Data/MultiDepth_device_state_log.csv` | Covered         | host/device saturation rate (%)                                              |
| Safety: rate limiting (slew limiter)               | E7                                      | `build/Debug/device_timing.csv`, `build/Debug/device_state_log.csv`, `Test Data/device_safety_validation_analysis.m`                                                                                        | Top-up          | max/p95 absolute torque slew (d tau/dt) under current limiter setting        |
| Safety: watchdog timeout behaviour                 | E5                                      | `build/Debug/device_state_log.csv`, `build/Debug/device_timing.csv`, `Test Data/device_state_log.csv`, `Test Data/device_timing.csv`                                                                        | Covered         | watchdog timeout active fraction and timeout windows                         |
| Hardware actuator torque response characterisation | E6, E7                                  | `Test Data/Motors_Contact_Move_device_timing.csv`, `Test Data/Motors_Contact_Move_device_state_log.csv`                                                                                                     | Top-up          | commanded vs applied torque step/ramp response metrics                       |
| Hardware stable contact (virtual wall interaction) | E3, E6, E7                              | `Test Data/lighttouchandhold.csv`, `Test Data/TouchHold1.csv`, `Test Data/MultiDepthHolds.csv`                                                                                                              | Covered         | hold drift and applied torque stability during contact                       |
| Hardware constrained-motion interaction            | E4, E6                                  | `Test Data/Motors_Contact_Move_simulation_validation_log.csv`, `Test Data/Motors_Contact_Move_device_state_log.csv`, `Test Data/Motors_Contact_Move_device_timing.csv`                                      | Covered         | sustained contact on surface + guided tangential motion with no push-through |

## 0.1) Newly Completed Runs (Simulation) - 30 Apr 2026

These are now usable directly in the Results chapter to strengthen claims without major rewrite.

### Single-depth hold (`Test Data/SingleDepthHold.csv`)
- Penetration statistics (m): mean 0.039970, median 0.040459, p95 0.040851, max 0.041363.
- Force vs penetration fit: effective stiffness slope 478.164 N/m, intercept 0.968589 N, R^2 0.9918.
- Constrained-motion force decomposition: median |F_n|/|F_t| = 19.866, p95 = 572.706.
- Sustained contact hold (2.80 s to 3.60 s): force mean 20.3251 N, std 0.1734 N, CV 0.0085.

### Multi-depth holds (`Test Data/MultiDepthHolds.csv`)
- Penetration statistics (m): mean 0.027978, median 0.026311, p95 0.056928, max 0.063802.
- Force vs penetration fit: effective stiffness slope 488.989 N/m, intercept 0.557136 N, R^2 0.9973.
- Constrained-motion force decomposition: median |F_n|/|F_t| = 10.611, p95 = 333.805.
- Sustained contact hold (4.90 s to 6.20 s): force mean 15.5800 N, std 8.0762 N, CV 0.5184.

### Immediate claim support unlocked from these two runs
- Virtual wall force model is strongly supported quantitatively: stiffness fit is consistent across runs (~478-489 N/m) with high linearity (R^2 > 0.99).
- Constraint-force directionality is supported: |F_n|/|F_t| median > 10 in both runs.
- Stable hold claim is strongly supported for the single-depth case (CV 0.0085).
- Multi-depth hold shows broader variability (CV 0.5184), which is useful as balanced evidence of non-uniform stability across operating regions.

### Motors-on constrained surface run (data captured)
- You have motors-on contact-and-surface-motion logs saved (`Motors_Contact_Move_*`).
- This means the hardware constrained-motion subsection now has at least one direct dataset.
- Minimum additional work is analysis/plot extraction only (no mandatory rerun unless you want repeats).

### Minimal top-up runs (to strengthen claims without major rewrite)

Run only these focused additions so you can keep the existing report structure and mostly adjust methodology/results wording:

1. Analyze existing motors-on constrained-motion logs to produce one normal-vs-tangential evidence figure (no rerun required for minimum evidence).
2. Slew A/B run (same trajectory, two limiter settings) to directly support rate-limiter effectiveness claim.
3. One watchdog injection run with deliberate command pause (for example 100 ms) to show a clean timeout event timeline.
4. One actuator response run with clear step/ramp command profile to support the torque-response subsection with numeric evidence.

### Main-body figure mapping to your current Results chapter

1. Communication: dropped packets vs requested rate and requested vs measured rate (already in chapter).
2. Communication: RTT CDF (already in chapter).
3. Communication: end-to-end latency histogram + parsed inter-arrival histogram (already in chapter).
4. Simulation: penetration depth vs force (add/keep as primary quantitative simulation figure).
5. Safety: watchdog timeout timeline with applied torque overlay.
6. Safety: raw vs clamped torque and saturation flags.
7. Safety: slew A/B comparison figure (new, high value for strengthening claim).
8. Hardware: one stable-contact hold trace (position/penetration + torque) with short quantitative caption.

## 1) Evidence Map (Claim -> Experiment)

| Claim in report | Evidence experiment(s) | Primary metric(s) |
|---|---|---|
| Simulation contact behavior is physically plausible | E1, E2, E3 | penetration depth (m), effective stiffness (N/m), hold drift (mm/s) |
| Controller is stable in hold/contact | E3, E4 | oscillation amplitude, settling time, overshoot |
| Host torque limiting prevents unsafe command spikes | E6 | host saturation rate (%), clamped-vs-raw torque delta |
| Device watchdog correctly rejects stale commands | E5 | watchdog timeout rate (%), timeout duration windows |
| Device slew limiter reduces abrupt torque changes | E7 | max/95th torque slew (N m/s), applied-vs-commanded slew |
| Device follows commands with acceptable error | E6, E7 | torque MAE/RMSE, magnitude error |
| End-to-end comms supports control loop assumptions | E8 | command inter-arrival stats, sequence continuity, latency stats |

## 2) Required Experiments

Run each experiment for at least 3 repeats unless stated otherwise.

### E1. Free-space baseline (simulation)
- Goal: establish zero-contact baseline and sensor/noise floor.
- Procedure: run nominal trajectory with no environment contact for 30-60 s.
- Log needed: `simulation_validation_log.csv`.
- Key outputs: false contact rate, baseline force/torque noise, baseline position drift.

### E2. Quasi-static penetration sweep (simulation)
- Goal: validate penetration-depth and force relationship.
- Procedure: apply slow approach into virtual surface at multiple target depths (for example 2, 5, 10, 15 mm), hold each depth 5-10 s.
- Log needed: `simulation_validation_log.csv`.
- Key outputs: penetration depth stats, fitted stiffness slope, linear-fit R^2.

### E3. Contact hold stability (simulation)
- Goal: show stable behavior during sustained contact.
- Procedure: pick one representative penetration depth and hold for 20-30 s.
- Key outputs: mean penetration depth, standard deviation, drift rate (mm/s), oscillation amplitude.

### E4. Dynamic trajectory under contact (simulation)
- Goal: characterize dynamic response.
- Procedure: run a slow sine or step profile while in/near contact.
- Key outputs: settling time, overshoot, peak error, frequency-dependent error (if sine sweep).

### E5. Watchdog timeout safety test (hardware)
- Goal: prove stale commands are rejected.
- Procedure:
1. Normal operation 10-20 s.
2. Intentionally pause/interrupt host command transmission for controlled windows (for example 50 ms, 100 ms, 500 ms).
3. Resume commands.
- Logs needed: `device_state_log.csv`, `device_timing.csv`.
- Key outputs: timeout detection latency, timeout active fraction, torque during timeout (should decay/zero per design).

### E6. Saturation behavior test (hardware)
- Goal: show host and device clamps engage as expected.
- Procedure: command force profiles that intentionally request torque beyond limits.
- Logs needed: `device_timing.csv`, `device_state_log.csv`.
- Key outputs: host saturation rate (%), device saturation rate (%), raw-vs-clamped torque envelope.

### E7. Slew limiter effectiveness test (hardware)
- Goal: show reduced abrupt torque transitions.
- Procedure: run same aggressive command profile under two settings (A/B):
1. nominal slew limit
2. high/relaxed slew limit (or disabled if safe)
- Logs needed: `device_timing.csv`, `device_state_log.csv`.
- Key outputs: max and 95th percentile slew, reduction percentage between A/B, applied torque smoothness.

### E8. Comms timing robustness test (hardware)
- Goal: verify command cadence and timing margins relative to timeout.
- Procedure: run at target loop rates (for example 100, 250, 500, 1000 Hz) or current production rate and collect long-enough traces (>=60 s).
- Logs needed: `device_timing.csv`.
- Key outputs: inter-arrival mean/median/p95/p99/max, fraction above timeout threshold, sequence gaps.

## 3) Minimum Metrics to Report

### Simulation metrics
- Penetration depth: min, max, mean, RMS, p95.
- Effective stiffness estimate: slope from force vs penetration regression with confidence interval.
- Hold stability: drift rate (mm/s), standard deviation, oscillation amplitude.
- Dynamic response: overshoot (%), settling time (s), steady-state error.

### Hardware/safety metrics
- Watchdog timeout rate (%) and total timeout duration.
- Timeout response: applied torque immediately before, during, after timeout window.
- Host saturation: per-joint and any-joint rate (%).
- Device saturation: per-joint and any-joint rate (%).
- Slew metrics: max, mean, p95, p99 of |d tau/dt|.
- Command tracking: MAE/RMSE per joint and magnitude error.
- Timing: inter-command gap distribution (mean, p95, p99, max), fraction above timeout.

## 4) Figures for Main Body (must include)

Keep main body concise: around 6-8 figures total.

1. Simulation penetration-depth vs contact force with fitted stiffness line.
2. Contact hold stability time-series (depth and force over time).
3. Safety timeline showing watchdog timeout flag and applied torque (single representative run).
4. Host raw vs clamped torque commands (saturation evidence).
5. Device commanded vs applied torque with error overlay (tracking evidence).
6. Slew-rate comparison A/B (nominal limiter vs relaxed/disabled) using CDF or boxplot.
7. Command inter-arrival histogram/CDF with timeout threshold marker.

## 5) Figures for Appendix (recommended)

1. Per-joint full-length torque traces for all runs.
2. Per-run watchdog timeout windows and raw event tables.
3. Additional saturation plots by experiment condition.
4. Detailed slew-rate time-series for each test condition.
5. Sequence-number continuity plots and packet-gap diagnostics.
6. Sensitivity plots (different loop rates / timeout values).
7. Repeatability plots across all 3+ trials per condition.

## 6) Experiment Recording Template

Use this table per run to keep data report-ready.

| Run ID | Experiment | Date | Config (timeout, slew, rate) | Duration | Key result summary | Include in main body? |
|---|---|---|---|---|---|---|
| E2-R1 | Penetration sweep |  |  |  |  |  |

## 7) Suggested Order of Execution

1. E1 baseline simulation.
2. E2 penetration sweep.
3. E3 hold stability.
4. E4 dynamic response.
5. E8 comms timing (to confirm watchdog margin).
6. E5 watchdog timeout injection.
7. E6 saturation stress.
8. E7 slew A/B comparison.

## 8) Acceptance Checklist Before Writing Results

- All required experiments completed with at least 3 repeats.
- Metrics table populated for every experiment.
- Main-body figures selected and captioned with quantitative takeaways.
- Appendix figures exported and referenced in text.
- Every major claim in the results section maps to at least one numeric metric and one figure.
