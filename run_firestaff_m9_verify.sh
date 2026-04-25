#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

GRAPHICS_DAT=${1:-$FIRESTAFF_DATA/GRAPHICS.DAT}
ROOT=$HERE
OUT_DIR=${2:-$ROOT/verification}
mkdir -p "$OUT_DIR"

TITLE_LOG="$OUT_DIR/title_hold.log"
MENU_LOG="$OUT_DIR/submenu_matrix.log"
SUMMARY_MD="$OUT_DIR/verification_summary.md"

"$ROOT/run_firestaff_m9_beta_harness.sh" \
  "$GRAPHICS_DAT" \
  "$OUT_DIR/title_hold" \
  --title-hold 2 > "$TITLE_LOG"

python3 - <<'PY' "$TITLE_LOG" "$SUMMARY_MD" "$OUT_DIR"
import re, sys
from pathlib import Path
log = Path(sys.argv[1]).read_text(encoding='utf-8')
out = Path(sys.argv[2])
out_dir = Path(sys.argv[3])
checks = {
    'bootPublishedFrameCount': '50',
    'titleHoldMode': '1',
    'titleHoldRepeatCount': '2',
    'titleFrontendOriginalFrameCount': '2',
    'titleFrontendFallbackFrameCount': '0',
    'titleFrontendHandoffReadyCount': '0',
    'titleFrontendHeldLastFrameCount': '0',
}
expected_files = [
    out_dir / 'title_hold_hold_0051.pgm',
    out_dir / 'title_hold_hold_0052.pgm',
]
failures = []
for key, expected in checks.items():
    m = re.search(rf'{key}=([^\n]+)', log)
    value = m.group(1).strip() if m else None
    if value != expected:
        failures.append(f'title-hold: {key} expected {expected}, got {value}')
for idx, expected_frame in enumerate(('1', '2')):
    m = re.search(rf'titleFrontendFrame\[{idx}\]=([^\n]+)', log)
    value = m.group(1).strip() if m else None
    if value != expected_frame:
        failures.append(f'title-hold: titleFrontendFrame[{idx}] expected {expected_frame}, got {value}')
for path in expected_files:
    if not path.exists():
        failures.append(f'title-hold: missing artifact {path.name}')
if not failures:
    a = expected_files[0].read_bytes()
    b = expected_files[1].read_bytes()
    if a == b:
        failures.append(f'title-hold: source-backed TITLE frames should advance for --title-hold 2 ({expected_files[0].name} == {expected_files[1].name})')
text = ['# ReDMCSB M9 verification summary\n', '\n']
if failures:
    text.append('## Title hold check: FAIL\n\n')
    for item in failures:
        text.append(f'- {item}\n')
else:
    text.append('## Title hold check: PASS\n\n')
    text.append('- bootPublishedFrameCount=50\n')
    text.append('- titleHoldMode=1\n')
    text.append('- titleHoldRepeatCount=2\n')
    text.append('- titleFrontendOriginalFrameCount=2\n')
    text.append('- titleFrontendFallbackFrameCount=0\n')
    text.append('- titleFrontendFrame[0]=1\n')
    text.append('- titleFrontendFrame[1]=2\n')
    for path in expected_files:
        text.append(f'- artifact present: {path.name}\n')
    text.append(f'- source-backed TITLE artifacts advance as expected: {expected_files[0].name} != {expected_files[1].name}\n')
out.write_text(''.join(text), encoding='utf-8')
if failures:
    raise SystemExit(1)
PY

"$ROOT/run_firestaff_m9_submenu_matrix.sh" "$GRAPHICS_DAT" "$OUT_DIR/submenu-matrix" > "$MENU_LOG"

python3 - <<'PY' "$OUT_DIR/submenu-matrix/submenu_invariants.md" "$SUMMARY_MD" "$OUT_DIR/submenu-matrix"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
matrix_path = subdir / 'submenu_matrix.md'
text = out.read_text(encoding='utf-8')
failures = []
for required in ['submenu_matrix.md', 'submenu_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'menu/submenu: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('menu/submenu: invariant status is not PASS')
if matrix_path.exists():
    matrix = matrix_path.read_text(encoding='utf-8')
    expected_rows = [
        'f', 'fn', 'fnn', 'fnnn',
        'fa', 'faa', 'faaa', 'faaaa',
        'faf', 'fai', 'fan', 'fann', 'fanan',
        'faaf', 'faai', 'faan', 'faaifn',
        'fab', 'fac', 'fabc', 'facb',
        'fna', 'fnaa', 'fnaaa', 'fnaaaa',
        'fnaf', 'fnai', 'fnan', 'fnann', 'fnanan',
        'fnaaf', 'fnaai', 'fnaan', 'fnaaifn',
        'fnab', 'fnac', 'fnabc', 'fnacb',
        'fnna', 'fnnaa', 'fnnaaa', 'fnnaaaa',
        'fnnaf', 'fnnai', 'fnnan', 'fnnann', 'fnnanan',
        'fnnaaf', 'fnnaai', 'fnnaan', 'fnnaaifn',
        'fnnab', 'fnnac', 'fnnabc', 'fnnacb',
    ]
    for seq in expected_rows:
        marker = f'| {seq} | 0 |'
        if marker not in matrix:
            failures.append(f'menu/submenu: missing expected passing row for {seq}')
    data_rows = [line for line in matrix.splitlines() if line.startswith('| ') and not line.startswith('| seq ') and not line.startswith('| --- ')]
    if len(data_rows) != len(expected_rows):
        failures.append(f'menu/submenu: expected {len(expected_rows)} matrix rows, found {len(data_rows)}')
if failures:
    text += '\n## Menu/submenu invariant check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## Menu/submenu invariant check: PASS\n\n'
text += '- artifact present: submenu_matrix.md\n'
text += '- artifact present: submenu_invariants.md\n'
text += '- matrix contains all expected rows with rc=0\n'
text += f'- matrix row count matches expected set ({len(expected_rows)})\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

printf '%s\n' "$SUMMARY_MD"
