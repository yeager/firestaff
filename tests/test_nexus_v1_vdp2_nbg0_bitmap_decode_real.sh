#!/usr/bin/env bash
# Decode an authentic NBG0 title capture only. No capture means no claim:
# this test deliberately skips rather than fabricate replacement game bytes.
set -euo pipefail

test_bin="${1:?missing test_nexus_v1_saturn_runtime_capture path}"
capture="${FIRESTAFF_NEXUS_NBG0_BITMAP_CAPTURE:-}"
frame="${FIRESTAFF_NEXUS_NBG0_BITMAP_FRAME:-0}"

if [[ -z "$capture" || ! -r "$capture" ]]; then
    repo_root=$(cd "$(dirname "$0")/.." && pwd)
    cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
    while IFS= read -r candidate; do
        if FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$candidate" \
           FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME="$frame" \
           FIRESTAFF_NEXUS_REQUIRE_NBG0_BITMAP=1 \
           "$test_bin" >/dev/null 2>&1; then
            capture="$candidate"
            break
        fi
    done < <(python3 "$repo_root/scripts/list_authenticated_nexus_saturn_captures.py" --cue "$cue")
fi

if [[ -z "$capture" || ! -r "$capture" ]]; then
    echo "SKIP: authentic Nexus NBG0 title capture is not staged"
    exit 77
fi

FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$capture" \
FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME="$frame" \
FIRESTAFF_NEXUS_REQUIRE_NBG0_BITMAP=1 \
"$test_bin"
