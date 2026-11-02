#!/bin/sh
set -eu

app=${1:?usage: test_csb_reject_pc_before_renderer.sh <firestaff-binary>}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

check_rejected_platform() {
    game=$1
    platform=$2
    expected_status=$3
    expected=$4

    set +e
    SDL_VIDEODRIVER=firestaff-invalid-driver "$app" \
        --game "$game" --platform "$platform" --data-dir "$tmpdir" \
        >"$tmpdir/stdout" 2>"$tmpdir/stderr"
    status=$?
    set -e

    if [ "$status" -ne "$expected_status" ]; then
        echo "FAIL: $game/$platform expected CLI phase-error exit status $expected_status, got $status" >&2
        cat "$tmpdir/stderr" >&2
        exit 1
    fi
    if ! grep -Fqx "$expected" "$tmpdir/stderr"; then
        echo "FAIL: $game/$platform did not emit the early platform diagnostic" >&2
        cat "$tmpdir/stderr" >&2
        exit 1
    fi
    if grep -Eiq 'SDL_Init|renderer|scan(ning)? (game )?data' "$tmpdir/stderr"; then
        echo "FAIL: $game/$platform initialized the renderer or scanner before rejection" >&2
        cat "$tmpdir/stderr" >&2
        exit 1
    fi
}

check_rejected_platform csb pc \
    1 \
    'firestaff: CSB has no original DOS/PC release; use --platform amiga, atari-st, or fm-towns'
check_rejected_platform dm2 atari-st \
    2 \
    "firestaff: game 'dm2' has no launchable version for --platform Atari ST"
check_rejected_platform dm1 pc-98 \
    2 \
    'firestaff: --platform must be auto, pc (dos), amiga, atari-st (atari, st), fm-towns (fmtowns, fm_towns), mac, pce (pc-engine), saturn, or apple-iigs (iigs)'
check_rejected_platform dm1 pc98 \
    2 \
    'firestaff: --platform must be auto, pc (dos), amiga, atari-st (atari, st), fm-towns (fmtowns, fm_towns), mac, pce (pc-engine), saturn, or apple-iigs (iigs)'

echo "PASS: unsupported CSB PC, DM2 Atari ST, and DM1 PC-98 requests fail before renderer initialization and media scanning"
