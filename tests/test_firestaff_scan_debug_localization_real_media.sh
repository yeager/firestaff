#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_firestaff_scan_debug_localization_real_media.sh <firestaff> <game> <platform> <edition> <archive>}
game=${2:?game id required}
platform=${3:?platform required}
edition=${4:?edition id required}
archive=${5:?authentic game archive required}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo "SKIP: authentic $game startup archive is required: $archive"
    exit 77
fi

case "$platform" in
    pc) platform_label=PC ;;
    fmtowns) platform_label='FM Towns' ;;
    *) echo "FAIL: unsupported localization-test platform: $platform" >&2; exit 2 ;;
esac

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/scan-debug-localization.XXXXXX")
cleanup() {
    python3 - "$scratch_root" "$scratch" <<'PY'
import pathlib
import shutil
import sys

root = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()
if target.parent == root and target.name.startswith("scan-debug-localization."):
    shutil.rmtree(target)
PY
}
trap cleanup EXIT HUP INT TERM

run_scan_probe() {
    local mode=$1
    local home="$scratch/$mode-home"
    local log="$scratch/$mode-firestaff.log"
    mkdir -p "$home"
    if [[ "$mode" == saved-language ]]; then
        cat > "$home/config.toml" <<'EOF'
language_index = 1
language_explicit = 1
EOF
    fi

    local lang_args=()
    if [[ "$mode" == explicit-language ]]; then
        lang_args=(--lang sv)
    fi

    if ! HOME="$home" USERPROFILE="$home" XDG_CONFIG_HOME="$home" \
         APPDATA="$home" FIRESTAFF_CONFIG_PATH="$home/config.toml" \
         FIRESTAFF_LOCALE_DIR="${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" \
         SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
         "$app" --game "$game" --platform "$platform" "${lang_args[@]}" \
             --data-dir "$archive" --debug --boot-probe \
             --boot-probe-frames 0 >"$log" 2>&1; then
        cat "$log" >&2
        return 1
    fi

    if ! grep -Fq \
        'task=search root task-label="KONTROLLERAR KANDIDATER FÖR ORIGINALFILER"' \
        "$log" ||
       ! grep -Fq \
        'task=matching editions task-label="MATCHAR SPELVERSIONER"' "$log"; then
        echo "FAIL: Swedish $game $platform scan omitted a localized task label ($mode)" >&2
        cat "$log" >&2
        return 1
    fi
    if ! grep -Fq \
        "selected game=$game platform=$platform_label edition=$edition" "$log" ||
       ! grep -Fq "FIRESTAFF BOOT PROBE READY: gameId=$game" "$log"; then
        echo "FAIL: localized $game startup did not select authentic $edition media ($mode)" >&2
        cat "$log" >&2
        return 1
    fi
    echo "PASS: $mode Swedish scan labels and authentic $game $edition selection"
}

run_scan_probe explicit-language
run_scan_probe saved-language

# The normal no-game start must localize the same scan labels while the
# launcher remains on its menu. Direct --game probes above do not cover this
# route: they can pass even if the app's default menu startup ignores the
# selected or saved language.
run_menu_scan_probe() {
    local mode=$1
    local home="$scratch/menu-$mode-home"
    local log="$scratch/menu-$mode-firestaff.log"
    local exit_status=0
    mkdir -p "$home"
    if [[ "$mode" == saved-language ]]; then
        cat > "$home/config.toml" <<'EOF'
language_index = 1
language_explicit = 1
EOF
    fi

    local lang_args=()
    if [[ "$mode" == explicit-language ]]; then
        lang_args=(--lang sv)
    fi

    HOME="$home" USERPROFILE="$home" XDG_CONFIG_HOME="$home" \
    APPDATA="$home" FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_LOCALE_DIR="${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --menu "${lang_args[@]}" --data-dir "$archive" \
            --debug --verbose --script wait-scan-complete,escape \
            --duration 120000 >"$log" 2>&1 || exit_status=$?

    if [[ $exit_status -ne 1 ]] ||
       ! grep -Fq 'startup game=none mode=menu' "$log" ||
       ! grep -Fq \
           'task=search root task-label="KONTROLLERAR KANDIDATER FÖR ORIGINALFILER"' \
           "$log" ||
       ! grep -Fq \
           'task=matching editions task-label="MATCHAR SPELVERSIONER"' "$log" ||
       ! grep -Fq 'scan complete elapsed-ms=' "$log" ||
       grep -Fq 'launch phase=game-handoff' "$log"; then
        echo "FAIL: Swedish no-game menu scan did not stay localized and game-free ($mode)" >&2
        cat "$log" >&2
        return 1
    fi
    echo "PASS: $mode Swedish no-game menu scan stays localized and does not launch a game"
}

run_menu_scan_probe explicit-language
run_menu_scan_probe saved-language
