#!/bin/sh
set -eu

app=${1:?usage: test_csb_v1_hint_oracle_native_7z_cli_boot.sh <firestaff-binary>}
utility_7z=${FIRESTAFF_CSB_UTILITY_7Z:-"$HOME/.firestaff/data/csb/Chaos Strikes Back Utility.stx.7z"}

# The supplied one-member 7z is the original Utility Disk container.  The
# native bounded reader must admit it directly; this test neither extracts it
# nor permits an external archive program to become a runtime dependency.
if [ ! -x "$app" ] || [ ! -f "$utility_7z" ]; then
    printf '%s\n' 'SKIP: original CSB Utility Disk 7z is not staged'
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

printf '%s\n' 'PASS: native CSB Hint Oracle starts from original Atari Utility Disk 7z in memory'
