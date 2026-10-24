#!/usr/bin/env sh
set -eu

app=${1:?usage: test_fmtowns_edition_option_validation.sh <firestaff>}

expect_rejected() {
    expected=$1
    shift
    if output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        "$@" --data-dir unused --duration 0 2>&1); then
        echo "FAIL: invalid edition option was accepted: $*" >&2
        exit 1
    fi
    if [ "$output" != "$expected" ]; then
        printf 'FAIL: unexpected diagnostic for %s\nexpected: %s\nactual: %s\n' \
            "$*" "$expected" "$output" >&2
        exit 1
    fi
}

expect_rejected \
    'firestaff: --dm1-fmtowns-ja requires --game dm1 and --platform fm-towns' \
    --dm1-fmtowns-ja
expect_rejected \
    'firestaff: --dm1-fmtowns-ja requires --game dm1 and --platform fm-towns' \
    --game csb --dm1-fmtowns-ja
expect_rejected \
    'firestaff: --dm1-fmtowns-ja requires --game dm1 and --platform fm-towns' \
    --game dm1 --dm1-fmtowns-ja --platform amiga
expect_rejected \
    'firestaff: --csb-fmtowns-ja requires --game csb and --platform fm-towns' \
    --csb-fmtowns-ja
expect_rejected \
    'firestaff: --csb-fmtowns-ja requires --game csb and --platform fm-towns' \
    --game dm1 --csb-fmtowns-ja
expect_rejected \
    'firestaff: --csb-fmtowns-ja requires --game csb and --platform fm-towns' \
    --game csb --csb-fmtowns-ja --platform auto

echo 'PASS: mismatched FM Towns edition options are rejected before media scanning'
