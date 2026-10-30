#!/usr/bin/env python3
"""Verify the authentic Nexus TITLE.BIN copy observed in a Ymir PC trace."""
from __future__ import annotations
import argparse
import csv
import hashlib
from pathlib import Path

TITLE_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
TITLE_OFFSET = 0xB8A2
SPAN_BYTES = 921
SOURCE_ADDRESS = 0x060B5622
READ_PC = "0x6023124"
CALLER_TARGET = "0x6023108"
CALLER_RETURN = "0x6023176"
DEFAULT_TARGET_FRAME = 7716


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def ensure(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, help="Ymir capture root or dumps directory")
    parser.add_argument("--target-frame", type=int, default=DEFAULT_TARGET_FRAME,
                        help=f"VDP1 frame containing the observed copy (default: {DEFAULT_TARGET_FRAME})")
    parser.add_argument("--data-dir", required=True, type=Path,
                        help="real Nexus data directory containing TITLE.BIN")
    args = parser.parse_args()
    target_frame = args.target_frame
    ensure(target_frame >= 0, "target frame must be non-negative")
    dumps = args.capture / "dumps" if (args.capture / "dumps").is_dir() else args.capture
    title = (args.data_dir / "TITLE.BIN").read_bytes()
    ensure(digest(title) == TITLE_SHA256,
           "TITLE.BIN hash differs from authentic Japanese retail data")
    source = title[TITLE_OFFSET:TITLE_OFFSET + SPAN_BYTES]
    ensure(len(source) == SPAN_BYTES, "TITLE.BIN source span is truncated")
    wram = (dumps / "wram-hi.bin").read_bytes()
    ensure(len(wram) == 0x100000, "WorkRAMH snapshot has an unexpected size")
    offset = SOURCE_ADDRESS - 0x06000000
    ensure(wram[offset:offset + SPAN_BYTES] == source,
           "final WorkRAMH differs from retail source bytes")

    receipt = dict(line.split("=", 1) for line in
                   (dumps / "sh2-memory-read-trace-bundle.txt").read_text().splitlines()
                   if "=" in line)
    ensure(receipt["format"] == "ymir-sh2-memory-read-trace-bundle-v3",
           "unsupported SH-2 read trace receipt")
    ensure(receipt["bundle_complete"] == receipt["trace_file_complete"] == "1",
           "SH-2 read trace bundle is incomplete")
    ensure(receipt["register_snapshot_file_complete"] == "1",
           "SH-2 register snapshot is incomplete")
    ensure(receipt["instruction_pc_filter_enabled"] == "1",
           "SH-2 instruction-PC filter is not enabled")
    ensure(receipt["instruction_pc"] == READ_PC,
           "SH-2 instruction PC differs from the expected retail copy")
    ensure(receipt["frame_index_column"] == "vdp1_frame",
           "SH-2 trace is missing VDP1 frame indices")
    ensure(int(receipt["master_dropped"]) + int(receipt["slave_dropped"]) == 0,
           "SH-2 read trace dropped rows")

    with (dumps / "sh2-memory-read-trace.csv").open(newline="") as f:
        trace = list(csv.DictReader(f))
    with (dumps / "sh2-memory-read-register-snapshots.csv").open(newline="") as f:
        snapshots = list(csv.DictReader(f))
    ensure(len(trace) == int(receipt["trace_rows"]),
           "SH-2 read rows differ from the receipt count")
    ensure(len(snapshots) == int(receipt["register_snapshot_rows"]),
           "register snapshot rows differ from the receipt count")
    ensure(all(r["instruction_pc"].lower() == READ_PC for r in trace),
           "SH-2 read trace contains a row from another instruction")
    ensure(all(r["vdp1_frame"] for r in trace),
           "a PC-filtered read row lacks a VDP1 frame index")
    ensure(len(trace) == len(snapshots),
           "SH-2 read and register snapshot counts are misaligned")

    # The matching read pass is one contiguous 921-byte operation in the selected frame.
    selected = [i for i, r in enumerate(trace)
                if r["sh2"] == "master" and r["vdp1_frame"] == str(target_frame)
                and int(r["address"], 16) >= SOURCE_ADDRESS
                and int(r["address"], 16) < SOURCE_ADDRESS + SPAN_BYTES]
    ensure(len(selected) == SPAN_BYTES,
           f"expected exactly 921 source-span read rows in frame {target_frame}")
    indices = list(range(selected[0], selected[0] + SPAN_BYTES))
    ensure(selected == indices,
           "source span rows are not contiguous in the global trace")
    rows = [trace[i] for i in selected]
    regs = [snapshots[i] for i in selected]
    destination_base = int(regs[0]["r4"], 16)
    destination_addresses = []
    for n, (r, s) in enumerate(zip(rows, regs)):
        ensure(int(r["address"], 16) == SOURCE_ADDRESS + n,
               "source reads are not in exact byte order")
        ensure(r["width"] == "1", "source read is not byte-width")
        ensure(r["caller_target"].lower() == CALLER_TARGET,
               "source read caller target differs from the retail copy helper")
        ensure(r["caller_return_address"].lower() == CALLER_RETURN,
               "source read caller return differs from the retail copy helper")
        ensure(s["vdp1_frame"] == str(target_frame),
               "register snapshot frame differs from source-read frame")
        ensure(s["pc"].lower() == READ_PC,
               "register snapshot PC differs from source-read PC")
        ensure(int(s["r5"], 16) == SOURCE_ADDRESS + n,
               "R5 does not match the source byte address")
        destination_addresses.append(int(s["r4"], 16))
    ensure(len(set(destination_addresses)) == SPAN_BYTES,
           "R4 destination snapshots are not unique")
    destination_deltas = [b - a for a, b in zip(destination_addresses, destination_addresses[1:])]
    ensure(destination_deltas.count(0xD1) == 4,
           "R4 destination row-stride transitions differ from the observed layout")
    ensure(all(delta in (1, 0xD1) for delta in destination_deltas),
           "R4 destination sequence contains an unexpected stride")
    vdp2_vram = (dumps / "vdp2-vram.bin").read_bytes()
    ensure(len(vdp2_vram) == 0x80000, "VDP2 VRAM snapshot has an unexpected size")
    vdp2_offsets = [address - 0x25E00000 for address in destination_addresses]
    ensure(all(0 <= address < len(vdp2_vram) for address in vdp2_offsets),
           "R4 maps outside the captured VDP2 VRAM address range")
    mapped_vram_bytes = bytes(vdp2_vram[address] for address in vdp2_offsets)
    ensure(mapped_vram_bytes == source,
           "final VDP2 VRAM bytes differ from TITLE.BIN source span")

    vdp2_receipt = dict(line.split("=", 1) for line in
                        (dumps / "vdp2-memory-write-bundle.txt").read_text().splitlines()
                        if "=" in line)
    ensure(vdp2_receipt["format"] == "ymir-vdp2-memory-write-bundle-v1",
           "unsupported VDP2 write trace receipt")
    ensure(vdp2_receipt["bundle_complete"] == "1",
           "VDP2 memory-write bundle is incomplete")
    ensure(vdp2_receipt["trace_file_complete"] == "1",
           "VDP2 write trace file is incomplete")
    ensure(vdp2_receipt["vdp2_vram_file_complete"] == "1",
           "final VDP2 VRAM snapshot is incomplete")
    ensure(int(vdp2_receipt["trace_dropped_total"]) == 0,
           "VDP2 write trace dropped rows")
    ensure(vdp2_receipt["trace_vdp1_frame_range_enabled"] == "1",
           "VDP2 write trace has no frame-range filter")
    ensure(int(vdp2_receipt["trace_vdp1_frame_begin"]) <= target_frame,
           "VDP2 write trace begins after the target frame")
    ensure(target_frame <= int(vdp2_receipt["trace_vdp1_frame_end_exclusive"]) - 1,
           "VDP2 write trace ends before the target frame")
    with (dumps / "vdp2-memory-write-trace.csv").open(newline="") as f:
        vdp2_rows = list(csv.DictReader(f))
    ensure(len(vdp2_rows) == int(vdp2_receipt["trace_rows"]),
           "VDP2 write rows differ from the receipt count")
    destination_offset_set = set(vdp2_offsets)
    frame_write_rows = [r for r in vdp2_rows
                        if r["vdp1_frame"] == str(target_frame) and r["memory"] == "VRAM"
                        and int(r["address"], 16) in destination_offset_set]
    ensure(len(frame_write_rows) == SPAN_BYTES,
           "same-frame VDP2 write coverage has missing or duplicate rows")
    frame_writes = {int(r["address"], 16): r for r in frame_write_rows}
    ensure(len(frame_writes) == SPAN_BYTES,
           "same-frame VDP2 write addresses are not unique")
    for n, address in enumerate(vdp2_offsets):
        row = frame_writes[address]
        ensure(row["width"] == "1", "same-frame VDP2 write is not byte-width")
        ensure(int(row["value"], 16) == source[n],
               "same-frame VRAM write value differs from TITLE.BIN")

    print(f"title_bin_sha256={digest(title)}")
    print(f"title_bin_span_offset=0x{TITLE_OFFSET:x}")
    print(f"title_bin_span_bytes={SPAN_BYTES}")
    print(f"title_bin_span_sha256={digest(source)}")
    print(f"source_address=0x{SOURCE_ADDRESS:08x}")
    print(f"destination_address=0x{destination_base:08x}")
    print(f"capture_frame={target_frame}")
    print(f"ordered_source_read_rows={len(rows)}")
    print("r5_matches_each_source_byte=1")
    print(f"r4_destination_snapshot_min=0x{min(destination_addresses):08x}")
    print(f"r4_destination_snapshot_max=0x{max(destination_addresses):08x}")
    print(f"r4_snapshot_rows_unique={len(set(destination_addresses))}")
    print("r4_stride_plus_0xd1_transitions=4")
    print("note=r4 values are register snapshots; no store-event trace was enabled")
    print(f"mapped_vdp2_vram_offset_min=0x{min(vdp2_offsets):x}")
    print(f"mapped_vdp2_vram_offset_max=0x{max(vdp2_offsets):x}")
    print(f"final_vdp2_vram_bytes_match_source={len(mapped_vram_bytes)}/{SPAN_BYTES}")
    print(f"final_vdp2_vram_mapped_span_sha256={digest(mapped_vram_bytes)}")
    print(f"same_frame_vdp2_write_rows={len(frame_writes)}")
    print(f"same_frame_vdp2_write_frame={target_frame}")
    print("same_frame_vdp2_write_values_match_source=1")
    print("same_frame_vdp2_write_dropped_rows=0")
    print(f"all_pc_filtered_rows={len(trace)}")
    print(f"unique_vdp1_frames={','.join(sorted({r['vdp1_frame'] for r in trace}, key=int))}")
    print("dropped_rows=0")
    print("final_workramh_matches_retail_span=1")
    print("read_values_at_access_time=not captured")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
