#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    printf 'usage: %s TRACE {jp_bin|us_bin}\n' "$0" >&2
    exit 2
fi

trace=$1
variant=$2
if [ ! -s "$trace" ]; then
    printf 'FAIL: missing coalesced Mednafen trace: %s\n' "$trace" >&2
    exit 1
fi

case "$variant" in
    jp_bin) dynamic_record=4df; dynamic_cl=df ;;
    us_bin) dynamic_record=4e0; dynamic_cl=e0 ;;
    *) printf 'FAIL: unknown Track02 variant: %s\n' "$variant" >&2; exit 2 ;;
esac

awk -v variant="$variant" -v dynamic_record="$dynamic_record" -v dynamic_cl="$dynamic_cl" '
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
    return substr(value, 1, length(prefix)) == prefix ? substr(value, length(prefix) + 1) : ""
}
$0 == "source=mednafen-pce-instrumented-coalesced" { ++source; next }
$1 == "dynamic_cd_read_transaction" {
    ++dynamic
    dynamic_line = NR
    expected = "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=" dynamic_cl " record_dl=04 record_ch=00 variant=" variant " record=" dynamic_record
    if ($0 != expected) bad = "dynamic CD_READ receipt is not the exact authenticated variant row"
    next
}
$1 == "later_system_card_e009_dispatch" {
    ++dispatch
    dispatch_line = NR
    if (NF != 8 || field($2, "caller_pc") == "" || field($3, "return_pc") == "" || field($4, "sector_count") == "" || field($5, "record_cl") == "" || field($6, "record_dl") == "" || field($7, "record_ch") == "" || field($8, "record") == "") {
        bad = "malformed later e009 dispatch row"
        next
    }
    caller = hex(field($2, "caller_pc")); returned = hex(field($3, "return_pc")); sectors = hex(field($4, "sector_count")); cl = hex(field($5, "record_cl")); dl = hex(field($6, "record_dl")); ch = hex(field($7, "record_ch")); record = hex(field($8, "record"))
    if (caller < 0 || caller > 65535 || returned != caller + 3 || sectors < 1 || sectors > 255 || cl < 0 || cl > 255 || dl < 0 || dl > 255 || ch < 0 || ch > 255 || record < 0 || record > 16777215 || record != cl + dl * 256 + ch * 65536) bad = "invalid later e009 dispatch fields"
    next
}
$1 == "cd_interface_raw_sector_read" {
    ++sector
    sector_line = NR
    if (NF != 7 || field($2, "lba") == "" || field($3, "bytes") != "2352" || field($4, "sector_fnv1a") !~ /^[0-9a-f]{8}$/ || field($5, "span_offset") != "0" || field($6, "span_bytes") != "32" || field($7, "span_fnv1a") !~ /^[0-9a-f]{8}$/) bad = "malformed complete raw-sector row"
    next
}
$1 == "later_system_card_e009_destination_span" {
    ++destination
    destination_line = NR
    if (NF != 7 || field($2, "caller_pc") == "" || field($3, "return_pc") == "" || field($4, "record") == "" || field($5, "destination") == "" || field($6, "bytes") != "32" || field($7, "fnv1a") !~ /^[0-9a-f]{8}$/) {
        bad = "malformed later e009 destination-span row"
        next
    }
    destination_caller = hex(field($2, "caller_pc")); destination_return = hex(field($3, "return_pc")); destination_record = hex(field($4, "record")); destination_ram = hex(field($5, "destination"))
    if (destination_caller != caller || destination_return != returned || destination_record != record || destination_ram < 0 || destination_ram > 65535) bad = "later e009 destination-span row does not match dispatch"
    next
}
$1 == "later_system_card_e009_return" {
    ++returned_row
    return_line = NR
    if (NF != 4 || field($2, "caller_pc") == "" || field($3, "return_pc") == "" || field($4, "record") == "") {
        bad = "malformed later e009 return row"
        next
    }
    return_caller = hex(field($2, "caller_pc")); return_pc = hex(field($3, "return_pc")); return_record = hex(field($4, "record"))
    if (return_caller != caller || return_pc != returned || return_record != record) bad = "later e009 return row does not match dispatch"
    next
}
$1 == "later_system_card_e009_post_return_step" {
    ++post_return
    post_return_line = NR
    if (NF != 5 || field($2, "caller_pc") == "" || field($3, "return_pc") == "" || field($4, "record") == "" || field($5, "next_pc") == "") {
        bad = "malformed later e009 post-return step row"
        next
    }
    post_return_caller = hex(field($2, "caller_pc")); post_return_return = hex(field($3, "return_pc")); post_return_record = hex(field($4, "record")); post_return_next = hex(field($5, "next_pc"))
    if (post_return_caller != caller || post_return_return != returned || post_return_record != record || post_return_next < 0 || post_return_next > 65535) bad = "later e009 post-return step row does not match return"
}
END {
    if (source != 1) bad = "expected exactly one coalesced Mednafen provenance row"
    else if (dynamic != 1) bad = "expected exactly one dynamic CD_READ row"
    else if (dispatch != 1) bad = "expected exactly one later e009 dispatch row"
    else if (sector != 1) bad = "expected exactly one complete raw-sector row"
    else if (destination != 1) bad = "expected exactly one later e009 destination-span row"
    else if (returned_row != 1) bad = "expected exactly one later e009 return row"
    else if (post_return != 1) bad = "expected exactly one later e009 post-return step row"
    else if (!(dynamic_line < dispatch_line && dispatch_line < sector_line && sector_line < destination_line && destination_line < return_line && return_line < post_return_line)) bad = "loader-to-RAM control-resumption order is incomplete"
    if (bad != "") { print "FAIL: " bad > "/dev/stderr"; exit 1 }
}
' "$trace"

printf 'PASS: coalesced original Mednafen trace retains the later loader-to-local-RAM control-resumption order; no payload or dungeon semantics assigned\n'
