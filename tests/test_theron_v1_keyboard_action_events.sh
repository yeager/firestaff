#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <us|jp>\n' "$0" >&2
    exit 2
fi

app=$1
region=$2
case "$region" in
    us)
        track02_name=TQUS02.bin
        expected_md5=f23601102138f87c33025877767ebf76
        startup_script='enter,enter,down,down,down,down,down,down,enter,down,enter,tab'
        data_root=${FIRESTAFF_THERON_US_RAW_BIN_ROOT:-${FIRESTAFF_THERON_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}}
        ;;
    jp)
        track02_name=TQJP02.bin
        expected_md5=b7afb338ad31be1025b53f9aff12d73a
        startup_script='enter,enter,enter,action,tab'
        data_root=${FIRESTAFF_THERON_JP_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
        ;;
    *)
        printf 'FAIL: unsupported Theron region: %s\n' "$region" >&2
        exit 2
        ;;
esac

track02="$data_root/$track02_name"
scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/build/test-scratch"}

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron %s raw Track 02 BIN is not staged\n' "$region"
    exit 77
fi

mkdir -p "$scratch_root"
probe=$(mktemp "$scratch_root/firestaff-theron-$region-keyboard-actions.XXXXXX")
output=$(mktemp "$scratch_root/firestaff-theron-$region-keyboard-actions-output.XXXXXX")
trap 'rm -f "$probe" "$output"' EXIT

# Start the authentic level-0 game, reach its floor sandals, and send each
# action through the normal SDL key event path used by a physical keyboard.
gameplay_events='key:down,key:down,key:q,key:g,key:i,key:i,key:p,key:u,key:return,wait:8'
FIRESTAFF_AUTOTEST=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 224 \
    --game theron \
    --theron-native "$region" \
    --data-dir "$data_root" \
    --debug \
    --lang en \
    --duration 8000 \
    --script "$startup_script,wait-game-runtime,$gameplay_events" \
    >"$output" 2>&1

python3 - "$probe" "$output" "$expected_md5" "$region" <<'PY'
import json
import re
import sys

probe_path, log_path, expected_md5, region = sys.argv[1:]
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

if probe.get("sourceId") != "theron":
    raise SystemExit(f"FAIL: SDL action scenario selected {probe.get('sourceId')!r}")
if not probe.get("startup", {}).get("levelLoaded"):
    raise SystemExit("FAIL: SDL action scenario did not load a dungeon")
if probe.get("bootAssetMd5") != expected_md5:
    raise SystemExit(f"FAIL: runtime did not use authenticated Theron {region} Track 02")
party = probe.get("party", {})
if (party.get("mapX"), party.get("mapY"), party.get("direction")) != (1, 2, 3):
    raise SystemExit(f"FAIL: keyboard action route changed the source-backed pose: {party}")
if probe.get("gameTick", 0) < 1:
    raise SystemExit("FAIL: Return did not advance the native Theron game tick")

with open(log_path, encoding="utf-8", errors="replace") as stream:
    events = [line.strip() for line in stream
              if "game-action game=theron " in line]
for input_name in ("pickup", "inventory-toggle", "drop", "use-item", "action"):
    if not any(f"input={input_name} " in line for line in events):
        raise SystemExit(f"FAIL: SDL key did not reach the {input_name} gameplay route")
for required in (
    'input=pickup result=1 action="ITEM" outcome="ITEM PICKED UP"',
    'input=drop result=1 action="ITEM" outcome="ITEM DROPPED"',
    'input=action result=1 action="WAIT" outcome="THERON TICK"',
):
    if not any(required in line for line in events):
        raise SystemExit(f"FAIL: authentic {region} SDL action receipt missing: {required}")
if not any(re.search(r"input=use-item result=0 ", line) for line in events):
    raise SystemExit("FAIL: U did not reach the bounded no-usable-item route after the drop")
PY

printf 'PASS: authentic Theron %s SDL keyboard actions G/I/P/U/Return\n' "$region"
