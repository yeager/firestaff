#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    printf 'usage: %s TRACE\n' "$0" >&2
    exit 2
fi

trace=$1
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if [ ! -s "$trace" ]; then
    printf 'FAIL: missing Mednafen trace: %s\n' "$trace" >&2
    exit 1
fi

# The E98A gate establishes that this is live controller flow.  The runtime
# handoff is only eligible when the same capture later supplies the complete
# dynamic $4090 CD_READ transaction and IRQ2 state envelope.
"$repo/scripts/verify_theron_post_e98a_controller_transfer_trace.sh" "$trace" >/dev/null

transaction_count=$(grep -c '^dynamic_cd_read_transaction ' "$trace" || true)
state_count=$(grep -c '^dynamic_cd_read_controller_state ' "$trace" || true)
if [ "$transaction_count" -ne 1 ] || [ "$state_count" -ne 1 ]; then
    printf 'FAIL: expected exactly one captured dynamic CD_READ transaction and controller-state receipt\n' >&2
    exit 1
fi

transaction=$(grep '^dynamic_cd_read_transaction ' "$trace")
state=$(grep '^dynamic_cd_read_controller_state ' "$trace")

value() {
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

require_transaction() {
    local key=$1
    local expected=$2
    local actual

    actual=$(value "$transaction" "$key") || {
        printf 'FAIL: dynamic CD_READ transaction omits %s\n' "$key" >&2
        exit 1
    }
    if [ "$actual" != "$expected" ]; then
        printf 'FAIL: dynamic CD_READ %s is %s, expected %s\n' "$key" "$actual" "$expected" >&2
        exit 1
    fi
}

require_state_hex_byte() {
    local key=$1
    local actual

    actual=$(value "$state" "$key") || {
        printf 'FAIL: dynamic controller state omits %s\n' "$key" >&2
        exit 1
    }
    case "$actual" in
        [0-9a-f][0-9a-f]) printf '%d\n' "0x$actual" ;;
        *) printf 'FAIL: dynamic controller state %s is not a lowercase byte\n' "$key" >&2; exit 1 ;;
    esac
}

require_transaction pc 4090
require_transaction return_pc 4093
require_transaction sector_count 01
require_transaction destination 3800
require_transaction record_register_mask 07

variant=$(value "$transaction" variant) || {
    printf 'FAIL: dynamic CD_READ transaction omits variant\n' >&2
    exit 1
}
record=$(value "$transaction" record) || {
    printf 'FAIL: dynamic CD_READ transaction omits captured record\n' >&2
    exit 1
}
case "$variant:$record" in
    jp_bin:0004df|us_bin:0004e0) ;;
    *)
        printf 'FAIL: captured record is not the source-locked Track 02 transaction for its variant\n' >&2
        exit 1
        ;;
esac

state_pc=$(value "$state" pc) || {
    printf 'FAIL: dynamic controller state omits pc\n' >&2
    exit 1
}
if [ "$state_pc" != e74c ]; then
    printf 'FAIL: dynamic controller state is not the original IRQ2 branch at e74c\n' >&2
    exit 1
fi

f5_after=$(require_state_hex_byte f5_after_cd_read)
f5_entry=$(require_state_hex_byte f5_at_irq2_entry)
status_1802=$(require_state_hex_byte status_1802)
status_1803=$(require_state_hex_byte status_1803)
f2_before=$(require_state_hex_byte f2_before_merge)
f2_branch=$(require_state_hex_byte f2_at_branch)

if [ "$f5_after" -ne "$f5_entry" ] ||
   [ "$f2_branch" -ne $(( (status_1802 & status_1803) | f2_before )) ]; then
    printf 'FAIL: captured controller state does not satisfy the original IRQ2 handoff\n' >&2
    exit 1
fi

printf 'PASS: live post-e98a transfer and captured dynamic %s CD_READ record %s bind to the existing Track 02 runtime handoff; manifest semantics remain unbound\n' \
    "$variant" "$record"
