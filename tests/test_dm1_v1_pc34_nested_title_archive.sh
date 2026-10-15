#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_pc34_nested_title_archive.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_NESTED_TITLE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN.zip"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic nested DM1 DOS ZIP is not staged'
    exit 77
fi

output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" --debug \
    --script 'wait20,enter,wait20,enter,wait20,enter' --duration 30000 2>&1) || {
    printf '%s\n' "$output" >&2
    exit 1
}

if ! grep -Fq "startup-source-probe game=dm1 platform=pc34 phase=anim-title file=$archive::dungeon-master/dmaster/TITLE" \
        <<<"$output" ||
   ! grep -Fq 'startup-source-ready game=dm1 platform=pc34 phase=anim-title bytes=12002' \
        <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic nested PC-34 TITLE member was not loaded' >&2
    exit 1
fi

python3 - "$output" <<'PY'
import re
import sys

match = re.search(
    r"startup-source-complete game=dm1 platform=pc34 phase=anim-title "
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+)",
    sys.argv[1])
if not match:
    raise SystemExit("FAIL: authentic nested PC-34 I34E title did not complete")
frames, source_vblanks, elapsed_ms = map(int, match.groups())
if frames != 53 or source_vblanks < frames * 5 or elapsed_ms <= 0:
    raise SystemExit(
        f"FAIL: nested PC-34 title cadence mismatch: {match.groups()}")
print(
    f"PASS: nested PC-34 TITLE presented {frames} original frames, "
    f"{source_vblanks} source VBlanks in {elapsed_ms} ms")
PY
