#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
towns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"${PWD}/.firestaff-dm2-fmtowns-debug-$$"}
trace_file="$scratch_root/twanim-debug.log"

if [ ! -x "$firestaff_cli" ] || [ ! -f "$towns_archive" ]; then
    echo 'SKIP: Firestaff and authentic DM2 FM Towns media are required'
    exit 77
fi

mkdir -p "$scratch_root"
trap 'rm -rf "$scratch_root"' EXIT HUP INT TERM

# Use the ordinary direct CLI route. Boot-probe fast-forward is intentionally
# excluded because it does not exercise the host scheduler or real-time trace.
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$towns_archive" --debug --duration 40000 \
    >"$trace_file" 2>&1 || {
        cat "$trace_file" >&2
        exit 1
    }

python3 - "$trace_file" <<'PY'
from pathlib import Path
import re
import sys

trace = Path(sys.argv[1]).read_text(encoding="utf-8", errors="replace")
frames = set()
finished = []
rejected = False
pattern = re.compile(
    r"startup-frame game=dm2 elapsed-ms=(\d+).*?"
    r"dm2-fmtowns-title=\{bound:(\d+) finished:(\d+) "
    r"rejected:(\d+) swoosh:(\d+) frame:(\d+)/(\d+) ticks-remaining:\d+\}"
)
for match in pattern.finditer(trace):
    elapsed, bound, done, reject, swoosh, frame, maximum = map(
        int, match.groups()
    )
    rejected |= reject != 0
    if bound and not done and not swoosh and maximum == 225 and 0 < frame < 225:
        frames.add(frame)
    if done and not reject:
        finished.append((elapsed, frame, maximum))

if (
    frames != set(range(1, 225))
    or not finished
    or finished[-1][1:] != (225, 0)
    or not 24000 <= finished[-1][0] < 40000
    or rejected
    or "phase=dm2-startup-menu" not in trace
):
    raise SystemExit(
        "FAIL: DM2 --debug omitted a TWANIM source frame or source completion"
    )

print(
    "PASS: DM2 --debug recorded all 224 FM Towns TWANIM source frames and "
    f"completed in {finished[-1][0]} ms"
)
PY
