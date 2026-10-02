#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}
towns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
dm1_towns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
csb_towns_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_towns_loose_root=${FIRESTAFF_CSB_FMTOWNS_LOOSE_ROOT:-"$HOME/.firestaff/data/csb/fmtowns_iso"}

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
    *"candidate-file="*"edition-result="*) ;;
    *) echo "FAIL: verbose omitted catalogued candidate filenames" >&2; exit 1 ;;
esac

combined_root=$(mktemp -d "${PWD}/.firestaff-cli-diagnostics.XXXXXX")
csb_mixed_root=
trap 'rm -rf "$combined_root" "$csb_mixed_root"' EXIT
ln -s "$mac_archive" "$combined_root/$(basename "$mac_archive")"
ln -s "$towns_archive" "$combined_root/$(basename "$towns_archive")"
combined_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_auto" >&2
    exit 1
}
case "$combined_auto" in
    *"startup game=dm2 mode=direct platform=auto"*"platform=FM Towns edition=fmtowns-ja matched source="*"platform=Macintosh edition=mac-en-retail matched source="*"selected game=dm2 platform=FM Towns edition=fmtowns-ja"*) ;;
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

# A bare game selection must not inherit an obsolete platform from saved
# launcher settings. Check the other two original FM Towns archives when
# they are staged, using the same direct CLI path as the DM2 check above.
if [ -f "$dm1_towns_archive" ]; then
    dm1_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_auto" >&2
        exit 1
    }
    case "$dm1_auto" in
        *"startup game=dm1 mode=direct platform=auto"*"selected game=dm1 platform=FM Towns"*) ;;
        *) echo "FAIL: bare DM1 did not select original FM Towns media" >&2; exit 1 ;;
    esac
fi
if [ -f "$csb_towns_archive" ]; then
    csb_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$csb_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_auto" >&2
        exit 1
    }
    case "$csb_auto" in
        *"startup game=csb mode=direct platform=auto"*"selected game=csb platform=FM Towns"*) ;;
        *) echo "FAIL: bare CSB did not select original FM Towns media" >&2; exit 1 ;;
    esac
    if [ -d "$csb_towns_loose_root/CDATA" ] &&
       [ -d "$csb_towns_loose_root/CJDATA" ]; then
        csb_mixed_root=$(mktemp -d "${PWD}/.firestaff-csb-mixed-media.XXXXXX")
        mkdir "$csb_mixed_root/csb"
        ln -s "$csb_towns_archive" "$csb_mixed_root/csb/$(basename "$csb_towns_archive")"
        ln -s "$csb_towns_loose_root" "$csb_mixed_root/csb/fmtowns_iso"
        csb_mixed=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game csb --data-dir "$csb_mixed_root" \
            --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$csb_mixed" >&2
            exit 1
        }
        case "$csb_mixed" in
            *"selected game=csb platform=FM Towns edition=fmtowns-en source="*"$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*"CSB READY:"*) ;;
            *) echo "FAIL: CSB selected the loose tree over its complete original CD image" >&2; exit 1 ;;
        esac
        rm -rf "$csb_mixed_root"
    fi
fi

echo "PASS: original-media startup diagnostics and FM Towns defaults"
