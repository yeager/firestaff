#!/bin/sh
set -eu

app="${1:?Firestaff executable is required}"
data_root="${FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR:-$HOME/.firestaff/data/csb}"
campaign_file="$data_root/Chaos Strikes Back.stx"
utility_file="$data_root/Chaos Strikes Back Utility.stx"

if [ ! -x "$app" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 77
fi

if [ ! -f "$campaign_file" ] || [ ! -f "$utility_file" ]; then
    campaign_archive=${FIRESTAFF_CSB_HINT_ORACLE_CAMPAIGN_ARCHIVE:-}
    utility_archive=${FIRESTAFF_CSB_HINT_ORACLE_UTILITY_ARCHIVE:-}
    seven_zip=$(command -v 7zz || command -v 7z || true)
    if [ -n "$seven_zip" ] && [ -f "$campaign_archive" ] &&
       [ -f "$utility_archive" ]; then
        scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
        mkdir -p "$scratch_root"
        data_root=$(mktemp -d "$scratch_root/csb-hint-oracle.XXXXXX")
        trap 'rm -rf "$data_root"' EXIT HUP INT TERM
        "$seven_zip" x -so "$campaign_archive" \
            'Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx' \
            > "$data_root/Chaos Strikes Back.stx"
        "$seven_zip" x -so "$utility_archive" \
            'Floppy Disks STX/Chaos Strikes Back for Atari ST Utility Disk v2.1 (English).stx' \
            > "$data_root/Chaos Strikes Back Utility.stx"
        if [ ! -s "$data_root/Chaos Strikes Back.stx" ] ||
           [ ! -s "$data_root/Chaos Strikes Back Utility.stx" ]; then
            echo "FAIL: original CSB Atari v2.1 STX members could not be extracted" >&2
            exit 1
        fi
        echo "Using original CSB Atari v2.1 STX members in isolated scratch: $data_root"
    else
        echo "SKIP: original CSB Atari R1 campaign and Utility Disk media are unavailable"
        exit 77
    fi
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
