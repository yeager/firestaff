#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-e98a-consumer.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

require_patch_fact() {
    if ! grep -Fq "$1" "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch"; then
        printf 'FAIL: Mednafen patch omits live receipt fact: %s\n' "$1" >&2
        exit 1
    fi
}

require_patch_fact 'dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=%s record=%06x'
require_patch_fact 'dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=%02x f5_at_irq2_entry=%02x status_1802=%02x status_1803=%02x f2_before_merge=%02x f2_at_branch=%02x'
require_patch_fact 'MemPeek(0x00fc, 1, true, true)'
require_patch_fact 'MemPeek(0x00fd, 1, true, true)'
require_patch_fact 'MemPeek(0x00fe, 1, true, true)'
if grep -Fq 'FIRESTAFF_THERON_TRACE_VARIANT' \
    "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch"; then
    printf 'FAIL: Mednafen patch accepts a manually supplied trace variant\n' >&2
    exit 1
fi

missing="$tmp/missing.trace"
wrong_record="$tmp/wrong-record.trace"

printf '%s\n' 'source=mednafen-pce-instrumented' > "$missing"
if "$repo/scripts/verify_theron_post_e98a_track02_runtime_handoff_trace.sh" "$missing" >/dev/null 2>&1; then
    printf 'FAIL: incomplete trace unexpectedly reached the runtime handoff\n' >&2
    exit 1
fi

cat > "$wrong_record" <<'EOF'
source=mednafen-pce-instrumented
boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4
post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27
dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=us_bin record=0004df
dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00
EOF
if "$repo/scripts/verify_theron_post_e98a_track02_runtime_handoff_trace.sh" "$wrong_record" >/dev/null 2>&1; then
    printf 'FAIL: mismatched variant/record unexpectedly reached the runtime handoff\n' >&2
    exit 1
fi

printf 'PASS: post-e98a Track 02 runtime consumer rejects incomplete and mismatched captures; the Mednafen patch emits live receipt fields\n'
