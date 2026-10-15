#!/bin/sh
set -eu
app=${1:?Firestaff executable is required}
mkdir -p .codex-scratch
work=$(mktemp -d "${PWD}/.codex-scratch/missing-selected-data.XXXXXX")
trap 'rm -rf "$work"' EXIT
mkdir "$work/home"
run_menu() {
    HOME="$work/home" USERPROFILE="$work/home" XDG_CONFIG_HOME="$work/home" APPDATA="$work/home" \
    FIRESTAFF_CONFIG_PATH="$work/config.toml" \
    FIRESTAFF_ORIGINALS_DIR="${2-}" FIRESTAFF_DATA= \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 --data-dir "$1" --duration 1000 --debug
}
# Absence is the fixture: no synthetic game files stand in for original media.
run_menu "$work/disconnected-volume/originals" >"$work/missing-directory.log" 2>&1 || { cat "$work/missing-directory.log"; exit 1; }
if [ -e "$work/disconnected-volume" ]; then
    echo "FAIL: launcher recreated a missing selected volume/data path"
    exit 1
fi
run_menu "$work/missing-original.zip" >"$work/missing-archive.log" 2>&1 || { cat "$work/missing-archive.log"; exit 1; }
if [ -e "$work/missing-original.zip" ]; then
    echo "FAIL: launcher turned a missing archive path into a directory"
    exit 1
fi
run_menu "$work/unplugged-override" "$work/unplugged-override" >"$work/missing-override.log" 2>&1 || { cat "$work/missing-override.log"; exit 1; }
if [ -e "$work/unplugged-override" ]; then
    echo "FAIL: launcher recreated a missing process-selected originals root"
    exit 1
fi
run_menu "$work/home/.firestaff/data" >"$work/default-directory.log" 2>&1 || { cat "$work/default-directory.log"; exit 1; }
if [ ! -d "$work/home/.firestaff/data" ]; then
    echo "FAIL: managed default data directory was not bootstrapped"
    exit 1
fi
echo "PASS: absent selected directory/archive/override stay absent; managed default directory is created"
