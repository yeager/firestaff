#!/usr/bin/env bash
# Decode the exact NBG1 bitmap and CRAM spans from the authenticated local
# Saturn capture. This is a capture-only hardware witness: it deliberately
# cannot identify a retail asset or authorize production presentation.
set -euo pipefail

test_bin="${1:?missing test_nexus_v1_vdp2_capture_compositor path}"
capture="${FIRESTAFF_NEXUS_NBG1_BITMAP_CAPTURE:-}"
frame="${FIRESTAFF_NEXUS_NBG1_BITMAP_FRAME:-0}"

if [[ -z "$capture" ]]; then
    repo_root=$(cd "$(dirname "$0")/.." && pwd)
    cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
    while IFS= read -r candidate; do
        if FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$candidate" \
           FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME="$frame" \
           "$test_bin" >/dev/null 2>&1; then
            capture="$candidate"
            break
        fi
    done < <(python3 "$repo_root/scripts/list_authenticated_nexus_saturn_captures.py" --cue "$cue")
fi

if [[ -z "$capture" || ! -r "$capture" ]]; then
    echo "SKIP: authentic Nexus NBG1 bitmap capture is not staged: $capture"
    exit 77
fi

FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$capture" \
FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME="$frame" \
"$test_bin"
