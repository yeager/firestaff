#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script="$repo/scripts/plan_theron_track02_dungeon_handoff_capture.sh"

[[ -x "$script" ]] || { echo 'FAIL: capture-plan script is not executable' >&2; exit 1; }
bash -n "$script"
for required in \
    'THERON_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_V1' \
    'route=dungeon_handoff' \
    'payload_policy=opaque_only' \
    'decoder_policy=forbidden' \
    'render_policy=no_draw' \
    'THERON_DUNGEON_CAPTURE_EXECUTE' \
    'THERON_DUNGEON_CAPTURE_TRACE_MD5' \
    'CUE Track 02 is not an authenticated Theron raw BIN' \
    'refusing to overwrite evidence'; do
    grep -Fq "$required" "$script" || { echo "FAIL: missing strict capture-plan fact: $required" >&2; exit 1; }
done

output=$(env -u MEDNAFEN_BIN -u THERON_US_CUE -u THERON_SYSTEM_CARD \
    -u THERON_DUNGEON_CAPTURE_ROOT -u THERON_DUNGEON_CAPTURE_LAYOUT_EPOCH \
    -u THERON_DUNGEON_CAPTURE_REPLAY_RECORD -u THERON_DUNGEON_CAPTURE_REPLAY_SECTOR \
    -u THERON_DUNGEON_CAPTURE_PLAN_FNV1A -u THERON_DUNGEON_CAPTURE_TRACE_MD5 "$script")
[[ "$output" == SKIP:* ]] || { echo 'FAIL: unstaged plan inputs must skip safely' >&2; exit 1; }

if env MEDNAFEN_BIN=/missing THERON_US_CUE=/missing THERON_SYSTEM_CARD=/missing \
    THERON_DUNGEON_CAPTURE_ROOT=/tmp THERON_DUNGEON_CAPTURE_LAYOUT_EPOCH=1 \
    THERON_DUNGEON_CAPTURE_REPLAY_RECORD=510 THERON_DUNGEON_CAPTURE_REPLAY_SECTOR=510 \
    THERON_DUNGEON_CAPTURE_PLAN_FNV1A=1 \
    THERON_DUNGEON_CAPTURE_TRACE_MD5=11111111111111111111111111111111 \
    "$script" >/dev/null 2>&1; then
    echo 'FAIL: invalid operator paths were accepted' >&2
    exit 1
fi
echo 'theron Track 02 dungeon handoff capture-plan script: PASS'
