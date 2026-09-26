#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <quest-item-names-test-binary>\n' "$0" >&2
    exit 2
fi

test_binary=$1
cue=${FIRESTAFF_THERON_US_CLONECD_RAW_CUE:-"$HOME/.firestaff/data/theron/raw-us-clonecd/Dungeon Master - Theron's Quest (USA).cue"}
expected_cue_md5=46bebca37c7c1a18375e6e1ca32c3090
expected_track02_md5=168bd6a63784e91885df8c47be62ab5a
track02_name="Dungeon Master - Theron's Quest (USA).bin"
test_root=${FIRESTAFF_TEST_TEMP_DIR:-"$(dirname "$test_binary")"}

if [[ ! -x "$test_binary" ]]; then
    printf 'FAIL: quest-item-names test binary is unavailable: %s\n' "$test_binary" >&2
    exit 1
fi
if [[ ! -f "$cue" ]]; then
    printf 'SKIP: authentic US CloneCD CUE is not staged\n'
    exit 77
fi

md5_file() {
    if command -v md5sum >/dev/null 2>&1; then
        md5sum "$1" | awk '{print $1}'
    elif command -v md5 >/dev/null 2>&1; then
        md5 -q "$1"
    else
        printf 'FAIL: neither md5sum nor md5 is available\n' >&2
        return 1
    fi
}

actual_cue_md5=$(md5_file "$cue")
if [[ "$actual_cue_md5" != "$expected_cue_md5" ]]; then
    printf 'FAIL: US CloneCD CUE identity mismatch: %s\n' "$actual_cue_md5" >&2
    exit 1
fi

cue_dir=$(dirname "$cue")
bin="$cue_dir/$track02_name"
if [[ ! -f "$bin" ]]; then
    printf 'SKIP: authentic US CloneCD BIN paired with the CUE is not staged\n'
    exit 77
fi
if [[ ! -d "$test_root" ]]; then
    printf 'FAIL: test temporary directory is unavailable: %s\n' "$test_root" >&2
    exit 1
fi

tmpdir=$(mktemp -d "$test_root/theron-clonecd-quest-names.XXXXXX")
cleanup() {
    rm -f "$tmpdir/TQUS02-clonecd.bin"
    rmdir "$tmpdir" 2>/dev/null || true
}
trap cleanup EXIT

# The authenticated CUE puts Track 02 at sector 3234 and Track 03 at 6605.
# Copy only this bounded MODE1/2352 span into a disposable test directory.
dd if="$bin" of="$tmpdir/TQUS02-clonecd.bin" \
    bs=2352 skip=3234 count=3371 2>/dev/null
actual_track02_md5=$(md5_file "$tmpdir/TQUS02-clonecd.bin")
if [[ "$actual_track02_md5" != "$expected_track02_md5" ]]; then
    printf 'FAIL: bounded authentic Track 02 identity mismatch: %s\n' \
        "$actual_track02_md5" >&2
    exit 1
fi

FIRESTAFF_THERON_TRACK02_CLONECD_RAW="$tmpdir/TQUS02-clonecd.bin" \
    "$test_binary"
