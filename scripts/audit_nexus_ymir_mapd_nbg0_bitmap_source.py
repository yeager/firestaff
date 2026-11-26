#!/usr/bin/env python3
"""Join authentic MAPD NBG0 bitmap bytes to Ymir's per-pixel layer trace."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct


WIDTH = 320
HEIGHT = 224
PIXELS = WIDTH * HEIGHT
MAGIC = b"YMLYR001"
EXPECTED_HASHES = {
    "vram": "7367347e6adcb72ac09c946d03a15e1486e12f32593cee7bf3992e58da3b3d9f",
    "cram": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
    "registers": "774fa7f2e815b14770b83feb83e8c15406041a931e65ed465b1062944a811dd1",
    "layers": "086d660dce00d0df4f38a33fe1591043676d0e72482cd0e867540d215a494526",
    "render_state": "2dd3b31b0d0ab7bbf6df8e392094063e1ef97b2cbadccd2a575d3b69050de878",
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


def read_layer_rows(path: Path) -> dict[int, tuple[tuple[int, ...], bytes, bytes]]:
    data = path.read_bytes()
    if not data.startswith(MAGIC):
        raise AuditError("layer trace is not YMLYR001")
    offset = len(MAGIC)
    rows: dict[int, tuple[tuple[int, ...], bytes, bytes]] = {}
    while offset < len(data):
        frame, serial, _ordinal, row, _field, layer, width = struct.unpack_from(
            "<QQIIBBH", data, offset
        )
        offset += 28
        if width != WIDTH:
            raise AuditError(f"unexpected layer width {width}")
        if (frame, serial, layer) == (891, 1280, 2):
            if not 0 <= row < HEIGHT or row in rows:
                raise AuditError(f"duplicate or invalid NBG0 row {row}")
            colors = struct.unpack_from(f"<{WIDTH}I", data, offset)
            offset += WIDTH * 4
            priorities = data[offset : offset + WIDTH]
            offset += WIDTH
            special = data[offset : offset + WIDTH]
            offset += WIDTH
            rows[row] = colors, priorities, special
        else:
            offset += WIDTH * 6
    if offset != len(data) or len(rows) != HEIGHT:
        raise AuditError(f"expected 224 NBG0 rows, got {len(rows)}")
    return rows


def read_scroll_rows(path: Path) -> list[dict[str, int]]:
    import csv

    rows: dict[int, dict[str, int]] = {}
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        for record in reader:
            if (record["frame_index"], record["vdp2_frame_serial"]) != ("891", "1280"):
                continue
            y = int(record["row_vcnt"])
            if not 0 <= y < HEIGHT or y in rows:
                raise AuditError(f"duplicate or invalid render-state row {y}")
            rows[y] = {
                key: int(record[key])
                for key in (
                    "scroll_h_11_8", "scroll_v_11_8", "scroll_inc_h_11_8",
                    "scroll_inc_v_11_8", "fractional_scroll_x_11_8",
                    "fractional_scroll_y_11_8",
                )
            }
    if len(rows) != HEIGHT:
        raise AuditError(f"expected 224 render-state rows, got {len(rows)}")
    return [rows[y] for y in range(HEIGHT)]


def audit(vram_path: Path, cram_path: Path, registers_path: Path,
          layers_path: Path, render_state_path: Path) -> dict[str, object]:
    for key, path in (("vram", vram_path), ("cram", cram_path),
                      ("registers", registers_path), ("layers", layers_path),
                      ("render_state", render_state_path)):
        require_hash(path, key)
    vram = vram_path.read_bytes()
    cram = cram_path.read_bytes()
    registers = registers_path.read_bytes()
    if len(vram) != 0x80000 or len(cram) != 0x1000 or len(registers) != 512:
        raise AuditError("authentic VDP snapshot has an unexpected size")
    reg = lambda offset: int.from_bytes(registers[offset : offset + 2], "big")
    if (reg(0x00), reg(0x20), reg(0x28), reg(0x2C), reg(0x3C), reg(0xE4)) != (
        0x8000, 0x0002, 0x0013, 0, 0, 0
    ):
        raise AuditError("register snapshot does not match the authenticated MAPD bitmap profile")
    layer_rows = read_layer_rows(layers_path)
    scroll_rows = read_scroll_rows(render_state_path)

    active = transparent = active_priority_mismatches = 0
    active_special_mismatches = active_rgb_mismatches = 0
    first_mismatches: list[dict[str, object]] = []
    for y in range(HEIGHT):
        colors, priorities, special = layer_rows[y]
        state = scroll_rows[y]
        for x in range(WIDTH):
            source_x = (
                state["fractional_scroll_x_11_8"] + state["scroll_h_11_8"] +
                x * state["scroll_inc_h_11_8"]
            ) >> 8
            source_y = (
                state["fractional_scroll_y_11_8"] + state["scroll_v_11_8"]
            ) >> 8
            source_x &= 511
            source_y &= 511
            color_index = vram[source_y * 512 + source_x]
            offset = (y * WIDTH + x) * 4
            if color_index == 0:
                transparent += 1
                if priorities[x] != 0 or special[x] != 0:
                    active_priority_mismatches += 1
                continue
            active += 1
            if priorities[x] != 4:
                active_priority_mismatches += 1
            if special[x] != 1:
                active_special_mismatches += 1
            cram_word = int.from_bytes(cram[color_index * 2 : color_index * 2 + 2], "big")
            rgb = tuple(((cram_word >> shift) & 31) << 3 for shift in (0, 5, 10))
            layer_rgb = (colors[x] & 0xFF, (colors[x] >> 8) & 0xFF,
                         (colors[x] >> 16) & 0xFF)
            if rgb != layer_rgb:
                active_rgb_mismatches += 1
                if len(first_mismatches) < 8:
                    first_mismatches.append({
                        "x": x, "y": y, "source_x": source_x,
                        "source_y": source_y, "index": color_index,
                        "cram_word": cram_word, "decoded_rgb": rgb,
                        "ymir_layer_rgb": layer_rgb,
                    })

    return {
        "result": "PASS" if (active, transparent) == (20302, 51378) and
                  not (active_priority_mismatches or active_special_mismatches or
                       active_rgb_mismatches) else "MISMATCH",
        "frame": 891,
        "vdp2_serial": 1280,
        "dimensions": [WIDTH, HEIGHT],
        "pixels": PIXELS,
        "source": "8bpp indexed NBG0 bitmap at VRAM 0, pitch 512 bytes, CRAM index equal to dot",
        "active_pixels": active,
        "transparent_index_zero_pixels": transparent,
        "priority_or_transparency_mismatches": active_priority_mismatches,
        "special_color_calc_mismatches": active_special_mismatches,
        "active_rgb_mismatches": active_rgb_mismatches,
        "first_rgb_mismatches": first_mismatches,
        "vram_sha256": EXPECTED_HASHES["vram"],
        "cram_sha256": EXPECTED_HASHES["cram"],
        "registers_sha256": EXPECTED_HASHES["registers"],
        "interpretation": "raw NBG0 source bytes match the authentic Ymir candidate layer using captured per-row scroll state; full Firestaff composition remains unverified",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vram", type=Path, required=True)
    parser.add_argument("--cram", type=Path, required=True)
    parser.add_argument("--registers", type=Path, required=True)
    parser.add_argument("--layers", type=Path, required=True)
    parser.add_argument("--render-state", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = audit(args.vram, args.cram, args.registers,
                       args.layers, args.render_state)
    except (OSError, ValueError, KeyError, struct.error) as error:
        parser.error(str(error))
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["result"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
