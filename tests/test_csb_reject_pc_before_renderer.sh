#!/bin/sh
set -eu

app=${1:?usage: test_csb_reject_pc_before_renderer.sh <firestaff-binary>}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

set +e
SDL_VIDEODRIVER=firestaff-invalid-driver "$app" \
    --game csb --platform pc --data-dir "$tmpdir" \
    >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

if [ "$status" -ne 1 ]; then
    echo "FAIL: expected the CLI phase-error exit status 1, got $status" >&2
    cat "$tmpdir/stderr" >&2
    exit 1
fi
if ! grep -Fqx \
    'firestaff: CSB has no original DOS/PC release; use --platform amiga, atari-st, or fm-towns' \
    "$tmpdir/stderr"; then
    echo "FAIL: expected early CSB platform diagnostic" >&2
    cat "$tmpdir/stderr" >&2
    exit 1
fi
if grep -Eiq 'SDL_Init|renderer|scan(ning)? (game )?data' "$tmpdir/stderr"; then
    echo "FAIL: renderer or scanner ran before rejecting the invalid platform" >&2
    cat "$tmpdir/stderr" >&2
    exit 1
fi

echo "PASS: CSB PC is rejected before renderer initialization and media scanning"
