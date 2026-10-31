#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_dos_fr_no_game_menu_cli_boot.sh <firestaff> <dm1-data-root>}
data_root=${2:?usage: test_dm1_v1_dos_fr_no_game_menu_cli_boot.sh <firestaff> <dm1-data-root>}
french_archive=${FIRESTAFF_DM1_DOS_FR_SOURCE:-"$data_root/Dungeon-Master_DOS_FR_EUDATA.zip"}
english_companion=${FIRESTAFF_DM1_PC34_EN_SOURCE:-"$data_root/Dungeon-Master_DOS_EN_Version-34.zip"}
expected_graphics_md5=f934d97e43e1ba6e5159839acbcd0611
game_language=${FIRESTAFF_DM1_DOS_GAME_LANGUAGE:-fr}
case "$game_language" in
    fr)
        dungeon_member=DUNGEONF.DAT
        expected_dungeon_md5=82c7802122e9cfb2dc117bc9d197f457
        language_name=French
        ;;
    de)
        dungeon_member=DUNGEONG.DAT
        expected_dungeon_md5=b9487db563ff9e89605c77ee44d17d14
        language_name=German
        ;;
    *)
        echo "FAIL: unsupported test game language: $game_language" >&2
        exit 2
        ;;
esac

if [[ ! -x "$app" ]]; then
    echo 'FAIL: Firestaff executable is unavailable' >&2
    exit 1
fi
if [[ ! -f "$french_archive" || ! -f "$english_companion" ]]; then
    echo 'SKIP: authentic French EUDATA and PC 3.4 startup companion ZIPs are required'
    exit 77
fi
french_listing=$(unzip -Z1 "$french_archive" 2>/dev/null) || {
    echo "SKIP: authentic DM1 French ZIP listing is unavailable: $french_archive"
    exit 77
}
english_listing=$(unzip -Z1 "$english_companion" 2>/dev/null) || {
    echo "SKIP: authentic PC 3.4 companion ZIP listing is unavailable: $english_companion"
    exit 77
}
if ! grep -Fxq 'GRAPHICS.DAT' <<<"$french_listing" ||
   ! grep -Fxq 'DUNGEONF.DAT' <<<"$french_listing" ||
   ! grep -Fxq 'DUNGEONG.DAT' <<<"$french_listing" ||
   ! grep -Fxq 'DATA/GRAPHICS.DAT' <<<"$english_listing" ||
   ! grep -Fxq 'DATA/DUNGEON.DAT' <<<"$english_listing"; then
    echo 'SKIP: authentic DM1 French and PC 3.4 companion members are not present'
    exit 77
fi
actual_dungeon_md5=$(unzip -p "$french_archive" "$dungeon_member" | \
    python3 -c 'import hashlib,sys; print(hashlib.md5(sys.stdin.buffer.read()).hexdigest())')
if [[ "$actual_dungeon_md5" != "$expected_dungeon_md5" ]]; then
    echo "FAIL: $language_name source world hash mismatch: $actual_dungeon_md5" >&2
    exit 1
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$(pwd)/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-fr-no-game-menu.XXXXXX")
cleanup() {
    local result=$?
    trap - EXIT HUP INT TERM
    if [[ $(firestaff_sha256_file "$french_archive") != "$french_hash" ||
          $(firestaff_sha256_file "$english_companion") != "$english_hash" ]]; then
        echo 'FAIL: an original DM1 archive changed during the no-game menu test' >&2
        result=1
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
    exit "$result"
}
french_hash=$(firestaff_sha256_file "$french_archive")
english_hash=$(firestaff_sha256_file "$english_companion")
trap cleanup EXIT HUP INT TERM

home="$scratch/home"
dm1_root="$home/.firestaff/data/dm1"
mkdir -p "$dm1_root"
ln -s "$french_archive" "$dm1_root/$(basename "$french_archive")"
ln -s "$english_companion" "$dm1_root/$(basename "$english_companion")"
log="$scratch/firestaff.log"
probe="$scratch/runtime.json"
navigation='wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:down,wait20,key:enter,wait20,key:enter'

# No game, platform, or data-root option: use the first-run default data tree,
# select DM1 and PC in M12, and let the explicit locale choose the authentic
# multilingual owner and its source-owned French or German world.
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --verbose --lang "$game_language" \
        --script "$navigation" --duration 90000
) >"$log" 2>&1 || {
    grep -E 'startup game=|scan complete|menu-action|game-handoff|DM1 READY|error|failed' "$log" >&2 || true
    exit 1
}

expected_graphics="$dm1_root/$(basename "$french_archive")::GRAPHICS.DAT"
expected_dungeon="$dm1_root/$(basename "$french_archive")::$dungeon_member"
for required in \
    'startup game=none mode=menu' \
    'scan complete' \
    "launch phase=game-handoff mode=menu game=dm1 platform=PC edition=pc34-multi source=$expected_graphics" \
    "DM1 READY: gameId=dm1 dataDir=$expected_graphics handoff=pc-img3 graphicsPath=$expected_graphics dungeonPath=$expected_dungeon"; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: %s DM1 no-game route is missing %s\n' "$language_name" "$required" >&2
        grep -E 'startup game=|scan complete|menu-action|game-handoff|DM1 READY|error|failed' "$log" >&2 || true
        exit 1
    fi
done

# Repeat the same no-game selection while recording the first native runtime
# receipt. This checks source ownership and playable-world admission, not only
# a menu handoff string.
probe_log="$scratch/runtime.log"
runtime_home="$scratch/runtime-home"
runtime_dm1_root="$runtime_home/.firestaff/data/dm1"
mkdir -p "$runtime_dm1_root"
ln -s "$french_archive" "$runtime_dm1_root/$(basename "$french_archive")"
ln -s "$english_companion" "$runtime_dm1_root/$(basename "$english_companion")"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$runtime_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$runtime_home" XDG_CONFIG_HOME="$runtime_home" APPDATA="$runtime_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --lang "$game_language" \
        --script "$navigation" --duration 30000
) >"$probe_log" 2>&1 || {
    grep -E 'startup game=|scan complete|menu-action|game-handoff|DM1 READY|error|failed' "$probe_log" >&2 || true
    exit 1
}

python3 - "$probe" "$expected_graphics_md5" "$language_name" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or probe["bootAssetMd5"] != sys.argv[2] or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: {sys.argv[3]} no-game M12 launch did not reach the authentic first runtime frame: {probe}")
print(f"PASS: {sys.argv[3]} no-game M12 launch reached the authentic DM1 runtime")
PY

echo "PASS: no-game M12 selected DM1 PC 3.4 multilingual media and the $language_name original world"
