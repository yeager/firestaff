#!/usr/bin/env python3
"""Verify the authentic MAPD NBG1 tile plane against Ymir's pixel trace."""

from __future__ import annotations

import csv
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys


WIDTH = 320
HEIGHT = 224
PIXELS = WIDTH * HEIGHT
TRACE_SHA256 = "e18e487e4352a39b1409c3d829345e1587a8dec468bf85723c579da970121b7d"
RECEIPT_SHA256 = "bb6c8e5822ed5b2a6b33782d4e6d0706ddb711bade5f88bab8dab4e44d55c876"
PREVIOUS_VRAM_SHA256 = "2b6fe0392b1ff23c0c55f08423d179a74d7f481990e37aadd4e6bfd657d9cf45"
CURRENT_VRAM_SHA256 = "7367347e6adcb72ac09c946d03a15e1486e12f32593cee7bf3992e58da3b3d9f"
CRAM_SHA256 = "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de"
REGISTERS_SHA256 = "774fa7f2e815b14770b83feb83e8c15406041a931e65ed465b1062944a811dd1"
PLANE_SHA256 = "a7d55feabe7ec09beb568808cb7674274f04b7e90be3758f8d91ed8aad2a0dea"
REFERENCE_SHA256 = "24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17"


class VerificationError(Exception):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def require_sha256(path: Path, expected: str) -> bytes:
    data = path.read_bytes()
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        raise VerificationError(f"SHA-256 mismatch for {path}: {actual}")
    return data


def read_be32(data: bytes, address: int) -> int:
    address &= 0x7FFFF
    return int.from_bytes(data[address : address + 4], "big")


def fetch_4bpp_dot(data: bytes, sample: dict[str, str]) -> tuple[int, int]:
    pattern = int(sample["raw_pnd"], 16)
    x = int(sample["scroll_x"]) & 7
    y = int(sample["scroll_y"]) & 7
    if pattern & 0x40000000:
        x ^= 7
    if pattern & 0x80000000:
        y ^= 7
    address = (int(sample["char_address"], 16) + y * 4 + x // 2) & 0x7FFFF
    packed = data[address]
    dot = packed >> 4 if x % 2 == 0 else packed & 0x0F
    return address, dot


def split_csv_line(line: str) -> list[str]:
    return line.rstrip("\r\n").split(",")


def verify_trace(
    trace_path: Path,
    receipt_path: Path,
    previous_vram: bytes,
    current_vram: bytes,
    plane: bytes,
    reference: bytes,
) -> tuple[int, int, int, dict[str, int]]:
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if (
        receipt.get("format") != "ymir-vdp2-pixel-fetch-receipt-v1"
        or receipt.get("complete") is not True
        or receipt.get("rows_written") != 143360
        or receipt.get("rows_dropped") != 0
        or receipt.get("write_errors") != 0
    ):
        raise VerificationError("Ymir pixel trace receipt is incomplete")

    selected: dict[tuple[int, int, int, int], dict[str, str]] = {}
    rows_written = 0
    with trace_path.open(newline="", encoding="utf-8") as stream:
        if stream.readline().rstrip("\r\n") != "# ymir-vdp2-pixel-fetch-csv-v1":
            raise VerificationError("unexpected Ymir pixel trace signature")
        columns = split_csv_line(stream.readline())
        positions = {name: index for index, name in enumerate(columns)}
        needed = {
            "frame", "vdp2_frame_serial", "field", "row", "layer", "screen_x",
            "pnd_source_valid", "pnd_access_allowed", "pnd_address", "raw_pnd",
            "char_address", "char_data_valid", "scroll_x", "scroll_y",
            "dot_address", "dot_value", "dot_value_valid", "priority",
        }
        if not needed.issubset(positions):
            raise VerificationError("Ymir pixel trace lacks required columns")
        for line in stream:
            values = split_csv_line(line)
            if len(values) != len(columns):
                raise VerificationError("malformed Ymir pixel trace row")
            rows_written += 1
            sample = {name: values[index] for name, index in positions.items()}
            frame = int(sample["frame"])
            serial = int(sample["vdp2_frame_serial"])
            if (frame, serial, int(sample["layer"]), int(sample["field"])) not in (
                (1278, 1279, 1, 0), (1279, 1280, 1, 0)
            ):
                continue
            key = (
                frame, serial, int(sample["row"]), int(sample["screen_x"])
            )
            if key in selected:
                raise VerificationError(f"duplicate traced pixel {key}")
            selected[key] = sample

    if rows_written != receipt["rows_written"]:
        raise VerificationError("Ymir trace row count disagrees with receipt")
    if len(selected) != PIXELS * 2:
        raise VerificationError(f"expected two full passes, got {len(selected)} pixels")

    source_counts = {"previous": 0, "current": 0}
    visible = 0
    inactive = 0
    target_source_counts = {"previous": 0, "current": 0}
    for frame, serial in ((1278, 1279), (1279, 1280)):
        for y in range(HEIGHT):
            for x in range(WIDTH):
                sample = selected[(frame, serial, y, x)]
                if (int(sample["frame"]), int(sample["vdp2_frame_serial"])) != (frame, serial):
                    raise VerificationError("trace pass identity or pixel coverage mismatch")
                if (sample["pnd_source_valid"] != "1" or
                        sample["pnd_access_allowed"] != "1" or
                        sample["char_data_valid"] != "1" or
                        sample["dot_value_valid"] != "1"):
                    raise VerificationError(f"invalid source fetch at ({x},{y})")

                pnd_address = int(sample["pnd_address"], 16)
                raw_pnd = int(sample["raw_pnd"], 16)
                matches = []
                if read_be32(previous_vram, pnd_address) == raw_pnd:
                    matches.append("previous")
                if read_be32(current_vram, pnd_address) == raw_pnd:
                    matches.append("current")
                if not matches:
                    raise VerificationError(f"PND misses both authentic snapshots at ({x},{y})")

                address = int(sample["dot_address"], 16)
                dot = int(sample["dot_value"])
                if not any(fetch_4bpp_dot(vram, sample) == (address, dot)
                           for vram, name in ((previous_vram, "previous"),
                                              (current_vram, "current"))
                           if name in matches):
                    raise VerificationError(f"4bpp nibble mismatch at ({x},{y})")

                priority = int(sample["priority"])
                if serial == 1279 and (priority != 0 or dot != 0):
                    raise VerificationError(
                        f"preceding pass is not transparent at ({x},{y})"
                    )
                if serial == 1280:
                    active = priority != 0
                    if active != (y >= 84):
                        raise VerificationError(
                            f"unexpected NBG1 visibility at ({x},{y})"
                        )
                    plane_offset = (y * WIDTH + x) * 4
                    if active:
                        visible += 1
                        if plane[plane_offset] != dot:
                            raise VerificationError(
                                f"Firestaff dot mismatch at ({x},{y}): "
                                f"{plane[plane_offset]} != {dot}"
                            )
                        if plane[plane_offset + 1 : plane_offset + 4] != \
                                reference[plane_offset : plane_offset + 3]:
                            raise VerificationError(
                                f"Firestaff NBG1 RGB mismatch at ({x},{y})"
                            )
                    else:
                        inactive += 1
                        if dot != 0:
                            raise VerificationError(
                                f"zero-priority target pixel has a nonzero dot at ({x},{y})"
                            )
                selected_source = (
                    "previous" if serial == 1279 or y < 84 else "current"
                )
                if serial == 1280 and selected_source not in matches:
                    raise VerificationError(
                        f"unexpected pass-source transition at ({x},{y})"
                    )
                source_counts[selected_source] += 1
                if serial == 1280:
                    target_source_counts[selected_source] += 1

    if visible != 44800 or inactive != 26880:
        raise VerificationError(f"unexpected NBG1 priority coverage: {visible}/{inactive}")
    if source_counts != {"previous": 98560, "current": 44800}:
        raise VerificationError(f"unexpected authentic pass-source counts: {source_counts}")
    if target_source_counts != {"previous": 26880, "current": 44800}:
        raise VerificationError(
            f"unexpected target pass-source counts: {target_source_counts}"
        )
    return visible, inactive, rows_written, target_source_counts


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: test_nexus_v1_vdp2_mapd_pixel_fetch_composition_real.py "
              "<runtime tilemap test> <build output plane> <build report>",
              file=sys.stderr)
        return 2

    root_text = os.environ.get("FIRESTAFF_NEXUS_YMIR_MAPD_COMPOSITION_DIR")
    if not root_text:
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_MAPD_COMPOSITION_DIR to the "
              "external authentic MAPD evidence directory")
        return 77

    root = Path(root_text)
    previous_path = root / "mapd-frame891-previous-vdp2-vram.bin"
    current_path = root / "vdp2-vram.bin"
    trace_path = root / "nexus-mapd-frame891-vdp2-pixels.csv"
    receipt_path = root / "nexus-mapd-frame891-vdp2-pixels.csv.receipt.json"
    cram_path = root / "vdp2-cram.bin"
    registers_path = root / "vdp2-registers.bin"
    reference_path_text = os.environ.get("FIRESTAFF_NEXUS_YMIR_MAPD_REFERENCE_FILE")
    if not reference_path_text:
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_MAPD_REFERENCE_FILE to the "
              "same-pass Ymir RGBX output")
        return 77
    reference_path = Path(reference_path_text)
    plane_path = Path(sys.argv[2])
    report_path = Path(sys.argv[3])

    try:
        previous = require_sha256(previous_path, PREVIOUS_VRAM_SHA256)
        current = require_sha256(current_path, CURRENT_VRAM_SHA256)
        require_sha256(cram_path, CRAM_SHA256)
        require_sha256(registers_path, REGISTERS_SHA256)
        require_sha256(trace_path, TRACE_SHA256)
        require_sha256(receipt_path, RECEIPT_SHA256)
        reference = require_sha256(reference_path, REFERENCE_SHA256)
        if len(previous) != 0x80000 or len(current) != 0x80000:
            raise VerificationError("authentic VDP2 snapshots must each be 512 KiB")
        if len(reference) != PIXELS * 4:
            raise VerificationError("same-pass Ymir RGBX reference must be 320x224")

        env = os.environ.copy()
        env.update({
            "FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_VRAM": str(current_path),
            "FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_CRAM": str(cram_path),
            "FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_REGS": str(registers_path),
            "FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_OUT": str(plane_path),
        })
        result = subprocess.run([sys.argv[1]], env=env, check=False)
        if result.returncode != 0:
            raise VerificationError("Firestaff's authentic NBG1 4bpp decode failed")

        plane = require_sha256(plane_path, PLANE_SHA256)
        if len(plane) != PIXELS * 4:
            raise VerificationError("Firestaff NBG1 plane must cover 320x224")
        visible, inactive, rows, target_sources = verify_trace(
            trace_path, receipt_path, previous, current, plane, reference
        )
        report = {
            "disc_sha256": "878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699",
            "save_state_sha256": "79b53c785456ddc8d4d19a5a510ecc008cc14bf08c0464ecfebb0d5849a40a52",
            "ipl_sha256": "dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac",
            "trace_sha256": sha256(trace_path),
            "trace_receipt_sha256": sha256(receipt_path),
            "previous_vram_sha256": sha256(previous_path),
            "target_vram_sha256": sha256(current_path),
            "cram_sha256": sha256(cram_path),
            "registers_sha256": sha256(registers_path),
            "firestaff_nbg1_plane_sha256": sha256(plane_path),
            "ymir_same_pass_rgbx_sha256": sha256(reference_path),
            "vdp1_frame": 891,
            "vdp2_frame_index": 1279,
            "vdp2_frame_serial": 1280,
            "dimensions": [WIDTH, HEIGHT],
            "trace_rows": rows,
            "active_nbg1_pixels": visible,
            "zero_priority_nbg1_pixels": inactive,
            "active_nbg1_dot_and_rgb_matches": visible,
            "target_pass_pnd_source_matches": target_sources,
            "active_nbg1_row_range_inclusive": [84, 223],
            "interpretation": (
                "authentic NBG1 layer contribution matches Ymir at visible pixels; "
                "not a full-frame composition or hardware-parity claim"
            ),
        }
        report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(
            "PASS: authentic MAPD NBG1 4bpp decode matches Ymir's exact "
            f"pixel source and same-pass RGB at {visible} visible pixels; "
            f"{inactive} zero-priority pixels stay excluded ({rows} trace rows). "
            "This validates the NBG1 contribution for one captured pass only."
        )
        return 0
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError,
            VerificationError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
