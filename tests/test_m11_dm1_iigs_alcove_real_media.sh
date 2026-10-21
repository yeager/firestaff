#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

probe=${1:?usage: test_m11_dm1_iigs_alcove_real_media.sh <probe>}
archive=${FIRESTAFF_DM1_IIGS_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Apple_IIGS,Software.7z"}
if [[ ! -f "$archive" ]] || ! command -v 7zz >/dev/null 2>&1; then
    echo 'SKIP: original Apple IIGS software archive or 7zz is unavailable'
    exit 77
fi
[[ -x "$probe" ]] || { echo 'FAIL: IIGS alcove probe is unavailable' >&2; exit 1; }

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/iigs-alcove-original.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT

before=$(firestaff_sha256_file "$archive")
member='Dungeon Master for Apple IIGS 2.1 (English) Original (Not working).po'
7zz x -so "$archive" "$member" >"$scratch/original-2.1.po"
[[ $(firestaff_sha256_file "$scratch/original-2.1.po") == \
    823ee053e95bbf093af51fb88eed534467103914bb5a0c4bd889d87cd9017087 ]] || {
    echo 'FAIL: original IIGS 2.1 disk identity' >&2
    exit 1
}

# Runtime has a native ZIP reader. Repackage unchanged preservation members
# only in private test scratch, matching the existing IIGS startup test path.
mkdir "$scratch/original-members"
7zz x -y "$archive" "-o$scratch/original-members" >/dev/null
(cd "$scratch/original-members" &&
    7zz a -tzip -mx=0 "$scratch/Software-original.zip" ./* >/dev/null)
FIRESTAFF_DM1_DATA_DIR="$scratch/Software-original.zip" "$probe"

[[ $(firestaff_sha256_file "$archive") == "$before" ]] || {
    echo 'FAIL: source IIGS archive changed' >&2
    exit 1
}
