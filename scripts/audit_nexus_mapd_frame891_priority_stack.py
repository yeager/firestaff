#!/usr/bin/env python3
"""Audit the retained authentic MAPD frame-891 priority-stack mismatch."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path


WIDTH = 320
HEIGHT = 224
PIXELS = WIDTH * HEIGHT
EXPECTED = {
    "stack": "cc6c926860104797078645bb3fd784c78fc6ad0f768aa2556cfa4fecdc68cddc",
    "reference": "24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17",
    "trace": "e18e487e4352a39b1409c3d829345e1587a8dec468bf85723c579da970121b7d",
    "receipt": "bb6c8e5822ed5b2a6b33782d4e6d0706ddb711bade5f88bab8dab4e44d55c876",
    "row_source_plane": "359f0ce1a93298f3117cf9c9b6c27550ed3b68ed0c3f65fd4d3e31aa7a42cf9d",
}


class AuditError(ValueError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AuditError(message)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def check_hash(path: Path, key: str) -> None:
    actual = sha256(path)
    require(actual == EXPECTED[key], f"{key} SHA-256 mismatch: {actual}")


def audit(stack_path: Path, reference_path: Path,
          trace_path: Path, receipt_path: Path,
          row_source_plane_path: Path) -> dict[str, object]:
    for key, path in (("stack", stack_path), ("reference", reference_path),
                      ("trace", trace_path), ("receipt", receipt_path),
                      ("row_source_plane", row_source_plane_path)):
        check_hash(path, key)

    stack = stack_path.read_bytes()
    reference = reference_path.read_bytes()
    row_source_plane = row_source_plane_path.read_bytes()
    require(len(stack) == PIXELS * 4 and len(reference) == PIXELS * 4,
            "stack or Ymir reference has the wrong 320x224 RGBA size")
    require(len(row_source_plane) == PIXELS * 4,
            "row-source plane has the wrong 320x224 RGBA size")
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    require(receipt.get("complete") is True and
            receipt.get("rows_written") == 143360 and
            receipt.get("rows_dropped") == 0 and
            receipt.get("write_errors") == 0 and
            receipt.get("frame_range") == [1278, 1280] and
            receipt.get("row_range") == [0, 224] and
            receipt.get("x_range") == [0, 320],
            "Ymir pixel-fetch receipt is incomplete or has different bounds")

    seen = bytearray(PIXELS)
    zero_priority_pixels = active_trace_pixels = 0
    priority_by_pixel = bytearray(PIXELS)
    with trace_path.open(newline="", encoding="ascii") as stream:
        require(stream.readline().startswith("# ymir-vdp2-pixel-fetch-csv-v1"),
                "unexpected Ymir pixel-fetch trace format")
        rows = csv.DictReader(stream)
        expected_fields = {
            "frame", "vdp2_frame_serial", "screen_x", "row", "priority",
            "dot_value", "dot_value_valid",
        }
        require(expected_fields.issubset(rows.fieldnames or []),
                "Ymir pixel-fetch CSV is missing required fields")
        row_count = 0
        for row in rows:
            if row["frame"] != "1279" or row["vdp2_frame_serial"] != "1280":
                continue
            x, y = int(row["screen_x"]), int(row["row"])
            require(0 <= x < WIDTH and 0 <= y < HEIGHT,
                    "target pass has out-of-range pixel coordinates")
            offset = y * WIDTH + x
            require(not seen[offset], f"duplicate target pixel at ({x},{y})")
            seen[offset] = 1
            row_count += 1
            require(row["dot_value_valid"] == "1",
                    f"invalid target dot at ({x},{y})")
            priority, dot = int(row["priority"]), int(row["dot_value"])
            require(priority in (0, 5),
                    f"unexpected authentic NBG1 priority {priority} at ({x},{y})")
            require((priority != 0) or dot == 0,
                    f"zero-priority trace has nonzero dot at ({x},{y})")
            priority_by_pixel[offset] = priority
            if priority == 0:
                zero_priority_pixels += 1
            else:
                active_trace_pixels += 1

    require(row_count == PIXELS and all(seen),
            f"expected {PIXELS} unique target pixel samples, got {row_count}")
    require((zero_priority_pixels, active_trace_pixels) == (26880, 44800),
            "unexpected authentic NBG1 visibility counts")

    trace = bytearray(PIXELS)
    with trace_path.open(newline="", encoding="ascii") as stream:
        stream.readline()
        for row in csv.DictReader(stream):
            if row["frame"] == "1279" and row["vdp2_frame_serial"] == "1280":
                trace[int(row["row"]) * WIDTH + int(row["screen_x"])] = 1
    require(all(trace), "target pass has missing per-pixel trace coordinates")

    row_source_matches = row_source_active_matches = 0
    stack_matches = priority_mismatches = 0
    for pixel in range(PIXELS):
        off = pixel * 4
        plane_rgb = row_source_plane[off + 1:off + 4]
        reference_rgb = reference[off:off + 3]
        if priority_by_pixel[pixel] == 5:
            row_source_active_matches += int(plane_rgb == reference_rgb)
        row_source_matches += int(plane_rgb == reference_rgb)
        stack_matches += int(stack[off + 1:off + 4] == reference_rgb)
        # The retained NBG1-only stack labels every pixel NBG1 (layer 2).
        priority_mismatches += int(stack[off] == 2 and priority_by_pixel[pixel] == 0)

    require(row_source_active_matches == 44800,
            f"row-source plane active match changed: {row_source_active_matches}/44800")
    require(priority_mismatches == zero_priority_pixels,
            "retained stack layer ownership changed unexpectedly")
    return {
        "result": "KNOWN_CAPTURE_ONLY_MISMATCH",
        "stack_code_head": "8f3b5841de3387c60fb5808ad35e7fbd7ae28666",
        "stack_sha256": EXPECTED["stack"],
        "ymir_reference_sha256": EXPECTED["reference"],
        "ymir_target_pass": {"vdp1_frame": 891, "vdp2_serial": 1280},
        "trace_visibility": {
            "zero_priority_and_dot_pixels": zero_priority_pixels,
            "active_nbg1_priority_5_pixels": active_trace_pixels,
        },
        "retained_stack": {
            "row_source_plane_rgb_matches": row_source_matches,
            "active_row_source_plane_rgb_matches": row_source_active_matches,
            "stack_rgb_matches": stack_matches,
            "stack_rgb_mismatches": PIXELS - stack_matches,
            "stack_nbg1_owner_on_trace_zero_priority_pixels": priority_mismatches,
        },
        "interpretation": (
            "Authentic per-pixel priority and dot state shows that the retained "
            "NBG1-only stack claims NBG1 ownership even where Ymir reports "
            "zero NBG1 priority. The row-source plane matches visible NBG1 "
            "pixels; whole-frame VDP1/VDP2 composition remains unverified."
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stack", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--trace", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--row-source-plane", type=Path, required=True)
    args = parser.parse_args()
    try:
        report = audit(args.stack, args.reference, args.trace, args.receipt,
                       args.row_source_plane)
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
