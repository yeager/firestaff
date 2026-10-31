#!/usr/bin/env python3
"""Verify authentic retail Nexus MAPD planes in a Ymir VDP2 write capture.

This checks producer-side VRAM bytes only. It does not establish Saturn layer
consumption, final pixel composition, or Firestaff display parity.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

TITLE_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
MAPD_OFFSET = 0xE278
MAP_HEADER_BYTES = 0x40
MAP_STRIDE = 0x1C04
MAP_BYTES = 0x1C00
VRAM_BYTES = 0x80000
MAP_VRAM_BASE = 0x5C000
PLANE_NAMES = ("N", "E", "X", "U", "S")
TRACE_HEADER = ["vdp1_frame", "memory", "address", "width", "value"]


class VerificationError(ValueError):
    """Raised when the capture does not prove the expected retail transfer."""


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_receipt(path: Path) -> dict[str, str]:
    try:
        return dict(line.split("=", 1) for line in path.read_text(encoding="ascii").splitlines()
                    if "=" in line)
    except (OSError, UnicodeError, ValueError) as error:
        raise VerificationError(f"invalid VDP2 write receipt: {error}") from error


def read_planes(title_path: Path) -> tuple[bytes, list[bytes]]:
    try:
        title = title_path.read_bytes()
    except OSError as error:
        raise VerificationError(f"cannot read TITLE.BIN: {error}") from error
    if digest(title) != TITLE_SHA256:
        raise VerificationError("TITLE.BIN is not the hash-pinned Japanese retail file")
    section = title[MAPD_OFFSET:]
    if section[:4] != b"MAPD" or section[8:12] != b"TIBG":
        raise VerificationError("retail MAPD/TIBG signatures are missing")

    planes = []
    for index, name in enumerate(PLANE_NAMES):
        start = MAP_HEADER_BYTES + index * MAP_STRIDE
        if section[start:start + 4] != b"\x00\x40\x00\x1c":
            raise VerificationError(f"MAPD plane {name} is not 64x28")
        plane = section[start + 4:start + 4 + MAP_BYTES]
        if len(plane) != MAP_BYTES:
            raise VerificationError(f"MAPD plane {name} is truncated")
        planes.append(plane)
    return title, planes


def replay_and_find_sequence(trace_path: Path, receipt: dict[str, str],
                             planes: list[bytes]) -> tuple[bytes, list[tuple[int, int]], int]:
    if receipt.get("format") != "ymir-vdp2-memory-write-bundle-v1":
        raise VerificationError("unsupported Ymir VDP2 memory-write receipt")
    if any(receipt.get(key) != "1" for key in (
            "bundle_complete", "trace_file_complete", "vdp2_vram_file_complete",
            "trace_vram", "trace_vdp1_frame_range_enabled")):
        raise VerificationError("VDP2 write capture or frame range is incomplete")
    if receipt.get("trace_dropped_total") != "0":
        raise VerificationError("VDP2 write capture dropped rows")
    try:
        expected_rows = int(receipt["trace_rows"])
        frame_begin = int(receipt["trace_vdp1_frame_begin"])
        frame_end = int(receipt["trace_vdp1_frame_end_exclusive"])
    except (KeyError, ValueError) as error:
        raise VerificationError("VDP2 receipt has invalid row/frame bounds") from error
    if frame_begin < 0 or frame_end <= frame_begin:
        raise VerificationError("VDP2 receipt has an invalid frame range")

    vram = bytearray(VRAM_BYTES)
    frame_writes: dict[int, list[tuple[int, int, int]]] = {}
    transfers: list[tuple[int, int]] = []
    previous_region: bytes | None = None
    current_frame: int | None = None
    first_frame: int | None = None
    last_frame: int | None = None
    rows_seen = 0

    def finish_frame(frame: int) -> None:
        nonlocal previous_region
        region = bytes(vram[MAP_VRAM_BASE:MAP_VRAM_BASE + MAP_BYTES])
        if region != previous_region:
            matches = [index for index, plane in enumerate(planes) if region == plane]
            if matches:
                if len(matches) != 1:
                    raise VerificationError(f"ambiguous retail plane match at frame {frame}")
                plane = planes[matches[0]]
                expected_writes = [
                    (MAP_VRAM_BASE + offset, 2,
                     int.from_bytes(plane[offset:offset + 2], "big"))
                    for offset in range(0, MAP_BYTES, 2)
                ]
                if frame_writes.get(frame, []) != expected_writes:
                    raise VerificationError(
                        f"frame {frame} writes do not equal one ordered retail word plane"
                    )
                transfers.append((frame, matches[0]))
        previous_region = region

    try:
        with trace_path.open(encoding="ascii", newline="") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != TRACE_HEADER:
                raise VerificationError("unsupported VDP2 write trace header")
            for row in reader:
                rows_seen += 1
                if row["vdp1_frame"] == "unknown":
                    raise VerificationError("VDP2 write trace contains an unbound frame")
                frame = int(row["vdp1_frame"])
                if not frame_begin <= frame < frame_end:
                    raise VerificationError(f"trace frame {frame} is outside its receipt range")
                if current_frame is not None and frame < current_frame:
                    raise VerificationError("VDP2 trace frame order moved backwards")
                if current_frame is not None and frame != current_frame:
                    finish_frame(current_frame)
                current_frame = frame
                first_frame = frame if first_frame is None else min(first_frame, frame)
                last_frame = frame if last_frame is None else max(last_frame, frame)
                if row["memory"] != "VRAM":
                    if row["memory"] != "CRAM":
                        raise VerificationError(f"unknown VDP2 memory type: {row['memory']}")
                    continue
                address = int(row["address"], 16)
                width = int(row["width"])
                value = int(row["value"], 16)
                if width not in (1, 2, 4) or value < 0 or value >= 1 << (8 * width):
                    raise VerificationError("VDP2 trace has an invalid write width or value")
                # Ymir's VDP2Memory::MapVRAMAddress wraps to 512 KiB and aligns
                # to the write width; WriteVRAM stores the value big-endian.
                offset = (address & 0x7FFFF) & ~(width - 1)
                payload = value.to_bytes(width, "big")
                vram[offset:offset + width] = payload
                if offset < MAP_VRAM_BASE + MAP_BYTES and offset + width > MAP_VRAM_BASE:
                    frame_writes.setdefault(frame, []).append((offset, width, value))
    except (OSError, UnicodeError, csv.Error, KeyError, ValueError) as error:
        if isinstance(error, VerificationError):
            raise
        raise VerificationError(f"cannot replay VDP2 write trace: {error}") from error

    if current_frame is not None:
        finish_frame(current_frame)
    if rows_seen != expected_rows:
        raise VerificationError(f"receipt says {expected_rows} rows; parsed {rows_seen}")
    if first_frame != int(receipt.get("first_vdp1_frame_index", "-1")):
        raise VerificationError("first trace frame differs from the receipt")
    if last_frame != int(receipt.get("last_vdp1_frame_index", "-1")):
        raise VerificationError("last trace frame differs from the receipt")
    if [index for _, index in transfers] != list(range(len(planes))):
        observed = ",".join(f"{PLANE_NAMES[index]}@{frame}" for frame, index in transfers)
        raise VerificationError(f"retail plane sequence is not exactly N,E,X,U,S: {observed}")
    return bytes(vram), transfers, rows_seen


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--title-bin", required=True, type=Path,
                        help="authentic Japanese retail TITLE.BIN")
    parser.add_argument("--capture", required=True, type=Path,
                        help="Ymir capture root or its dumps directory")
    args = parser.parse_args()
    dumps = args.capture / "dumps" if (args.capture / "dumps").is_dir() else args.capture
    try:
        title, planes = read_planes(args.title_bin)
        receipt = read_receipt(dumps / "vdp2-memory-write-bundle.txt")
        replay, transfers, rows = replay_and_find_sequence(
            dumps / "vdp2-memory-write-trace.csv", receipt, planes)
        final_snapshot = (dumps / "vdp2-vram.bin").read_bytes()
        if len(final_snapshot) != VRAM_BYTES:
            raise VerificationError("final Ymir VDP2 VRAM snapshot has an invalid size")
        if replay != final_snapshot:
            raise VerificationError("replayed writes differ from the final Ymir VDP2 VRAM")
    except (OSError, UnicodeError, VerificationError, KeyError, ValueError) as error:
        print(f"NEXUS_YMIR_RETAIL_MAPD_SEQUENCE_INVALID: {error}")
        return 1

    print(f"title_bin_sha256={digest(title)}")
    print(f"trace_rows={rows}")
    print("retail_mapd_transfer_sequence=" + ",".join(
        f"{PLANE_NAMES[index]}@{frame}" for frame, index in transfers))
    print(f"replayed_vdp2_vram_sha256={digest(replay)}")
    print("replay_matches_final_vdp2_vram=1")
    print("producer_side_vram_match=1")
    print("display_pixel_parity=not_established")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
