#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"

if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 0
fi

# CSB has no original DOS release.  This must be checked before a source
# directory, archive cache, or a same-hash Atari ST GRAPHICS.DAT can select a
# runtime.  The empty root makes the test independent of locally licensed
# media and of any existing user cache.
empty_root="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-no-pc.XXXXXX")"
trap 'rm -rf "$empty_root"' EXIT HUP INT TERM

set +e
output="$(SDL_VIDEODRIVER=dummy FIRESTAFF_AUDIO_ENABLE_SDL=0 "$firestaff_cli" \
    --game csb --data-dir "$empty_root" --platform pc --boot-probe 2>&1)"
status=$?
set -e

if [ "$status" -eq 0 ]; then
    echo "FAIL: CSB --platform pc unexpectedly launched" >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

case "$output" in
    *"CSB has no original DOS/PC release"*) ;;
    *)
        echo "FAIL: CSB PC rejection did not identify the DOS platform boundary" >&2
        printf '%s\n' "$output" >&2
        exit 1
        ;;
esac

case "$output" in
    *"CSB READY:"*|*"dataDir="*"csb-st"*|*"dataDir="*"csb-pc"*)
        echo "FAIL: CSB PC rejection selected a runtime or cached foreign platform" >&2
        printf '%s\n' "$output" >&2
        exit 1
        ;;
esac

echo "PASS: CSB PC/DOS request fails closed without selecting foreign media"
