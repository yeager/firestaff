#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
archive=${FIRESTAFF_CSB_FMTOWNS_RAR:-"$HOME/.firestaff/data/csb/Chaos Strikes Back for FM-Towns.rar"}

if [ ! -x "$firestaff_cli" ] || [ ! -f "$archive" ]; then
    echo "SKIP: authentic CSB FM Towns RAR or Firestaff executable is unavailable"
    exit 77
fi
if ! command -v unrar >/dev/null 2>&1 &&
   ! command -v 7zz >/dev/null 2>&1 &&
   ! command -v 7z >/dev/null 2>&1 &&
   ! command -v bsdtar >/dev/null 2>&1; then
    echo 'SKIP: no supported external RAR reader is installed'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform fm-towns --data-dir "$archive" \
    --enable-external-archive-tools --boot-probe --boot-probe-frames 2 \
    --boot-probe-expect-phase csb-fmtowns-title \
    --boot-probe-expect-asset-md5 405b757038eea3c263e60f240854d6de \
    --duration 0 2>&1) || {
    printf '%s\n' "$direct_output" >&2
    exit 1
}
case "$direct_output" in
    *'variant=csb-fmtowns-en'*'phase=csb-fmtowns-title'*'levelLoaded=0'*) ;;
    *)
        echo 'FAIL: CSB FM Towns RAR did not reach its authenticated title phase' >&2
        printf '%s\n' "$direct_output" >&2
        exit 1
        ;;
esac

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --platform fm-towns --data-dir "$archive" \
    --enable-external-archive-tools \
    --script 'key:enter,key:enter,key:enter' --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
case "$menu_output" in
    *'CSB READY: gameId=csb'*'variant=csb-fmtowns-en'*'route=startup'*) ;;
    *)
        echo 'FAIL: CSB FM Towns RAR did not launch from the startup menu' >&2
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: original CSB FM Towns RAR changed during startup' >&2
    exit 1
fi
echo 'PASS: authentic CSB FM Towns RAR reaches title and startup-menu handoff in memory'
