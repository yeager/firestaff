#!/usr/bin/env bash
set -euo pipefail
path=${NEXUS_V1_SATURN_SAVE_IMAGE:-}
sha=${NEXUS_V1_SATURN_SAVE_SHA256:-}
if [[ -z "$path" || -z "$sha" ]]; then echo "SKIP: set NEXUS_V1_SATURN_SAVE_IMAGE and SHA256"; exit 77; fi
[[ -f "$path" && $(wc -c < "$path" | tr -d ' ') == 8192 ]]
[[ $(shasum -a 256 "$path" | awk '{print $1}') == "$sha" ]]
echo "nexus saturn save capture real input: PASS"
