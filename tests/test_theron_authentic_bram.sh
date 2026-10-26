#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source "$repo/scripts/prepare_theron_authentic_bram.sh"

md5_file() {
    if command -v md5 >/dev/null 2>&1; then
        md5 -q "$1"
    else
        md5sum "$1" | awk '{print $1}'
    fi
}

authentic_bram=${THERON_CAPTURE_AUTHENTIC_BRAM:-${HOME:-}/.firestaff/data/theron/theron-us-akutuba-complete-authentic.bram}
if [[ ! -f "$authentic_bram" ]]; then
    printf '%s\n' 'SKIP: authenticated Theron USA Backup RAM is unavailable'
    exit 0
fi

mkdir -p "$repo/build"
temporary_directory=$(mktemp -d "$repo/build/theron-authentic-bram.XXXXXX")
trap 'rm -rf "$temporary_directory"' EXIT
mkdir -p "$temporary_directory/sav"

copied_name=$(theron_prepare_authentic_bram \
    "$authentic_bram" "$temporary_directory/sav" ffabc8d19b0915d4d9632a7ae2e90a97)
if [[ "$copied_name" != firestaff-theron-authentic.sav ]] ||
   ! cmp -s "$authentic_bram" "$temporary_directory/sav/$copied_name"; then
    printf '%s\n' 'FAIL: authentic Backup RAM was not copied byte-for-byte under the Mednafen save name' >&2
    exit 1
fi

stub_arguments="$temporary_directory/mednafen-arguments"
mednafen_stub() {
    printf '%s\0' "$@" >"$stub_arguments"
}
capture_sav_args=(-filesys.fname_sav "$copied_name")
mednafen_stub "${capture_sav_args[@]}"
python3 - "$stub_arguments" <<'PY'
import sys
from pathlib import Path

arguments = Path(sys.argv[1]).read_bytes().split(b"\0")[:-1]
if arguments != [b"-filesys.fname_sav", b"firestaff-theron-authentic.sav"]:
    raise SystemExit(f"FAIL: Mednafen save arguments were incorrect: {arguments!r}")
PY

launch_marker="$temporary_directory/mednafen-launched"
if copied_name=$(theron_prepare_authentic_bram \
    "$authentic_bram" "$temporary_directory/sav" 00000000000000000000000000000000 \
    2>"$temporary_directory/invalid-digest.stderr"); then
    mednafen_stub "${capture_sav_args[@]}"
    : >"$launch_marker"
fi
if [[ -e "$launch_marker" ]] ||
   [[ -e "$temporary_directory/sav/firestaff-theron-authentic.sav" ]]; then
    printf '%s\n' 'FAIL: a 2048-byte Backup RAM with the wrong digest was accepted or left in the profile' >&2
    exit 1
fi
if ! grep -Fq 'is not the authenticated USA Backup RAM image' "$temporary_directory/invalid-digest.stderr"; then
    printf '%s\n' 'FAIL: wrong authentic Backup RAM digest did not report an authentication failure' >&2
    exit 1
fi

if copied_name=$(theron_prepare_authentic_bram \
    "$temporary_directory/missing-authentic-bram" "$temporary_directory/sav" \
    ffabc8d19b0915d4d9632a7ae2e90a97 2>"$temporary_directory/invalid.stderr"); then
    mednafen_stub "${capture_sav_args[@]}"
    : >"$launch_marker"
fi
if [[ -e "$launch_marker" ]]; then
    printf '%s\n' 'FAIL: Mednafen launched after authentic Backup RAM copying failed' >&2
    exit 1
fi
if ! grep -Fq 'could not copy THERON_CAPTURE_AUTHENTIC_BRAM' "$temporary_directory/invalid.stderr"; then
    printf '%s\n' 'FAIL: missing authentic Backup RAM did not report the copy failure' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron Backup RAM is copied, authenticated, and passed to the Mednafen stub'
