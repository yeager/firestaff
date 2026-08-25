#!/usr/bin/env bash
# An operator-produced raw Saturn capture. This proves hardware state only;
# no source asset is joined or promoted to a drawing route.
set -euo pipefail
if [ "$#" -ne 1 ]; then exit 2; fi
capture="${FIRESTAFF_NEXUS_NBG1_BITMAP_CAPTURE:-/tmp/firestaff-nexus-menu-capture-20260825/nexus-menu-frames1550-1551.capture}"
if [ ! -f "$capture" ]; then
    echo "SKIP: NBG1 bitmap capture not present: $capture"
    exit 77
fi
FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$capture" \
FIRESTAFF_NEXUS_REQUIRE_NBG1_BITMAP=1 \
exec "$1"
