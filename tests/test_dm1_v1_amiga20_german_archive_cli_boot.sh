#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_amiga20_german_archive_cli_boot.sh <firestaff-binary> <media-probe>}
media_probe=${2:?usage: test_dm1_v1_amiga20_german_archive_cli_boot.sh <firestaff-binary> <media-probe>}
source_archive=${FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z"}
member='Floppy Disks ADF/Dungeon Master for Amiga v2.0 (German) Cracked.adf'
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
expected_adf_md5=e1a0cb7ef2b8262d3f048929511052e4
expected_graphics_md5=0679e39da9dcc2e855cb33c6c64ddcb5
expected_dungeon_md5=59e332cf47852d5c7c88a9538f0adcea

if [[ ! -x "$app" || ! -x "$media_probe" || ! -f "$source_archive" ]]; then
    echo 'SKIP: authentic DM1 Amiga software archive is not staged'
    exit 77
fi
for tool in 7zz zip python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "SKIP: $tool is required to repack the original German ADF bytes into ZIP"
        exit 77
    fi
done

media_output=$(FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE="$source_archive" \
    "$media_probe" --check-german-adf "$source_archive" 2>&1) || {
    printf '%s\n' "$media_output" >&2
    exit 1
}
if ! grep -Fq "graphics_md5=$expected_graphics_md5 graphics_lang=3 swsh_md5=916cb3cd462e7582d2a39c73e40b3339 dungeon_md5=$expected_dungeon_md5 world_trim_0=1 world_trim_2=0 f0437=1 f0441=1" <<<"$media_output"; then
    printf '%s\n' "$media_output" >&2
    echo 'FAIL: authentic German A20 dungeon length does not match its source-owned world layout' >&2
    exit 1
fi
echo 'PASS: authentic German A20 SWSH, title, entrance and dungeon receipts match original media'

mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-amiga20-de.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM
adf="$scratch/Dungeon-Master-Amiga-2.0-DE.adf"
archive="$scratch/Dungeon-Master-Amiga-2.0-DE.zip"

# Repack the exact original ADF member. The ZIP exists only to exercise the
# product's bounded-memory ZIP/ADF readers without requiring 7z at runtime.
7zz x -so "$source_archive" "$member" >"$adf"
python3 - "$adf" "$expected_adf_md5" <<'PY'
import hashlib
import pathlib
import sys

data = pathlib.Path(sys.argv[1]).read_bytes()
actual = hashlib.md5(data).hexdigest()
if len(data) != 901120 or actual != sys.argv[2]:
    raise SystemExit(
        f"FAIL: original German ADF changed: bytes={len(data)} md5={actual}")
PY
zip -0 -j -q "$archive" "$adf"
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --verbose \
    --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-asset-md5 "$expected_graphics_md5" --duration 0 2>&1) || {
    printf '%s\n' "$direct_output" >&2
    exit 1
}
if ! grep -Fq "selected game=dm1 platform=Amiga edition=amiga20-de source=$archive::Dungeon-Master-Amiga-2.0-DE.adf::graphics.dat" <<<"$direct_output" ||
   ! grep -Fq "inventory-file=dungeon matched source=$archive::Dungeon-Master-Amiga-2.0-DE.adf::Dungeon.dat" <<<"$direct_output" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$direct_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$direct_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$direct_output"; then
    printf '%s\n' "$direct_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 CLI launch did not load its paired dungeon' >&2
    exit 1
fi
echo 'PASS: authentic German Amiga 2.0 CLI launch loads both original files'

trace_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --duration 0 2>&1) || {
    printf '%s\n' "$trace_output" >&2
    exit 1
}
if ! grep -Fq 'platform=amiga-v20 phase=title-f0437' <<<"$trace_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' <<<"$trace_output" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$trace_output"; then
    printf '%s\n' "$trace_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 CLI skipped its authenticated source title or entrance' >&2
    exit 1
fi
echo 'PASS: direct German Amiga 2.0 launch completes F0437 and F0441'

# The same authentic package must be discoverable through M12's normal game
# menu and launch the German edition selected by its media, without synthetic
# fixtures or external archive tools.
menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" --debug \
    --script 'enter,enter,enter' --duration 5000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq 'handoff=amiga-img2' <<<"$menu_output" ||
   ! grep -Fq 'edition=amiga20-de' <<<"$menu_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=title-f0437' <<<"$menu_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 start-menu launch did not select its edition' >&2
    exit 1
fi
echo 'PASS: authentic German Amiga 2.0 start-menu launch reaches runtime'
