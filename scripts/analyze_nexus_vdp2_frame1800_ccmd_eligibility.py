#!/usr/bin/env python3
"""Check whether authentic Ymir frame 1800 contains CCMD-eligible pixels.

This is a capture audit, not a compositor. It joins Ymir's per-pixel layer
trace to the same-frame VDP2 pattern-name data and sprite priority settings.
The fixed hashes and register profile prevent a similarly named capture from
being mistaken for the verified frame.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import struct
import sys


EXPECTED = {
    "vdp2-vram.bin":
        "317915dac02650cc13283cf9dce6c89c0e1d1520428a7c242245795598890faa",
    "vdp2-regs-le16.bin":
        "82ddd6c84bfff8f6a7bf0e32a2b7965bca54ee104eea15a8ca2c70078d177b40",
    "nexus-frame-1800.composition.v1.bin":
        "7bdd93ba2f92a35f7a621109b98b7941ac00421dcc22fc44063df8786c44555d",
}
TRACE_HEADER_SIZE = 32
TRACE_RECORD_SIZE = 24
TRACE_WIDTH = 320
TRACE_HEIGHT = 224
NBG1_TRACE_LAYER = 3
SPRITE_TRACE_LAYER = 0
NBG1_MAP_BASE = 0x5C000
PND_COLOR_CALC_ENABLE = 0x1000


class AuditError(ValueError):
    pass


def read_verified(path: Path, expected_sha256: str) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise AuditError(f"cannot read {path}: {error}") from error
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected_sha256:
        raise AuditError(
            f"{path}: SHA-256 mismatch; expected {expected_sha256}, got {actual}"
        )
    return data


def read_trace(data: bytes) -> tuple[int, int, int, int, int]:
    if len(data) < TRACE_HEADER_SIZE or data[:8] != b"YMCMP1\0\0":
        raise AuditError("composition trace has no YMCMP1 header")
    frame, = struct.unpack_from("<Q", data, 8)
    width, height, record_size, header_size = struct.unpack_from("<4I", data, 16)
    expected_size = header_size + height * (4 + width * record_size)
    if (frame != 1800 or width != TRACE_WIDTH or height != TRACE_HEIGHT or
            record_size != TRACE_RECORD_SIZE or
            header_size != TRACE_HEADER_SIZE or len(data) != expected_size):
        raise AuditError(
            f"unexpected trace dimensions/header: frame={frame}, "
            f"size={width}x{height}, record={record_size}, "
            f"header={header_size}, bytes={len(data)}"
        )
    return frame, width, height, record_size, header_size


def u16le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "little")


def audit(vram: bytes, registers: bytes, trace: bytes) -> None:
    if len(vram) != 0x80000 or len(registers) != 0x200:
        raise AuditError("unexpected VDP2 VRAM or register dump size")

    bgon = u16le(registers, 0x20)
    map1a = u16le(registers, 0x44)
    map1b = u16le(registers, 0x46)
    ccctl = u16le(registers, 0xEC)
    sfccmd = u16le(registers, 0xEE)
    spctl = u16le(registers, 0xE0)
    if (bgon, map1a, map1b, ccctl, sfccmd, spctl) != (
            0x0007, 0x1717, 0x1717, 0x0142, 0x0004, 0x1536):
        raise AuditError(
            "frame-1800 register profile changed: "
            f"BGON={bgon:#06x}, MAP1A/B={map1a:#06x}/{map1b:#06x}, "
            f"CCCTL={ccctl:#06x}, SFCCMD={sfccmd:#06x}, SPCTL={spctl:#06x}"
        )

    frame, width, height, record_size, header_size = read_trace(trace)
    nbg1_pixels = 0
    nbg1_cce_pixels = 0
    sprite_priorities: dict[int, int] = {}
    final_matches_slot0 = 0
    position = header_size
    for y in range(height):
        row, = struct.unpack_from("<I", trace, position)
        position += 4
        if row != y:
            raise AuditError(f"row index mismatch at {y}: trace stores {row}")
        for x in range(width):
            record = trace[position:position + record_size]
            position += record_size
            layer0 = record[0]
            priority0 = record[1]
            color0, = struct.unpack_from("<I", record, 2)
            final_color, = struct.unpack_from("<I", record, 20)
            if (final_color & 0xFFFFFF) == (color0 & 0xFFFFFF):
                final_matches_slot0 += 1

            if layer0 == NBG1_TRACE_LAYER:
                nbg1_pixels += 1
                cell_index = (y // 8) * 64 + (x // 8)
                pnd_address = NBG1_MAP_BASE + cell_index * 4
                pnd_first_word = u16le(vram, pnd_address)
                if pnd_first_word & PND_COLOR_CALC_ENABLE:
                    nbg1_cce_pixels += 1
            elif layer0 == SPRITE_TRACE_LAYER:
                sprite_priorities[priority0] = (
                    sprite_priorities.get(priority0, 0) + 1
                )

    sprite_cc_enable = bool(ccctl & 0x0040)
    sprite_cc_condition = (spctl >> 12) & 0x3
    sprite_cc_value = (spctl >> 8) & 0x7
    # SPCTL condition 1 is priority equality. The other condition modes need
    # separate source and pixel evidence and are intentionally not inferred.
    eligible_sprite_pixels = (
        sum(count for priority, count in sprite_priorities.items()
            if priority == sprite_cc_value)
        if sprite_cc_enable and sprite_cc_condition == 1 else 0
    )

    print(f"frame={frame} dimensions={width}x{height}")
    print(f"ccctl={ccctl:#06x} ccmd_additive={int(bool(ccctl & 0x0100))}")
    print(f"nbg1_cc_enable={int(bool(ccctl & 0x0002))} "
          f"sfccmd_nbg1={(sfccmd >> 2) & 0x3}")
    print(f"top_nbg1_pixels={nbg1_pixels} "
          f"top_nbg1_pixels_with_pnd_cce={nbg1_cce_pixels}")
    print(f"sprite_cc_enable={int(sprite_cc_enable)} "
          f"spctl_condition={sprite_cc_condition} "
          f"spctl_priority_value={sprite_cc_value}")
    print("top_sprite_pixels_by_priority=" + ",".join(
        f"{priority}:{count}" for priority, count in sorted(sprite_priorities.items())
    ))
    print(f"top_sprite_pixels_matching_spctl={eligible_sprite_pixels}")
    print(f"final_rgb_matches_trace_slot0={final_matches_slot0}/{width * height}")
    if nbg1_pixels != 148 or nbg1_cce_pixels != 0 or \
            eligible_sprite_pixels != 0 or final_matches_slot0 != width * height:
        raise AuditError("authentic frame-1800 eligibility receipt changed")
    print("result=no_visible_nbg1_or_sprite_pixel_enters_ccmd_additive_path")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_dir", type=Path,
                        help="authentic frame-1800 VDP2 capture directory")
    parser.add_argument("composition_trace", type=Path,
                        help="hash-pinned authentic YMCMP1 trace")
    args = parser.parse_args()
    try:
        vram = read_verified(args.capture_dir / "nexus-frame-1800.vdp2-vram.bin",
                             EXPECTED["vdp2-vram.bin"])
        registers = read_verified(
            args.capture_dir / "nexus-frame-1800.vdp2-regs-le16.bin",
            EXPECTED["vdp2-regs-le16.bin"])
        trace = read_verified(args.composition_trace,
                              EXPECTED["nexus-frame-1800.composition.v1.bin"])
        audit(vram, registers, trace)
    except AuditError as error:
        print(f"NEXUS_VDP2_CCMD_ELIGIBILITY_INVALID: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
