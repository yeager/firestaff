#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_dos_fr_original_save_cli_resume.sh <firestaff-binary>}
save_root=${FIRESTAFF_DM1_DOS_FR_SAVE_DIR:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_unpacked/dungeon-master-fr/dungeon_master"}
data_dir="$save_root/EUDATA"
save_path="$save_root/DMSAVE.DAT"
backup_path="$save_root/DMSAVE.BAK"

# This is an opt-in original-media regression. It does not create, rewrite,
# or unpack game data: the supplied French DOS pair and matching IMG3 media
# remain the sole source for both direct CLI and M12 start-menu resume.
if [[ ! -x "$app" || ! -f "$save_path" || ! -f "$backup_path" ||
      ! -f "$data_dir/GRAPHICS.DAT" || ! -f "$data_dir/DUNGEON.DAT" ]]; then
    printf '%s\n' 'SKIP: authentic unpacked French DM1 save and IMG3 media are not staged'
    exit 77
fi

probe_resume() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }

    # These values are read from the authenticated DMSAVE.DAT F0435 snapshot:
    # map 5, four champions, party (4,18,2), and the saved game tick. Do not
    # substitute a fixture or generated save to satisfy this route.
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output" &&
    grep -Fq 'map=5' <<<"$output" &&
    grep -Fq 'party=4,18,2' <<<"$output" &&
    grep -Fq 'champions=4' <<<"$output" &&
    grep -Fq 'runtimeTick=195221' <<<"$output"
}

common=(--game dm1 --platform pc --data-dir "$data_dir" --save "$save_path"
        --boot-probe --boot-probe-expect-runtime
        --boot-probe-expect-level-loaded 1 --boot-probe-expect-map 5
        --boot-probe-expect-party 4,18,2 --boot-probe-expect-champions 4
        --boot-probe-expect-runtime-tick-min 195221
        --boot-probe-expect-runtime-tick-max 195221 --duration 0)

probe_resume "${common[@]}"
probe_resume --menu "${common[@]}" --script enter,enter,enter

printf '%s\n' 'PASS: authentic French DM1 DMSAVE reaches the same native runtime through CLI and start menu'
