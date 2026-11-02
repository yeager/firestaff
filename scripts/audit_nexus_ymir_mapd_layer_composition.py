#!/usr/bin/env python3
"""Reconstruct one authentic Nexus MAPD Ymir pass from its per-layer trace."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import struct


WIDTH = 320
HEIGHT = 224
PIXELS = WIDTH * HEIGHT
MAGIC = b"YMLYR001"
HEADER_SIZE = 28
LAYER_COUNT = 7
TARGET_FRAME = 891
TARGET_SERIAL = 1280
EXPECTED_HASHES = {
    "layers": "086d660dce00d0df4f38a33fe1591043676d0e72482cd0e867540d215a494526",
    "render_state": "2dd3b31b0d0ab7bbf6df8e392094063e1ef97b2cbadccd2a575d3b69050de878",
    "render_state_receipt": "f48c424f0155fb0c5e5dae9b3daf6990adf3e84638ba44e39f997d4f4dafd392",
    "reference": "24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17",
}


class AuditError(ValueError):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require_hash(path: Path, key: str) -> None:
    actual = sha256(path)
    if actual != EXPECTED_HASHES[key]:
        raise AuditError(f"{key} SHA-256 mismatch: {actual}")


def read_layer_dump(path: Path) -> dict[tuple[int, int], tuple[tuple[int, ...], bytes, bytes]]:
    data = path.read_bytes()
    if not data.startswith(MAGIC):
        raise AuditError("Ymir layer dump does not start with YMLYR001")
    offset = len(MAGIC)
    rows: dict[tuple[int, int], tuple[tuple[int, ...], bytes, bytes]] = {}
    while offset < len(data):
        if offset + HEADER_SIZE > len(data):
            raise AuditError("truncated Ymir layer record header")
        frame, serial, _ordinal, row, _field, layer, width = struct.unpack_from(
            "<QQIIBBH", data, offset
        )
        offset += HEADER_SIZE
        if (frame, serial) != (TARGET_FRAME, TARGET_SERIAL):
            offset += width * 6
            continue
        if not (0 <= row < HEIGHT and 0 <= layer < LAYER_COUNT) or width != WIDTH:
            raise AuditError(f"invalid target layer record row={row} layer={layer} width={width}")
        key = (row, layer)
        if key in rows:
            raise AuditError(f"duplicate target layer record row={row} layer={layer}")
        colors = struct.unpack_from(f"<{width}I", data, offset)
        offset += width * 4
        priorities = data[offset : offset + width]
        offset += width
        special = data[offset : offset + width]
        offset += width
        rows[key] = (colors, priorities, special)
    if offset != len(data):
        raise AuditError("layer dump ended at an invalid record boundary")
    if len(rows) != HEIGHT * LAYER_COUNT:
        raise AuditError(f"expected {HEIGHT * LAYER_COUNT} target rows, found {len(rows)}")
    return rows


def read_render_state(path: Path, receipt: Path) -> tuple[list[int], list[bool], list[int]]:
    text = receipt.read_text(encoding="utf-8")
    required_receipt_fields = (
        "frame_range_half_open=891:892",
        "row_vcnt_range_half_open=0:224",
        "write_error=0",
        "rendered_frame_output_count=2",
    )
    if any(field not in text for field in required_receipt_fields):
        raise AuditError("Ymir render-state receipt lacks the expected complete target-pass bounds")
    ratios: list[int | None] = [None] * HEIGHT
    nbg0_enabled: list[bool | None] = [None] * HEIGHT
    with path.open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            if (row.get("frame_index"), row.get("vdp2_frame_serial")) != (
                "891", str(TARGET_SERIAL)
            ):
                continue
            y = int(row["row_vcnt"])
            ratio = int(row["color_calculation_ratio"])
            try:
                bg_enabled = {
                    key: int(row[key])
                    for key in ("nbg0_enabled", "layer_enabled", "rbg1_enabled")
                }
            except (KeyError, ValueError) as error:
                raise AuditError("target render-state row lacks valid NBG0 enable flags") from error
            if any(value not in (0, 1) for value in bg_enabled.values()):
                raise AuditError(f"invalid target NBG0 enable flags at row {y}")
            if not 0 <= y < HEIGHT or ratios[y] is not None:
                raise AuditError(f"duplicate or out-of-range render-state row {y}")
            # Ymir's layer 2 aliases NBG0 and RBG1. The render-state trace is
            # sampled immediately before NBG0 drawing, so use it to reject a
            # stale layer-2 buffer when the hardware layer is disabled. This
            # MAPD audit is specifically for NBG0, not the RBG1 path.
            if (bg_enabled["nbg0_enabled"] != 1 or
                    bg_enabled["layer_enabled"] != 1 or
                    bg_enabled["rbg1_enabled"] != 0):
                raise AuditError(f"target pass is not an enabled NBG0 row at {y}")
            ratios[y] = ratio
            nbg0_enabled[y] = bool(
                bg_enabled["layer_enabled"] and bg_enabled["nbg0_enabled"]
            )
    if any(value is None for value in ratios):
        raise AuditError("target render-state pass is missing rows")
    if any(value is None for value in nbg0_enabled):
        raise AuditError("target render-state pass is missing NBG0 enable state")
    return (
        [int(value) for value in ratios],
        [bool(value) for value in nbg0_enabled],
        [y for y, value in enumerate(ratios) if value != 18],
    )


def rgb_from_layer_color(color: int) -> tuple[int, int, int]:
    # Ymir's YMLYR layer colors pack BGR in the low three bytes.
    return color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF


def audit(layers_path: Path, render_state_path: Path, receipt_path: Path,
          reference_path: Path, output_path: Path | None) -> dict[str, object]:
    for key, path in (("layers", layers_path), ("render_state", render_state_path),
                      ("render_state_receipt", receipt_path), ("reference", reference_path)):
        require_hash(path, key)
    rows = read_layer_dump(layers_path)
    ratios, nbg0_enabled, unexpected_ratio_rows = read_render_state(
        render_state_path, receipt_path
    )
    if unexpected_ratio_rows:
        raise AuditError(f"unexpected NBG0 color-calculation ratio on rows {unexpected_ratio_rows[:8]}")
    reference = reference_path.read_bytes()
    if len(reference) != PIXELS * 4:
        raise AuditError("Ymir RGBX reference has the wrong 320x224 size")

    output = bytearray(PIXELS * 4)
    top_counts = [0] * LAYER_COUNT
    pair_counts: dict[str, int] = {}
    special_calculation_pixels = 0
    for y in range(HEIGHT):
        for x in range(WIDTH):
            candidates: list[tuple[int, int, int, int]] = []
            for layer in range(6):
                if layer == 2 and not nbg0_enabled[y]:
                    continue
                colors, priorities, special = rows[y, layer]
                priority = priorities[x]
                if priority:
                    candidates.append((priority, layer, colors[x], special[x]))
            # Ymir orders higher priority first and lower layer id first on ties.
            candidates.sort(key=lambda value: (-value[0], value[1]))
            top = candidates[0] if candidates else (0, 7, 0, 0)
            under = candidates[1] if len(candidates) > 1 else (0, 7, 0, 0)
            color = rgb_from_layer_color(top[2])
            if top[1] == 2 and top[3]:
                lower = rgb_from_layer_color(under[2])
                ratio = ratios[y]
                color = tuple(
                    lower[channel] + (((color[channel] - lower[channel]) * ratio) >> 5)
                    for channel in range(3)
                )
                special_calculation_pixels += 1
            top_counts[top[1] if top[1] < LAYER_COUNT else 6] += 1
            pair = f"{top[1]}>{under[1]}"
            pair_counts[pair] = pair_counts.get(pair, 0) + 1
            offset = (y * WIDTH + x) * 4
            output[offset : offset + 4] = bytes((*color, 0xFF))

    exact = 0
    for pixel in range(PIXELS):
        offset = pixel * 4
        exact += output[offset : offset + 3] == reference[offset : offset + 3]
    result = {
        "result": "PASS" if exact == PIXELS else "MISMATCH",
        "frame": TARGET_FRAME,
        "vdp2_serial": TARGET_SERIAL,
        "dimensions": [WIDTH, HEIGHT],
        "pixels": PIXELS,
        "exact_rgb_pixels": exact,
        "rgb_mismatches": PIXELS - exact,
        "special_color_calculation_pixels": special_calculation_pixels,
        "nbg0_enabled_rows": sum(nbg0_enabled),
        "rbg1_enabled_rows": 0,
        "top_layer_pixels_by_id": top_counts,
        "top_under_layer_pairs": pair_counts,
        "layer_trace_sha256": EXPECTED_HASHES["layers"],
        "render_state_sha256": EXPECTED_HASHES["render_state"],
        "reference_sha256": EXPECTED_HASHES["reference"],
        "interpretation": (
            "authentic Ymir layer candidates reconstruct this Ymir MAPD pass "
            "with row-local NBG0 enablement checked; candidate rows do not "
            "prove the active-layer set for every Ymir layer or Firestaff "
            "raw-snapshot composition"
        ),
    }
    if output_path:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_bytes(output)
        result["reconstructed_rgbx_sha256"] = sha256(output_path)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--layers", type=Path, required=True)
    parser.add_argument("--render-state", type=Path, required=True)
    parser.add_argument("--render-state-receipt", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--output-rgbx", type=Path)
    args = parser.parse_args()
    try:
        result = audit(args.layers, args.render_state, args.render_state_receipt,
                       args.reference, args.output_rgbx)
    except (OSError, ValueError, KeyError, struct.error) as error:
        parser.error(str(error))
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["result"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
