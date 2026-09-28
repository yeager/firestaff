#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_pc9821_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_PC9821_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_PC-9821_JA.zip"}

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

file_sha256() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | awk '{print $1}'
    else
        shasum -a 256 "$1" | awk '{print $1}'
    fi
}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 PC-9821 archive is not staged'
    exit 77
fi

archive_hash_before=$(file_sha256 "$archive")

# Exercise the selected-media route through the start menu, then drive the
# source-coordinate title and New Game flow to the first real map/party.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc98 --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# Exercise the scaled start-menu selection route at a HiDPI-sized output.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform pc98 \
    --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:934:679,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

probe_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --platform pc98 \
    --data-dir "$archive" --boot-probe --boot-probe-frames 0 \
    --boot-probe-expect-phase dm2-startup-menu \
    --boot-probe-expect-startup-active 1 --boot-probe-expect-title-ready 1 \
    --duration 0 2>&1) || {
    printf '%s\n' "$probe_output" >&2
    exit 1
}
printf '%s\n' "$probe_output" | grep -q \
    'dm2SceneReady=1'
printf '%s\n' "$probe_output" | grep -Fq "assetMd5=a80c555a858ef7770e1d7f3d2e37fec3"
printf '%s\n' "$probe_output" | grep -Fq \
    'Dungeon-Master-II-Skullkeep_PC-9821_JA.zip'

if [ "$archive_hash_before" != "$(file_sha256 "$archive")" ]; then
    echo 'FAIL: DM2 PC-9821 archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 PC-9821 ZIP start menu and original title assets'
