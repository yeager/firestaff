#!/usr/bin/env python3
"""Verify an authentic Japanese TITLE.BIN span resident in the title NBG0 bitmap.

This verifies a byte/storage join for one retained Ymir menu capture. It does
not establish same-run SH-2 source reads, visible pixel ownership, or runtime
renderer admission.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_title_vdp2_source import read_title_assets

TITLE_BIN_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
TITLE_BIN_SOURCE_OFFSET = 0x2526
SOURCE_BYTES = 31616
WRAM_SOURCE_OFFSET = 0x0AC2A6
VDP2_DESTINATION_OFFSET = 0x1008
ROW_BYTES = 304
ROW_COUNT = 104
ROW_STRIDE = 512
COPY_FRAME = 7716
SNAPSHOT_FRAME = 8047
TAIL_ROW = 96
NBG0_BYTES = ROW_COUNT * ROW_BYTES


def receipt(path: Path, expected_format: str | None = None) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in path.read_text(encoding="ascii").splitlines():
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key in values:
            raise ValueError(f"duplicate {key} in {path.name}")
        values[key] = value
    if expected_format is not None and values.get("format") != expected_format:
        raise ValueError(f"unsupported {path.name} format")
    return values


def read_vdp2_trace(path: Path) -> list[tuple[int, str, int, int, int]]:
    rows: list[tuple[int, str, int, int, int]] = []
    with path.open("r", encoding="ascii", newline="") as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames != ["vdp1_frame", "memory", "address", "width", "value"]:
            raise ValueError("unexpected VDP2 write trace columns")
        for row in reader:
            rows.append((int(row["vdp1_frame"], 10), row["memory"],
                         int(row["address"], 16) & 0x7FFFF,
                         int(row["width"], 10), int(row["value"], 16)))
    return rows


def traced_byte(value: int) -> int:
    """Return Ymir's byte-width VDP2MemoryTracer value without bus-lane shifts."""
    return value & 0xFF


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path,
                        help="Ymir dump directory containing VDP2/WRAM receipts")
    asset_source = parser.add_mutually_exclusive_group(required=True)
    asset_source.add_argument("--data-dir", type=Path,
                              help="directory containing authenticated Nexus title assets")
    asset_source.add_argument("--cue", type=Path,
                              help="authentic Nexus CUE; members are read in memory")
    args = parser.parse_args()

    try:
        title_bin = read_title_assets(args.data_dir, args.cue)[0]
        title_hash = hashlib.sha256(title_bin).hexdigest()
        if title_hash != TITLE_BIN_SHA256:
            raise ValueError("TITLE.BIN is not the authenticated Japanese retail file")
        source = title_bin[TITLE_BIN_SOURCE_OFFSET:TITLE_BIN_SOURCE_OFFSET + SOURCE_BYTES]
        if len(source) != SOURCE_BYTES:
            raise ValueError("TITLE.BIN source span is truncated")

        wram = (args.capture / "wram-hi.bin").read_bytes()
        vram = (args.capture / "vdp2-vram.bin").read_bytes()
        registers = (args.capture / "vdp2-registers.bin").read_bytes()
        if len(wram) != 0x100000 or len(vram) != 0x80000 or len(registers) != 0x200:
            raise ValueError("Ymir memory snapshot has an unexpected size")
        wram_source = wram[WRAM_SOURCE_OFFSET:WRAM_SOURCE_OFFSET + SOURCE_BYTES]
        if wram_source != source:
            raise ValueError("WorkRAM source range differs from authentic TITLE.BIN")

        snapshot = receipt(args.capture / "vdp2-memory-write-bundle.txt",
                           "ymir-vdp2-memory-write-bundle-v1")
        if (snapshot.get("bundle_complete") != "1" or
                snapshot.get("trace_file_complete") != "1" or
                snapshot.get("vdp2_vram_file_complete") != "1" or
                snapshot.get("trace_dropped_total") != "0" or
                int(snapshot.get("current_vdp1_frame_index", "-1")) != SNAPSHOT_FRAME or
                snapshot.get("trace_vdp1_frame_range_enabled") != "1" or
                int(snapshot.get("trace_vdp1_frame_begin", "-1")) != 7000 or
                int(snapshot.get("trace_vdp1_frame_end_exclusive", "-1")) != 8050 or
                int(snapshot.get("first_vdp1_frame_index", "-1")) != 7550 or
                int(snapshot.get("last_vdp1_frame_index", "-1")) != COPY_FRAME):
            raise ValueError("VDP2 snapshot/trace receipt is incomplete or unpaired")

        register_snapshot = receipt(args.capture / "vdp-register-snapshot.txt",
                                    "ymir-vdp-register-snapshot-v1")
        if int(register_snapshot.get("vdp1_frame_index", "-1")) != SNAPSHOT_FRAME:
            raise ValueError("VDP2 registers and memory snapshot are from different frames")

        if register_snapshot.get("register_byte_order") != "big_endian":
            raise ValueError("unsupported VDP2 register byte order")
        expected_registers = {
            0x00: 0x8000,  # TVMD: display enabled
            0x20: 0x0001,  # BGON: NBG0 enabled
            0x28: 0x0013,  # CHCTLA: NBG0 8bpp bitmap
            0x2C: 0x0000,  # BMPNA
            0x30: 0x0000,  # PNCN0
            0x34: 0x0000,  # PNCN2
            0x70: 0x0000,  # SCXIN0
            0x72: 0x0000,  # SCXDN0
            0x74: 0x0000,  # SCYIN0
            0x76: 0x0000,  # SCYDN0
        }
        for offset, expected in expected_registers.items():
            actual = int.from_bytes(registers[offset:offset + 2], "big")
            if actual != expected:
                raise ValueError(f"unexpected NBG0 register at 0x{offset:02x}")

        matched = 0
        mismatch_indices: list[int] = []
        for index, expected in enumerate(source):
            row, column = divmod(index, ROW_BYTES)
            address = VDP2_DESTINATION_OFFSET + row * ROW_STRIDE + column
            if vram[address] == expected:
                matched += 1
            else:
                mismatch_indices.append(index)
        expected_tail_start = TAIL_ROW * ROW_BYTES
        if (matched != 30561 or len(mismatch_indices) != 1055 or
                any(index < expected_tail_start for index in mismatch_indices)):
            raise ValueError("NBG0 snapshot no longer matches the admitted byte layout")

        writes = read_vdp2_trace(args.capture / "vdp2-memory-write-trace.csv")
        if len(writes) != int(snapshot.get("trace_rows", "-1")):
            raise ValueError("VDP2 trace row count differs from its receipt")
        if not writes:
            raise ValueError("VDP2 write trace is empty")
        trace_frames = [frame for frame, _area, _address, _width, _value in writes]
        first_frame = int(snapshot.get("first_vdp1_frame_index", "-1"))
        last_frame = int(snapshot.get("last_vdp1_frame_index", "-1"))
        if (min(trace_frames) != first_frame or max(trace_frames) != last_frame or
                last_frame != COPY_FRAME):
            raise ValueError("VDP2 trace frame range differs from its receipt")
        post_copy_vdp2_writes = [
            row for row in writes
            if COPY_FRAME < row[0] <= SNAPSHOT_FRAME and
            row[1] in ("VRAM", "CRAM")]
        if post_copy_vdp2_writes:
            raise ValueError("VDP2 VRAM or CRAM changed after the frame-7716 copy")
        tail_rows = []
        tail_start_address = VDP2_DESTINATION_OFFSET + TAIL_ROW * ROW_STRIDE
        tail_end_address = VDP2_DESTINATION_OFFSET + ROW_COUNT * ROW_STRIDE
        for frame, area, address, width, value in writes:
            if frame != COPY_FRAME or area != "VRAM" or width != 1:
                continue
            if tail_start_address <= address < tail_end_address:
                row, column = divmod(address - VDP2_DESTINATION_OFFSET, ROW_STRIDE)
                if TAIL_ROW <= row < ROW_COUNT and column < ROW_BYTES:
                    index = row * ROW_BYTES + column
                    tail_rows.append((index, traced_byte(value)))
        expected_indices = list(range(expected_tail_start, SOURCE_BYTES))
        if [index for index, _ in tail_rows] != expected_indices:
            raise ValueError("frame-7716 NBG0 tail writes are incomplete or reordered")
        for index, value in tail_rows:
            row, column = divmod(index, ROW_BYTES)
            address = VDP2_DESTINATION_OFFSET + row * ROW_STRIDE + column
            if vram[address] != value:
                raise ValueError("frame-7716 NBG0 tail writes differ from final VRAM")
        equal_tail_writes = sum(value == source[index]
                                for index, value in tail_rows)
        if equal_tail_writes != 1377:
            raise ValueError("frame-7716 NBG0 tail value join differs from capture")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_TITLE_BIN_STORAGE_INVALID: {error}")
        return 1

    print(f"title_bin_sha256={title_hash}")
    print(f"title_bin_source_offset=0x{TITLE_BIN_SOURCE_OFFSET:x}")
    print(f"title_bin_source_bytes={SOURCE_BYTES}")
    print("title_bin_wram_source_join=verified")
    print("wram_source_address=0x060ac2a6")
    print(f"snapshot_frame={SNAPSHOT_FRAME}")
    print("nbg0_register_mode=8bpp_bitmap_enabled")
    print(f"nbg0_source_storage_matches={matched}/{SOURCE_BYTES}")
    print(f"nbg0_tail_frame_{COPY_FRAME}_writes={len(tail_rows)}")
    print(f"nbg0_tail_source_equal_writes={equal_tail_writes}/{len(tail_rows)}")
    print("same_run_sh2_source_read=unproven")
    print("final_display_pixel_ownership=unproven")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
