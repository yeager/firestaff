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

# Every route below is read-only.  Retain hashes for both independent
# originals so this regression fails if a future launcher path writes either
# supplied save while probing it.
save_hash_before=$(sha256sum "$save_path")
backup_hash_before=$(sha256sum "$backup_path")

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

probe_saved_pose() {
    local selected_save=$1
    local common=(--game dm1 --platform pc --data-dir "$data_dir" --save "$selected_save"
                  --boot-probe --boot-probe-expect-runtime
                  --boot-probe-expect-level-loaded 1 --boot-probe-expect-map 5
                  --boot-probe-expect-party 4,18,2 --boot-probe-expect-champions 4
                  --boot-probe-expect-runtime-tick-min 195221
                  --boot-probe-expect-runtime-tick-max 195221 --duration 0)
    probe_resume "${common[@]}"
    probe_resume --menu "${common[@]}" --script enter,enter,enter
}

# Both supplied original files are independent on-disk recovery candidates.
# Keep their direct and M12 resume routes source-backed rather than assuming
# that a verified primary makes its backup safe to load.
probe_saved_pose "$save_path"
probe_saved_pose "$backup_path"

# One input at a time is applied only after F0435 restores the original pose.
# The four resulting positions/orientations are source observations from this
# supplied snapshot, not an attempted reconstruction of a C13 interaction.
probe_input() {
    local input=$1
    local party=$2
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$data_dir" --save "$save_path" \
        --boot-probe --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 --boot-probe-expect-map 5 \
        --boot-probe-expect-party "$party" --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'inputs=1' <<<"$output" &&
    grep -Fq "party=$party" <<<"$output"
}

probe_input up 4,19,2
probe_input down 4,17,2
probe_input left 4,18,1
probe_input right 4,18,3

# At this authentic F0435 pose the cell in front is neither an attack target
# nor a door.  `action` must therefore follow the native one-tick WAIT path:
# retain the saved pose and advance only the runtime tick.  This records the
# observed route without inventing an encounter, a door state, or a new save.
action_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$data_dir" --save "$save_path" \
    --boot-probe --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --boot-probe-expect-map 5 \
    --boot-probe-expect-party 4,18,2 \
    --boot-probe-expect-runtime-tick-min 195222 \
    --boot-probe-expect-runtime-tick-max 195222 \
    --script action --duration 0 2>&1) || {
    printf '%s\n' "$action_output" >&2
    exit 1
}
grep -Fq 'inputs=1' <<<"$action_output" &&
grep -Fq 'party=4,18,2' <<<"$action_output" &&
grep -Fq 'runtimeTick=195222' <<<"$action_output"

[[ "$save_hash_before" == "$(sha256sum "$save_path")" ]]
[[ "$backup_hash_before" == "$(sha256sum "$backup_path")" ]]

printf '%s\n' 'PASS: authentic French DM1 DMSAVE reaches the same native runtime through CLI and start menu'
