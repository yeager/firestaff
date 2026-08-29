#!/usr/bin/env bash
# Decode the exact NBG1 bitmap and CRAM spans from the authenticated local
# Saturn capture. This is a capture-only hardware witness: it deliberately
# cannot identify a retail asset or authorize production presentation.
set -euo pipefail

test_bin="${1:?missing test_nexus_v1_vdp2_capture_compositor path}"
capture="${FIRESTAFF_NEXUS_NBG1_BITMAP_CAPTURE:-}"
frame="${FIRESTAFF_NEXUS_NBG1_BITMAP_FRAME:-0}"

if [[ -z "$capture" ]]; then
    echo "SKIP: set FIRESTAFF_NEXUS_NBG1_BITMAP_CAPTURE to an authenticated Nexus capture"
    exit 77
fi

if [[ ! -r "$capture" ]]; then
    echo "SKIP: authentic Nexus NBG1 bitmap capture is not staged: $capture"
    exit 77
fi

FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$capture" \
FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME="$frame" \
"$test_bin"
