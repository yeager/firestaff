#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
root="${FIRESTAFF_NEXUS_CAPTURE_ROOT:-$HOME/.firestaff/external/nexus-capture}"
track1="${FIRESTAFF_NEXUS_TRACK1:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan) (Track 1).bin}"
for manifest in "$root"/*/manifest.txt; do
    [[ -f "$manifest" ]] || continue
    dir=${manifest%/*}
    fifo="$dir/cdb-fifo.trace"
    source="$dir/sh2-source-writes.trace"
    [[ -s "$fifo" && -s "$source" && -s "$track1" ]] || continue
    fifo_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_CD_FIFO_WORDS_sha256=//p' "$manifest" | tail -1)
    source_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES_sha256=//p' "$manifest" | tail -1)
    [[ ${#fifo_expected} -eq 64 && ${#source_expected} -eq 64 ]] || continue
    [[ $(sha256sum "$fifo" | cut -d' ' -f1) == "$fifo_expected" ]] || continue
    [[ $(sha256sum "$source" | cut -d' ' -f1) == "$source_expected" ]] || continue
    if receipt=$(python3 "$repo_root/scripts/verify_nexus_title_mapd_cd_ram_source.py" \
            "$fifo" "$source" "$track1"); then
        echo "$receipt"
        echo "Nexus title MAPD CD-to-RAM source: PASS"
        exit 0
    fi
done
echo "SKIP: no authenticated Nexus MAPD CD-to-RAM capture"; exit 77
