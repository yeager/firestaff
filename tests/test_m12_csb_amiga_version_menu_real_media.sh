#!/usr/bin/env bash
set -euo pipefail

test_binary=${1:?usage: test_m12_csb_amiga_version_menu_real_media.sh <test-binary> <data-root>}
data_root=${2:?usage: test_m12_csb_amiga_version_menu_real_media.sh <test-binary> <data-root>}
csb_data="$data_root/csb"

if [[ ! -x "$test_binary" || ! -d "$csb_data" ]]; then
    echo 'SKIP: authentic CSB data directory is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-csb-amiga-version.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM

FIRESTAFF_CONFIG_PATH="$scratch/config.toml" \
HOME="$scratch" USERPROFILE="$scratch" XDG_CONFIG_HOME="$scratch" \
APPDATA="$scratch" FIRESTAFF_DATA="$data_root" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$test_binary" "$csb_data"
