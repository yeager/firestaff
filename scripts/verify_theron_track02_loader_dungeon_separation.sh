#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    printf 'usage: %s MEDNAFEN_TRACE\n' "$0" >&2
    exit 2
fi

trace=$1

if [ ! -s "$trace" ]; then
    printf 'FAIL: missing Mednafen trace: %s\n' "$trace" >&2
    exit 1
fi

row_count() {
    grep -c "^$1" "$trace" || true
}

field() {
    local row=$1
    local key=$2
    local item

    for item in $row; do
        case "$item" in
            "$key="*) printf '%s\n' "${item#*=}"; return 0 ;;
        esac
    done
    return 1
}

if [ "$(row_count 'source=mednafen-pce-instrumented$')" -ne 1 ] ||
   [ "$(row_count 'dynamic_cd_read_transaction ')" -ne 1 ]; then
    printf 'FAIL: expected one instrumented Mednafen source row and one dynamic CD_READ row\n' >&2
    exit 1
fi

transaction=$(grep '^dynamic_cd_read_transaction ' "$trace")
variant=$(field "$transaction" variant) || {
    printf 'FAIL: dynamic CD_READ transaction omits variant\n' >&2
    exit 1
}
record=$(field "$transaction" record) || {
    printf 'FAIL: dynamic CD_READ transaction omits record\n' >&2
    exit 1
}
destination=$(field "$transaction" destination) || {
    printf 'FAIL: dynamic CD_READ transaction omits destination\n' >&2
    exit 1
}
sector_count=$(field "$transaction" sector_count) || {
    printf 'FAIL: dynamic CD_READ transaction omits sector_count\n' >&2
    exit 1
}

if [ "$destination" != '3800' ] || [ "$sector_count" != '01' ]; then
    printf 'FAIL: dynamic CD_READ is not the observed one-sector loader transfer to $3800\n' >&2
    exit 1
fi

# 0x0b52 is the separately hash-verified JP/US initial-level envelope record.
# The dynamically observed 0x04df/0x04e0 transaction enters $3800 and must
# never be promoted into that later level record without another live read.
case "$variant:$record" in
    jp_bin:0004df|us_bin:0004e0) ;;
    *)
        printf 'FAIL: dynamic CD_READ is not the source-locked stage-two loader record\n' >&2
        exit 1
        ;;
esac

if [ "$record" = '000b52' ] || [ "$record" = '0b52' ]; then
    printf 'FAIL: loader trace was incorrectly presented as the initial-level record\n' >&2
    exit 1
fi

printf 'PASS: Mednafen stage-two %s record %s remains an executable-loader receipt; initial-level record 0x0b52 and all dungeon/object semantics remain unbound\n' \
    "$variant" "$record"
