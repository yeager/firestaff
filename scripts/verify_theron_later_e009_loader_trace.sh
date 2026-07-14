#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    printf 'usage: %s TRACE {jp_bin|us_bin}\n' "$0" >&2
    exit 2
fi

trace=$1
variant=$2
if [ ! -s "$trace" ]; then
    printf 'FAIL: missing Mednafen trace: %s\n' "$trace" >&2
    exit 1
fi

case "$variant" in
    jp_bin) dynamic_record=4df ;;
    us_bin) dynamic_record=4e0 ;;
    *) printf 'FAIL: unknown Track02 variant: %s\n' "$variant" >&2; exit 2 ;;
esac

awk -v variant="$variant" -v dynamic_record="$dynamic_record" '
function hex(s,    i, c, digit, value) {
    if (s !~ /^[0-9a-f]+$/) return -1
    value = 0
    for (i = 1; i <= length(s); ++i) {
        c = substr(s, i, 1)
        digit = index("0123456789abcdef", c) - 1
        if (digit < 0) return -1
        value = value * 16 + digit
    }
    return value
}
function field(value, key,    prefix) {
    prefix = key "="
    if (substr(value, 1, length(prefix)) == prefix)
        return substr(value, length(prefix) + 1)
    return ""
}
$0 == "source=mednafen-pce-instrumented" { ++source; next }
$1 == "dynamic_cd_read_transaction" {
    ++dynamic
    expected = "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=" \
        (variant == "jp_bin" ? "df record_dl=04" : "e0 record_dl=04") \
        " record_ch=00 variant=" variant " record=" dynamic_record
    if ($0 != expected) bad = "dynamic CD_READ receipt is not the exact authenticated variant row"
    next
}
$1 == "later_system_card_e009_dispatch" {
    ++dispatch
    if (NF != 8 || field($2, "caller_pc") == "" ||
        field($3, "return_pc") == "" || field($4, "sector_count") == "" ||
        field($5, "record_cl") == "" || field($6, "record_dl") == "" ||
        field($7, "record_ch") == "" || field($8, "record") == "") {
        bad = "malformed later e009 dispatch row"
        next
    }
    caller = hex(field($2, "caller_pc")); returned = hex(field($3, "return_pc"))
    sectors = hex(field($4, "sector_count")); cl = hex(field($5, "record_cl"))
    dl = hex(field($6, "record_dl")); ch = hex(field($7, "record_ch"))
    record = hex(field($8, "record"))
    if (caller < 0 || caller > 65535 || returned != caller + 3 || sectors < 1 ||
        cl < 0 || cl > 255 || dl < 0 || dl > 255 || ch < 0 || ch > 255 ||
        record < 0 || record > 16777215 || record != cl + dl * 256 + ch * 65536) {
        bad = "invalid later e009 dispatch fields"
    }
    next
}
$1 == "later_system_card_e009_return" {
    ++returned_row
    if (NF != 4 || field($2, "caller_pc") == "" ||
        field($3, "return_pc") == "" || field($4, "record") == "") {
        bad = "malformed later e009 return row"
        next
    }
    return_caller = hex(field($2, "caller_pc")); return_pc = hex(field($3, "return_pc"))
    return_record = hex(field($4, "record"))
    if (return_caller < 0 || return_pc < 0 || return_record < 0 ||
        return_caller != caller || return_pc != returned || return_record != record) {
        bad = "later e009 return row does not match dispatch"
    }
}
END {
    if (source != 1) bad = "expected exactly one Mednafen provenance row"
    else if (dynamic != 1) bad = "expected exactly one dynamic CD_READ row"
    else if (dispatch != 1) bad = "expected exactly one later e009 dispatch row"
    else if (returned_row != 1) bad = "expected exactly one later e009 return row"
    if (bad != "") { print "FAIL: " bad > "/dev/stderr"; exit 1 }
}
' "$trace"

printf 'PASS: original Mednafen trace retains one complete later e009 loader envelope; no payload or dungeon semantics assigned\n'
