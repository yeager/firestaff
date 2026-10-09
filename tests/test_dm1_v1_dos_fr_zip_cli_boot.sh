#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_dos_fr_zip_cli_boot.sh <firestaff-binary>}
data_source=${FIRESTAFF_DM1_DOS_FR_SOURCE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_EUDATA.zip"}
companion_source=${FIRESTAFF_DM1_PC34_EN_SOURCE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
expected_graphics_md5=f934d97e43e1ba6e5159839acbcd0611
expected_swsh_md5=a66b607f3850e604b6703e90bbfb5189

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable not found: %s\n' "$app" >&2
    exit 1
fi

if [[ -f "$data_source" ]]; then
    case "${data_source##*.}" in
        [zZ][iI][pP]) ;;
        *) printf 'FAIL: expected a ZIP archive: %s\n' "$data_source" >&2; exit 1 ;;
    esac
    archive_listing=$(unzip -Z1 "$data_source") || {
        printf 'FAIL: cannot read ZIP archive: %s\n' "$data_source" >&2
        exit 1
    }
    if ! grep -Fxq 'GRAPHICS.DAT' <<<"$archive_listing" ||
       ! grep -Fxq 'DUNGEON.DAT' <<<"$archive_listing"; then
        printf 'SKIP: authentic DM1 French DOS ZIP is not staged: %s\n' "$data_source"
        exit 77
    fi
else
    printf '%s\n' 'SKIP: repacked authentic DM1 French DOS ZIP is not staged'
    exit 77
fi

if [[ ! -f "$companion_source" ]]; then
    printf 'SKIP: authentic DM1 PC 3.4 companion ZIP is not staged: %s\n' "$companion_source"
    exit 77
fi

# Exercise the historical filename as well as ZIP contents: launch must be
# determined by archive members, never by the filename chosen by the user.
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$(pwd)/.codex-scratch"}
mkdir -p "$scratch_root"
named_archive_dir=$(mktemp -d "$scratch_root/dm1-fr-zip.XXXXXX")
trap 'rm -rf "$named_archive_dir"' EXIT
cp "$data_source" "$named_archive_dir/Dungeon-Master_DOS_FR_EUDATA.zip"
cp "$companion_source" "$named_archive_dir/Dungeon-Master_DOS_EN_Version-34.zip"
actual_swsh_md5=$(python3 - "$named_archive_dir/Dungeon-Master_DOS_EN_Version-34.zip" <<'PY'
import hashlib
import sys
import zipfile

with zipfile.ZipFile(sys.argv[1]) as archive:
    data = archive.read("SWOOSH")
print(hashlib.md5(data).hexdigest())
PY
)
if [[ "$actual_swsh_md5" != "$expected_swsh_md5" ]]; then
    printf 'FAIL: unexpected PC 3.4 companion SWOOSH digest: %s\n' "$actual_swsh_md5" >&2
    exit 1
fi
data_source="$named_archive_dir/Dungeon-Master_DOS_FR_EUDATA.zip"

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

# The French EUDATA ZIP does not own the shared PC 3.4 SWSH startup member.
# It must not be presented as a launchable PC edition until the authentic
# English companion archive is present beside it.
fr_only_dir=$(mktemp -d "$scratch_root/dm1-fr-only.XXXXXX")
cp "$data_source" "$fr_only_dir/Dungeon-Master_DOS_FR_EUDATA.zip"
set +e
fr_only_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$fr_only_dir" --verbose \
    --boot-probe --boot-probe-frames 0 2>&1)
fr_only_status=$?
set -e
rm -rf "$fr_only_dir"
if [[ $fr_only_status -eq 0 ]] ||
   ! grep -Fq 'game unavailable for --game: dm1' <<<"$fr_only_output"; then
    printf '%s\n' "$fr_only_output" >&2
    printf '%s\n' 'FAIL: incomplete French DM1 PC media was accepted without its startup companion' >&2
    exit 1
fi

probe --game dm1 --platform pc --data-dir "$data_source" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform pc --data-dir "$data_source" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform pc --data-dir "$data_source" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$data_source" <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 French DOS ZIP start menu did not bind IMG3 source media' >&2
    exit 1
fi

# Require the authentic French M12 -> DOS route to reach the first playable
# frame; the launch receipt above alone cannot prove that the selected source
# initialized its native level and party position.
menu_runtime_probe_json="$named_archive_dir/menu-runtime.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$data_source" \
    --script enter,enter,enter --duration 10000 >/dev/null 2>&1
python3 - "$menu_runtime_probe_json" "$expected_graphics_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != sys.argv[2] or
        startup["receiptReady"] != 1 or startup["phase"] != "dm1-runtime" or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(
        f"FAIL: authentic French DOS M12 route did not reach the first "
        f"runtime frame: {probe}")
print("PASS: authentic DM1 French DOS start menu reached the first runtime frame")
PY

# The hash-verified French payload starts at (map=0,x=1,y=3,dir=2). Its first
# native forward input lands at y=4. Check that source-owned movement after the
# launcher handoff rather than only accepting a title/runtime receipt.
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$data_source" \
    --script up --boot-probe --boot-probe-frames 500 --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 French DOS ZIP input did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic DM1 French DOS ZIP reaches CLI, menu, IMG3 handoff, and native movement'
