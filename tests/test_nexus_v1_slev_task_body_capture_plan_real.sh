#!/usr/bin/env bash
set -euo pipefail
root=${NEXUS_V1_SLEV_CORPUS_DIR:-}
manifest=${NEXUS_V1_SLEV_CORPUS_SHA256_MANIFEST:-}
trace_dir=${NEXUS_V1_SLEV_TRACE_DIR:-}
if [[ -z "$root" || -z "$manifest" || -z "$trace_dir" ]]; then
    echo "SKIP: set SLEV corpus, SHA-256 manifest, and trace directory"
    exit 77
fi
[[ -d "$root" && -d "$trace_dir" && -f "$manifest" ]]
[[ $(grep -Ec '^[0-9a-fA-F]{64} [ *]SLEV(0[0-9]|1[0-5])\.BIN$' "$manifest") == 16 ]]
[[ $(wc -l < "$manifest" | tr -d ' ') == 16 ]]
(cd "$root" && shasum -a 256 -c "$manifest")
for level in $(seq -w 0 15); do
    [[ -s "$trace_dir/SLEV${level}.trace" ]]
    grep -Fqx 'FIRESTAFF_NEXUS_SLEV_SH2_TRACE_V1' "$trace_dir/SLEV${level}.trace"
done
echo "nexus SLEV00-15 task-body external inputs: PASS"
