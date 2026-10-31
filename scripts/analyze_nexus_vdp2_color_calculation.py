#!/usr/bin/env python3
"""Report VDP2 layer gates and global mixer mode from an authentic register dump.

The input is Ymir's 512-byte VDP2 register span. Byte order is inferred from
TVMD, BGON, and CHCTLA unless explicitly selected. This tool only interprets
captured register bytes; it does not reconstruct pixels or admit a renderer
path.
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
        with path.open("rb") as capture:
            data = capture.read(REGISTER_BYTES + 1)
    except OSError as error:
        raise CaptureError(f"cannot read {path}: {error}") from error
    if len(data) != REGISTER_BYTES:
        size = f"at least {len(data)}" if len(data) > REGISTER_BYTES else str(len(data))
        raise CaptureError(
            f"{path}: expected {REGISTER_BYTES} register bytes, got {size}"
        )
    return data


def register(data: bytes, address: int, byte_order: str) -> int:
    return int.from_bytes(data[address:address + 2], byte_order)


def register_score(data: bytes, byte_order: str) -> int:
    tvmd = register(data, 0x000, byte_order)
    bgon = register(data, 0x020, byte_order)
    chctla = register(data, 0x028, byte_order)
    score = 0
    if tvmd & 0x8000:
        score += 3
    if bgon & 0x001F:
        score += 4
    if not bgon & ~0x1F3F:
        score += 1
    if bgon & 0x0002:
        score += 2
        if chctla & 0x0200:
            score += 1
    return score


def detect_byte_order(data: bytes) -> str:
    big_score = register_score(data, "big")
    little_score = register_score(data, "little")
    return "little" if little_score > big_score else "big"


def report(data: bytes, byte_order: str) -> None:
    tvmd = register(data, 0x000, byte_order)
    bgon = register(data, 0x020, byte_order)
    ccctl = register(data, 0x0EC, byte_order)
    sfccmd = register(data, 0x0EE, byte_order)
    ccrna = register(data, 0x108, byte_order)

    print(f"register_byte_order={byte_order}")
    print(f"TVMD=0x{tvmd:04x}")
    print(f"BGON=0x{bgon:04x}")
    print(f"CCCTL=0x{ccctl:04x}")
    print(f"SFCCMD=0x{sfccmd:04x}")
    print(f"CCRNA=0x{ccrna:04x}")

    layers = (
        ("NBG0", 0, 0),
        ("NBG1", 1, 1),
        ("NBG2", 2, 2),
        ("NBG3", 3, 3),
        ("RBG0", 4, 4),
        # Rotation background 1 reuses NBG0's CCCTL and SFCCMD selector.
        ("RBG1", 5, 0),
    )
    eligible_backgrounds: list[str] = []
    ccctl_gates: list[str] = []
    for name, bgon_bit, ccctl_bit in layers:
        background_enabled = bool(bgon & (1 << bgon_bit))
        calculation_enabled = bool(ccctl & (1 << ccctl_bit))
        if calculation_enabled:
            ccctl_gates.append(name)
        special_mode = (sfccmd >> (ccctl_bit * 2)) & 0x3
        print(
            f"{name}_background={'enabled' if background_enabled else 'disabled'}"
        )
        print(f"{name}_ccctl_gate={'enabled' if calculation_enabled else 'disabled'}")
        print(f"{name}_sfccmd_mode={special_mode}")
        if background_enabled and calculation_enabled:
            eligible_backgrounds.append(name)

    sprite_enabled = bool(ccctl & (1 << 6))
    ccmd = bool(ccctl & (1 << 8))
    ccrtmd = bool(ccctl & (1 << 9))
    if sprite_enabled:
        ccctl_gates.append("SPRITES")
    print(f"SPRITES_ccctl_gate={'enabled' if sprite_enabled else 'disabled'}")
    print(f"CCMD={'additive_saturating' if ccmd else 'ratio'}")
    print(f"CCRTMD={'source_pixel_ratio' if ccrtmd else 'register_ratio'}")
    print(f"ratio_applied={'no' if ccmd else 'yes'}")
    print(f"ccctl_color_calculation_gates={','.join(ccctl_gates) if ccctl_gates else 'none'}")
    print(
        "eligible_background_calculation="
        f"{','.join(eligible_backgrounds) if eligible_backgrounds else 'none'}"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "registers", type=Path,
        help="authentic 512-byte Ymir VDP2 register dump",
    )
    parser.add_argument(
        "--byte-order", choices=("auto", "big", "little"), default="auto",
        help="register word byte order (default: infer from TVMD/BGON/CHCTLA)",
    )
    args = parser.parse_args(argv)
    try:
        data = read_capture(args.registers)
    except CaptureError as error:
        print(f"NEXUS_VDP2_COLOR_CALCULATION_INVALID: {error}", file=sys.stderr)
        return 1
    byte_order = detect_byte_order(data) if args.byte_order == "auto" else args.byte_order
    report(data, byte_order)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
