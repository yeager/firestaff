#!/usr/bin/env python3
"""
Source-lock gate: DM1 V1 Creature Render → ReDMCSB DUNVIEW.C G0219.

Verifies that the Firestaff dm1_v1_creature_render_pc34_compat.h/.c
aspect table and palette tables match the ReDMCSB source verbatim.

Source references:
  ReDMCSB DUNVIEW.C line 1625-1653: G0219_as_Graphic558_CreatureAspects (DM1)
  ReDMCSB DUNVIEW.C line 1821: G0221 PaletteChanges_Creature_D3 (I34E)
  ReDMCSB DUNVIEW.C line 1822: G0222 PaletteChanges_Creature_D2 (I34E)
  ReDMCSB DEFS.H line 2392: M618_GRAPHIC_FIRST_CREATURE = 584
"""

import os
import re
import sys

# ReDMCSB G0219 (DM1 V1 / line 1625-1653) — firstNativeBitmapRelativeIndex
# Each entry: { firstNative, ?, widthFront, heightFront, widthSide, heightSide,
#                widthAttack, heightAttack, coordSet_transparent, replColors }
# We extract firstNative and coordSet_transparent and replColors.
REDMCSB_G0219_FIRST_NATIVE = [
    0,   # 00 Giant Scorpion
    4,   # 01 Swamp Slime
    6,   # 02 Giggler
    10,  # 03 Wizard Eye
    12,  # 04 Pain Rat
    16,  # 05 Ruster
    19,  # 06 Screamer
    21,  # 07 Rockpile
    23,  # 08 Ghost
    25,  # 09 Stone Golem
    29,  # 10 Mummy
    33,  # 11 Black Flame
    35,  # 12 Skeleton
    39,  # 13 Couatl
    43,  # 14 Vexirk
    47,  # 15 Magenta Worm
    51,  # 16 Trolin
    55,  # 17 Giant Wasp
    59,  # 18 Animated Armour
    63,  # 19 Materializer
    67,  # 20 Water Elemental
    69,  # 21 Oitu
    73,  # 22 Demon
    77,  # 23 Lord Chaos
    81,  # 24 Red Dragon
    85,  # 25 Lord Order
    86,  # 26 Grey Lord
]

REDMCSB_G0219_COORD_TRANS = [
    0x1D, 0x0B, 0x0B, 0x24, 0x14, 0x18, 0x0D, 0x04,
    0x04, 0x14, 0x04, 0x14, 0x04, 0x1D, 0x04, 0x14,
    0x04, 0x24, 0x04, 0x0D, 0x14, 0x14, 0x04, 0x14,
    0x14, 0x14, 0x14
]

REDMCSB_G0219_REPL_COLORS = [
    0x01, 0x20, 0x00, 0x31, 0x34, 0x34, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x30, 0x78,
    0x65, 0x00, 0x00, 0xA9, 0x65, 0xA9, 0xCB, 0x00,
    0xCB, 0xCB, 0xCB
]

# ReDMCSB DUNVIEW.C line 1821 (I34E)
REDMCSB_G0221_PALETTE_D3 = [0, 12, 1, 3, 4, 3, 0, 6, 3, 0, 0, 11, 0, 2, 0, 13]
# ReDMCSB DUNVIEW.C line 1822 (I34E)
REDMCSB_G0222_PALETTE_D2 = [0, 1, 2, 3, 4, 3, 6, 7, 5, 0, 0, 11, 12, 13, 14, 15]

M618_GRAPHIC_FIRST_CREATURE = 584


def find_firestaff_root():
    """Walk up from script location to find CMakeLists.txt."""
    d = os.path.dirname(os.path.abspath(__file__))
    for _ in range(5):
        if os.path.isfile(os.path.join(d, "CMakeLists.txt")):
            return d
        d = os.path.dirname(d)
    return None


def parse_c_aspects(src_path):
    """Extract aspect table entries from the C source file."""
    with open(src_path, "r") as f:
        content = f.read()

    # Find s_aspects array
    m = re.search(r'static const DM1_CreatureAspect s_aspects\[27\]\s*=\s*\{(.*?)\};',
                  content, re.DOTALL)
    if not m:
        return None

    entries = []
    for line in m.group(1).split('\n'):
        # Match { firstNative, firstDerived, coordSet, replColors, gi }
        em = re.search(r'\{\s*(\d+)\s*,\s*(\d+)\s*,\s*0x([0-9A-Fa-f]+)\s*,\s*0x([0-9A-Fa-f]+)\s*,\s*0x([0-9A-Fa-f]+)\s*\}', line)
        if em:
            entries.append({
                'firstNative': int(em.group(1)),
                'firstDerived': int(em.group(2)),
                'coordTrans': int(em.group(3), 16),
                'replColors': int(em.group(4), 16),
                'graphicInfo': int(em.group(5), 16),
            })
    return entries


def parse_c_palette(src_path, name):
    """Extract a palette array from the C source."""
    with open(src_path, "r") as f:
        content = f.read()

    pattern = rf'static const unsigned char {name}\[16\]\s*=\s*\{{(.*?)\}}'
    m = re.search(pattern, content, re.DOTALL)
    if not m:
        return None

    return [int(x.strip()) for x in m.group(1).split(',') if x.strip()]


def main():
    root = find_firestaff_root()
    if not root:
        print("FAIL: Cannot find Firestaff project root", file=sys.stderr)
        return 1

    src_path = os.path.join(root, "dm1_v1_creature_render_pc34_compat.c")
    if not os.path.isfile(src_path):
        print(f"FAIL: Source not found: {src_path}", file=sys.stderr)
        return 1

    errors = 0

    # 1. Parse and verify aspect table
    aspects = parse_c_aspects(src_path)
    if aspects is None or len(aspects) != 27:
        print(f"FAIL: Expected 27 aspect entries, got {len(aspects) if aspects else 'None'}", file=sys.stderr)
        return 1

    for i in range(27):
        a = aspects[i]
        if a['firstNative'] != REDMCSB_G0219_FIRST_NATIVE[i]:
            print(f"FAIL: Creature {i} firstNative: got {a['firstNative']}, "
                  f"expected {REDMCSB_G0219_FIRST_NATIVE[i]}", file=sys.stderr)
            errors += 1
        if a['coordTrans'] != REDMCSB_G0219_COORD_TRANS[i]:
            print(f"FAIL: Creature {i} coordSet_transparent: got 0x{a['coordTrans']:02X}, "
                  f"expected 0x{REDMCSB_G0219_COORD_TRANS[i]:02X}", file=sys.stderr)
            errors += 1
        if a['replColors'] != REDMCSB_G0219_REPL_COLORS[i]:
            print(f"FAIL: Creature {i} replColors: got 0x{a['replColors']:02X}, "
                  f"expected 0x{REDMCSB_G0219_REPL_COLORS[i]:02X}", file=sys.stderr)
            errors += 1

    # 2. Verify palette D3
    d3 = parse_c_palette(src_path, "s_paletteD3")
    if d3 != REDMCSB_G0221_PALETTE_D3:
        print(f"FAIL: Palette D3 mismatch: got {d3}, expected {REDMCSB_G0221_PALETTE_D3}", file=sys.stderr)
        errors += 1

    # 3. Verify palette D2
    d2 = parse_c_palette(src_path, "s_paletteD2")
    if d2 != REDMCSB_G0222_PALETTE_D2:
        print(f"FAIL: Palette D2 mismatch: got {d2}, expected {REDMCSB_G0222_PALETTE_D2}", file=sys.stderr)
        errors += 1

    # 4. Verify M618 constant in header
    hdr_path = os.path.join(root, "dm1_v1_creature_render_pc34_compat.h")
    with open(hdr_path, "r") as f:
        hdr = f.read()

    m = re.search(r'#define\s+DM1_GRAPHIC_FIRST_CREATURE\s+(\d+)', hdr)
    if not m or int(m.group(1)) != M618_GRAPHIC_FIRST_CREATURE:
        val = int(m.group(1)) if m else 'not found'
        print(f"FAIL: DM1_GRAPHIC_FIRST_CREATURE: got {val}, expected {M618_GRAPHIC_FIRST_CREATURE}", file=sys.stderr)
        errors += 1

    # 5. Verify GraphicInfo mask constants
    expected_masks = {
        'DM1_GI_MASK_ADDITIONAL': 0x0003,
        'DM1_GI_MASK_FLIP_NON_ATTACK': 0x0004,
        'DM1_GI_MASK_SIDE': 0x0008,
        'DM1_GI_MASK_BACK': 0x0010,
        'DM1_GI_MASK_ATTACK': 0x0020,
        'DM1_GI_MASK_SPECIAL_D2_FRONT': 0x0080,
        'DM1_GI_MASK_D2_FRONT_IS_FLIPPED': 0x0100,
        'DM1_GI_MASK_FLIP_ATTACK': 0x0200,
    }
    for name, val in expected_masks.items():
        pattern = rf'#define\s+{name}\s+0x([0-9A-Fa-f]+)u'
        m = re.search(pattern, hdr)
        if not m or int(m.group(1), 16) != val:
            got = int(m.group(1), 16) if m else 'not found'
            print(f"FAIL: {name}: got {got}, expected 0x{val:04X}", file=sys.stderr)
            errors += 1

    if errors:
        print(f"\n{errors} source-lock errors detected", file=sys.stderr)
        return 1

    print(f"PASS: DM1 V1 creature render source lock verified")
    print(f"  - 27 creature aspects match ReDMCSB G0219")
    print(f"  - Palette D3 matches G0221 (16 entries)")
    print(f"  - Palette D2 matches G0222 (16 entries)")
    print(f"  - M618_GRAPHIC_FIRST_CREATURE = {M618_GRAPHIC_FIRST_CREATURE}")
    print(f"  - 8 GraphicInfo masks match DEFS.H")
    return 0


if __name__ == "__main__":
    sys.exit(main())
