#!/usr/bin/env python3
"""Inventory CCE-marked NBG1 cells in paired authentic VDP2 captures.

This scans raw Ymir/Mednafen VDP2 snapshots. It does not synthesize capture
data or prove that a marked cell contributes a nontransparent pixel.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from analyze_nexus_vdp2_composition import visible_character_spans
from nexus_vdp2_registers import detect_byte_order, read_u16


VRAM_SUFFIX = "vdp2-vram.bin"
REGISTER_SUFFIX = "vdp2-registers.bin"
VDP2_VRAM_BYTES = 0x80000
NBG1_ENABLE = 0x0002
NBG1_BITMAP_MODE = 0x0200
PND_CCE = 0x1000


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_root", type=Path,
                        help="root directory containing paired VDP2 snapshots")
    parser.add_argument("--width", type=int, default=320)
    parser.add_argument("--height", type=int, default=224)
    args = parser.parse_args()
    if not args.capture_root.is_dir():
        parser.error(f"capture root is not a directory: {args.capture_root}")
    if args.width <= 0 or args.height <= 0:
        parser.error("capture dimensions must be positive")

    vram_files = sorted(p for p in args.capture_root.rglob("*" + VRAM_SUFFIX)
                        if p.is_file())
    matched_pairs = eligible = unsupported = malformed = 0
    cce_captures: list[tuple[Path, str, int, list[tuple[int, int]]]] = []
    for vram_path in vram_files:
        register_path = vram_path.with_name(
            vram_path.name[:-len(VRAM_SUFFIX)] + REGISTER_SUFFIX
        )
        if not register_path.is_file():
            continue
        matched_pairs += 1
        try:
            vram = vram_path.read_bytes()
            registers = register_path.read_bytes()
            if len(vram) != VDP2_VRAM_BYTES or len(registers) < 0x92:
                malformed += 1
                continue
            byte_order = detect_byte_order(registers)
            bgon = read_u16(registers, 0x20, byte_order)
            chctla = read_u16(registers, 0x28, byte_order)
            if not (bgon & NBG1_ENABLE) or (chctla & NBG1_BITMAP_MODE):
                continue
            try:
                name_addresses, _ = visible_character_spans(
                    vram, registers, byte_order, 1, args.width, args.height
                )
            except ValueError:
                unsupported += 1
                continue
            eligible += 1
            cce_cells = []
            for address in sorted(name_addresses):
                pnd = int.from_bytes(vram[address:address + 2], "little")
                if pnd & PND_CCE:
                    cce_cells.append((address, pnd))
            if cce_cells:
                cce_captures.append(
                    (vram_path, byte_order, len(name_addresses), cce_cells)
                )
        except OSError as error:
            malformed += 1
            print(f"unreadable_capture={vram_path}: {error}", file=sys.stderr)

    print(f"vdp2_vram_dumps={len(vram_files)}")
    print(f"exact_register_pairs={matched_pairs}")
    print(f"enabled_character_mode_nbg1={eligible}")
    print(f"unsupported_nbg1_modes={unsupported}")
    print(f"malformed_capture_pairs={malformed}")
    print(f"captures_with_visible_map_pnd_cce={len(cce_captures)}")
    for path, byte_order, cell_count, cells in cce_captures:
        examples = ",".join(f"{address:#07x}:{pnd:#06x}"
                             for address, pnd in cells[:8])
        print(f"capture={path} byte_order={byte_order} visible_map_cells={cell_count} "
              f"cce_cells={len(cells)} examples={examples}")
    return 1 if malformed else 0


if __name__ == "__main__":
    raise SystemExit(main())
