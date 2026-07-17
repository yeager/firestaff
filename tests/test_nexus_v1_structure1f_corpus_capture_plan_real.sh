#!/usr/bin/env bash
set -euo pipefail

root=${NEXUS_V1_DGN_CORPUS_DIR:-}
manifest=${NEXUS_V1_DGN_CORPUS_SHA256_MANIFEST:-}
if [[ -z "$root" || -z "$manifest" ]]; then
    echo "SKIP: set NEXUS_V1_DGN_CORPUS_DIR and NEXUS_V1_DGN_CORPUS_SHA256_MANIFEST"
    exit 77
fi
[[ -d "$root" && -f "$manifest" ]]
[[ $(grep -Ec '^[0-9a-fA-F]{64} [ *]LEV(0[0-9]|1[0-5])\.DGN$' "$manifest") == 16 ]]
[[ $(wc -l < "$manifest" | tr -d ' ') == 16 ]]
(cd "$root" && shasum -a 256 -c "$manifest")
echo "nexus structure1f LEV00-15 external corpus: PASS"
