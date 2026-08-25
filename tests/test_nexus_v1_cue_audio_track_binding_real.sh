#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <test_nexus_v1_cue_audio_track_binding_real binary>" >&2
    exit 2
fi

root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
cue="$root/Dungeon Master Nexus (Japan).cue"
if [ ! -f "$cue" ]; then
    echo "SKIP: authentic Nexus Japanese CUE is unavailable" >&2
    exit 77
fi
FIRESTAFF_NEXUS_DATA_DIR="$root" exec "$1"
