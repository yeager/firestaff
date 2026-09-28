#!/bin/sh
set -eu

app=${1:?usage: test_csb_v1_hint_oracle_native_7z_cli_boot.sh <firestaff-binary>}
utility_7z=${FIRESTAFF_CSB_ATARI_MULTI_7Z:-"$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z"}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
menu_scratch="$scratch_root/csb-hint-oracle-menu-$$"
menu_probe="$menu_scratch/runtime.json"

# The Atari preservation archive is solid and contains multiple disk images
# alongside the original Hint Oracle hard-disk files. The native bounded
# reader must admit those original members directly; this test neither
# extracts them nor permits a host archive program at runtime.
if [ ! -x "$app" ] || [ ! -f "$utility_7z" ]; then
    printf '%s\n' 'SKIP: original CSB Atari multi-member 7z is not staged'
    exit 77
fi

output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --csb-hint-oracle --data-dir "$utility_7z" --duration 0 2>&1)" || {
    printf '%s\n' "$output" >&2
    exit 1
}

case "$output" in
    *'CSB HINT ORACLE READY: save=auto source=atari-r1 native=in-memory'*) ;;
    *)
        printf '%s\n' "$output" >&2
        printf '%s\n' 'FAIL: native CSB Hint Oracle did not admit the original Utility Disk 7z' >&2
        exit 1
        ;;
esac

mkdir -p "$menu_scratch"
trap 'rm -f "$menu_probe"; rmdir "$menu_scratch" 2>/dev/null || true' EXIT HUP INT TERM
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 960 --height 600 --menu --game csb --platform atari-st \
    --data-dir "$utility_7z" \
    --script 'down,down,down,down,down,enter' --duration 2000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
python3 - "$menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb-hint-oracle-atari-r1" or
        startup["receiptReady"] != 1 or startup["active"] != 1 or
        startup["startupActive"] != 0):
    raise SystemExit("FAIL: Atari Utility Disk menu entry did not hand off to Hint Oracle")
print("PASS: CSB start menu launches the original Atari multi-member Hint Oracle")
PY

printf '%s\n' 'PASS: native CSB Hint Oracle starts from original Atari multi-member 7z through CLI and start menu'
