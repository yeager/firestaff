#!/usr/bin/env bash
set -euo pipefail
trace=${NEXUS_V1_PRS3_ORIGINAL_TRACE_EXPORT:-}; sha=${NEXUS_V1_PRS3_ORIGINAL_TRACE_SHA256:-}
if [[ -z "$trace" || -z "$sha" ]]; then echo "SKIP: set PRS3 original trace export and SHA-256"; exit 77; fi
[[ -s "$trace" && $(shasum -a 256 "$trace" | awk '{print $1}') == "$sha" ]]
head -n 1 "$trace" | grep -Fx 'NEXUS_PRS3_SH2_VDP1_TRACE_V10' >/dev/null
for field in input_read_bytes output_write_bytes output_fnv1a64 complete_output_store_range_observed vdp1_command_sequence; do grep -q "^${field}=" "$trace"; done
echo "nexus PRS3 original execution external input: PASS"
