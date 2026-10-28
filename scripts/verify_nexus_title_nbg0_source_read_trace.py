#!/usr/bin/env python3
"""Verify a frame-tagged SH-2 read pass over authentic Nexus title bytes.

This validates address-level reads only. The retained read trace may have
dropped unrelated rows after reaching its global capacity; this tool checks
that one observed frame contains a complete, sequential pass over the source
address span. Ymir's SH-2 read trace does not carry read values, so the tool
does not claim value equality at read time or final NBG0 pixel ownership.
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
SOURCE_ADDRESS = 0x060AC2A6
SOURCE_END = SOURCE_ADDRESS + SOURCE_BYTES
SOURCE_READ_FRAME = 6893
LAST_SOURCE_WRITE_FRAME = 6890
SOURCE_READ_PC = "0x6023124"
SOURCE_READ_CALLER_RETURN = "0x6023176"
SOURCE_READ_CALLER_TARGET = "0x6023108"
TRACE_FRAME_BEGIN = 0
TRACE_FRAME_END = 8050


def receipt(path: Path, expected_format: str) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in path.read_text(encoding="ascii").splitlines():
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key in values:
            raise ValueError(f"duplicate {key} in {path.name}")
        values[key] = value
    if values.get("format") != expected_format:
        raise ValueError(f"unsupported {path.name} format")
    return values


def require_trace_file(
        bundle: dict[str, str], name: str,
        frame_keys: tuple[str, str, str] = (
            "frame_range_enabled", "frame_begin", "frame_end_exclusive")) -> int:
    if (bundle.get("trace_file_complete") != "1" or
            bundle.get("bundle_complete") != "1"):
        raise ValueError(f"{name} trace file or receipt is incomplete")
    enabled_key, begin_key, end_key = frame_keys
    if (bundle.get(enabled_key) != "1" or
            int(bundle.get(begin_key, "-1")) != TRACE_FRAME_BEGIN or
            int(bundle.get(end_key, "-1")) != TRACE_FRAME_END):
        raise ValueError(f"{name} trace does not cover the admitted frame interval")
    return int(bundle.get("trace_rows", "-1"))


def verify_contiguous_source_read(rows: list[dict[str, str]]) -> int:
    selected = [row for row in rows
                if int(row["vdp1_frame"], 10) == SOURCE_READ_FRAME]
    if len(selected) != SOURCE_BYTES:
        raise ValueError(
            f"frame {SOURCE_READ_FRAME} has {len(selected)} rows; "
            f"expected a {SOURCE_BYTES}-byte source pass")
    for offset, row in enumerate(selected):
        if row["sh2"] != "master":
            raise ValueError("source read pass is not owned by the master SH-2")
        if (row["pc"].lower() != SOURCE_READ_PC or
                row["caller_return_address"].lower() != SOURCE_READ_CALLER_RETURN or
                row["caller_target"].lower() != SOURCE_READ_CALLER_TARGET):
            raise ValueError("source read pass has an unexpected SH-2 call context")
        if int(row["width"], 10) != 1:
            raise ValueError("source read pass is not byte-wide")
        address = int(row["address"], 16)
        if address != SOURCE_ADDRESS + offset:
            raise ValueError("source read pass is not contiguous and ordered")
    return len(selected)


def validate_frame_rows(rows: list[dict[str, str]]) -> None:
    for row in rows:
        if row["sh2"] not in ("master", "slave"):
            raise ValueError("unexpected SH-2 owner in read trace")
        if row["vdp1_frame"] == "unknown":
            raise ValueError("frame-filtered read trace contains an unknown frame")
        frame = int(row["vdp1_frame"], 10)
        address = int(row["address"], 16)
        width = int(row["width"], 10)
        if width not in (1, 2, 4):
            raise ValueError("invalid SH-2 read width")
        if not TRACE_FRAME_BEGIN <= frame < TRACE_FRAME_END:
            raise ValueError("read row lies outside the requested frame interval")
        if address < SOURCE_ADDRESS or address + width > SOURCE_END:
            raise ValueError("read row extends outside the filtered source span")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path,
                        help="Ymir dump folder containing memory-read receipts")
    asset_source = parser.add_mutually_exclusive_group(required=True)
    asset_source.add_argument("--data-dir", type=Path,
                              help="directory containing authenticated Nexus title assets")
    asset_source.add_argument("--cue", type=Path,
                              help="authentic Nexus CUE; members are read in memory")
    args = parser.parse_args()

    try:
        title_bin = read_title_assets(args.data_dir, args.cue)[0]
        if hashlib.sha256(title_bin).hexdigest() != TITLE_BIN_SHA256:
            raise ValueError("TITLE.BIN is not the authenticated Japanese retail file")
        source = title_bin[TITLE_BIN_SOURCE_OFFSET:TITLE_BIN_SOURCE_OFFSET + SOURCE_BYTES]
        if len(source) != SOURCE_BYTES:
            raise ValueError("TITLE.BIN source span is truncated")
        wram = (args.capture / "wram-hi.bin").read_bytes()
        if len(wram) != 0x100000:
            raise ValueError("Ymir WorkRAMH snapshot has an unexpected size")
        if wram[WRAM_SOURCE_OFFSET:WRAM_SOURCE_OFFSET + SOURCE_BYTES] != source:
            raise ValueError("final WorkRAMH source span differs from authentic TITLE.BIN")

        read_bundle = receipt(
            args.capture / "sh2-memory-read-trace-bundle.txt",
            "ymir-sh2-memory-read-trace-bundle-v2")
        expected_read_rows = require_trace_file(read_bundle, "SH-2 read")
        if (int(read_bundle.get("address_begin", "-1"), 0) != SOURCE_ADDRESS or
                int(read_bundle.get("address_end_exclusive", "-1"), 0) != SOURCE_END):
            raise ValueError("SH-2 read trace address bounds differ from TITLE.BIN span")
        with (args.capture / "sh2-memory-read-trace.csv").open(
                "r", encoding="ascii", newline="") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != ["sh2", "vdp1_frame", "pc",
                                     "caller_return_address", "caller_target",
                                     "address", "width"]:
                raise ValueError("unexpected SH-2 read trace columns")
            read_rows = list(reader)
        if len(read_rows) != expected_read_rows:
            raise ValueError("SH-2 read trace row count differs from its receipt")
        validate_frame_rows(read_rows)
        read_count = verify_contiguous_source_read(read_rows)
        read_dropped = int(read_bundle.get("master_dropped", "-1")) + int(
            read_bundle.get("slave_dropped", "-1"))
        if read_dropped < 0:
            raise ValueError("SH-2 read trace drop count is missing")

        write_bundle = receipt(
            args.capture / "sh2-memory-write-trace-bundle.txt",
            "ymir-sh2-memory-write-trace-bundle-v1")
        expected_write_rows = require_trace_file(
            write_bundle, "SH-2 write",
            ("sh2_ram_write_frame_range_enabled",
             "sh2_ram_write_frame_begin",
             "sh2_ram_write_frame_end_exclusive"))
        if (int(write_bundle.get("master_dropped", "-1")) != 0 or
                int(write_bundle.get("slave_dropped", "-1")) != 0):
            raise ValueError("SH-2 write trace is missing rows")
        with (args.capture / "sh2-memory-write-trace.csv").open(
                "r", encoding="ascii", newline="") as stream:
            write_reader = csv.DictReader(stream)
            required_write_columns = {"vdp1_frame", "address", "width"}
            if (write_reader.fieldnames is None or
                    not required_write_columns <= set(write_reader.fieldnames)):
                raise ValueError(
                    "SH-2 write trace lacks frame, address, or width fields")
            write_rows = list(write_reader)
        if len(write_rows) != expected_write_rows:
            raise ValueError("SH-2 write trace row count differs from its receipt")
        source_writes = 0
        for row in write_rows:
            frame = row["vdp1_frame"]
            if frame == "unknown":
                raise ValueError("frame-filtered write trace contains an unknown frame")
            frame_index = int(frame, 10)
            address = int(row["address"], 16)
            width = int(row["width"], 10)
            if not TRACE_FRAME_BEGIN <= frame_index < TRACE_FRAME_END:
                raise ValueError("write row lies outside the requested frame interval")
            if width not in (1, 2, 4):
                raise ValueError("invalid SH-2 write width")
            if address >= SOURCE_END or address + width <= SOURCE_ADDRESS:
                raise ValueError("write row lies outside the filtered source span")
            if (frame_index > LAST_SOURCE_WRITE_FRAME and
                    address < SOURCE_END and address + width > SOURCE_ADDRESS):
                source_writes += 1
        if source_writes:
            raise ValueError(
                "SH-2 modified the TITLE.BIN source span after frame "
                f"{LAST_SOURCE_WRITE_FRAME}")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_SOURCE_READ_INVALID: {error}")
        return 1

    print(f"title_bin_sha256={TITLE_BIN_SHA256}")
    print(f"title_bin_source_address=0x{SOURCE_ADDRESS:08x}")
    print(f"title_bin_source_bytes={SOURCE_BYTES}")
    print("final_workramh_source_join=verified")
    print(f"source_read_frame={SOURCE_READ_FRAME}")
    print(f"source_read_pc={SOURCE_READ_PC}")
    print("source_read_call_context="
          f"target:{SOURCE_READ_CALLER_TARGET},pc:{SOURCE_READ_PC},"
          f"caller_return:{SOURCE_READ_CALLER_RETURN}")
    print(f"contiguous_source_read_rows={read_count}")
    print(f"contiguous_source_read_bytes={SOURCE_BYTES}/{SOURCE_BYTES}")
    print(f"read_trace_dropped_rows={read_dropped}")
    print("contiguous_source_read_pass=observed")
    print("read_values_at_access_time=unproven")
    print("final_display_pixel_ownership=unproven")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
