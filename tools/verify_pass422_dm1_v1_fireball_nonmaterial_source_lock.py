#!/usr/bin/env python3
"""Source lock for pass422 fireball/lightning explosion creature parity.

Primary source: ReDMCSB PROJEXPL.C
- F0213_EXPLOSION_Create lines 123-148: fireball/lightning explosion creation
  resolves party-square damage first, else group damage.
- F0213 lines 137-142: non-material creature fire/lightning damage is quartered
  before the fire-resistance random subtraction and all-creatures damage call.
Secondary: CSBWin/combined probe has the same PROJEXPL.C lineage around its
flattened F0213 blocks; not authoritative for this lock.
"""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
checks = [
    (root / "memory_projectile_pc34_compat.c", "party square first; creature damage is in the else branch"),
    (root / "memory_projectile_pc34_compat.c", "groupAttackApplied >>= 2"),
    (root / "firestaff_m10_projectile_probe.c", "group preempted (PROJEXPL.C F0213)"),
    (root / "firestaff_m10_projectile_probe.c", "group attack quartered before resistance"),
]
missing = []
for path, needle in checks:
    text = path.read_text(encoding="utf-8")
    if needle not in text:
        missing.append(f"{path.relative_to(root)}: missing {needle!r}")
if missing:
    print("FAIL pass422 source lock")
    for item in missing:
        print("-", item)
    sys.exit(1)
print("OK pass422 fireball/non-material source lock")
