#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

GRAPHICS_DAT=${1:-$FIRESTAFF_DATA/GRAPHICS.DAT}
OUT_DIR=${2:-$HERE/internal-matrix}
mkdir -p "$OUT_DIR"

python3 - <<'PY' "$GRAPHICS_DAT" "$OUT_DIR" "$HERE"
import re
import subprocess
import sys
from pathlib import Path

graphics_dat = sys.argv[1]
out_dir = Path(sys.argv[2])
here = Path(sys.argv[3])
runner = here / 'run_firestaff_m9_beta_harness.sh'

# Goal A: submenu-internal event family classification
# Test each internal class in isolation and mixed combinations
seqs = [
    # No submenu activity (menu only)
    'f', 'fn', 'fnn',
    # Pure hold: frame/idle in submenu
    'faf', 'fai', 'fnaf', 'fnai', 'fnnaf', 'fnnai',
    # Pure reactivate
    'faa', 'fnaa', 'fnnaa',
    # Pure exit: back, cancel, advance-suppressed
    'fab', 'fac', 'fan', 'fnab', 'fnac', 'fnan', 'fnnab', 'fnnac', 'fnnan',
    # Pure return-to-menu
    'far', 'fnar', 'fnnar',
    # Mixed: reactivate + hold
    'faaf', 'faai', 'fnaaf', 'fnaai', 'fnnaaf', 'fnnaai',
    # Mixed: reactivate + exit
    'faan', 'fnaan', 'fnnaan',
    # Mixed: hold + exit
    'fabc', 'facb', 'fnabc', 'fnacb', 'fnnabc', 'fnnacb',
    # Mixed: reactivate + hold + exit
    'faaifn', 'fnaaifn', 'fnnaaifn',
    # Mixed with return: reactivate + return
    'faar', 'fnaar', 'fnnaar',
    # Mixed with return: hold + return
    'fafr', 'fnafr', 'fnnafr',
    # Mixed with return: reactivate + hold + return
    'faafr', 'fnaafr', 'fnnaafr',
]

keys = [
    'lastSubmenuInternalClass',
    'lastSubmenuInternalMask',
    'submenuInternalMask',
    'submenuCumulativeInternalClass',
]

rows = []
for seq in seqs:
    prefix = out_dir / f'run_{seq}'
    p = subprocess.run(
        [str(runner), graphics_dat, str(prefix), '--unified-boot', seq],
        cwd=here,
        text=True,
        capture_output=True,
    )
    text = p.stdout + '\n' + p.stderr
    row = {'seq': seq, 'rc': str(p.returncode)}
    for key in keys:
        m = re.search(rf'{key}=([^\n]+)', text)
        row[key] = m.group(1).strip() if m else ''
    rows.append(row)

md = []
md.append('# ReDMCSB M10 submenu-internal classification matrix\n\n')
md.append('| seq | rc | ' + ' | '.join(keys) + ' |\n')
md.append('| --- | --- | ' + ' | '.join(['---'] * len(keys)) + ' |\n')
for r in rows:
    cols = ' | '.join(r.get(k, '') for k in keys)
    md.append(f"| {r['seq']} | {r['rc']} | {cols} |\n")

failures = []
def get(seq):
    for row in rows:
        if row['seq'] == seq:
            return row
    raise KeyError(seq)

# All must pass
for row in rows:
    if row['rc'] != '0':
        failures.append(f"rc: {row['seq']} has rc={row['rc']}, expected 0")

# Internal class enum: NONE=0, HOLD=1, REACTIVATE=2, EXIT=3, NAVIGATE=4, RETURN=5, MIXED=6
# Internal mask: NONE=0, HOLD=1, REACTIVATE=2, EXIT=4, NAVIGATE=8, RETURN=16

# No submenu activity → NONE
for seq in ('f', 'fn', 'fnn'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '0':
        failures.append(f"no-submenu internal: {seq} has class={row['submenuCumulativeInternalClass']}, expected 0 (NONE)")
    if row['submenuInternalMask'] != '0':
        failures.append(f"no-submenu internal mask: {seq} has mask={row['submenuInternalMask']}, expected 0")

# Pure hold (frame/idle) → HOLD (class=1, mask=1)
for seq in ('faf', 'fai', 'fnaf', 'fnai', 'fnnaf', 'fnnai'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '1':
        failures.append(f"pure hold: {seq} has class={row['submenuCumulativeInternalClass']}, expected 1 (HOLD)")
    if row['submenuInternalMask'] != '1':
        failures.append(f"pure hold mask: {seq} has mask={row['submenuInternalMask']}, expected 1")

# Cross-lane consistency for pure hold
def expect_consistent(label, seqs_list):
    base = None
    for seq in seqs_list:
        row = get(seq)
        p = tuple(row[k] for k in keys)
        if base is None:
            base = p
        elif p != base:
            failures.append(f"{label}: {seq} has {p}, expected {base}")

expect_consistent('frame hold cross-lane', ['faf', 'fnaf', 'fnnaf'])
expect_consistent('idle hold cross-lane', ['fai', 'fnai', 'fnnai'])

# Pure reactivate → REACTIVATE (class=2, mask=2)
for seq in ('faa', 'fnaa', 'fnnaa'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '2':
        failures.append(f"pure reactivate: {seq} has class={row['submenuCumulativeInternalClass']}, expected 2")
    if row['submenuInternalMask'] != '2':
        failures.append(f"pure reactivate mask: {seq} has mask={row['submenuInternalMask']}, expected 2")

expect_consistent('reactivate cross-lane', ['faa', 'fnaa', 'fnnaa'])

# Pure exit (back/cancel/advance) → EXIT (class=3, mask=4)
for seq in ('fab', 'fac', 'fan', 'fnab', 'fnac', 'fnan', 'fnnab', 'fnnac', 'fnnan'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '3':
        failures.append(f"pure exit: {seq} has class={row['submenuCumulativeInternalClass']}, expected 3 (EXIT)")
    if row['submenuInternalMask'] != '4':
        failures.append(f"pure exit mask: {seq} has mask={row['submenuInternalMask']}, expected 4")

expect_consistent('back exit cross-lane', ['fab', 'fnab', 'fnnab'])
expect_consistent('cancel exit cross-lane', ['fac', 'fnac', 'fnnac'])
expect_consistent('advance exit cross-lane', ['fan', 'fnan', 'fnnan'])

# Pure return → RETURN (class=5, mask=16)
for seq in ('far', 'fnar', 'fnnar'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '5':
        failures.append(f"pure return: {seq} has class={row['submenuCumulativeInternalClass']}, expected 5 (RETURN)")
    if row['submenuInternalMask'] != '16':
        failures.append(f"pure return mask: {seq} has mask={row['submenuInternalMask']}, expected 16")

expect_consistent('return cross-lane', ['far', 'fnar', 'fnnar'])

# Mixed: reactivate + hold → mask=3 (1|2), class=MIXED (6)
for seq in ('faaf', 'faai', 'fnaaf', 'fnaai', 'fnnaaf', 'fnnaai'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"reactivate+hold: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6 (MIXED)")
    if row['submenuInternalMask'] != '3':
        failures.append(f"reactivate+hold mask: {seq} has mask={row['submenuInternalMask']}, expected 3 (HOLD|REACTIVATE)")

# Mixed: reactivate + exit → mask=6 (2|4), class=MIXED
for seq in ('faan', 'fnaan', 'fnnaan'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"reactivate+exit: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6")
    if row['submenuInternalMask'] != '6':
        failures.append(f"reactivate+exit mask: {seq} has mask={row['submenuInternalMask']}, expected 6 (REACTIVATE|EXIT)")

# Mixed: hold + exit (back+cancel combos include exit behavior) → mask depends
for seq in ('fabc', 'facb', 'fnabc', 'fnacb', 'fnnabc', 'fnnacb'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '3':
        failures.append(f"exit combo: {seq} has class={row['submenuCumulativeInternalClass']}, expected 3 (EXIT)")
    if row['submenuInternalMask'] != '4':
        failures.append(f"exit combo mask: {seq} has mask={row['submenuInternalMask']}, expected 4 (EXIT)")

# Mixed: reactivate + hold + exit → mask=7 (1|2|4), class=MIXED
for seq in ('faaifn', 'fnaaifn', 'fnnaaifn'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"full mix: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6")
    if row['submenuInternalMask'] != '7':
        failures.append(f"full mix mask: {seq} has mask={row['submenuInternalMask']}, expected 7 (HOLD|REACTIVATE|EXIT)")

# Mixed with return: reactivate + return → mask=18 (2|16)
for seq in ('faar', 'fnaar', 'fnnaar'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"reactivate+return: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6")
    if row['submenuInternalMask'] != '18':
        failures.append(f"reactivate+return mask: {seq} has mask={row['submenuInternalMask']}, expected 18 (REACTIVATE|RETURN)")

# Mixed: hold + return → mask=17 (1|16)
for seq in ('fafr', 'fnafr', 'fnnafr'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"hold+return: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6")
    if row['submenuInternalMask'] != '17':
        failures.append(f"hold+return mask: {seq} has mask={row['submenuInternalMask']}, expected 17 (HOLD|RETURN)")

# Mixed: reactivate + hold + return → mask=19 (1|2|16)
for seq in ('faafr', 'fnaafr', 'fnnaafr'):
    row = get(seq)
    if row['submenuCumulativeInternalClass'] != '6':
        failures.append(f"reactivate+hold+return: {seq} has class={row['submenuCumulativeInternalClass']}, expected 6")
    if row['submenuInternalMask'] != '19':
        failures.append(f"reactivate+hold+return mask: {seq} has mask={row['submenuInternalMask']}, expected 19 (HOLD|REACTIVATE|RETURN)")

inv = []
inv.append('# ReDMCSB M10 submenu-internal classification invariants\n\n')
if failures:
    inv.append('Status: FAIL\n\n')
    for item in failures:
        inv.append(f'- {item}\n')
else:
    inv.append('Status: PASS\n\n')
    inv.append('- No-submenu rows (f/fn/fnn) report NONE (class=0, mask=0)\n')
    inv.append('- Pure frame/idle hold rows report HOLD (class=1, mask=1)\n')
    inv.append('- Pure reactivate rows report REACTIVATE (class=2, mask=2)\n')
    inv.append('- Pure exit rows (back/cancel/advance) report EXIT (class=3, mask=4)\n')
    inv.append('- Pure return-to-menu rows report RETURN (class=5, mask=16)\n')
    inv.append('- Mixed reactivate+hold → MIXED (class=6, mask=3)\n')
    inv.append('- Mixed reactivate+exit → MIXED (class=6, mask=6)\n')
    inv.append('- Combined exit events (bc/cb) stay pure EXIT (class=3, mask=4)\n')
    inv.append('- Full mix (reactivate+hold+exit) → MIXED (class=6, mask=7)\n')
    inv.append('- Return combinations produce correct cumulative masks with MIXED class\n')
    inv.append('- Cross-lane patterns are consistent for all internal families\n')

(out_dir / 'internal_matrix.md').write_text(''.join(md), encoding='utf-8')
(out_dir / 'internal_invariants.md').write_text(''.join(inv), encoding='utf-8')
print(out_dir / 'internal_matrix.md')
print(out_dir / 'internal_invariants.md')
if failures:
    raise SystemExit(1)
PY
