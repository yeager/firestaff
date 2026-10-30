#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_atari_only_auto_start.sh <firestaff>}
archive=${FIRESTAFF_DM1_ATARI_ST_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip"}
case "$(basename "$archive")" in
    Dungeon-Master_Atari-ST_EN_Version-12.zip)
        edition=st12-en
        expected_md5s=(
            b3cfd84e44cdf07ce2eeba47e87f772b
            9ce2eaf7a9e78620e3f17594437caffa
        )
        ;;
    Dungeon-Master_Atari-ST_FR_Version-13.zip)
        edition=st13-fr
        expected_md5s=(0d7af44dd14f383464288abdcec76afc)
        ;;
    *)
        printf 'FAIL: unsupported DM1 Atari AUTO archive identity: %s\n' "$archive" >&2
        exit 2
        ;;
esac
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic DM1 Atari ST archive is not staged'
    exit 77
fi
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-atari-only-auto.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        python3 - "$scratch_root" "$scratch" <<'PY'
import pathlib
import shutil
import sys

root = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()
if target.parent == root and target.name.startswith("dm1-atari-only-auto."):
    shutil.rmtree(target)
PY
    fi
}
trap cleanup EXIT HUP INT TERM

archive_hash=$(firestaff_sha256_file "$archive")
direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --data-dir "$archive" --boot-probe \
    --boot-probe-frames 2 --duration 0 2>&1)
matched=0
for expected_md5 in "${expected_md5s[@]}"; do
    if grep -Fq "assetMd5=$expected_md5" <<<"$direct_output"; then
        matched=1
        break
    fi
done
if [[ $matched != 1 ]] ||
   ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$direct_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$direct_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$direct_output"; then
    printf '%s\n' "$direct_output" >&2
    echo 'FAIL: bare DM1 CLI did not AUTO-select the authentic Atari ST archive' >&2
    exit 1
fi

data_root="$scratch/data"
home="$scratch/home"
mkdir -p "$data_root/dm1" "$home"
ln -s "$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)" "$data_root/dm1/$(basename "$archive")"
menu_log="$scratch/menu.log"
HOME="$home" XDG_CONFIG_HOME="$home/.config" APPDATA="$home" \
FIRESTAFF_CONFIG_PATH="$home/config.toml" FIRESTAFF_DATA="$data_root" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --debug --verbose --width 1920 --height 1080 --data-dir "$data_root" \
    --script 'wait-scan-complete,wait20,click:700:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 90000 >"$menu_log" 2>&1 || {
        cat "$menu_log" >&2
        exit 1
    }
if ! grep -Fq 'startup game=none mode=menu' "$menu_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Atari ST edition=$edition source=$data_root/dm1/$(basename "$archive")" "$menu_log"; then
    cat "$menu_log" >&2
    echo 'FAIL: no-game M12 did not select the sole authentic Atari ST edition' >&2
    exit 1
fi
if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original DM1 Atari ST archive changed during AUTO startup checks' >&2
    exit 1
fi
echo "PASS: DM1 bare CLI AUTO and no-game M12 both select authentic Atari ST $edition media"
