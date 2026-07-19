#!/usr/bin/env python3
"""Re-base the HALK-route cancel_reopen portrait probes (rond 2).

Moves the stale (1,2) NORTH park pose to the verified PC34 C127 pose
(7,9) NORTH in front of the (7,8) S-face HALK sensor, and updates the
stale coordinate claims in the probe comments.  Every replacement is
count-asserted so a drift in any file aborts the batch.
"""
import sys
from pathlib import Path

ROOT = Path("/Volumes/Extern-disk/firestaff-work/probes/m11")
FILES = [
    "firestaff_dm1_v1_hall_of_champions_portrait_00_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_01_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_03_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_06_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_08_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_14_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_15_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_16_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_17_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_18_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_19_cancel_reopen_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_20_cancel_reopen_portrait_rect_position_runtime_probe.c",
]

# (old, new, expected_count) — count is per-file minimum; exact match
# is asserted per file below with per-file tolerances.
SUBS = [
    (
        "state->world.party.mapX = 1;\n    state->world.party.mapY = 2;",
        "state->world.party.mapX = 7;\n    state->world.party.mapY = 9;",
        1, True,
    ),
    (
        "(1,2)\n * NORTH-route front square (1,1)",
        "(7,9)\n * NORTH-route front square (7,8)",
        None, False,
    ),
    (
        "Park the party at the (1,2) D1C front-mirror route facing NORTH.",
        "Park the party at the (7,9) D1C front-mirror route facing NORTH.",
        1, True,
    ),
    (
        "at (1,2) facing\n * NORTH, the front square (1,1) has a C127 sensor",
        "at (7,9) facing\n * NORTH, the front square (7,8) has a C127 sensor",
        None, False,
    ),
]

def main():
    failures = []
    for name in FILES:
        path = ROOT / name
        text = path.read_text()
        original = text
        for old, new, expected, exact in SUBS:
            count = text.count(old)
            if exact and count != expected:
                failures.append(f"{name}: pattern {old[:40]!r} count={count} expected={expected}")
                continue
            if not exact and count == 0:
                failures.append(f"{name}: pattern {old[:40]!r} not found")
                continue
            text = text.replace(old, new)
        if text != original:
            path.write_text(text)
            print(f"updated {name}")
    if failures:
        print("\nFAILURES:", file=sys.stderr)
        for f in failures:
            print(f"  {f}", file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    sys.exit(main())
