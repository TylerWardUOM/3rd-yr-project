%% =========================================================
% DEVICE COMMUNICATION ANALYSIS
%
% This script analyses:
%
% 1) device_timing.csv
%    Matched state-command timing log
%    Use for:
%      - host-side end-to-end latency
%      - matched sequence integrity
%      - command/state linkage
%      - TX timing
%
% 2) device_state_log.csv
%    Raw parsed state-packet log
%    Use for:
%      - every valid packet parsed on the host
%      - raw state sequence progression
%      - host-side parsed-packet inter-arrival timing
%      - MCU timestamp progression
%      - estimated parsed packet rate
%
% IMPORTANT:
% device_state_log.csv is now generated INSIDE the packet parse loop.
% Therefore each row corresponds to one valid parsed state packet,
% not merely the newest retained packet from a host update cycle.
%
% NOTE:
% Host and MCU clocks are not synchronized.
% Therefore:
%   - host-side latency metrics are trustworthy
%   - MCU timestamp analysis is useful for trend/rate checks
%   - absolute host-to-MCU one-way latency cannot be inferred
%% =========================================================

clear;
clc;
close all;

%% =========================================================
% LOAD FILES
%% =========================================================

timingFile = "device_timing.csv";
stateFile  = "device_state_log.csv";

Timing = readtable(timingFile, "VariableNamingRule", "preserve");
State  = readtable(stateFile,  "VariableNamingRule", "preserve");

fprintf("Loaded files:\n");
fprintf("  %s\n", timingFile);
fprintf("  %s\n\n", stateFile);

%% =========================================================
% PART 1: MATCHED CLOSED-LOOP TIMING ANALYSIS
%% =========================================================

fprintf("========================================\n");
fprintf("MATCHED CLOSED-LOOP TIMING ANALYSIS\n");
fprintf("========================================\n");

matchedRows = (Timing.("rx_state_seq") == Timing.("ref_state_seq"));

numRows = height(Timing);
numMatched = sum(matchedRows);
numUnmatched = sum(~matchedRows);

fprintf("Total rows:           %d\n", numRows);
fprintf("Matched rows:         %d\n", numMatched);
fprintf("Unmatched rows:       %d\n", numUnmatched);
fprintf("Matched fraction:     %.4f\n", numMatched / max(numRows,1));

Tm = Timing(matchedRows, :);

% Host-side timing metrics
host_e2e_ms = (Tm.("t_tx_done_ns") - Tm.("t_rx_parse_ns")) / 1e6;
publish_ms  = (Tm.("t_tool_publish_ns") - Tm.("t_rx_parse_ns")) / 1e6;
pipeline_ms = (Tm.("t_wrench_consume_ns") - Tm.("t_tool_publish_ns")) / 1e6;
tx_ms       = (Tm.("t_tx_done_ns") - Tm.("t_tx_start_ns")) / 1e6;

% Sequence gaps within matched rows
matched_state_seq_gap = [NaN; diff(Tm.("rx_state_seq"))];
matched_cmd_seq_gap   = [NaN; diff(Tm.("tx_cmd_seq"))];

fprintf("\n========================================\n");
fprintf("MATCHED SEQUENCE GAP ANALYSIS\n");
fprintf("========================================\n");

fprintf("Mean matched state seq gap:   %.3f\n", mean(matched_state_seq_gap(2:end), 'omitnan'));
fprintf("Median matched state seq gap: %.3f\n", median(matched_state_seq_gap(2:end), 'omitnan'));
fprintf("Max matched state seq gap:    %d\n", max(matched_state_seq_gap(2:end)));

fprintf("Mean matched cmd seq gap:     %.3f\n", mean(matched_cmd_seq_gap(2:end), 'omitnan'));
fprintf("Median matched cmd seq gap:   %.3f\n", median(matched_cmd_seq_gap(2:end), 'omitnan'));
fprintf("Max matched cmd seq gap:      %d\n", max(matched_cmd_seq_gap(2:end)));

fprintf("Rows with matched state skip (>1): %d\n", sum(matched_state_seq_gap(2:end) > 1));
fprintf("Rows with matched cmd skip   (>1): %d\n", sum(matched_cmd_seq_gap(2:end) > 1));

fprintf("\n========================================\n");
fprintf("HOST-SIDE END-TO-END LATENCY\n");
fprintf("========================================\n");

fprintf("Mean: %.4f ms\n", mean(host_e2e_ms));
fprintf("Median: %.4f ms\n", median(host_e2e_ms));
fprintf("Std:  %.4f ms\n", std(host_e2e_ms));
fprintf("P95:  %.4f ms\n", prctile(host_e2e_ms,95));
fprintf("P99:  %.4f ms\n", prctile(host_e2e_ms,99));
fprintf("Max:  %.4f ms\n", max(host_e2e_ms));

fprintf("\n");
fprintf("Publish delay mean:   %.4f ms\n", mean(publish_ms));
fprintf("Publish delay median: %.4f ms\n", median(publish_ms));
fprintf("Pipeline delay mean:  %.4f ms\n", mean(pipeline_ms));
fprintf("Pipeline delay median:%.4f ms\n", median(pipeline_ms));
fprintf("TX duration mean:     %.4f ms\n", mean(tx_ms));
fprintf("TX duration median:   %.4f ms\n", median(tx_ms));

% Outliers
tx_outlier_thresh  = prctile(tx_ms, 99);
e2e_outlier_thresh = prctile(host_e2e_ms, 99);

tx_outliers  = tx_ms > tx_outlier_thresh;
e2e_outliers = host_e2e_ms > e2e_outlier_thresh;

fprintf("\n========================================\n");
fprintf("MATCHED OUTLIER SUMMARY\n");
fprintf("========================================\n");

fprintf("TX outliers above P99 threshold:     %d\n", sum(tx_outliers));
fprintf("Host E2E outliers above P99 thresh:  %d\n", sum(e2e_outliers));

[~, idxWorst] = sort(host_e2e_ms, 'descend');
topN = min(10, length(idxWorst));
worstRows = Tm(idxWorst(1:topN), :);

disp(" ");
disp("Worst matched host E2E rows:");
disp(worstRows(:, {'rx_state_seq','state_mcu_us','tx_cmd_seq','ref_state_seq', ...
                  't_rx_parse_ns','t_tx_start_ns','t_tx_done_ns'}));

%% =========================================================
% PART 2: RAW PARSED STATE-PACKET ANALYSIS
%
% Because device_state_log.csv is emitted inside the parse loop,
% each row corresponds to one valid parsed state packet.
%
% Therefore the metrics below now represent:
% - actual parsed packet sequence progression on the host
% - actual parsed packet inter-arrival timing on the host
% - actual parsed packet MCU timestamp progression
%
% This is much stronger than the old "freshest per host update" log.
%% =========================================================

fprintf("\n========================================\n");
fprintf("RAW PARSED STATE-PACKET ANALYSIS\n");
fprintf("========================================\n");

numStateRows = height(State);
fprintf("Total raw parsed state rows: %d\n", numStateRows);

raw_state_seq_gap = [NaN; diff(State.("rx_state_seq"))];
raw_host_dt_ms    = [NaN; diff(State.("t_rx_parse_ns"))] / 1e6;
raw_mcu_dt_ms     = [NaN; diff(State.("state_mcu_us"))] / 1000.0;

% Effective MCU packet period per state increment
raw_effective_mcu_period_ms = raw_mcu_dt_ms ./ raw_state_seq_gap;

% Valid subsets (exclude NaN and impossible zero/negative gaps)
valid_gap_idx = raw_state_seq_gap > 0;
valid_host_dt = raw_host_dt_ms(2:end);
valid_mcu_dt  = raw_mcu_dt_ms(2:end);
valid_eff_mcu = raw_effective_mcu_period_ms(valid_gap_idx);

fprintf("\n========================================\n");
fprintf("RAW STATE SEQUENCE ANALYSIS\n");
fprintf("========================================\n");

fprintf("Mean raw state seq gap:    %.4f\n", mean(raw_state_seq_gap(2:end), 'omitnan'));
fprintf("Median raw state seq gap:  %.4f\n", median(raw_state_seq_gap(2:end), 'omitnan'));
fprintf("Max raw state seq gap:     %d\n", max(raw_state_seq_gap(2:end)));
fprintf("Rows with raw state skip (>1): %d\n", sum(raw_state_seq_gap(2:end) > 1));

fprintf("\n========================================\n");
fprintf("RAW PARSED-PACKET HOST TIMING ANALYSIS\n");
fprintf("========================================\n");

fprintf("Mean raw host inter-arrival:   %.4f ms\n", mean(valid_host_dt, 'omitnan'));
fprintf("Median raw host inter-arrival: %.4f ms\n", median(valid_host_dt, 'omitnan'));
fprintf("Std raw host inter-arrival:    %.4f ms\n", std(valid_host_dt, 'omitnan'));
fprintf("P95 raw host inter-arrival:    %.4f ms\n", prctile(valid_host_dt,95));
fprintf("P99 raw host inter-arrival:    %.4f ms\n", prctile(valid_host_dt,99));

raw_measured_rate_hz = 1000 / mean(valid_host_dt, 'omitnan');
fprintf("Estimated parsed packet rate on host: %.2f Hz\n", raw_measured_rate_hz);

fprintf("\n========================================\n");
fprintf("RAW MCU TIMING TREND ANALYSIS\n");
fprintf("========================================\n");

fprintf("Mean raw MCU dt between parsed rows:   %.4f ms\n", mean(valid_mcu_dt, 'omitnan'));
fprintf("Median raw MCU dt between parsed rows: %.4f ms\n", median(valid_mcu_dt, 'omitnan'));
fprintf("Std raw MCU dt between parsed rows:    %.4f ms\n", std(valid_mcu_dt, 'omitnan'));
fprintf("P95 raw MCU dt between parsed rows:    %.4f ms\n", prctile(valid_mcu_dt,95));

fprintf("Mean effective MCU period per state:   %.4f ms\n", mean(valid_eff_mcu, 'omitnan'));
fprintf("Median effective MCU period per state: %.4f ms\n", median(valid_eff_mcu, 'omitnan'));
fprintf("P95 effective MCU period per state:    %.4f ms\n", prctile(valid_eff_mcu,95));
fprintf("P99 effective MCU period per state:    %.4f ms\n", prctile(valid_eff_mcu,99));

raw_mcu_rate_hz = 1000 / mean(valid_eff_mcu, 'omitnan');
fprintf("Estimated MCU state rate from seq+MCU time: %.2f Hz\n", raw_mcu_rate_hz);

%% =========================================================
% PART 3: MATCHED VS RAW LOG COMPARISON
%% =========================================================

fprintf("\n========================================\n");
fprintf("MATCHED VS RAW LOG COMPARISON\n");
fprintf("========================================\n");

fprintf("Matched rows: %d\n", height(Tm));
fprintf("Raw parsed state rows: %d\n", height(State));
fprintf("Matched / raw fraction: %.4f\n", height(Tm) / max(height(State),1));

fprintf("Mean matched state seq gap: %.4f\n", mean(matched_state_seq_gap(2:end), 'omitnan'));
fprintf("Mean raw state seq gap:     %.4f\n", mean(raw_state_seq_gap(2:end), 'omitnan'));

%% =========================================================
% PLOTS: MATCHED TIMING
%% =========================================================

figure;
histogram(host_e2e_ms, 50);
xlabel("Host end-to-end latency (ms)");
ylabel("Count");
title("Matched host-side end-to-end latency distribution");
grid on;

figure;
histogram(tx_ms, 50);
xlabel("Serial TX duration (ms)");
ylabel("Count");
title("Matched torque packet transmit duration");
grid on;

figure;
plot(host_e2e_ms, 'o-');
xlabel("Matched sample index");
ylabel("Host end-to-end latency (ms)");
title("Matched host E2E latency over time");
grid on;

figure;
plot(tx_ms, 'o-');
xlabel("Matched sample index");
ylabel("TX duration (ms)");
title("Matched serial TX duration over time");
grid on;

figure;
plot(matched_state_seq_gap, 'o-');
xlabel("Matched sample index");
ylabel("Matched state sequence gap");
title("Matched state sequence gaps");
grid on;

figure;
plot(matched_cmd_seq_gap, 'o-');
xlabel("Matched sample index");
ylabel("Matched command sequence gap");
title("Matched command sequence gaps");
grid on;

%% =========================================================
% PLOTS: RAW PARSED STATE LOG
%% =========================================================

figure;
histogram(valid_host_dt, 50);
xlabel("Parsed state packet inter-arrival time on host (ms)");
ylabel("Count");
title("Raw parsed state packet inter-arrival distribution");
grid on;

figure;
plot(raw_host_dt_ms, 'o-');
xlabel("Parsed state packet index");
ylabel("Host inter-arrival time (ms)");
title("Raw parsed state packet host inter-arrival time");
grid on;

figure;
plot(raw_state_seq_gap, 'o-');
xlabel("Parsed state packet index");
ylabel("Raw state sequence gap");
title("Raw parsed state packet sequence gaps");
grid on;

figure;
plot(raw_mcu_dt_ms, 'o-');
xlabel("Parsed state packet index");
ylabel("Elapsed MCU time between parsed packets (ms)");
title("Raw parsed state packet MCU time increment");
grid on;

figure;
plot(raw_effective_mcu_period_ms, 'o-');
xlabel("Parsed state packet index");
ylabel("Effective MCU period per state (ms)");
title("Estimated MCU state period from raw parsed packet log");
grid on;

%% =========================================================
% OPTIONAL SUMMARY TABLES
%% =========================================================

timingSummary = table( ...
    mean(host_e2e_ms), median(host_e2e_ms), std(host_e2e_ms), prctile(host_e2e_ms,95), prctile(host_e2e_ms,99), max(host_e2e_ms), ...
    mean(publish_ms), median(publish_ms), ...
    mean(pipeline_ms), median(pipeline_ms), ...
    mean(tx_ms), median(tx_ms), ...
    mean(matched_state_seq_gap(2:end), 'omitnan'), max(matched_state_seq_gap(2:end)), ...
    mean(matched_cmd_seq_gap(2:end), 'omitnan'), max(matched_cmd_seq_gap(2:end)), ...
    'VariableNames', { ...
    'hostE2E_mean_ms','hostE2E_median_ms','hostE2E_std_ms','hostE2E_p95_ms','hostE2E_p99_ms','hostE2E_max_ms', ...
    'publish_mean_ms','publish_median_ms', ...
    'pipeline_mean_ms','pipeline_median_ms', ...
    'tx_mean_ms','tx_median_ms', ...
    'matchedStateSeqGap_mean','matchedStateSeqGap_max', ...
    'matchedCmdSeqGap_mean','matchedCmdSeqGap_max' ...
    });

stateSummary = table( ...
    mean(valid_host_dt, 'omitnan'), median(valid_host_dt, 'omitnan'), std(valid_host_dt, 'omitnan'), ...
    prctile(valid_host_dt,95), prctile(valid_host_dt,99), ...
    raw_measured_rate_hz, ...
    mean(raw_state_seq_gap(2:end), 'omitnan'), median(raw_state_seq_gap(2:end), 'omitnan'), max(raw_state_seq_gap(2:end)), ...
    mean(valid_mcu_dt, 'omitnan'), median(valid_mcu_dt, 'omitnan'), ...
    mean(valid_eff_mcu, 'omitnan'), median(valid_eff_mcu, 'omitnan'), ...
    prctile(valid_eff_mcu,95), prctile(valid_eff_mcu,99), ...
    raw_mcu_rate_hz, ...
    'VariableNames', { ...
    'rawHostDt_mean_ms','rawHostDt_median_ms','rawHostDt_std_ms','rawHostDt_p95_ms','rawHostDt_p99_ms', ...
    'rawHostParseRate_Hz', ...
    'rawStateSeqGap_mean','rawStateSeqGap_median','rawStateSeqGap_max', ...
    'rawMcuDt_mean_ms','rawMcuDt_median_ms', ...
    'effectiveMcuPeriod_mean_ms','effectiveMcuPeriod_median_ms', ...
    'effectiveMcuPeriod_p95_ms','effectiveMcuPeriod_p99_ms', ...
    'effectiveMcuRate_Hz' ...
    });

disp(" ");
disp("Matched timing summary:");
disp(timingSummary);

disp(" ");
disp("Raw parsed state summary:");
disp(stateSummary);

writetable(timingSummary, "device_timing_summary.csv");
writetable(stateSummary,  "device_state_summary.csv");