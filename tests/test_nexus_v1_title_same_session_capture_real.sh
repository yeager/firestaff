#!/usr/bin/env bash
# Require one hash-bound session containing authentic TITLE.BIN CD->RAM,
# Work-RAM instruction reads, the matching VDP2 writes/register route, and
# complete VDP1/VDP2 hardware state. Asset semantics remain fail-closed.
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
root="${FIRESTAFF_NEXUS_CAPTURE_ROOT:-$HOME/.firestaff/external/nexus-capture}"
track1="${FIRESTAFF_NEXUS_TRACK1:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan) (Track 1).bin}"
for manifest in "$root"/*/manifest.txt; do
    [[ -f "$manifest" ]] || continue
    dir=${manifest%/*}; raw="$dir/runtime-vdp12.raw"
    fifo="$dir/cdb-fifo.trace"; ram="$dir/sh2-source-writes.trace"
    instruction="$dir/sh2-instruction-byte.trace"
    writes="$dir/vdp2-writes.trace"
    registers="$dir/vdp2-writer-registers.trace"
    [[ -s "$raw" && -s "$fifo" && -s "$ram" && -s "$instruction" &&
       -s "$writes" && -s "$registers" ]] || continue
    raw_expected=$(sed -n 's/^raw_sha256=//p' "$manifest" | tail -1)
    fifo_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_CD_FIFO_WORDS_sha256=//p' "$manifest" | tail -1)
    ram_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES_sha256=//p' "$manifest" | tail -1)
    instruction_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_SH2_INSTRUCTION_BYTE_READS_sha256=//p' "$manifest" | tail -1)
    writes_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_VDP2_WRITES_sha256=//p' "$manifest" | tail -1)
    registers_expected=$(sed -n 's/^FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS_sha256=//p' "$manifest" | tail -1)
    [[ ${#raw_expected} -eq 64 && ${#fifo_expected} -eq 64 &&
       ${#ram_expected} -eq 64 && ${#instruction_expected} -eq 64 &&
       ${#writes_expected} -eq 64 && ${#registers_expected} -eq 64 ]] || continue
    [[ $(sha256sum "$raw" | cut -d' ' -f1) == "$raw_expected" ]] || continue
    [[ $(sha256sum "$fifo" | cut -d' ' -f1) == "$fifo_expected" ]] || continue
    [[ $(sha256sum "$ram" | cut -d' ' -f1) == "$ram_expected" ]] || continue
    [[ $(sha256sum "$instruction" | cut -d' ' -f1) == "$instruction_expected" ]] || continue
    [[ $(sha256sum "$writes" | cut -d' ' -f1) == "$writes_expected" ]] || continue
    [[ $(sha256sum "$registers" | cut -d' ' -f1) == "$registers_expected" ]] || continue
    python3 "$repo_root/scripts/verify_nexus_title_cdb_fifo_words.py" "$fifo" "$track1" >/dev/null || continue
    python3 "$repo_root/scripts/verify_nexus_title_nbg0_ram_source.py" "$ram" >/dev/null || continue
    python3 "$repo_root/scripts/verify_nexus_title_nbg0_instruction_byte_source.py" \
        "$instruction" "$writes" >/dev/null || continue
    python3 "$repo_root/scripts/verify_nexus_title_nbg0_copy_routing.py" \
        "$registers" >/dev/null || continue
    python3 "$repo_root/scripts/verify_nexus_title_nbg0_copy_values.py" \
        "$registers" "$writes" >/dev/null || continue
    python3 "$repo_root/scripts/verify_nexus_title_nbg0_copy_callchain.py" \
        --cue "${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}" \
        --writer-registers "$registers" >/dev/null || continue
    python3 "$repo_root/scripts/analyze_nexus_saturn_hardware_capture.py" "$raw" "$manifest" >/dev/null || continue
    echo "title_same_session_disc_cd_fifo_ram_instruction_vdp12=verified"
    echo "title_work_ram_to_vdp2_values=verified"
    echo "title_mapd_asset_transform_and_display_consumer=unbound"
    echo "Nexus title same-session capture boundary: PASS"
    exit 0
done
echo "SKIP: no complete hash-bound same-session Nexus title capture"; exit 77
