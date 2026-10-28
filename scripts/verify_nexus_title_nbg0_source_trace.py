#!/usr/bin/env python3
"""Verify the Japanese retail title-source writes into the NBG0 bitmap.

This bounded receipt joins an exact TITLE.BIN span to frame-tagged Ymir VDP2
writes and the later frame-bound NBG0 snapshot. It does not decode the full
bitmap or authorize presentation.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_vdp2_bitmap_source import read_assets


JAPANESE_TITLE_BIN_SHA256 = (
    "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
)
JAPANESE_CUE_SHA256 = (
    "878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699"
)
NBG0_BYTES = 512 * 256
NBG0_TITLE_OFFSET = 0xD404
TITLE_SOURCE_OFFSET = 0xB8A2
TITLE_SOURCE_BYTES = 921
CLEAR_FRAME = 6869
COPY_FRAME = 7729
SNAPSHOT_FRAME = 8060
SNAPSHOT_NBG0_SHA256 = (
    "0a3b8b5ba0bd9e9c4b549d136961b44439d2534cbb7060c5bcbdca6da9c53d08"
)
TITLE_RECORD_OFFSET = 0xB438
TITLE_RECORD_END = 0xD068
TITLE_RECORD_RELATIVE_OFFSET = 0x046A


def read_receipt(path: Path, expected_format: str) -> dict[str, str]:
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


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_bytes(width: int, value: int) -> bytes:
    if width not in (1, 2) or value < 0 or value >= 1 << (width * 8):
        raise ValueError("unsupported or truncated VDP2 write")
    return value.to_bytes(width, "big")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path,
                        help="external Ymir dump directory")
    parser.add_argument("--cue", type=Path, required=True,
                        help="authentic Japanese retail CUE")
    args = parser.parse_args()

    try:
        if sha256(args.cue.read_bytes()) != JAPANESE_CUE_SHA256:
            raise ValueError("Japanese retail CUE hash mismatch")
        title_bin = read_assets(None, args.cue, {"TITLE.BIN"})["TITLE.BIN"]
        if sha256(title_bin) != JAPANESE_TITLE_BIN_SHA256:
            raise ValueError("Japanese TITLE.BIN hash mismatch")
        if (title_bin[:4] != b"RES*" or
                int.from_bytes(title_bin[4:8], "big") != len(title_bin) or
                int.from_bytes(title_bin[8:10], "big") != 60):
            raise ValueError("TITLE.BIN RES* directory header is invalid")
        title_records = []
        for index in range(60):
            entry = title_bin[12 + index * 12:24 + index * 12]
            if len(entry) != 12:
                raise ValueError("TITLE.BIN RES* directory is truncated")
            title_records.append((entry[:4], int.from_bytes(entry[4:8], "big"),
                                  int.from_bytes(entry[8:12], "big")))
        title_record = next((row for row in title_records
                             if row[0] == b"TITL" and row[1] == 2), None)
        if (title_record is None or title_record[2] != TITLE_RECORD_OFFSET or
                title_bin[TITLE_RECORD_OFFSET:TITLE_RECORD_OFFSET + 10] !=
                b"TITL\0\0\0\2PP"):
            raise ValueError("TITLE.BIN TITL record 2 identity is invalid")
        following_offsets = [offset for _magic, _record_id, offset in title_records
                             if offset > TITLE_RECORD_OFFSET]
        record_end = min(following_offsets) if following_offsets else len(title_bin)
        source = title_bin[TITLE_SOURCE_OFFSET:
                           TITLE_SOURCE_OFFSET + TITLE_SOURCE_BYTES]
        if (len(source) != TITLE_SOURCE_BYTES or record_end != TITLE_RECORD_END or
                TITLE_SOURCE_OFFSET - TITLE_RECORD_OFFSET !=
                TITLE_RECORD_RELATIVE_OFFSET or
                TITLE_SOURCE_OFFSET < TITLE_RECORD_OFFSET or
                TITLE_SOURCE_OFFSET + TITLE_SOURCE_BYTES > record_end):
            raise ValueError("TITLE.BIN source span is outside TITL record 2")
        source_pixels = [NBG0_TITLE_OFFSET + index
                         for index, value in enumerate(source) if value]
        source_rows = {address // 512 for address in source_pixels}
        if len(source_rows) != 1:
            raise ValueError("joined source span is not the observed one-row layer")
        source_x_min = min(address % 512 for address in source_pixels)
        source_x_max = max(address % 512 for address in source_pixels)
        source_y = next(iter(source_rows))

        vdp_receipt = read_receipt(
            args.bundle / "vdp2-memory-write-bundle.txt",
            "ymir-vdp2-memory-write-bundle-v1")
        capture_receipt = read_receipt(
            args.bundle / "vdp1-capture-bundle.txt",
            "ymir-vdp1-capture-bundle-v1")
        register_receipt = read_receipt(
            args.bundle / "vdp-register-snapshot.txt",
            "ymir-vdp-register-snapshot-v1")
        if (vdp_receipt.get("bundle_complete") != "1" or
                vdp_receipt.get("trace_file_complete") != "1" or
                vdp_receipt.get("trace_dropped_total") != "0" or
                int(vdp_receipt["trace_rows"]) < 1 or
                int(vdp_receipt["first_vdp1_frame_index"]) > CLEAR_FRAME or
                int(vdp_receipt["last_vdp1_frame_index"]) != COPY_FRAME or
                int(vdp_receipt["current_vdp1_frame_index"]) != SNAPSHOT_FRAME):
            raise ValueError("VDP2 trace receipt is incomplete or has another frame range")
        if (capture_receipt.get("trace_file_complete") != "1" or
                capture_receipt.get("trace_dropped_total") != "0" or
                int(capture_receipt["trace_rows_for_capture_frame"]) < 1 or
                int(capture_receipt["capture_frame_index"]) != SNAPSHOT_FRAME or
                int(register_receipt["vdp1_frame_index"]) != SNAPSHOT_FRAME or
                register_receipt.get("register_byte_order") != "big_endian"):
            raise ValueError("memory snapshot lacks a matching complete frame receipt")

        trace_path = args.bundle / "vdp2-memory-write-trace.csv"
        rows_seen = 0
        first_trace_frame: int | None = None
        last_trace_frame: int | None = None
        clear_values: dict[int, int] = {}
        clear_counts: dict[int, int] = {}
        writes_by_frame: dict[int, list[tuple[int, bytes]]] = {}
        with trace_path.open(encoding="ascii", newline="") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != [
                    "vdp1_frame", "memory", "address", "width", "value"]:
                raise ValueError("VDP2 trace header is unsupported")
            for row in reader:
                rows_seen += 1
                if row["vdp1_frame"] == "unknown":
                    raise ValueError("VDP2 trace contains an unbound frame")
                frame = int(row["vdp1_frame"])
                first_trace_frame = (frame if first_trace_frame is None else
                                     min(first_trace_frame, frame))
                last_trace_frame = (frame if last_trace_frame is None else
                                    max(last_trace_frame, frame))
                if row["memory"] != "VRAM":
                    continue
                address = int(row["address"], 16)
                width = int(row["width"])
                value = int(row["value"], 16)
                data = write_bytes(width, value)
                if address < 0 or address + width > 0x80000:
                    raise ValueError("VDP2 trace contains an out-of-range VRAM write")
                if frame == CLEAR_FRAME:
                    start = max(address, NBG0_TITLE_OFFSET)
                    end = min(address + width,
                              NBG0_TITLE_OFFSET + TITLE_SOURCE_BYTES)
                    for offset in range(start, end):
                        clear_values[offset] = data[offset - address]
                        clear_counts[offset] = clear_counts.get(offset, 0) + 1
                if frame >= CLEAR_FRAME and address < NBG0_TITLE_OFFSET + TITLE_SOURCE_BYTES and \
                        address + width > NBG0_TITLE_OFFSET:
                    writes_by_frame.setdefault(frame, []).append((address, data))

        if (rows_seen != int(vdp_receipt["trace_rows"]) or
                first_trace_frame != int(vdp_receipt["first_vdp1_frame_index"]) or
                last_trace_frame != int(vdp_receipt["last_vdp1_frame_index"])):
            raise ValueError("VDP2 trace rows do not match the frame receipt")
        expected_offsets = set(range(NBG0_TITLE_OFFSET,
                                     NBG0_TITLE_OFFSET + TITLE_SOURCE_BYTES))
        if (set(clear_values) != expected_offsets or
                any(count != 1 for count in clear_counts.values()) or
                any(clear_values.values())):
            raise ValueError("clear frame does not zero the complete title-source span")
        copy_writes = writes_by_frame.get(COPY_FRAME, [])
        copy_addresses = [address for address, data in copy_writes if len(data) == 1]
        if (len(copy_writes) != 608 or len(copy_addresses) != 608 or
                len(set(copy_addresses)) != 608):
            raise ValueError("title-copy frame is not the bounded 608-byte sequence")

        replay = bytearray(TITLE_SOURCE_BYTES)
        for frame in sorted(writes_by_frame):
            for address, data in writes_by_frame[frame]:
                start = max(address, NBG0_TITLE_OFFSET)
                end = min(address + len(data),
                          NBG0_TITLE_OFFSET + TITLE_SOURCE_BYTES)
                replay[start - NBG0_TITLE_OFFSET:end - NBG0_TITLE_OFFSET] = \
                    data[start - address:end - address]
                if frame == COPY_FRAME:
                    expected = source[start - NBG0_TITLE_OFFSET:
                                       end - NBG0_TITLE_OFFSET]
                    if data[start - address:end - address] != expected:
                        raise ValueError("title-copy frame byte differs from retail source")
        if bytes(replay) != source:
            raise ValueError("frame-tagged VDP2 writes do not reconstruct TITLE.BIN span")

        vram = (args.bundle / "vdp2-vram.bin").read_bytes()
        registers = (args.bundle / "vdp2-registers.bin").read_bytes()
        if len(vram) != 0x80000 or len(registers) != 512:
            raise ValueError("VDP2 snapshot has an invalid size")
        snapshot = vram[NBG0_TITLE_OFFSET:
                        NBG0_TITLE_OFFSET + TITLE_SOURCE_BYTES]
        nbg0 = vram[:NBG0_BYTES]
        if snapshot != source:
            raise ValueError("frame-bound VDP2 snapshot differs from retail source")
        if sha256(nbg0) != SNAPSHOT_NBG0_SHA256:
            raise ValueError("NBG0 bitmap differs from the authenticated title capture")
        regs = {name: int.from_bytes(registers[offset:offset + 2], "big")
                for name, offset in (("TVMD", 0), ("BGON", 0x20),
                                     ("CHCTLA", 0x28), ("BMPNA", 0x2C),
                                     ("PLSZ", 0x2A), ("MPOFN", 0x3C))}
        if (regs["TVMD"] != 0x8000 or regs["BGON"] != 1 or
                regs["CHCTLA"] != 0x0013 or regs["BMPNA"] & 7 or
                regs["MPOFN"] & 7 or regs["PLSZ"] != 2):
            raise ValueError("frame-bound VDP2 registers do not select the observed NBG0 bitmap")
    except (KeyError, OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_SOURCE_TRACE_INVALID: {error}")
        return 1

    print("title_nbg0_source_trace=verified")
    print(f"japanese_cue_sha256={JAPANESE_CUE_SHA256}")
    print(f"japanese_title_bin_sha256={JAPANESE_TITLE_BIN_SHA256}")
    print(f"title_source_offset=0x{TITLE_SOURCE_OFFSET:x}")
    print(f"title_source_bytes={TITLE_SOURCE_BYTES}")
    print("title_source_record=TITL/id2")
    print(f"title_source_record_offset=0x{TITLE_RECORD_OFFSET:x}")
    print(f"title_source_record_relative_offset=0x{TITLE_RECORD_RELATIVE_OFFSET:x}")
    print(f"title_source_sha256={sha256(source)}")
    print(f"title_source_nonzero_bytes={sum(value != 0 for value in source)}")
    print(f"title_source_nonzero_bounds={source_x_min},{source_y}-{source_x_max},{source_y}")
    print(f"vdp2_trace_rows={rows_seen}")
    print(f"title_span_clear_frame={CLEAR_FRAME}")
    print(f"title_span_copy_frame={COPY_FRAME}")
    print(f"title_span_copy_writes={len(copy_writes)}")
    print(f"title_snapshot_frame={SNAPSHOT_FRAME}")
    print(f"title_snapshot_nbg0_sha256={sha256(nbg0)}")
    for name, value in regs.items():
        print(f"{name}=0x{value:04x}")
    print("title_background_decode=not_verified")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
