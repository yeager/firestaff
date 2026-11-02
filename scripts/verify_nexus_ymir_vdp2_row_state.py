#!/usr/bin/env python3
"""Verify and replay a complete Ymir VDP2 write/scanline event bundle."""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path


VRAM_SIZE = 0x80000
CRAM_SIZE = 0x1000
EVENT_FIELDS = ["sequence", "event", "vdp1_frame", "row", "memory", "address", "width", "value"]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def parse_receipt(path: Path) -> dict[str, str]:
    receipt: dict[str, str] = {}
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        require(key not in receipt, f"duplicate receipt key at line {line_number}: {key}")
        receipt[key] = value
    return receipt


def parse_watch_range(value: str, size: int) -> tuple[int, int]:
    try:
        address_text, length_text = value.split(":", 1)
        address, length = int(address_text, 0), int(length_text, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected ADDRESS:LENGTH, for example 0x5ca00:8") from error
    if address < 0 or length <= 0 or address + length > size:
        raise argparse.ArgumentTypeError("watch range must fit inside the selected VDP2 memory")
    return address, length


def sha256(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()


def write_count(events: list[dict[str, str]]) -> int:
    return sum(row["event"] == "write" for row in events)


def load_events(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        require(reader.fieldnames == EVENT_FIELDS, "render event CSV has an unexpected schema")
        return list(reader)


def validate_bundle(dumps: Path) -> tuple[dict[str, str], list[dict[str, str]], bytearray, bytearray]:
    receipt = parse_receipt(dumps / "vdp2-memory-write-bundle.txt")
    require(receipt.get("format") == "ymir-vdp2-memory-write-bundle-v1", "unsupported Ymir bundle format")
    for key in (
        "bundle_complete",
        "trace_file_complete",
        "render_event_trace_file_complete",
        "initial_vram_file_complete",
        "initial_cram_file_complete",
        "vdp2_vram_file_complete",
        "vdp2_cram_file_complete",
        "row_state_reconstruction_complete",
    ):
        require(receipt.get(key) == "1", f"Ymir bundle is incomplete: {key} != 1")
    require(receipt.get("trace_dropped_total") == "0", "Ymir trace dropped events")
    require(receipt.get("trace_vram") == "1" and receipt.get("trace_cram") == "1",
            "both VRAM and CRAM must be traced")
    require(receipt.get("trace_vdp1_frame_range_enabled") == "0", "frame-range filtering omits writes")
    require(int(receipt.get("address_begin", "-1"), 16) == 0, "trace address range does not start at zero")
    require(int(receipt.get("address_end_exclusive", "0"), 16) >= VRAM_SIZE,
            "trace address range does not include all VDP2 VRAM")

    start_vram = bytearray((dumps / "vdp2-vram-start.bin").read_bytes())
    start_cram = bytearray((dumps / "vdp2-cram-start.bin").read_bytes())
    final_vram = (dumps / "vdp2-vram.bin").read_bytes()
    final_cram = (dumps / "vdp2-cram.bin").read_bytes()
    require(len(start_vram) == len(final_vram) == VRAM_SIZE, "VDP2 VRAM snapshot size is unexpected")
    require(len(start_cram) == len(final_cram) == CRAM_SIZE, "VDP2 CRAM snapshot size is unexpected")

    events = load_events(dumps / "vdp2-render-event-trace.csv")
    require(len(events) == int(receipt["render_event_rows"]), "render event count differs from the receipt")
    require(write_count(events) == int(receipt["trace_rows"]), "write count differs from the legacy receipt field")
    require(events, "render event trace is empty")
    require(any(row["event"] == "scanline" for row in events), "render event trace has no scanline markers")

    previous_sequence = -1
    for index, row in enumerate(events):
        sequence = int(row["sequence"])
        require(sequence > previous_sequence, f"event sequence is not strictly increasing at row {index + 2}")
        previous_sequence = sequence
        require(row["vdp1_frame"] == "unknown" or row["vdp1_frame"].isdigit(),
                f"invalid VDP1 frame at event row {index + 2}")
        if row["event"] == "scanline":
            require(row["row"].isdigit(), f"scanline event has no row at event row {index + 2}")
            require(not any(row[field] for field in ("memory", "address", "width", "value")),
                    f"scanline event contains write fields at row {index + 2}")
        else:
            require(row["event"] == "write", f"unknown event type at row {index + 2}")
            require(not row["row"], f"write event unexpectedly has a row at event row {index + 2}")
            require(row["memory"] in ("VRAM", "CRAM"), f"unknown memory type at event row {index + 2}")
            width = int(row["width"])
            require(width in (1, 2), f"unsupported write width at event row {index + 2}")
            address = int(row["address"], 16)
            value = int(row["value"], 16)
            memory_size = VRAM_SIZE if row["memory"] == "VRAM" else CRAM_SIZE
            require(0 <= address <= memory_size - width, f"write exceeds {row['memory']} at event row {index + 2}")
            require(0 <= value < 1 << (width * 8), f"write value exceeds width at event row {index + 2}")
    return receipt, events, start_vram, start_cram


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, help="Ymir capture root or its dumps directory")
    parser.add_argument("--watch-vram", action="append", default=[], metavar="ADDRESS:LENGTH",
                        help="emit bytes from this VRAM range at every scanline boundary; repeatable")
    parser.add_argument("--watch-cram", action="append", default=[], metavar="ADDRESS:LENGTH",
                        help="emit bytes from this CRAM range at every scanline boundary; repeatable")
    args = parser.parse_args()
    dumps = args.capture / "dumps" if (args.capture / "dumps").is_dir() else args.capture
    receipt, events, vram, cram = validate_bundle(dumps)
    final_vram = (dumps / "vdp2-vram.bin").read_bytes()
    final_cram = (dumps / "vdp2-cram.bin").read_bytes()
    vram_watches = [parse_watch_range(value, VRAM_SIZE) for value in args.watch_vram]
    cram_watches = [parse_watch_range(value, CRAM_SIZE) for value in args.watch_cram]
    scanlines = 0

    for index, row in enumerate(events):
        if row["event"] == "write":
            memory = vram if row["memory"] == "VRAM" else cram
            address = int(row["address"], 16)
            width = int(row["width"])
            value = int(row["value"], 16)
            memory[address : address + width] = value.to_bytes(width, "big")
            continue

        frame = row["vdp1_frame"]
        prefix = f"row_state sequence={row['sequence']} vdp1_frame={frame} row={row['row']}"
        print(f"{prefix} vram_sha256={sha256(vram)} cram_sha256={sha256(cram)}")
        for watch_index, (address, length) in enumerate(vram_watches):
            value = vram[address : address + length].hex()
            print(f"watch_vram index={watch_index} range=0x{address:x}:0x{length:x} bytes={value}")
        for watch_index, (address, length) in enumerate(cram_watches):
            value = cram[address : address + length].hex()
            print(f"watch_cram index={watch_index} range=0x{address:x}:0x{length:x} bytes={value}")
        scanlines += 1

    require(bytes(vram) == final_vram, "replayed VRAM does not match the export-time snapshot")
    require(bytes(cram) == final_cram, "replayed CRAM does not match the export-time snapshot")
    print(f"vdp1_frame_range_enabled={receipt['trace_vdp1_frame_range_enabled']}")
    print(f"ordered_writes={receipt['trace_rows']}")
    print(f"ordered_scanline_markers={scanlines}")
    print(f"replayed_vram_sha256={sha256(vram)}")
    print(f"replayed_cram_sha256={sha256(cram)}")
    print("replay_vram_matches_final=1")
    print("replay_cram_matches_final=1")
    print("row_state_reconstruction=verified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
