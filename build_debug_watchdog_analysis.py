import csv
import statistics
from pathlib import Path

base = Path(r"c:/Users/tman0/Code/3rd-yr-project/build/Debug")
state_file = base / 'device_state_log.csv'
timing_file = base / 'device_timing.csv'

timeout_ns = 10_000_000  # firmware cmd_timeout_us = 10000 us

# Read device_state_log.csv
state_rows = []
with state_file.open('r', newline='') as f:
    r = csv.DictReader(f)
    for row in r:
        state_rows.append(row)

if not state_rows:
    print('No device_state_log.csv rows found')
else:
    total_state = len(state_rows)
    # New semantics: watchdog_active==1 => packet TIMED OUT (safety fault).
    wd_timeout_count = sum(int(row.get('watchdog_active', '0') or 0) for row in state_rows)
    wd_recent_count = total_state - wd_timeout_count
    applied_count = sum((abs(float(row.get('applied_tau1', '0') or 0)) + abs(float(row.get('applied_tau2','0') or 0)))>1e-6 for row in state_rows)
    print(f"device_state_log.csv: rows={total_state}, watchdog_timeout={wd_timeout_count} ({wd_timeout_count/total_state*100:.2f}%), watchdog_recent={wd_recent_count} ({wd_recent_count/total_state*100:.2f}%), applied_nonzero={applied_count}")

# Read device_timing.csv and compute t_tx_start_ns gaps where tx_cmd_seq increments
times = []
seqs = []
with timing_file.open('r', newline='') as f:
    r = csv.DictReader(f)
    for row in r:
        try:
            seq = int(row.get('tx_cmd_seq','0') or 0)
            t = int(row.get('t_tx_start_ns','0') or 0)
        except:
            continue
        # only consider positive seq and time
        if seq>0 and t>0:
            seqs.append(seq)
            times.append((seq,t))

if not times:
    print('No tx times found in device_timing.csv')
else:
    # sort by seq
    times.sort()
    seq_list = [s for s,t in times]
    t_list = [t for s,t in times]
    # compute diffs between consecutive t_list where seq increments by 1
    diffs_ns = []
    for i in range(1,len(times)):
        if seq_list[i] == seq_list[i-1] + 1:
            diffs_ns.append(t_list[i]-t_list[i-1])
    if not diffs_ns:
        print('No consecutive seq gaps to analyze')
    else:
        diffs_ms = [d/1e6 for d in diffs_ns]
        over_timeout = sum(1 for d in diffs_ns if d >= timeout_ns)
        pct_over = over_timeout / len(diffs_ns) * 100
        print(f"device_timing.csv: tx_cmd_seq samples={len(diffs_ns)}, mean_gap_ms={statistics.mean(diffs_ms):.3f}, median_gap_ms={statistics.median(diffs_ms):.3f}, max_gap_ms={max(diffs_ms):.1f}")
        print(f"fraction of gaps >= {timeout_ns/1e6:.1f} ms = {pct_over:.2f}% (would trigger timeout)")

    # also print a few largest gaps
    largest = sorted(diffs_ms, reverse=True)[:5]
    print('largest gaps ms (top 5):', ', '.join(f"{x:.1f}" for x in largest))

# If contact samples exist, compute watchdog percent during contact
try:
    contact_rows = [row for row in state_rows if (abs(float(row.get('applied_tau1','0') or 0))+abs(float(row.get('applied_tau2','0') or 0)))>1e-3]
    if contact_rows:
        c_total = len(contact_rows)
        c_wd = sum(int(r.get('watchdog_active','0') or 0) for r in contact_rows)
        print(f"During contact: samples={c_total}, watchdog_active={c_wd} ({c_wd/c_total*100:.2f}%)")
    else:
        print('No contact-applied torque samples found (applied_tau ~ 0)')
except Exception as e:
    print('Contact analysis failed:', e)

# Done
print('analysis complete')
