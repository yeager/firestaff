#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <original-CSB-CTRaw.7z>\n' "$0" >&2
    exit 2
fi
app=$1
source_archive=$2
if [[ ! -x "$app" || ! -f "$source_archive" ]]; then
    echo 'SKIP: Firestaff or authentic CSB Amiga 3.5 CTRaw archive is unavailable'
    exit 77
fi

for tool in 7zz python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "SKIP: $tool is required for development-only original-media staging"
        exit 77
    fi
done

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga35-ctraw-boot.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 ]]; then
        for log in "$scratch"/*.log; do
            [[ ! -f "$log" ]] || cat "$log" >&2
        done
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
source_hash_before=$(firestaff_sha256_file "$source_archive")
multi_archive="$scratch/CSB-Multilingual.zip"
python3 - "$source_archive" "$multi_archive" <<'PYMEDIA'
import pathlib
import subprocess
import sys
import zipfile
import zlib

member = "ChaosStrikesBack_Disk1of2_GameDisk_EnglishFrenchGerman.raw"
raw = subprocess.run(["7zz", "x", "-so", "-bd", sys.argv[1], member],
                     stdout=subprocess.PIPE, check=True).stdout
if (len(raw) != 3331587 or raw[:4] != b"CAPS" or
        zlib.crc32(raw) != 0x91568E76):
    raise SystemExit("FAIL: original multilingual CTRaw member identity changed")
archive = pathlib.Path(sys.argv[2])
with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_STORED) as output:
    output.writestr(member, raw)
with zipfile.ZipFile(archive) as output:
    if output.namelist() != [member] or output.read(member) != raw:
        raise SystemExit("FAIL: staging changed the original multilingual CTRaw bytes")
PYMEDIA
multi_hash_before=$(firestaff_sha256_file "$multi_archive")

# Mount the unchanged preservation archive for English, and the single-member
# ZIP above for multilingual. Runtime ZIP/7z and CAPS/OFS reads stay native;
# development staging never repairs or replaces any original capture bytes.
for route in auto amiga multi; do
    selected_archive=$source_archive
    expected_edition=amiga35-en
    expected_graphics_md5=291e1bc6803e3dc4b974c60117ca5d68
    member='ChaosStrikesBack_Disk1of2 GameDisk_English.raw'
    if [[ "$route" = multi ]]; then
        selected_archive=$multi_archive
        expected_edition=amiga35-multi
        expected_graphics_md5=cefaddfdf5651df2c91f61b5611a8362
        member='ChaosStrikesBack_Disk1of2_GameDisk_EnglishFrenchGerman.raw'
    fi
    input_script=enter,up
    if [[ "$route" = multi ]]; then input_script=click:138:70,click:250:50,up; fi
    route_home="$scratch/$route-home"
    mkdir -p "$route_home"
    args=(--width 320 --height 200 --game csb --data-dir "$selected_archive")
    if [[ "$route" != auto ]]; then args+=(--platform amiga); fi
    log="$scratch/$route.log"
    if ! HOME="$route_home" XDG_CONFIG_HOME="$route_home" APPDATA="$route_home" \
        FIRESTAFF_CONFIG_PATH="$route_home/config.toml" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "${args[@]}" \
        --debug --boot-probe --boot-probe-frames 800 --script "$input_script" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-runtime-tick-min 1 --duration 0 >"$log" 2>&1; then
        printf 'FAIL: original CSB CTRaw %s launch failed\n' "$route" >&2
        exit 1
    fi
    for required in \
        "platform=Amiga edition=$expected_edition" \
        "source=$selected_archive::$member::Graphics.DAT" \
        "assetMd5=$expected_graphics_md5" \
        'phase=inactive' \
        'startupActive=0' \
        'levelLoaded=1' \
        'party=9,1,2'; do
        if ! grep -Fq "$required" "$log"; then
            printf 'FAIL: original CSB CTRaw %s route lacks %s\n' "$route" "$required" >&2
            exit 1
        fi
    done
    if ! grep -Eq 'runtimeTick=[1-9][0-9]*([[:space:]]|$)' "$log"; then
        echo 'FAIL: original CSB CTRaw launch did not advance runtime ticks' >&2
        exit 1
    fi
    echo "PASS: original CSB CTRaw $route CLI reaches loaded runtime and moves using native archive/raw provenance"
done
if [[ "$multi_hash_before" != "$(firestaff_sha256_file "$multi_archive")" ]]; then
    echo 'FAIL: staged original multilingual CTRaw ZIP changed during launch' >&2
    exit 1
fi
if [[ "$source_hash_before" != "$(firestaff_sha256_file "$source_archive")" ]]; then
    echo 'FAIL: original CSB CTRaw archive changed during launch' >&2
    exit 1
fi
echo 'PASS: original CSB Amiga 3.5 CTRaw native CLI boot routes'
