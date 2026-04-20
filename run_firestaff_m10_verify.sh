#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

GRAPHICS_DAT=${1:-$FIRESTAFF_DATA/GRAPHICS.DAT}
ROOT=$HERE/../..
OUT_DIR=${2:-$ROOT/tmp/firestaff/verification-m10}
mkdir -p "$OUT_DIR"

SUMMARY_MD="$OUT_DIR/verification_summary.md"

# Phase 1: M9 frozen verify (must pass unchanged)
echo "=== Phase 1: M9 frozen verify ==="
"$ROOT/tmp/firestaff/run_firestaff_m9_verify.sh" "$GRAPHICS_DAT" "$OUT_DIR" || {
    echo "FAIL: M9 verify did not pass"
    exit 1
}
echo "M9 verify: PASS"

# Phase 2: M10 round-trip matrix
echo "=== Phase 2: M10 round-trip matrix ==="
ROUNDTRIP_DIR="$OUT_DIR/roundtrip-matrix"
"$ROOT/tmp/firestaff/run_firestaff_m10_roundtrip_matrix.sh" "$GRAPHICS_DAT" "$ROUNDTRIP_DIR" || {
    echo "FAIL: M10 round-trip matrix did not pass"
    exit 1
}
echo "M10 round-trip matrix: PASS"

# Append round-trip results to summary
python3 - <<'PY' "$ROUNDTRIP_DIR/roundtrip_invariants.md" "$SUMMARY_MD" "$ROUNDTRIP_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['roundtrip_matrix.md', 'roundtrip_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'round-trip: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('round-trip: invariant status is not PASS')
if failures:
    text += '\n## M10 round-trip invariant check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 round-trip invariant check: PASS\n\n'
text += '- artifact present: roundtrip_matrix.md\n'
text += '- artifact present: roundtrip_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 3: M10 internal classification matrix (Goal A)
echo "=== Phase 3: M10 internal classification matrix ==="
INTERNAL_DIR="$OUT_DIR/internal-matrix"
"$ROOT/tmp/firestaff/run_firestaff_m10_internal_matrix.sh" "$GRAPHICS_DAT" "$INTERNAL_DIR" || {
    echo "FAIL: M10 internal classification matrix did not pass"
    exit 1
}
echo "M10 internal classification matrix: PASS"

python3 - <<'PY' "$INTERNAL_DIR/internal_invariants.md" "$SUMMARY_MD" "$INTERNAL_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['internal_matrix.md', 'internal_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'internal: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('internal: invariant status is not PASS')
if failures:
    text += '\n## M10 internal classification check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 internal classification check: PASS\n\n'
text += '- artifact present: internal_matrix.md\n'
text += '- artifact present: internal_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 4: M10 VGA palette probe
echo "=== Phase 4: M10 VGA palette probe ==="
VGA_DIR="$OUT_DIR/vga-palette"
"$ROOT/tmp/firestaff/run_firestaff_m10_vga_palette_probe.sh" "$GRAPHICS_DAT" "$VGA_DIR" || {
    echo "FAIL: M10 VGA palette probe did not pass"
    exit 1
}
echo "M10 VGA palette probe: PASS"

python3 - <<'PY' "$VGA_DIR/vga_palette_invariants.md" "$SUMMARY_MD" "$VGA_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['vga_palette_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'vga-palette: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('vga-palette: invariant status is not PASS')
ppm_file = subdir / 'title_hold_vga.ppm'
if not ppm_file.exists():
    failures.append('vga-palette: missing artifact title_hold_vga.ppm')
if failures:
    text += '\n## M10 VGA palette check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 VGA palette check: PASS\n\n'
text += '- artifact present: title_hold_vga.ppm\n'
text += '- artifact present: vga_palette_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 5: Dungeon header probe
echo "=== Phase 5: M10 dungeon header probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
DUNGEON_DIR="$OUT_DIR/dungeon-header"
"$ROOT/tmp/firestaff/run_firestaff_m10_dungeon_header_probe.sh" "$DUNGEON_DAT" "$DUNGEON_DIR" || {
    echo "FAIL: M10 dungeon header probe did not pass"
    exit 1
}
echo "M10 dungeon header probe: PASS"

python3 - <<'PY' "$DUNGEON_DIR/dungeon_header_invariants.md" "$SUMMARY_MD" "$DUNGEON_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['dungeon_header_probe.md', 'dungeon_header_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'dungeon-header: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('dungeon-header: invariant status is not PASS')
if failures:
    text += '\n## M10 dungeon header check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 dungeon header check: PASS\n\n'
text += '- artifact present: dungeon_header_probe.md\n'
text += '- artifact present: dungeon_header_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 6: Dungeon tile decode probe
echo "=== Phase 6: M10 dungeon tile decode probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
TILE_DIR="$OUT_DIR/dungeon-tiles"
"$ROOT/tmp/firestaff/run_firestaff_m10_dungeon_tile_probe.sh" "$DUNGEON_DAT" "$TILE_DIR" || {
    echo "FAIL: M10 dungeon tile decode probe did not pass"
    exit 1
}
echo "M10 dungeon tile decode probe: PASS"

python3 - <<'PY' "$TILE_DIR/dungeon_tile_invariants.md" "$SUMMARY_MD" "$TILE_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['dungeon_tile_probe.md', 'dungeon_tile_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'dungeon-tiles: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('dungeon-tiles: invariant status is not PASS')
if failures:
    text += '\n## M10 dungeon tile decode check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 dungeon tile decode check: PASS\n\n'
text += '- artifact present: dungeon_tile_probe.md\n'
text += '- artifact present: dungeon_tile_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 7: Dungeon text decode probe
echo "=== Phase 7: M10 dungeon text decode probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
TEXT_DIR="$OUT_DIR/dungeon-text"
"$ROOT/tmp/firestaff/run_firestaff_m10_dungeon_text_probe.sh" "$DUNGEON_DAT" "$TEXT_DIR" || {
    echo "FAIL: M10 dungeon text decode probe did not pass"
    exit 1
}
echo "M10 dungeon text decode probe: PASS"

python3 - <<'PY' "$TEXT_DIR/dungeon_text_invariants.md" "$SUMMARY_MD" "$TEXT_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['dungeon_text_probe.md', 'dungeon_text_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'dungeon-text: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('dungeon-text: invariant status is not PASS')
if failures:
    text += '\n## M10 dungeon text decode check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 dungeon text decode check: PASS\n\n'
text += '- artifact present: dungeon_text_probe.md\n'
text += '- artifact present: dungeon_text_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 8: Dungeon doors & sensors (actuators) probe
echo "=== Phase 8: M10 dungeon doors/sensors probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
DOOR_SENSOR_DIR="$OUT_DIR/dungeon-doors-sensors"
"$ROOT/tmp/firestaff/run_firestaff_m10_dungeon_doors_sensors_probe.sh" "$DUNGEON_DAT" "$DOOR_SENSOR_DIR" || {
    echo "FAIL: M10 dungeon doors/sensors probe did not pass"
    exit 1
}
echo "M10 dungeon doors/sensors probe: PASS"

python3 - <<'PY' "$DOOR_SENSOR_DIR/dungeon_doors_sensors_invariants.md" "$SUMMARY_MD" "$DOOR_SENSOR_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['dungeon_doors_sensors_probe.md', 'dungeon_doors_sensors_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'dungeon-doors-sensors: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('dungeon-doors-sensors: invariant status is not PASS')
if failures:
    text += '\n## M10 dungeon doors/sensors check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 dungeon doors/sensors check: PASS\n\n'
text += '- artifact present: dungeon_doors_sensors_probe.md\n'
text += '- artifact present: dungeon_doors_sensors_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 9: Dungeon monsters & items probe
echo "=== Phase 9: M10 dungeon monsters/items probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
MONSTER_ITEM_DIR="$OUT_DIR/dungeon-monsters-items"
"$ROOT/tmp/firestaff/run_firestaff_m10_dungeon_monsters_items_probe.sh" "$DUNGEON_DAT" "$MONSTER_ITEM_DIR" || {
    echo "FAIL: M10 dungeon monsters/items probe did not pass"
    exit 1
}
echo "M10 dungeon monsters/items probe: PASS"

python3 - <<'PY' "$MONSTER_ITEM_DIR/dungeon_monsters_items_invariants.md" "$SUMMARY_MD" "$MONSTER_ITEM_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['dungeon_monsters_items_probe.md', 'dungeon_monsters_items_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'dungeon-monsters-items: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('dungeon-monsters-items: invariant status is not PASS')
if failures:
    text += '\n## M10 dungeon monsters/items check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 dungeon monsters/items check: PASS\n\n'
text += '- artifact present: dungeon_monsters_items_probe.md\n'
text += '- artifact present: dungeon_monsters_items_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 10: Movement + Champions probe
echo "=== Phase 10: M10 movement + champions probe ==="
DUNGEON_DAT=${DUNGEON_DAT:-$FIRESTAFF_DATA/DUNGEON.DAT}
MOVE_CHAMP_DIR="$OUT_DIR/movement-champions"
"$ROOT/tmp/firestaff/run_firestaff_m10_movement_champions_probe.sh" "$DUNGEON_DAT" "$MOVE_CHAMP_DIR" || {
    echo "FAIL: M10 movement + champions probe did not pass"
    exit 1
}
echo "M10 movement + champions probe: PASS"

python3 - <<'PY' "$MOVE_CHAMP_DIR/movement_champions_invariants.md" "$SUMMARY_MD" "$MOVE_CHAMP_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['movement_champions_probe.md', 'movement_champions_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'movement-champions: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('movement-champions: invariant status is not PASS')
if failures:
    text += '\n## M10 movement + champions check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 movement + champions check: PASS\n\n'
text += '- artifact present: movement_champions_probe.md\n'
text += '- artifact present: movement_champions_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 11: Sensor execution probe
echo "=== Phase 11: M10 sensor execution probe ==="
SENSOR_EXEC_DIR="$OUT_DIR/sensor-execution"
"$ROOT/tmp/firestaff/run_firestaff_m10_sensor_execution_probe.sh" "$DUNGEON_DAT" "$SENSOR_EXEC_DIR" || {
    echo "FAIL: M10 sensor execution probe did not pass"
    exit 1
}
echo "M10 sensor execution probe: PASS"

python3 - <<'PY' "$SENSOR_EXEC_DIR/sensor_execution_invariants.md" "$SUMMARY_MD" "$SENSOR_EXEC_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['sensor_execution_probe.md', 'sensor_execution_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'sensor-execution: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('sensor-execution: invariant status is not PASS')
if failures:
    text += '\n## M10 sensor execution check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 sensor execution check: PASS\n\n'
text += '- artifact present: sensor_execution_probe.md\n'
text += '- artifact present: sensor_execution_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 12: Timeline / turn scheduler probe
echo "=== Phase 12: M10 timeline probe ==="
TIMELINE_DIR="$OUT_DIR/timeline"
"$ROOT/tmp/firestaff/run_firestaff_m10_timeline_probe.sh" "$TIMELINE_DIR" || {
    echo "FAIL: M10 timeline probe did not pass"
    exit 1
}
echo "M10 timeline probe: PASS"

python3 - <<'PY' "$TIMELINE_DIR/timeline_invariants.md" "$SUMMARY_MD" "$TIMELINE_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['timeline_probe.md', 'timeline_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'timeline: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('timeline: invariant status is not PASS')
if failures:
    text += '\n## M10 timeline check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 timeline check: PASS\n\n'
text += '- artifact present: timeline_probe.md\n'
text += '- artifact present: timeline_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 13: Combat system probe
echo "=== Phase 13: M10 combat probe ==="
COMBAT_DIR="$OUT_DIR/combat"
"$ROOT/tmp/firestaff/run_firestaff_m10_combat_probe.sh" "$DUNGEON_DAT" "$COMBAT_DIR" || {
    echo "FAIL: M10 combat probe did not pass"
    exit 1
}
echo "M10 combat probe: PASS"

python3 - <<'PY' "$COMBAT_DIR/combat_invariants.md" "$SUMMARY_MD" "$COMBAT_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['combat_probe.md', 'combat_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'combat: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('combat: invariant status is not PASS')
if failures:
    text += '\n## M10 combat check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 combat check: PASS\n\n'
text += '- artifact present: combat_probe.md\n'
text += '- artifact present: combat_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 14: Magic system probe
echo "=== Phase 14: M10 magic probe ==="
MAGIC_DIR="$OUT_DIR/magic"
"$ROOT/tmp/firestaff/run_firestaff_m10_magic_probe.sh" "$DUNGEON_DAT" "$MAGIC_DIR" || {
    echo "FAIL: M10 magic probe did not pass"
    exit 1
}
echo "M10 magic probe: PASS"

python3 - <<'PY' "$MAGIC_DIR/magic_invariants.md" "$SUMMARY_MD" "$MAGIC_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['magic_probe.md', 'magic_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'magic: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('magic: invariant status is not PASS')
if failures:
    text += '\n## M10 magic check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 magic check: PASS\n\n'
text += '- artifact present: magic_probe.md\n'
text += '- artifact present: magic_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 15: Save / Load system probe
echo "=== Phase 15: M10 savegame probe ==="
SAVEGAME_DIR="$OUT_DIR/savegame"
"$ROOT/tmp/firestaff/run_firestaff_m10_savegame_probe.sh" "$DUNGEON_DAT" "$SAVEGAME_DIR" || {
    echo "FAIL: M10 savegame probe did not pass"
    exit 1
}
echo "M10 savegame probe: PASS"

python3 - <<'PY' "$SAVEGAME_DIR/savegame_invariants.md" "$SUMMARY_MD" "$SAVEGAME_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['savegame_probe.md', 'savegame_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'savegame: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('savegame: invariant status is not PASS')
if failures:
    text += '\n## M10 savegame check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 savegame check: PASS\n\n'
text += '- artifact present: savegame_probe.md\n'
text += '- artifact present: savegame_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 16: Creature AI probe
echo "=== Phase 16: M10 creature AI probe ==="
CREATURE_AI_DIR="$OUT_DIR/creature-ai"
"$ROOT/tmp/firestaff/run_firestaff_m10_creature_ai_probe.sh" "$DUNGEON_DAT" "$CREATURE_AI_DIR" || {
    echo "FAIL: M10 creature AI probe did not pass"
    exit 1
}
echo "M10 creature AI probe: PASS"

python3 - <<'PY' "$CREATURE_AI_DIR/creature_ai_invariants.md" "$SUMMARY_MD" "$CREATURE_AI_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['creature_ai_probe.md', 'creature_ai_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'creature-ai: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('creature-ai: invariant status is not PASS')
if failures:
    text += '\n## M10 creature AI check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 creature AI check: PASS\n\n'
text += '- artifact present: creature_ai_probe.md\n'
text += '- artifact present: creature_ai_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 17: Projectile & explosion flight probe
echo "=== Phase 17: M10 projectile probe ==="
PROJECTILE_DIR="$OUT_DIR/projectile"
"$ROOT/tmp/firestaff/run_firestaff_m10_projectile_probe.sh" "$DUNGEON_DAT" "$PROJECTILE_DIR" || {
    echo "FAIL: M10 projectile probe did not pass"
    exit 1
}
echo "M10 projectile probe: PASS"

python3 - <<'PY' "$PROJECTILE_DIR/projectile_invariants.md" "$SUMMARY_MD" "$PROJECTILE_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['projectile_probe.md', 'projectile_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'projectile: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('projectile: invariant status is not PASS')
if failures:
    text += '\n## M10 projectile check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 projectile check: PASS\n\n'
text += '- artifact present: projectile_probe.md\n'
text += '- artifact present: projectile_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 18: Champion lifecycle probe
echo "=== Phase 18: M10 champion lifecycle probe ==="
CHAMPION_LIFECYCLE_DIR="$OUT_DIR/champion-lifecycle"
"$ROOT/tmp/firestaff/run_firestaff_m10_champion_lifecycle_probe.sh" "$DUNGEON_DAT" "$CHAMPION_LIFECYCLE_DIR" || {
    echo "FAIL: M10 champion lifecycle probe did not pass"
    exit 1
}
echo "M10 champion lifecycle probe: PASS"

python3 - <<'PY' "$CHAMPION_LIFECYCLE_DIR/champion_lifecycle_invariants.md" "$SUMMARY_MD" "$CHAMPION_LIFECYCLE_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['champion_lifecycle_probe.md', 'champion_lifecycle_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'champion-lifecycle: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('champion-lifecycle: invariant status is not PASS')
if failures:
    text += '\n## M10 champion lifecycle check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 champion lifecycle check: PASS\n\n'
text += '- artifact present: champion_lifecycle_probe.md\n'
text += '- artifact present: champion_lifecycle_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 19: Runtime dynamics probe
echo "=== Phase 19: M10 runtime dynamics probe ==="
RUNTIME_DYNAMICS_DIR="$OUT_DIR/runtime-dynamics"
"$ROOT/tmp/firestaff/run_firestaff_m10_runtime_dynamics_probe.sh" "$DUNGEON_DAT" "$RUNTIME_DYNAMICS_DIR" || {
    echo "FAIL: M10 runtime dynamics probe did not pass"
    exit 1
}
echo "M10 runtime dynamics probe: PASS"

python3 - <<'PY' "$RUNTIME_DYNAMICS_DIR/runtime_dynamics_invariants.md" "$SUMMARY_MD" "$RUNTIME_DYNAMICS_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['runtime_dynamics_probe.md', 'runtime_dynamics_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'runtime-dynamics: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('runtime-dynamics: invariant status is not PASS')
if failures:
    text += '\n## M10 runtime dynamics check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 runtime dynamics check: PASS\n\n'
text += '- artifact present: runtime_dynamics_probe.md\n'
text += '- artifact present: runtime_dynamics_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

# Phase 20: Tick orchestrator probe (M10 culmination)
echo "=== Phase 20: M10 tick orchestrator probe ==="
TICK_ORCH_DIR="$OUT_DIR/tick-orchestrator"
"$ROOT/tmp/firestaff/run_firestaff_m10_tick_orchestrator_probe.sh" "$DUNGEON_DAT" "$TICK_ORCH_DIR" || {
    echo "FAIL: M10 tick orchestrator probe did not pass"
    exit 1
}
echo "M10 tick orchestrator probe: PASS"

python3 - <<'PY' "$TICK_ORCH_DIR/tick_orchestrator_invariants.md" "$SUMMARY_MD" "$TICK_ORCH_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['tick_orchestrator_probe.md', 'tick_orchestrator_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'tick-orchestrator: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('tick-orchestrator: invariant status is not PASS')
if failures:
    text += '\n## M10 tick orchestrator check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 tick orchestrator check: PASS\n\n'
text += '- artifact present: tick_orchestrator_probe.md\n'
text += '- artifact present: tick_orchestrator_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY

echo "=== M10 verification complete ==="
printf '%s\n' "$SUMMARY_MD"
