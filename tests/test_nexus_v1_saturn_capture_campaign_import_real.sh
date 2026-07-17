#!/usr/bin/env bash
set -euo pipefail
manifest=${NEXUS_V1_SATURN_CAMPAIGN_EXPORT:-}
raw=${NEXUS_V1_SATURN_CAMPAIGN_RAW_TRACE:-}
sha=${NEXUS_V1_SATURN_CAMPAIGN_TRACE_SHA256:-}
if [[ -z "$manifest" || -z "$raw" || -z "$sha" ]]; then echo "SKIP: set campaign export, raw trace, and SHA-256"; exit 77; fi
[[ -s "$manifest" && -s "$raw" && $(shasum -a 256 "$raw" | awk '{print $1}') == "$sha" ]]
grep -Fqx 'FIRESTAFF_NEXUS_MEDNAFEN_SATURN_CAMPAIGN_V1' "$manifest"
grep -Fqx 'producer=mednafen-saturn-debugger' "$manifest"
for kind in 'prs3=' 'structure1f=' 'slev=' 'sal='; do [[ $(grep -Fc "$kind" "$manifest") == 1 ]]; done
echo "nexus Mednafen Saturn campaign external inputs: PASS"
