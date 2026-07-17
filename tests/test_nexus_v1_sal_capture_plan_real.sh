#!/usr/bin/env bash
set -euo pipefail
root=${NEXUS_V1_SNDLEV_CORPUS_DIR:-}
assets=${NEXUS_V1_SNDLEV_SHA256_MANIFEST:-}
targets=${NEXUS_V1_SAL_CAPTURE_TARGET_LIST:-}
if [[ -z "$root" || -z "$assets" || -z "$targets" ]]; then
    echo "SKIP: set SNDLEV corpus, SHA-256 manifest, and target list"
    exit 77
fi
[[ -d "$root" && -f "$assets" && -f "$targets" ]]
[[ $(grep -Ec '^[0-9a-fA-F]{64} [ *](SNDLEV(0[0-9]|1[0-5])\.(SAL|MAP)|SDDRVS\.TSK)$' "$assets") == 33 ]]
[[ $(wc -l < "$assets" | tr -d ' ') == 33 ]]
(cd "$root" && shasum -a 256 -c "$assets")
awk 'NF == 3 && $1 >= 0 && $1 <= 15 && $2 ~ /^[0-9A-Fa-f]{1,2}$/ && !seen[$1 FS $2]++ { next } { exit 1 } END { if (NR == 0) exit 1 }' "$targets"
while read -r level selector trace; do [[ -s "$trace" ]]; grep -Fqx 'FIRESTAFF_NEXUS_SAL_DRIVER_TRACE_V1' "$trace"; done < "$targets"
echo "nexus SNDLEV/SAL external corpus: PASS"
