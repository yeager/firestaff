#!/bin/sh
set -eu

runtime_test="${1:?Atari M11 runtime test executable is required}"
firestaff_cli="${2:-}"
data_dir="${FIRESTAFF_CSB_ATARI_DATA:-$HOME/.firestaff/data/csb}"
archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_dir/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"

if [ ! -x "$runtime_test" ]; then
    echo "SKIP: Atari M11 runtime test executable is unavailable"
    exit 0
fi
if [ ! -f "$archive" ]; then
    echo "SKIP: local CSB Atari archive is unavailable: $archive"
    exit 0
fi

if command -v 7zz >/dev/null 2>&1; then
    seven_zip=7zz
elif command -v 7z >/dev/null 2>&1; then
    seven_zip=7z
else
    echo "SKIP: 7z/7zz is unavailable for local CSB archive verification"
    exit 0
fi

tmp_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-runtime-XXXXXX")"
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

# This is the original hard-disk package, not a fixture.  ANIM.C needs its
# two startup files; FTLCODE then loads the source GRAPHICS/DUNGEON pair and
# MINI.DAT supplies the authenticated GAMEBLOCK for the F0435/F0433 roundtrip.
if ! "$seven_zip" e -y -o"$tmp_dir" "$archive" \
    'HardDisk/2009-02-22 PP/ANIMATE.DAT' \
    'HardDisk/2009-02-22 PP/ANIMATE.SCR' \
    'HardDisk/2009-02-22 PP/DUNGEON.DAT' \
    'HardDisk/2009-02-22 PP/GRAPHICS.DAT' \
    'HardDisk/2009-02-22 PP/MINI.DAT' >/dev/null; then
    echo "FAIL: could not extract original CSB Atari runtime members" >&2
    exit 1
fi

for member in ANIMATE.DAT ANIMATE.SCR DUNGEON.DAT GRAPHICS.DAT MINI.DAT; do
    if [ ! -f "$tmp_dir/$member" ]; then
        echo "FAIL: CSB Atari archive is missing $member" >&2
        exit 1
    fi
done

FIRESTAFF_CSB_DATA_DIR="$tmp_dir" \
FIRESTAFF_CSB_ATARI_MINI="$tmp_dir/MINI.DAT" \
FIRESTAFF_CSB_TEST_QUICKSAVE_PATH="$tmp_dir/CSBGAME.DAT" \
    "$runtime_test"

# `--save` is an explicit original-container request.  It must survive the
# M12 launch intent and bypass ANIM.C: ReDMCSB LOADSAVE.C F0435 restores the
# GAMEBLOCK directly rather than replaying ANIMATE.SCR.  This uses only the
# extracted original MINI.DAT and keeps the archive test skip-safe.
if [ -x "$firestaff_cli" ]; then
    probe_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$tmp_dir" --platform atari-st \
        --save "$tmp_dir/MINI.DAT" --boot-probe \
        --boot-probe-expect-runtime --boot-probe-expect-startup-active 0 2>&1)" || {
        printf '%s\n' "$probe_output" >&2
        exit 1
    }
    case "$probe_output" in
        *"phase=inactive"*"startupActive=0"*"levelLoaded=1"*) ;;
        *)
            echo "FAIL: Atari MINI.DAT CLI resume did not reach live runtime" >&2
            printf '%s\n' "$probe_output" >&2
            exit 1
            ;;
    esac

    # The usual M12 start-menu path owns a fresh Atari run.  The separate
    # menu-resume regression below exercises MINI.DAT/F0435; retain this
    # start-path receipt so a regression cannot leave only explicit saves
    # launchable.  The extracted package is still the authenticated original
    # archive member set, never a synthetic flat-data fixture.
    menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy "$firestaff_cli" \
        --menu --game csb --data-dir "$tmp_dir" --platform atari-st \
        --script enter --duration 1000 2>&1)" || {
        printf '%s\n' "$menu_output" >&2
        exit 1
    }
    case "$menu_output" in
        *"CSB READY: gameId=csb"*"route=startup"*) ;;
        *)
            echo "FAIL: CSB Atari start-menu Enter did not launch native media" >&2
            printf '%s\n' "$menu_output" >&2
            exit 1
            ;;
    esac
fi
echo "PASS: original CSB Atari MINI.DAT runtime/save archive corpus"
