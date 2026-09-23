#!/bin/sh
set -eu

probe=$1
sav_root=${MEDNAFEN_SAV_DIR:-"$HOME/.mednafen/sav"}
invalid=
theron_root=${FIRESTAFF_WORKSPACE_DATA_DIR:-"$HOME/.firestaff/data"}/theron
track02=${FIRESTAFF_THERON_US_TRACK02_BIN:-"$theron_root/TQUS02.bin"}
valid=${FIRESTAFF_THERON_AUTHENTIC_PROGRESS_BRAM:-"$theron_root/theron-us-akutuba-complete-authentic.bram"}
main_ram=${FIRESTAFF_THERON_AUTHENTIC_PROGRESS_MAIN_RAM:-"$theron_root/capture/theron-us-akutuba-complete-main-ram.bin"}
save_manager_code=${FIRESTAFF_THERON_SAVE_MANAGER_CODE_DUMP:-"$theron_root/capture/theron-us-save-manager-page6d.bin"}

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
    [ -n "$invalid" ] && [ -f "$track02" ] || exit 77
exec "$probe" "$valid" "$invalid" "$track02" "$main_ram" "$save_manager_code"
