#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_ATARI_DATA:-$HOME/.firestaff/data/csb}"
archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_dir/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"

if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
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

test_root="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-menu-resume-XXXXXX")"
test_home="$test_root/home"
runtime_dir="$test_root/runtime"
config_dir="$test_home/Library/Application Support/Firestaff"
output="$test_root/launcher.log"
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

mkdir -p "$config_dir" "$runtime_dir"

# This is an explicit original-data route.  MINI.DAT supplies the saved
# GAMEBLOCK, while the four runtime members remain source files from the
# Atari hard-disk archive.  Do not replace any of them with a test fixture.
if ! "$seven_zip" e -y -o"$runtime_dir" "$archive" \
    'HardDisk/2009-02-22 PP/ANIMATE.DAT' \
    'HardDisk/2009-02-22 PP/ANIMATE.SCR' \
    'HardDisk/2009-02-22 PP/DUNGEON.DAT' \
    'HardDisk/2009-02-22 PP/GRAPHICS.DAT' \
    'HardDisk/2009-02-22 PP/MINI.DAT' >/dev/null; then
    echo "FAIL: could not extract original CSB Atari menu-resume members" >&2
    exit 1
fi

for member in ANIMATE.DAT ANIMATE.SCR DUNGEON.DAT GRAPHICS.DAT MINI.DAT; do
    if [ ! -f "$runtime_dir/$member" ]; then
        echo "FAIL: CSB Atari archive is missing $member" >&2
        exit 1
    fi
done

# M12 discovers a normal Resume row from the persisted launcher setting.
# It must carry exactly this original MINI.DAT path through the M12 intent;
# the user does not pass --save on this route.
printf '%s\n' \
    'quick_resume_enabled = 1' \
    "last_save_path = \"$runtime_dir/MINI.DAT\"" \
    "data_dir = \"$runtime_dir\"" \
    > "$config_dir/startup-menu.toml"

if ! HOME="$test_home" SDL_VIDEODRIVER=dummy \
    FIRESTAFF_SKIP_INTRO=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    "$firestaff_cli" --menu --game csb --data-dir "$runtime_dir" --platform atari-st \
    --script 'up,up,enter' --duration 1200 >"$output" 2>&1; then
    cat "$output" >&2
    exit 1
fi

if [ "${FIRESTAFF_CSB_MENU_RESUME_VERBOSE:-0}" = 1 ]; then
    cat "$output"
fi

if ! grep -Fq 'CSB READY:' "$output" ||
   ! grep -Fq 'route=f0435-resume' "$output"; then
    echo "FAIL: normal M12 Resume did not reach the Atari F0435 save route" >&2
    cat "$output" >&2
    exit 1
fi

echo "PASS: original CSB Atari MINI.DAT resumes through the M12 start menu"
