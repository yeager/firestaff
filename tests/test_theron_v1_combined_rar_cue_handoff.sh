#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <handoff-test-binary>\n' "$0" >&2
    exit 2
fi

test_binary=$1
archive="$HOME/.firestaff/data/theron/Theron's Quest for PC-Engine (US and Japanese versions).rar"
unrar=$(command -v unrar || true)
if [[ ! -f "$archive" || -z "$unrar" ]]; then
    printf 'SKIP: authentic combined Theron RAR or unrar is unavailable\n'
    exit 77
fi
if [[ ! -x "$test_binary" ]]; then
    printf 'FAIL: Theron handoff test binary is unavailable: %s\n' "$test_binary" >&2
    exit 1
fi

temporary_root=$(mktemp -d "$(dirname "$test_binary")/firestaff-theron-rar-cue.XXXXXX")
trap 'rm -rf "$temporary_root"' EXIT
if ! "$unrar" x -inul "$archive" TQUS.cue TQUS01.ogg TQUS19.iso \
        TQUS02End.iso "$temporary_root/"; then
    printf 'FAIL: authentic combined RAR could not extract the US CUE members\n' >&2
    exit 1
fi
for member in TQUS.cue TQUS01.ogg TQUS19.iso TQUS02End.iso; do
    if [[ ! -s "$temporary_root/$member" ]]; then
        printf 'FAIL: authentic combined RAR is missing %s\n' "$member" >&2
        exit 1
    fi
done

SDL_AUDIODRIVER=dummy FIRESTAFF_THERON_CUE="$temporary_root/TQUS.cue" \
    "$test_binary"
