#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
probe=${1:?usage: test_firestaff_prodos_real_media.sh <test_firestaff_prodos_real_media>}
archive=${FIRESTAFF_DM1_IIGS_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Apple_IIGS,Software.7z"}
if [[ ! -f "$archive" ]] || ! command -v 7zz >/dev/null 2>&1; then
    echo 'SKIP: original Apple IIGS software archive or 7zz is unavailable'; exit 77
fi
[[ -x "$probe" ]] || { echo 'FAIL: ProDOS test executable is unavailable' >&2; exit 1; }
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/prodos-original.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT
archive_before=$(firestaff_sha256_file "$archive")
for version in 2.0 2.1; do
    member="Dungeon Master for Apple IIGS $version (English) Original (Not working).po"
    # Development-only archive unpacking; no runtime extractor or game cache.
    7zz x -so "$archive" "$member" >"$scratch/original-$version.po"
    case "$version" in
        2.0) expected=0579ef4e8bada108e85f022a2f2cf1f2e273a7b1b447b6dcc3de316c99dfd5bd ;;
        2.1) expected=823ee053e95bbf093af51fb88eed534467103914bb5a0c4bd889d87cd9017087 ;;
    esac
    [[ $(firestaff_sha256_file "$scratch/original-$version.po") == "$expected" ]] || {
        echo "FAIL: unchanged original IIGS $version disk SHA256 identity" >&2; exit 1;
    }
    (
        cd "$scratch"
        7zz a -tzip -mx=0 "disk-$version.zip" "original-$version.po" >/dev/null
        7zz a -tzip -mx=0 "outer-$version.zip" "disk-$version.zip" >/dev/null
    )
done
# Remove the last physical block from a private original copy. Game bytes are
# never synthesized; the declared native volume no longer fits its transport.
dd if="$scratch/original-2.0.po" of="$scratch/truncated.po" bs=512 count=1599 2>/dev/null
(cd "$scratch" && 7zz a -tzip -mx=0 malformed.zip truncated.po >/dev/null)
cp "$scratch/disk-2.0.zip" "$scratch/zip-alias.Po"
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
"$probe" "$scratch/original-2.0.po" "$scratch/original-2.1.po" \
    "$scratch/disk-2.0.zip" "$scratch/disk-2.1.zip" \
    "$scratch/outer-2.0.zip" "$scratch/outer-2.1.zip" \
    "$scratch/truncated.po" "$scratch/malformed.zip" "$scratch/zip-alias.Po" "$archive"
[[ $(firestaff_sha256_file "$archive") == "$archive_before" ]] || {
    echo 'FAIL: source IIGS archive changed' >&2; exit 1;
}
