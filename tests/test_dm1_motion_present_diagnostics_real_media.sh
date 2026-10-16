#!/bin/sh
set -eu
app=${1:?Firestaff executable is required}
archive=${FIRESTAFF_DM1_PC_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo "SKIP: original DM1 PC34 archive is required"
    exit 77
fi
mkdir -p .codex-scratch
work=$(mktemp -d "${PWD}/.codex-scratch/dm1-motion-diagnostics.XXXXXX")
trap 'rm -rf "$work"' EXIT
motion_script=key:enter,key:enter,key:enter,wait-game-runtime,up,wait10,left,wait10,right,wait10,down
step=0
while [ "$step" -lt 20 ]; do
    motion_script="$motion_script,wait10,up"
    step=$((step + 1))
done
run_case() {
    motion_event=""
    if [ "$#" -gt 0 ]; then
        motion_event=key:kp5
    fi
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    FIRESTAFF_CONFIG_PATH="$work/config.toml" \
    FIRESTAFF_AUTOTEST=1 \
    FIRESTAFF_AUTOTEST_MOTION_EVENT="$motion_event" \
    FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=key:return "$app" \
        --game dm1 --platform pc --data-dir "$archive" --presentation-mode v1 \
        --width 640 --height 400 --duration 20000 \
        --script "$motion_script" "$@"
}
run_case --debug --debug-full-redraw >"$work/debug.log" 2>&1
python3 - "$work/debug.log" <<'CHECK'
import re
import sys
trace = open(sys.argv[1]).read()
rows = []
for line in trace.splitlines():
    if "firestaff: dm1-motion-present " not in line:
        continue
    fields = dict(re.findall(r"([a-z-]+)=([^ ]+)", line))
    assert fields["input-event-timing"] in {"0", "1"}, line
    for name in ("event-queue-age-ms", "input-event-to-source-outcome-ms",
                 "input-event-to-present-return-ms"):
        assert float(fields[name]) >= 0, line
    assert fields["physical-scanout"] == "unmeasured", line
    values = [float(fields[name]) for name in (
        "source-outcome-to-draw-ms", "draw-to-present-return-ms",
        "source-outcome-to-present-return-ms")]
    assert all(value >= 0 for value in values), line
    assert abs(values[0] + values[1] - values[2]) <= 0.003, line
    rows.append(fields)
assert rows, "No source outcome reached successful presentation"
identities = [(row["launch"], row["outcome-serial"]) for row in rows]
assert len(identities) == len(set(identities)), "Repeated redraw duplicated outcome timing"
assert any(row["moved"] == "1" for row in rows), "No accepted original-world move"
assert any(row["turned"] == "1" for row in rows), "No accepted original-world turn"
assert any(row["blocked"] == "1" and row["moved"] == "0" for row in rows), "No blocked command distinguished"
assert any(row["input-event-timing"] == "1" for row in rows), "No SDL event-to-present sample"
print("PASS: original PC34 movement/turn/block stages, unique outcome timing and explicit measurement limits")
CHECK
run_case >"$work/ordinary.log" 2>&1
if grep -q 'firestaff: dm1-motion-' "$work/ordinary.log"; then
    echo "FAIL: movement stage diagnostics emitted without --debug"
    exit 1
fi
echo "PASS: ordinary DM1 launch emits no movement stage diagnostics"
