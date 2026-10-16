#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_amiga21_22_english_archive_cli_boot.sh <firestaff-binary> <media-probe>}
media_probe=${2:?usage: test_dm1_v1_amiga21_22_english_archive_cli_boot.sh <firestaff-binary> <media-probe>}
source_archive=${FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z"}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
expected_graphics_md5=b35931b55db649a1bd2d415b61b29801
expected_swsh_md5=658db79c6bb87f3ab09eb7005cd91313
expected_dungeon_md5=30028fb6a301ecb20127ef0b3af32b05

if [[ ! -x "$app" || ! -x "$media_probe" || ! -f "$source_archive" ]]; then
    echo 'SKIP: authentic DM1 Amiga software archive is not staged'
    exit 77
fi
for tool in zip python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "SKIP: $tool is required to wrap the original ADF bytes in ZIP"
        exit 77
    fi
done

run_edition() (
    set -euo pipefail
    version=$1
    case "$version" in
        21)
            version_name=A21E
            expected_adf_md5=156946bb0a06dc0445ec8664b927759d
            ;;
        22)
            version_name=A22E
            expected_adf_md5=89043407605be5d76e1ffa428c1540db
            ;;
        *)
            echo "FAIL: unsupported Amiga English version $version" >&2
            exit 2
            ;;
    esac
    check_mode="--check-a${version}-adf"
    emit_mode="--emit-a${version}-adf"
    minor=${version:1:1}
    media_output=$("$media_probe" "$check_mode" "$source_archive" 2>&1) || {
        printf '%s\n' "$media_output" >&2
        exit 1
    }
    if ! grep -Fq "$version_name media graphics_md5=$expected_graphics_md5" <<<"$media_output" ||
       ! grep -Fq "swsh_md5=$expected_swsh_md5 dungeon_md5=$expected_dungeon_md5" <<<"$media_output" ||
       ! grep -Fq 'graphics_lang=1 graphics_version=2' <<<"$media_output" ||
       ! grep -Fq 'world_trim_0=0 world_trim_2=1 f0437=1 f0441=1' <<<"$media_output"; then
        printf '%s\n' "$media_output" >&2
        echo "FAIL: authentic $version_name media receipt changed" >&2
        exit 1
    fi
    echo "PASS: authentic $version_name graphics, SWSH, dungeon, title and entrance receipts"

    mkdir -p "$scratch_root"
    scratch=$(mktemp -d "$scratch_root/dm1-amiga${version}-en.XXXXXX")
    trap 'rm -rf "$scratch"' EXIT HUP INT TERM
    adf="$scratch/Dungeon-Master-Amiga-2.${minor}-EN.adf"
    archive="$scratch/Dungeon-Master-Amiga-2.${minor}-EN.zip"
    "$media_probe" "$emit_mode" "$source_archive" >"$adf"
    python3 - "$adf" "$expected_adf_md5" <<'PY'
import hashlib
import pathlib
import sys

data = pathlib.Path(sys.argv[1]).read_bytes()
actual = hashlib.md5(data).hexdigest()
if len(data) != 901120 or actual != sys.argv[2]:
    raise SystemExit(f"FAIL: original ADF changed: bytes={len(data)} md5={actual}")
PY
    zip -0 -j -q "$archive" "$adf"
    unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

    direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform amiga --data-dir "$archive" --debug \
        --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-asset-md5 "$expected_graphics_md5" --duration 0 2>&1) || {
        printf '%s\n' "$direct_output" >&2
        exit 1
    }
    if ! grep -Fq 'edition=amiga21-en' <<<"$direct_output" ||
       ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$direct_output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$direct_output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$direct_output" ||
       ! grep -Fq 'platform=amiga-v21 phase=title-f0437' <<<"$direct_output" ||
       ! grep -Fq 'platform=amiga-v21 phase=entrance-f0441' <<<"$direct_output" ||
       ! grep -Fq "inventory-file=dungeon matched source=$archive::Dungeon-Master-Amiga-2.${minor}-EN.adf::DUNGEON.DAT" <<<"$direct_output" ||
       ! grep -Fq "dungeonPath=$archive::Dungeon-Master-Amiga-2.${minor}-EN.adf::DUNGEON.DAT" <<<"$direct_output"; then
        printf '%s\n' "$direct_output" >&2
        echo "FAIL: authentic $version_name CLI launch did not complete startup and dungeon load" >&2
        exit 1
    fi
    echo "PASS: authentic $version_name CLI launch completes F0437, F0441 and dungeon load"

    menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 --platform amiga --data-dir "$archive" --debug \
        --script 'enter,enter,enter' --duration 5000 2>&1) || {
        printf '%s\n' "$menu_output" >&2
        exit 1
    }
    if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
       ! grep -Fq 'handoff=amiga-img2' <<<"$menu_output" ||
       ! grep -Fq 'edition=amiga21-en' <<<"$menu_output" ||
       ! grep -Fq 'platform=amiga-v21 phase=title-f0437' <<<"$menu_output" ||
       ! grep -Fq 'platform=amiga-v21 phase=entrance-f0441' <<<"$menu_output"; then
        printf '%s\n' "$menu_output" >&2
        echo "FAIL: authentic $version_name start-menu launch did not reach runtime" >&2
        exit 1
    fi
    echo "PASS: authentic $version_name start-menu launch reaches runtime"
)

run_edition 21
run_edition 22
