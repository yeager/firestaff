#!/bin/sh
set -eu

app="${1:?Firestaff executable is required}"
data_root="${FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR:-$HOME/.firestaff/data/csb}"

if [ ! -x "$app" ] || [ ! -f "$data_root/Chaos Strikes Back.stx" ] || \
   [ ! -f "$data_root/Chaos Strikes Back Utility.stx" ]; then
    echo "SKIP: original CSB Atari R1 campaign and Utility Disk media are unavailable"
    exit 77
fi

# The R1 MINI.DAT and HCSB pair are discovered from the original STX members.
# The CLI receipt is emitted only after those members have been read and the
# native save decoder has accepted the original GAMEBLOCK.
output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --csb-hint-oracle --data-dir "$data_root" 2>&1)" || {
    printf '%s\n' "$output" >&2
    exit 1
}
case "$output" in
    *"CSB HINT ORACLE READY: save=auto source=atari-r1 native=in-memory"*) ;;
    *)
        echo "FAIL: native CSB Hint Oracle did not admit the original Atari R1 media" >&2
        printf '%s\n' "$output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB Hint Oracle starts from original Atari R1 STX media in memory"
