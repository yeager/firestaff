#!/bin/sh
set -eu

probe=$1
sav_root=${MEDNAFEN_SAV_DIR:-"$HOME/.mednafen/sav"}
invalid=
theron_root=${FIRESTAFF_WORKSPACE_DATA_DIR:-"$HOME/.firestaff/data"}/theron
track02=${FIRESTAFF_THERON_US_TRACK02_BIN:-"$theron_root/TQUS02.bin"}
jp_track02=${FIRESTAFF_THERON_JP_TRACK02_BIN:-"$theron_root/TQJP02.bin"}
valid=${FIRESTAFF_THERON_AUTHENTIC_PROGRESS_BRAM:-"$theron_root/theron-us-akutuba-complete-authentic.bram"}
main_ram=${FIRESTAFF_THERON_AUTHENTIC_PROGRESS_MAIN_RAM:-"$theron_root/capture/theron-us-akutuba-complete-main-ram.bin"}
save_manager_code=${FIRESTAFF_THERON_SAVE_MANAGER_CODE_DUMP:-"$theron_root/capture/theron-us-save-manager-page6d.bin"}
jp_empty_bram=${FIRESTAFF_THERON_JP_EMPTY_BRAM:-"$sav_root/Dungeon Master - Theron's Quest (Japan).0daf24b401cbe8c84e88114eab6a3625.sav"}

for candidate in "$sav_root"/*Theron*.sav "$sav_root"/TQUS.*.sav "$sav_root"/firestaff-tqus-mednafen.*.sav; do
    [ -f "$candidate" ] || continue
    size=$(wc -c < "$candidate" | tr -d ' ')
    if [ "$size" != 2048 ] && [ -z "$invalid" ]; then invalid=$candidate; fi
done

# CI and hosts without operator-owned original-emulator artifacts report an
# explicit CTest skip. A missing corpus must never look like a passing
# real-artifact verification.
[ -f "$valid" ] && [ "$(wc -c < "$valid" | tr -d ' ')" = 2048 ] &&
    [ -f "$main_ram" ] && [ "$(wc -c < "$main_ram" | tr -d ' ')" = 8192 ] &&
    [ -f "$save_manager_code" ] && [ "$(wc -c < "$save_manager_code" | tr -d ' ')" = 8192 ] &&
    [ -n "$invalid" ] && [ -f "$track02" ] && [ -f "$jp_track02" ] || exit 77
export FIRESTAFF_THERON_BRAM_PATH="$valid"
if [ -f "$jp_empty_bram" ]; then
    if command -v md5 >/dev/null 2>&1; then
        jp_empty_md5=$(md5 -q "$jp_empty_bram")
    else
        jp_empty_md5=$(md5sum "$jp_empty_bram" | awk '{print $1}')
    fi
    [ "$(wc -c < "$jp_empty_bram" | tr -d ' ')" = 2048 ] &&
        [ "$jp_empty_md5" = dbdedb0ec809227b289c2bc5b18b9c9d ] || {
            echo "FAIL: JP empty Backup RAM is not the authenticated original" >&2
            exit 1
        }
    empty_tmp_root=${TMPDIR:-/tmp}
    empty_tmp_dir=$(mktemp -d "$empty_tmp_root/firestaff-theron-jp-empty.XXXXXX")
    empty_bram_link="$empty_tmp_dir/jp-empty.bram"
    cleanup_empty_bram_link() {
        rm -f "$empty_bram_link"
        rmdir "$empty_tmp_dir"
    }
    trap cleanup_empty_bram_link EXIT HUP INT TERM
    ln -s "$jp_empty_bram" "$empty_bram_link"
    set +e
    "$probe" "$valid" "$invalid" "$track02" "$main_ram" \
        "$save_manager_code" "$jp_track02" "$empty_bram_link"
    probe_status=$?
    set -e
    exit "$probe_status"
fi
if [ -n "${FIRESTAFF_THERON_JP_EMPTY_BRAM:-}" ]; then
    echo "FAIL: configured authentic JP empty Backup RAM is unavailable" >&2
    exit 1
fi
exec "$probe" "$valid" "$invalid" "$track02" "$main_ram" "$save_manager_code" "$jp_track02"
