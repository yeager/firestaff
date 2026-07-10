#!/usr/bin/env bash
set -euo pipefail

changed_files() {
    git diff --name-only --diff-filter=ACMR @{u}..HEAD 2>/dev/null ||
        git diff --name-only --diff-filter=ACMR HEAD~1..HEAD 2>/dev/null ||
        true
}

changed="$(changed_files)"
if [ -z "$changed" ]; then
    echo "pre-push: no pushed file changes detected; skipping repo-local checks."
    exit 0
fi

if printf '%s\n' "$changed" | grep -q '^po/'; then
    bash po/validate_po_layout.sh
else
    echo "pre-push: skip po layout; no po/ changes."
fi

if printf '%s\n' "$changed" | grep -Eq '^(src/shared/asset_status_m12\.c|src/shared/asset_find_by_hash\.c|src/dm2/dm2_v1_.*\.c|docs/VERIFIED_HASHES\.md)$'; then
    python3 tools/asset-validate/compare_md5_to_sha256.py
else
    echo "pre-push: skip hash harmonization; no hash registry/runtime changes."
fi

if printf '%s\n' "$changed" | grep -Eiq '[.](dat|bin|iso|img|cue|zip|7z|rar|dsk|adf|st|msa|srm|sav|dgn|dmdf|bpk|prs3)$'; then
    python3 tools/asset-validate/no_game_data_in_git.py
else
    echo "pre-push: skip game-data scan; no game-data-like payload changes."
fi
