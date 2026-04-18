%% =========================================================
% COMMUNICATION TEST ANALYSIS
%
% This script analyses two experiment types:
%
% 1) Packet rate / integrity tests
%    Files: rate_100hz.csv, rate_250hz.csv, ...
%
% 2) Round-trip latency tests
%    Files: rtt_1ms.csv, rtt_2ms.csv, rtt_10ms.csv
%
% It compares all CSV files within each experiment type.
%
% Intended use:
% - dissertation figures
% - communication performance summary tables
% - validation of packet timing, drop behaviour, and RTT
%% =========================================================

clear;
clc;
close all;

%% =========================================================
% USER PATHS
%% =========================================================

dataFolder = "C:\Users\tman0\Code\3rd-yr-project\Test Data";

rateFiles = [
    "rate_100hz.csv"
    "rate_250hz.csv"
    "rate_500hz.csv"
    "rate_1000hz.csv"
    "rate_1250hz.csv"
    "rate_2000hz.csv"
];

rttFiles = [
    "rtt_1ms.csv"
    "rtt_2ms.csv"
    "rtt_10ms.csv"
];

%% =========================================================
% PART 1: PACKET RATE / INTEGRITY ANALYSIS
%
% Expected columns:
% arrival_ns, seq, t_mcu_us, q1, q2, dt_arrival_ms,
% dropped_since_last, out_of_order
%
% Meaning:
% - arrival_ns: host timestamp when packet was received
% - seq: packet sequence number
% - t_mcu_us: MCU-side timestamp in microseconds
% - dt_arrival_ms: inter-arrival time between consecutive packets
% - dropped_since_last: number of missing packets since previous packet
% - out_of_order: flag indicating sequence disorder
%% =========================================================

fprintf("====================================================\n");
fprintf("PACKET RATE / INTEGRITY ANALYSIS\n");
fprintf("====================================================\n");

numRate = length(rateFiles);

rateNames           = strings(numRate,1);
requestedRateHz     = zeros(numRate,1);
meanDtMs            = zeros(numRate,1);
stdDtMs             = zeros(numRate,1);
measuredRateHz      = zeros(numRate,1);
totalPackets        = zeros(numRate,1);
totalDrops          = zeros(numRate,1);
dropFraction        = zeros(numRate,1);
totalOutOfOrder     = zeros(numRate,1);
outOfOrderFraction  = zeros(numRate,1);
p95DtMs             = zeros(numRate,1);
p99DtMs             = zeros(numRate,1);

rateTables = cell(numRate,1);

for i = 1:numRate
    fileName = rateFiles(i);
    fullPath = fullfile(dataFolder, fileName);

    T = readtable(fullPath);
    rateTables{i} = T;
    rateNames(i) = erase(fileName, ".csv");

    % Extract requested rate from file name, e.g. "rate_1000hz.csv"
    token = regexp(fileName, 'rate_(\d+)hz\.csv', 'tokens', 'once');
    requestedRateHz(i) = str2double(token{1});

    % Basic stats
    totalPackets(i) = height(T);

    % Ignore the first dt sample if it is zero or artificial
    dt = T.dt_arrival_ms;
    dtValid = dt(dt > 0);

    meanDtMs(i) = mean(dtValid);
    stdDtMs(i)  = std(dtValid);
    p95DtMs(i)  = prctile(dtValid,95);
    p99DtMs(i)  = prctile(dtValid,99);

    measuredRateHz(i) = 1000 / meanDtMs(i);

    totalDrops(i) = sum(T.dropped_since_last);
    totalOutOfOrder(i) = sum(T.out_of_order);

    % Fraction relative to received packets
    dropFraction(i) = totalDrops(i) / max(totalPackets(i),1);
    outOfOrderFraction(i) = totalOutOfOrder(i) / max(totalPackets(i),1);

    fprintf("\n%s\n", fileName);
    fprintf("  Requested rate:      %.0f Hz\n", requestedRateHz(i));
    fprintf("  Measured rate:       %.2f Hz\n", measuredRateHz(i));
    fprintf("  Mean dt:             %.4f ms\n", meanDtMs(i));
    fprintf("  Std dt (jitter):     %.4f ms\n", stdDtMs(i));
    fprintf("  P95 dt:              %.4f ms\n", p95DtMs(i));
    fprintf("  P99 dt:              %.4f ms\n", p99DtMs(i));
    fprintf("  Total packets:       %d\n", totalPackets(i));
    fprintf("  Total drops:         %d\n", totalDrops(i));
    fprintf("  Drop fraction:       %.6f\n", dropFraction(i));
    fprintf("  Out-of-order total:  %d\n", totalOutOfOrder(i));
    fprintf("  Out-of-order frac:   %.6f\n", outOfOrderFraction(i));
end

% Sort by requested rate for cleaner plots
[requestedRateHz, sortIdx] = sort(requestedRateHz);
rateNames          = rateNames(sortIdx);
meanDtMs           = meanDtMs(sortIdx);
stdDtMs            = stdDtMs(sortIdx);
p95DtMs            = p95DtMs(sortIdx);
p99DtMs            = p99DtMs(sortIdx);
measuredRateHz     = measuredRateHz(sortIdx);
totalPackets       = totalPackets(sortIdx);
totalDrops         = totalDrops(sortIdx);
dropFraction       = dropFraction(sortIdx);
totalOutOfOrder    = totalOutOfOrder(sortIdx);
outOfOrderFraction = outOfOrderFraction(sortIdx);
rateTables         = rateTables(sortIdx);

%% ---------------------------------------------------------
% Summary table for rate tests
%% ---------------------------------------------------------
rateSummary = table( ...
    rateNames, requestedRateHz, measuredRateHz, ...
    meanDtMs, stdDtMs, p95DtMs, p99DtMs, ...
    totalPackets, totalDrops, dropFraction, ...
    totalOutOfOrder, outOfOrderFraction ...
);

disp(" ");
disp("Rate test summary table:");
disp(rateSummary);

%% ---------------------------------------------------------
% Plot 1: Requested vs measured receive rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, measuredRateHz, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Measured receive rate (Hz)");
title("Requested vs measured packet rate");
grid on;

%% ---------------------------------------------------------
% Plot 2: Jitter (std of inter-arrival time) vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, stdDtMs, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Inter-arrival jitter std (ms)");
title("Packet timing jitter vs requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 3: Mean inter-arrival time vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, meanDtMs, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Mean inter-arrival time (ms)");
title("Mean packet inter-arrival time vs requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 4: Total dropped packets vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, totalDrops, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Total dropped packets");
title("Dropped packets vs requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 5: Drop fraction vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, dropFraction, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Drop fraction");
title("Packet drop fraction vs requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 6: Out-of-order fraction vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, outOfOrderFraction, 'o-');
xlabel("Requested packet rate (Hz)");
ylabel("Out-of-order fraction");
title("Out-of-order packet fraction vs requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 7: Overlay inter-arrival traces for all rate tests
%
% This helps show jitter/noise patterns across rates.
%% ---------------------------------------------------------
figure;
hold on;
for i = 1:numRate
    T = rateTables{i};
    dt = T.dt_arrival_ms;
    plot(dt, 'DisplayName', rateNames(i));
end
hold off;
xlabel("Packet index");
ylabel("Inter-arrival time (ms)");
title("Inter-arrival timing traces for all rate tests");
legend("Location","best");
grid on;

%% ---------------------------------------------------------
% Plot 8: Boxplot of inter-arrival time by requested rate
%% ---------------------------------------------------------
allDt = [];
allGroups = [];

for i = 1:numRate
    T = rateTables{i};
    dt = T.dt_arrival_ms;
    dt = dt(dt > 0);

    allDt = [allDt; dt]; %#ok<AGROW>
    allGroups = [allGroups; repmat(requestedRateHz(i), length(dt), 1)]; %#ok<AGROW>
end

figure;
boxplot(allDt, allGroups);
xlabel("Requested packet rate (Hz)");
ylabel("Inter-arrival time (ms)");
title("Distribution of packet inter-arrival time by requested rate");
grid on;

%% ---------------------------------------------------------
% Plot 8: Packet inter-arrival timing statistics vs requested rate
%% ---------------------------------------------------------
figure;
plot(requestedRateHz, meanDtMs, 'o-', 'DisplayName', 'Mean');
hold on;
plot(requestedRateHz, p99DtMs, '^-', 'DisplayName', 'P99');
hold off;
xlabel("Requested packet rate (Hz)");
ylabel("Inter-arrival time (ms)");
title("Packet inter-arrival timing statistics vs requested rate");
legend("Location","best");
grid on;


%% ---------------------------------------------------------
% Plot 9: CDF of packet inter-arrival time
%% ---------------------------------------------------------
figure;
hold on;
for i = 1:numRate
    if requestedRateHz(i)==1000 || requestedRateHz(i)==2000
        T = rateTables{i};
        dt = sort(T.dt_arrival_ms(T.dt_arrival_ms > 0));
        y = (1:length(dt))/length(dt);
        plot(dt, y, 'DisplayName', sprintf('%d Hz', requestedRateHz(i)));
    end
end
hold off;
xlabel("Inter-arrival time (ms)");
ylabel("Cumulative probability");
title("CDF of packet inter-arrival time");
legend("Location","best");
grid on;

%% =========================================================
% PART 2: ROUND-TRIP LATENCY ANALYSIS
%
% Expected columns:
% seq, send_ns, recv_ns, rtt_ms, success
%
% Meaning:
% - send_ns: host timestamp when ping packet was sent
% - recv_ns: host timestamp when reply was received
% - rtt_ms: measured round-trip latency in milliseconds
% - success: 1 if response received, 0 otherwise
%% =========================================================

fprintf("\n====================================================\n");
fprintf("ROUND-TRIP LATENCY ANALYSIS\n");
fprintf("====================================================\n");

numRtt = length(rttFiles);

rttNames          = strings(numRtt,1);
testIntervalMs    = zeros(numRtt,1);
successRate       = zeros(numRtt,1);
meanRttMs         = zeros(numRtt,1);
stdRttMs          = zeros(numRtt,1);
p95RttMs          = zeros(numRtt,1);
p99RttMs          = zeros(numRtt,1);
minRttMs          = zeros(numRtt,1);
maxRttMs          = zeros(numRtt,1);
numSamples        = zeros(numRtt,1);

rttTables = cell(numRtt,1);

for i = 1:numRtt
    fileName = rttFiles(i);
    fullPath = fullfile(dataFolder, fileName);

    T = readtable(fullPath);
    rttTables{i} = T;
    rttNames(i) = erase(fileName, ".csv");

    % Extract nominal interval from filename, e.g. rtt_2ms.csv
    token = regexp(fileName, 'rtt_(\d+)ms\.csv', 'tokens', 'once');
    testIntervalMs(i) = str2double(token{1});

    numSamples(i) = height(T);
    successRate(i) = mean(T.success);

    rttValid = T.rtt_ms(T.success == 1);

    meanRttMs(i) = mean(rttValid);
    stdRttMs(i)  = std(rttValid);
    p95RttMs(i)  = prctile(rttValid,95);
    p99RttMs(i)  = prctile(rttValid,99);
    minRttMs(i)  = min(rttValid);
    maxRttMs(i)  = max(rttValid);

    fprintf("\n%s\n", fileName);
    fprintf("  Nominal interval: %.0f ms\n", testIntervalMs(i));
    fprintf("  Success rate:     %.4f\n", successRate(i));
    fprintf("  Mean RTT:         %.4f ms\n", meanRttMs(i));
    fprintf("  Std RTT:          %.4f ms\n", stdRttMs(i));
    fprintf("  P95 RTT:          %.4f ms\n", p95RttMs(i));
    fprintf("  P99 RTT:          %.4f ms\n", p99RttMs(i));
    fprintf("  Min RTT:          %.4f ms\n", minRttMs(i));
    fprintf("  Max RTT:          %.4f ms\n", maxRttMs(i));
    fprintf("  Samples:          %d\n", numSamples(i));
end

% Sort by nominal RTT test interval
[testIntervalMs, sortIdx] = sort(testIntervalMs);
rttNames       = rttNames(sortIdx);
successRate    = successRate(sortIdx);
meanRttMs      = meanRttMs(sortIdx);
stdRttMs       = stdRttMs(sortIdx);
p95RttMs       = p95RttMs(sortIdx);
p99RttMs       = p99RttMs(sortIdx);
minRttMs       = minRttMs(sortIdx);
maxRttMs       = maxRttMs(sortIdx);
numSamples     = numSamples(sortIdx);
rttTables      = rttTables(sortIdx);

%% ---------------------------------------------------------
% Summary table for RTT tests
%% ---------------------------------------------------------
rttSummary = table( ...
    rttNames, testIntervalMs, successRate, ...
    meanRttMs, stdRttMs, p95RttMs, p99RttMs, ...
    minRttMs, maxRttMs, numSamples ...
);

disp(" ");
disp("RTT summary table:");
disp(rttSummary);

%% ---------------------------------------------------------
% Plot 1: Mean RTT vs nominal test interval
%% ---------------------------------------------------------
figure;
plot(testIntervalMs, meanRttMs, 'o-');
xlabel("Nominal ping interval (ms)");
ylabel("Mean RTT (ms)");
title("Mean round-trip latency vs nominal test interval");
grid on;

%% ---------------------------------------------------------
% Plot 2: RTT variability vs nominal test interval
%% ---------------------------------------------------------
figure;
plot(testIntervalMs, stdRttMs, 'o-');
xlabel("Nominal ping interval (ms)");
ylabel("RTT std (ms)");
title("Round-trip latency variability vs nominal test interval");
grid on;

%% ---------------------------------------------------------
% Plot 3: P95 and P99 RTT vs nominal test interval
%% ---------------------------------------------------------
figure;
plot(testIntervalMs, p95RttMs, 'o-', 'DisplayName','P95 RTT');
hold on;
plot(testIntervalMs, p99RttMs, 's-', 'DisplayName','P99 RTT');
hold off;
xlabel("Nominal ping interval (ms)");
ylabel("RTT (ms)");
title("RTT percentiles vs nominal test interval");
legend("Location","best");
grid on;

%% ---------------------------------------------------------
% Plot 4: Success rate vs nominal test interval
%% ---------------------------------------------------------
figure;
plot(testIntervalMs, successRate, 'o-');
xlabel("Nominal ping interval (ms)");
ylabel("Success rate");
title("Ping success rate vs nominal test interval");
ylim([0 1.05]);
grid on;

%% ---------------------------------------------------------
% Plot 5: Overlay RTT traces
%% ---------------------------------------------------------
figure;
hold on;
for i = 1:numRtt
    T = rttTables{i};
    rttValid = T.rtt_ms(T.success == 1);
    plot(rttValid, 'DisplayName', rttNames(i));
end
hold off;
xlabel("Sample index");
ylabel("RTT (ms)");
title("RTT traces for all round-trip tests");
legend("Location","best");
grid on;

%% ---------------------------------------------------------
% Plot 6: RTT boxplot by test interval
%% ---------------------------------------------------------
allRtt = [];
allRttGroups = [];

for i = 1:numRtt
    T = rttTables{i};
    rttValid = T.rtt_ms(T.success == 1);

    allRtt = [allRtt; rttValid]; %#ok<AGROW>
    allRttGroups = [allRttGroups; repmat(testIntervalMs(i), length(rttValid), 1)]; %#ok<AGROW>
end
%% ---------------------------------------------------------
% Plot 7: CDF of round-trip latency
%% ---------------------------------------------------------
figure;
boxplot(allRtt, allRttGroups);
xlabel("Nominal ping interval (ms)");
ylabel("RTT (ms)");
title("Distribution of round-trip latency by test interval");
grid on;

figure;
hold on;
for i = 1:numRtt
    T = rttTables{i};
    rttValid = sort(T.rtt_ms(T.success == 1));
    y = (1:length(rttValid)) / length(rttValid);
    plot(rttValid, y, 'DisplayName', rttNames(i));
end
hold off;
xlabel("RTT (ms)");
ylabel("Cumulative probability");
title("CDF of round-trip latency");
legend("Location","best");
grid on;

%% =========================================================
% OPTIONAL: SAVE SUMMARY TABLES
%% =========================================================

writetable(rateSummary, fullfile(dataFolder, "rate_summary.csv"));
writetable(rttSummary,  fullfile(dataFolder, "rtt_summary.csv"));

fprintf("\nSummary tables written to:\n");
fprintf("  %s\n", fullfile(dataFolder, "rate_summary.csv"));
fprintf("  %s\n", fullfile(dataFolder, "rtt_summary.csv"));