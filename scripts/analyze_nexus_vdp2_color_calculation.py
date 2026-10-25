#!/usr/bin/env python3
"""Report VDP2 layer gates and global mixer mode from an authentic register dump.

The input is Ymir's 512-byte little-endian VDP2 register span. This tool only
interprets captured register bytes; it does not reconstruct pixels or admit a
renderer path.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


REGISTER_BYTES = 0x200


class CaptureError(ValueError):
    pass


def read_capture(path: Path) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise CaptureError(f"cannot read {path}: {error}") from error
    if len(data) != REGISTER_BYTES:
        raise CaptureError(
            f"{path}: expected {REGISTER_BYTES} register bytes, got {len(data)}"
        )
    return data


def register(data: bytes, address: int) -> int:
    return int.from_bytes(data[address:address + 2], "little")


def report(data: bytes) -> None:
    tvmd = register(data, 0x000)
    bgon = register(data, 0x020)
    ccctl = register(data, 0x0EC)
    sfccmd = register(data, 0x0EE)
    ccrna = register(data, 0x108)

    print(f"TVMD=0x{tvmd:04x}")
    print(f"BGON=0x{bgon:04x}")
    print(f"CCCTL=0x{ccctl:04x}")
    print(f"SFCCMD=0x{sfccmd:04x}")
    print(f"CCRNA=0x{ccrna:04x}")

    layers = (
        ("NBG0", 0),
        ("NBG1", 1),
        ("NBG2", 2),
        ("NBG3", 3),
        ("RBG0", 4),
    )
    admitted: list[str] = []
    for name, bit in layers:
        enabled = bool(ccctl & (1 << bit))
        special_mode = (sfccmd >> (bit * 2)) & 0x3
        print(
            f"{name}_color_calculation="
            f"{'enabled' if enabled else 'disabled'}"
        )
        print(f"{name}_sfccmd_mode={special_mode}")
        if enabled:
            admitted.append(name)

    sprite_enabled = bool(ccctl & (1 << 6))
    ccmd = bool(ccctl & (1 << 8))
    ccrtmd = bool(ccctl & (1 << 9))
    if sprite_enabled:
        admitted.append("SPRITES")
    print(f"SPRITES_color_calculation={'enabled' if sprite_enabled else 'disabled'}")
    print(f"CCMD={'additive_saturating' if ccmd else 'ratio'}")
    print(f"CCRTMD={'source_pixel_ratio' if ccrtmd else 'register_ratio'}")
    print(f"CCRNA=0x{ccrna:04x}")
    print(f"ratio_applied={'no' if ccmd else 'yes'}")
    print(f"color_calculation_layers={','.join(admitted) if admitted else 'none'}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "registers", type=Path,
        help="authentic 512-byte Ymir VDP2 register dump (little-endian uint16)",
    )
    args = parser.parse_args(argv)
    try:
        data = read_capture(args.registers)
    except CaptureError as error:
        print(f"NEXUS_VDP2_COLOR_CALCULATION_INVALID: {error}", file=sys.stderr)
        return 1
    report(data)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
