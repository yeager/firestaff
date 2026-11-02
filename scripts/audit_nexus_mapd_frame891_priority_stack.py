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
          trace_path: Path, receipt_path: Path) -> dict[str, object]:
    for key, path in (("stack", stack_path), ("reference", reference_path),
                      ("trace", trace_path), ("receipt", receipt_path)):
        check_hash(path, key)

    stack = stack_path.read_bytes()
    reference = reference_path.read_bytes()
    require(len(stack) == PIXELS * 4 and len(reference) == PIXELS * 4,
            "stack or Ymir reference has the wrong 320x224 RGBA size")
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
    upper_trace_pixels = active_trace_pixels = 0
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
            if y < 84:
                require(priority == 0 and dot == 0,
                        f"Ymir upper-row sample is not transparent at ({x},{y})")
                upper_trace_pixels += 1
            else:
                require(priority == 5,
                        f"unexpected active NBG1 priority at ({x},{y})")
                active_trace_pixels += 1

    require(row_count == PIXELS and all(seen),
            f"expected {PIXELS} unique target pixel samples, got {row_count}")
    require((upper_trace_pixels, active_trace_pixels) == (26880, 44800),
            "unexpected authentic NBG1 visibility counts")

    stack_layers = {stack[index] for index in range(0, len(stack), 4)}
    require(stack_layers == {2},
            f"retained Firestaff stack layer ownership changed: {sorted(stack_layers)}")
    upper_matches = active_matches = 0
    for pixel in range(PIXELS):
        y = pixel // WIDTH
        equal = stack[pixel * 4 + 1:pixel * 4 + 4] == \
            reference[pixel * 4:pixel * 4 + 3]
        if y < 84:
            upper_matches += int(equal)
        else:
            active_matches += int(equal)

    require((upper_matches, active_matches) == (11, 44800),
            f"retained stack comparison changed: {upper_matches}/26880 upper, "
            f"{active_matches}/44800 active RGB matches")
    return {
        "result": "KNOWN_CAPTURE_ONLY_MISMATCH",
        "stack_code_head": "8f3b5841de3387c60fb5808ad35e7fbd7ae28666",
        "stack_sha256": EXPECTED["stack"],
        "ymir_reference_sha256": EXPECTED["reference"],
        "ymir_target_pass": {"vdp1_frame": 891, "vdp2_serial": 1280},
        "trace_visibility": {
            "zero_priority_and_dot_pixels": upper_trace_pixels,
            "active_nbg1_priority_5_pixels": active_trace_pixels,
        },
        "retained_stack": {
            "unique_layer_owners": sorted(stack_layers),
            "upper_row_rgb_matches": upper_matches,
            "upper_row_rgb_mismatches": 26880 - upper_matches,
            "active_row_rgb_matches": active_matches,
            "active_row_rgb_mismatches": 44800 - active_matches,
        },
        "interpretation": (
            "The retained stack reproduces active NBG1 pixels but uses NBG1 "
            "for Ymir-transparent upper rows. This is a known whole-frame "
            "composition gap, not a parity pass. Do not repair it with a "
            "fixed row clip; capture row-time VRAM source state."
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stack", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--trace", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    try:
        report = audit(args.stack, args.reference, args.trace, args.receipt)
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
