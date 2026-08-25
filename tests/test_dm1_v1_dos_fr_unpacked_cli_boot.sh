#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_dos_fr_unpacked_cli_boot.sh <firestaff-binary>}
data_dir=${FIRESTAFF_DM1_DOS_FR_UNPACKED_DIR:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_unpacked/dungeon-master-fr/dungeon_master/EUDATA"}
expected_graphics_md5=f934d97e43e1ba6e5159839acbcd0611

if [[ ! -x "$app" || ! -f "$data_dir/GRAPHICS.DAT" || ! -f "$data_dir/DUNGEON.DAT" ]]; then
    printf '%s\n' 'SKIP: manually unpacked authentic DM1 French DOS RAR2 media is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_graphics_md5" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform pc --data-dir "$data_dir" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --menu --platform pc --data-dir "$data_dir" \
    --script enter --boot-probe --boot-probe-frames 2 --duration 0

printf '%s\n' 'PASS: manually unpacked authentic DM1 French DOS media reaches CLI and menu runtime'
