%% Constrained motion proxy/device plot
% Opens constrained_simulation_validation_log.csv and plots:
% - device trajectory
% - proxy trajectory
% - penetration depth over time
% - normal/tangential force behaviour
%
% Purpose:
% Demonstrates that the proxy remains constrained to the virtual surface
% while the device/tool moves during physical constrained-motion interaction.

clear; clc; close all;

%% ------------------------------------------------------------------------
% Configuration
% -------------------------------------------------------------------------

scriptDir = fileparts(mfilename('fullpath'));
if isempty(scriptDir)
    scriptDir = pwd;
end

inFile = fullfile(scriptDir, 'constrained_simulation_validation_log.csv');

% Hold/contact window used in your analysis
% tStart = 18.5;
% tEnd   = 24.2;
tStart = 20.5;
tEnd   = 23.2;
% Optional output
saveFigures = true;
outPrefix = 'constrained_motion';

if ~isfile(inFile)
    error('Input file not found: %s', inFile);
end

fprintf('Loading constrained-motion log from: %s\n', inFile);

%% ------------------------------------------------------------------------
% Load data
% -------------------------------------------------------------------------

T = readtable(inFile, 'VariableNamingRule', 'preserve');

requiredCols = { ...
    't_sec', ...
    'device_x','device_y','device_z', ...
    'proxy_x','proxy_y','proxy_z', ...
    'force_x','force_y','force_z', ...
    'normal_x','normal_y','normal_z'};

for k = 1:numel(requiredCols)
    if ~ismember(requiredCols{k}, T.Properties.VariableNames)
        error('Missing required column: %s', requiredCols{k});
    end
end

% Normalise time
tRaw = T.t_sec;
validTime = isfinite(tRaw) & tRaw > 0;

if ~any(validTime)
    error('No valid timestamps found.');
end

t0 = tRaw(find(validTime, 1, 'first'));
t = tRaw - t0;

validRows = isfinite(t) & t >= 0;
T = T(validRows, :);
t = t(validRows);

% Extract vectors
pDev = [T.device_x, T.device_y, T.device_z];
pProxy = [T.proxy_x, T.proxy_y, T.proxy_z];
F = [T.force_x, T.force_y, T.force_z];
N = [T.normal_x, T.normal_y, T.normal_z];

% Keep only finite rows
valid = all(isfinite(pDev), 2) & all(isfinite(pProxy), 2) & ...
        all(isfinite(F), 2) & all(isfinite(N), 2);

T = T(valid, :);
t = t(valid);
pDev = pDev(valid, :);
pProxy = pProxy(valid, :);
F = F(valid, :);
N = N(valid, :);

%% ------------------------------------------------------------------------
% Select constrained-motion window
% -------------------------------------------------------------------------

win = t >= tStart & t <= tEnd;

if ~any(win)
    error('No samples found in selected window %.2f to %.2f s.', tStart, tEnd);
end

tw = t(win);
pDevW = pDev(win, :);
pProxyW = pProxy(win, :);
FW = F(win, :);
NW = N(win, :);

% Re-zero time for plotting
tw0 = tw - tw(1);

%% ------------------------------------------------------------------------
% Compute useful metrics
% -------------------------------------------------------------------------

% Distance between device and proxy is penetration-like constraint error
constraintError = vecnorm(pDevW - pProxyW, 2, 2);

forceMag = vecnorm(FW, 2, 2);

% Normalise normals
Nnorm = vecnorm(NW, 2, 2);
validN = Nnorm > 1e-9;
Nhat = zeros(size(NW));
Nhat(validN, :) = NW(validN, :) ./ Nnorm(validN);

% Normal force component
Fn = sum(FW .* Nhat, 2);
FnormalVec = Fn .* Nhat;

% Tangential force component
FtVec = FW - FnormalVec;
Ft = vecnorm(FtVec, 2, 2);

FnAbs = abs(Fn);
FtAbs = abs(Ft);

ratio = FnAbs ./ max(FtAbs, 1e-9);

fprintf('\n=== Constrained Motion Metrics ===\n');
fprintf('Window: %.2f to %.2f s\n', tStart, tEnd);
fprintf('Mean device-proxy separation: %.4f m\n', mean(constraintError, 'omitnan'));
fprintf('Max device-proxy separation: %.4f m\n', max(constraintError, [], 'omitnan'));
fprintf('Mean force magnitude: %.3f N\n', mean(forceMag, 'omitnan'));
fprintf('Std force magnitude: %.3f N\n', std(forceMag, 'omitnan'));
fprintf('Median |Fn|/|Ft|: %.3f\n', median(ratio(isfinite(ratio)), 'omitnan'));
fprintf('95th percentile |Fn|/|Ft|: %.3f\n', prctile(ratio(isfinite(ratio)), 95));

%% ------------------------------------------------------------------------
% Figure 1: device, proxy, and virtual wall (XY projection)
% -------------------------------------------------------------------------

figure('Name', 'Constrained Motion - Device, Proxy, and Virtual Wall', 'Color', 'w');

% --- Virtual wall (assume plane normal mostly in X) ---
% Approximate the wall as the mean proxy X position
x_wall = mean(pProxyW(:,1), 'omitnan');

% Shift coordinates so virtual wall is at x = 0
pDev_shift = pDevW;
pProxy_shift = pProxyW;

pDev_shift(:,1) = pDevW(:,1) - x_wall;
pProxy_shift(:,1) = pProxyW(:,1) - x_wall;

% Virtual wall line at x = 0
y_range = linspace(min(pProxy_shift(:,2)), max(pProxy_shift(:,2)), 200);
x_line = zeros(size(y_range));

% Device and proxy trajectories
plot(pDev_shift(:,1), pDev_shift(:,2), '-', 'LineWidth', 1.2); 
hold on;
plot(pProxy_shift(:,1), pProxy_shift(:,2), '-', 'LineWidth', 1.8);

% Virtual wall
plot(x_line, y_range, 'k--', 'LineWidth', 2);

axis equal;
grid on;

xlabel('Normal displacement from virtual wall (m)');
ylabel('Tangential position along wall (m)');
title('Constrained Motion: Device, Proxy, and Virtual Wall');

legend({ ...
    'Device trajectory', ...
    'Proxy trajectory', ...
    'Virtual wall (x = 0)'}, ...
    'Location', 'best');

subtitle('Proxy remains constrained to the virtual surface while device exhibits bounded penetration');

% Optional console metric
maxPenetrationFromWall = max(abs(pDev_shift(:,1)), [], 'omitnan');
fprintf('Max normal displacement from wall: %.4f m\n', maxPenetrationFromWall);


%% ------------------------------------------------------------------------
% Figure 2: time-domain constraint behaviour
% -------------------------------------------------------------------------

figure('Name', 'Constrained Motion - Constraint Error and Force', 'Color', 'w');

subplot(3,1,1);
plot(tw0, pDevW(:,2), '-', 'LineWidth', 1.2);
hold on;
plot(tw0, pProxyW(:,2), '-', 'LineWidth', 1.5);
grid on;
xlabel('Time in selected window (s)');
ylabel('y position (m)');
title('Tangential Motion Along Surface');
legend({'Device/tool', 'Proxy'}, 'Location', 'best');

subplot(3,1,2);
plot(tw0, constraintError, 'LineWidth', 1.3);
grid on;
xlabel('Time in selected window (s)');
ylabel('|p_{dev} - p_{proxy}| (m)');
title('Device--Proxy Separation / Constraint Error');

subplot(3,1,3);
plot(tw0, forceMag, 'LineWidth', 1.3);
grid on;
xlabel('Time in selected window (s)');
ylabel('|F| (N)');
title('Rendered Force Magnitude During Constrained Motion');

if saveFigures
    exportgraphics(gcf, fullfile(scriptDir, sprintf('%s_constraint_timeseries.png', outPrefix)), ...
        'Resolution', 300);
end

%% ------------------------------------------------------------------------
% Figure 3: normal/tangential force ratio
% -------------------------------------------------------------------------

figure('Name', 'Constrained Motion - Normal/Tangential Force Ratio', 'Color', 'w');

histogram(ratio(isfinite(ratio) & ratio < 250), 60);
grid on;

xlabel('|F_n| / |F_t|');
ylabel('Sample count');
title('Normal-to-Tangential Force Ratio During Constrained Motion');

xline(median(ratio(isfinite(ratio)), 'omitnan'), '--', ...
    sprintf('Median = %.2f', median(ratio(isfinite(ratio)), 'omitnan')), ...
    'LineWidth', 1.4, ...
    'LabelOrientation', 'horizontal');

xline(prctile(ratio(isfinite(ratio)), 95), '--', ...
    sprintf('95th = %.2f', prctile(ratio(isfinite(ratio)), 95)), ...
    'LineWidth', 1.4, ...
    'LabelOrientation', 'horizontal');

if saveFigures
    exportgraphics(gcf, fullfile(scriptDir, sprintf('%s_force_ratio.png', outPrefix)), ...
        'Resolution', 300);
end

fprintf('\nSaved figures to:\n');
fprintf('  %s\n', fullfile(scriptDir, sprintf('%s_proxy_device_xy.png', outPrefix)));
fprintf('  %s\n', fullfile(scriptDir, sprintf('%s_constraint_timeseries.png', outPrefix)));
fprintf('  %s\n', fullfile(scriptDir, sprintf('%s_force_ratio.png', outPrefix)));