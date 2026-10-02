#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}
towns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

if [ ! -x "$firestaff_cli" ] || [ ! -f "$mac_archive" ] ||
   [ ! -f "$towns_archive" ]; then
    echo "SKIP: original DM2 Macintosh and FM Towns archives are required"
    exit 77
fi

mac_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform auto --data-dir "$mac_archive" \
    --debug --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$mac_debug" >&2
    exit 1
}
case "$mac_debug" in
    *"task=search root path="*"candidate-file=GRAPHICS.DAT"*"selected game=dm2 platform=Macintosh edition=mac-en-retail"*) ;;
    *) echo "FAIL: Mac debug trace omitted searched source or selected edition" >&2; exit 1 ;;
esac

towns_verbose=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform auto --data-dir "$towns_archive" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$towns_verbose" >&2
    exit 1
}
case "$towns_verbose" in
    *"selected game=dm2 platform=FM Towns edition=fmtowns-ja"*) ;;
    *) echo "FAIL: FM Towns verbose trace omitted selected edition" >&2; exit 1 ;;
esac
case "$towns_verbose" in
    *"candidate-file="*) echo "FAIL: verbose emitted debug-only candidates" >&2; exit 1 ;;
esac

combined_root=$(mktemp -d)
trap 'rm -rf "$combined_root"' EXIT
ln -s "$mac_archive" "$combined_root/$(basename "$mac_archive")"
ln -s "$towns_archive" "$combined_root/$(basename "$towns_archive")"
combined_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_auto" >&2
    exit 1
}
case "$combined_auto" in
    *"platform=FM Towns edition=fmtowns-ja matched source="*"platform=Macintosh edition=mac-en-retail matched source="*"selected game=dm2 platform=FM Towns edition=fmtowns-ja"*) ;;
    *) echo "FAIL: DM2 AUTO did not prefer FM Towns when Mac retail was also present" >&2; exit 1 ;;
esac
combined_mac=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform mac --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_mac" >&2
    exit 1
}
case "$combined_mac" in
    *"selected game=dm2 platform=Macintosh edition=mac-en-retail"*) ;;
    *) echo "FAIL: explicit DM2 Macintosh choice was overridden by FM Towns default" >&2; exit 1 ;;
esac

echo "PASS: original-media startup diagnostics and DM2 FM Towns default"
