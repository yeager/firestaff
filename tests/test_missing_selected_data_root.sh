#!/bin/sh
set -eu
app=${1:?Firestaff executable is required}
mkdir -p .codex-scratch
work=$(mktemp -d "${PWD}/.codex-scratch/missing-selected-data.XXXXXX")
trap 'rm -rf "$work"' EXIT
mkdir "$work/home"
run_menu() {
    selected_data_path=${1-}
    originals_override=${2-}
    if [ -n "$selected_data_path" ]; then
        set -- --data-dir "$selected_data_path"
    else
        set --
    fi
    HOME="$work/home" USERPROFILE="$work/home" XDG_CONFIG_HOME="$work/home" APPDATA="$work/home" \
    FIRESTAFF_CONFIG_PATH="$work/config.toml" \
    FIRESTAFF_ORIGINALS_DIR="$originals_override" FIRESTAFF_DATA= \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 "$@" --duration 1000 --debug
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
# A real, accepted folder can disappear while an external disk is offline.
# Its saved intent must survive absence and a later reconnect, even though
# the built-in fallback directory already exists from the first-run case.
for offline_leaf in offline-originals collection/dm1; do
    offline_path="$work/$offline_leaf"
    intent_marker=$(basename "$offline_path")
    mkdir -p "$offline_path"
    run_menu "$offline_path" >"$work/selected.log" 2>&1 || { cat "$work/selected.log"; exit 1; }
    saved_selection=$(grep '^data_dir = ' "$work/config.toml")
    case "$saved_selection" in
        *"$intent_marker"*) ;;
        *) echo "FAIL: initial folder selection was not persisted"; exit 1 ;;
    esac
    rmdir "$offline_path"
    run_menu >"$work/offline.log" 2>&1 || { cat "$work/offline.log"; exit 1; }
    if [ "$saved_selection" != "$(grep '^data_dir = ' "$work/config.toml")" ] ||
       [ -e "$offline_path" ]; then
        echo "FAIL: offline selected folder was forgotten or recreated"
        exit 1
    fi
    mkdir -p "$offline_path"
    run_menu >"$work/reconnected.log" 2>&1 || { cat "$work/reconnected.log"; exit 1; }
    if [ "$saved_selection" != "$(grep '^data_dir = ' "$work/config.toml")" ]; then
        echo "FAIL: reconnected selected folder was forgotten"
        exit 1
    fi
done
echo "PASS: saved folder selection survives absence and reconnect"
echo "PASS: absent selected directory/archive/override stay absent; managed default directory is created"
