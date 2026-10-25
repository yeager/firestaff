#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_firestaff_dm1_scan_debug_localization_real_media.sh <firestaff>}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
archive=${FIRESTAFF_DM1_PC_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic DM1 PC 3.4 archive is required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-scan-debug.XXXXXX")
cleanup() {
    python3 - "$scratch_root" "$scratch" <<'PY'
import pathlib
import shutil
import sys

root = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()
if target.parent == root and target.name.startswith("dm1-scan-debug."):
    shutil.rmtree(target)
PY
}
trap cleanup EXIT HUP INT TERM

home="$scratch/home"
log="$scratch/firestaff.log"
mkdir -p "$home"
if ! HOME="$home" USERPROFILE="$home" XDG_CONFIG_HOME="$home" \
     APPDATA="$home" FIRESTAFF_CONFIG_PATH="$home/config.toml" \
     FIRESTAFF_LOCALE_DIR="${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" \
     SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
     "$app" --game dm1 --platform pc --lang sv --data-dir "$archive" \
         --debug --boot-probe --boot-probe-frames 0 >"$log" 2>&1; then
    cat "$log" >&2
    exit 1
fi

if ! grep -Fq \
    'task=search root task-label="KONTROLLERAR KANDIDATER FÖR ORIGINALFILER"' \
    "$log"; then
    echo 'FAIL: Swedish DM1 startup scan omitted its localized debug task label' >&2
    cat "$log" >&2
    exit 1
fi
if ! grep -Fq 'selected game=dm1 platform=PC edition=pc34-en' "$log" ||
   ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' "$log"; then
    echo 'FAIL: localized scan probe did not select authentic DM1 PC 3.4' >&2
    cat "$log" >&2
    exit 1
fi

echo 'PASS: --lang sv DM1 PC original-media startup prints localized scan labels'

# The regular settings route must localize the synchronous platform scan too.
# This is distinct from --lang, which explicitly overrides saved launcher
# settings. Keep the saved profile isolated and use the same authenticated
# original PC archive without copying or modifying its media.
saved_home="$scratch/saved-profile-home"
saved_log="$scratch/saved-profile-firestaff.log"
mkdir -p "$saved_home"
cat > "$saved_home/config.toml" <<'EOF'
language_index = 1
language_explicit = 1
EOF
if ! HOME="$saved_home" USERPROFILE="$saved_home" \
     XDG_CONFIG_HOME="$saved_home" APPDATA="$saved_home" \
     FIRESTAFF_CONFIG_PATH="$saved_home/config.toml" \
     FIRESTAFF_LOCALE_DIR="${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" \
     SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
     "$app" --game dm1 --platform pc --data-dir "$archive" \
         --debug --boot-probe --boot-probe-frames 0 >"$saved_log" 2>&1; then
    cat "$saved_log" >&2
    exit 1
fi

if ! grep -Fq \
    'task=search root task-label="KONTROLLERAR KANDIDATER FÖR ORIGINALFILER"' \
    "$saved_log" ||
   ! grep -Fq 'selected game=dm1 platform=PC edition=pc34-en' "$saved_log" ||
   ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' "$saved_log"; then
    echo 'FAIL: saved Swedish launcher language was not used by the DM1 PC startup scan' >&2
    cat "$saved_log" >&2
    exit 1
fi

echo 'PASS: saved Swedish launcher profile localizes the authentic DM1 PC startup scan'
