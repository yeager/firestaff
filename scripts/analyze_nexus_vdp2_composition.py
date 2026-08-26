#!/usr/bin/env python3
"""Decode bounded VDP2 composition registers from an authentic raw witness.

The register values are an original Saturn observation.  Retained external
Mednafen witnesses use two authenticated register serializations; the helper
selects the plausible order per frame.  They identify the
enabled VDP2 layer and hardware configuration for the captured frame, but do
not identify the retail asset that filled VRAM or authorize host composition.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from nexus_vdp2_registers import detect_byte_order, read_u16


REGISTERS = {
    0x00: "TVMD",
    0x02: "EXTEN",
    0x06: "VRSIZE",
    0x0E: "RAMCTL",
    0x20: "BGON",
    0x22: "MZCTL",
    0x24: "SFSEL",
    0x26: "SFCODE",
    0x28: "CHCTLA",
    0x2A: "CHCTLB",
    0x2C: "BMPNA",
    0x2E: "BMPNB",
    0x30: "PNCN0",
    0x32: "PNCN1",
    0x34: "PNCN2",
    0x36: "PNCN3",
    0x38: "PNCNR",
    0x3A: "PLSZ",
    0x3C: "MPOFN",
    0x3E: "MPOFR",
    0xE0: "SPCTL",
    0xE2: "SDCTL",
    0xE4: "CRAOFA",
    0xE6: "CRAOFR",
    0xEC: "CCCTL",
    0xF8: "PRINA",
    0xFA: "PRINB",
    0xFC: "PRIR",
}


def enabled_layers(bgon: int) -> list[str]:
    names = ["NBG0", "NBG1", "NBG2", "NBG3", "RBG0"]
    return [name for bit, name in enumerate(names) if bgon & (1 << bit)]


def nbg_mode(chctla: int, layer: int) -> tuple[str, int, int, int]:
    """Return (mode, colour_code, bitmap_size_code, bitmap_palette)."""
    shift = layer * 8
    bitmap = (chctla >> (1 + shift)) & 1
    if layer == 0:
        colour_code = (chctla >> 4) & 7
        palette = None
    else:
        colour_code = (chctla >> 12) & 3
        palette = 0
    if not bitmap:
        return "character", colour_code, 0, -1
    # BMPNA bits 8..10 are NBG1's bitmap palette selector; the caller
    # supplies the register value separately for the active layer.
    return "bitmap", colour_code, (chctla >> (2 + shift)) & 3, palette or 0


def nbg_map_registers(registers: bytes, byte_order: str) -> list[tuple[int, int]]:
    """Decode the eight NBG map registers from the raw VDP2 register image.

    Mednafen's VDP2 renderer consumes 0x40..0x4e as four pairs.  Each pair
    contains the two six-bit plane numbers for one NBG layer; the hardware
    register image stores the pair as a 16-bit value.  Keep this as an
    observation only: the values describe the selected map planes, not the
    provenance of the bytes currently present in VRAM.
    """
    result: list[tuple[int, int]] = []
    for layer in range(4):
        first = read_u16(registers, 0x40 + layer * 4, byte_order) & 0x3F
        second = read_u16(registers, 0x42 + layer * 4, byte_order) & 0x3F
        result.append((first, second))
    return result


def nbg_map_register_bytes(registers: bytes, byte_order: str,
                           layer: int) -> tuple[int, int, int, int]:
    """Return all four six-bit map numbers for one NBG layer."""
    if layer not in range(4):
        raise ValueError("invalid NBG layer")
    first = read_u16(registers, 0x40 + layer * 4, byte_order)
    second = read_u16(registers, 0x42 + layer * 4, byte_order)
    return (first & 0x3F, (first >> 8) & 0x3F,
            second & 0x3F, (second >> 8) & 0x3F)


def visible_character_spans(vram: bytes, registers: bytes, byte_order: str,
                            layer: int, width: int = 320,
                            height: int = 224) -> tuple[set[int], set[int]]:
    """Return observed NBG character name-table and character byte addresses.

    This mirrors only the bounded address calculation used by the Saturn
    VDP2 character fetcher for a normal 8x8 tile layer.  It deliberately
    returns addresses rather than pixels: a name-table route is useful for
    ruling an asset in or out as a consumer, but it does not prove palette,
    priority, compositor output, or title semantics.
    """
    if layer not in (0, 1):
        raise ValueError("only NBG0/NBG1 character layers are supported")
    if len(vram) != 0x80000 or len(registers) < 0x92:
        raise ValueError("VDP2 witness is truncated")
    if width <= 0 or height <= 0:
        raise ValueError("visible dimensions are invalid")
    chctla = read_u16(registers, 0x28, byte_order)
    pncn = read_u16(registers, 0x30 + layer * 2, byte_order)
    plsz = read_u16(registers, 0x3A, byte_order)
    map_words = nbg_map_register_bytes(registers, byte_order, layer)
    char_size = (chctla >> (layer * 8)) & 1
    if (chctla >> (1 + layer * 8)) & 1:
        raise ValueError("layer is in bitmap mode")
    pnd_size = (pncn >> 15) & 1
    if pnd_size or char_size:
        raise ValueError("only two-word 8x8 character PNDs are supported")
    # The normal character route uses the layer's integer scroll registers:
    # NBG0 at 0x70/0x78 and NBG1 at 0x80/0x88.
    x_scroll = read_u16(registers, 0x70 + layer * 0x10, byte_order) & 0x7FF
    y_scroll = read_u16(registers, 0x78 + layer * 0x10, byte_order) & 0x7FF
    plane_size = (plsz >> (layer * 2)) & 0x3
    map_offset = (read_u16(registers, 0x3C, byte_order) >> (layer * 4)) & 0x7
    psshift = 13 - pnd_size - (char_size << 1)
    name_addresses: set[int] = set()
    character_addresses: set[int] = set()
    for cell_y in range((height + 7) // 8):
        iy = cell_y * 8 + y_scroll
        for cell_x in range((width + 7) // 8):
            ix = cell_x * 8 + x_scroll
            map_index = ((ix >> (9 + bool(plane_size & 1))) & 1) | \
                        ((iy >> (9 + bool(plane_size & 2) - 1)) & 2)
            plane_index = ((ix >> 9) & plane_size & 1) | \
                          ((iy >> 8) & plane_size & 2)
            map_word = map_words[map_index]
            map_base = ((map_offset << 6) + (map_word & ~plane_size)) << psshift
            plane_offset = plane_index << psshift
            page_offset = ((((ix >> 3) & 0x3F) >> char_size) +
                           ((((iy >> 3) & 0x3F) >> char_size) <<
                            (6 - char_size))) << (1 - pnd_size)
            name_word = (map_base + plane_offset + page_offset) & 0x3FFFF
            name_byte = name_word * 2
            name_addresses.add(name_byte)
            char_number = int.from_bytes(vram[name_byte + 2:name_byte + 4],
                                          "little") & 0x7FFF
            # VDP2's character address is a 16-bit-word address; return the
            # raw-capture byte address used by all source-span analyzers.
            character_addresses.add((char_number << 5) & 0x7FFFF)
    return name_addresses, character_addresses


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument(
        "--capture-frames",
        type=int,
        default=None,
        help="number of frames in the authenticated capture (defaults to frame + 1)",
    )
    parser.add_argument("--require-layer", action="append", choices=("NBG0", "NBG1", "NBG2", "NBG3", "RBG0"), default=[])
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP2_COMPOSITION_INVALID: negative frame")
        return 1
    capture_frames = args.capture_frames if args.capture_frames is not None else args.frame + 1
    if capture_frames <= args.frame:
        print("NEXUS_VDP2_COMPOSITION_INVALID: capture frame count does not include selected frame")
        return 1
    try:
        frames, _ = frame_regions(args.capture.read_bytes(), capture_frames)
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP2_COMPOSITION_INVALID: {error}")
        return 1

    registers = frames[args.frame]["vdp2-regs"]
    byte_order = detect_byte_order(registers)
    values = {
        name: read_u16(registers, offset, byte_order)
        for offset, name in REGISTERS.items()
        if offset + 2 <= len(registers)
    }
    bgon = values["BGON"]
    layers = enabled_layers(bgon)
    nbg0_mode, nbg0_colour, nbg0_size, _ = nbg_mode(values["CHCTLA"], 0)
    nbg1_mode, nbg1_colour, nbg1_size, _ = nbg_mode(values["CHCTLA"], 1)
    nbg0_palette = values["BMPNA"] & 7
    nbg1_palette = (values["BMPNA"] >> 8) & 7
    map_registers = nbg_map_registers(registers, byte_order)
    print(f"frame={args.frame}")
    print(f"register_byte_order={byte_order}")
    print("registers=" + ",".join(f"{name}=0x{value:04x}" for name, value in values.items()))
    print("enabled_layers=" + (",".join(layers) if layers else "none"))
    print(
        f"NBG0_mode={nbg0_mode} colour_code={nbg0_colour} "
        f"bitmap_size_code={nbg0_size} bitmap_palette={nbg0_palette}"
    )
    print(
        f"NBG1_mode={nbg1_mode} colour_code={nbg1_colour} "
        f"bitmap_size_code={nbg1_size} bitmap_palette={nbg1_palette}"
    )
    print(
        "nbg_map_offsets="
        + ",".join(f"NBG{index}=0x{(values['MPOFN'] >> (index * 4)) & 7:x}" for index in range(4))
    )
    print(
        "nbg_map_registers="
        + ",".join(
            f"NBG{index}=0x{first:02x}/0x{second:02x}"
            for index, (first, second) in enumerate(map_registers)
        )
    )
    print(
        "nbg_priorities="
        + f"NBG0={(values['PRINA'] & 7)},NBG1={(values['PRINA'] >> 8) & 7},"
        + f"NBG2={(values['PRINB'] & 7)},NBG3={(values['PRINB'] >> 8) & 7}"
    )
    print(
        "nbg_cram_offsets="
        + ",".join(f"NBG{index}={(values['CRAOFA'] >> (index * 4)) & 7}" for index in range(4))
    )
    if nbg1_mode == "character":
        try:
            name_addresses, character_addresses = visible_character_spans(
                frames[args.frame]["vdp2-vram"], registers, byte_order, 1)
            print(f"NBG1_visible_name_table_address_range=0x{min(name_addresses):05x}-"
                  f"0x{max(name_addresses):05x} count={len(name_addresses)}")
            print(f"NBG1_visible_character_address_range=0x{min(character_addresses):05x}-"
                  f"0x{max(character_addresses):05x} count={len(character_addresses)}")
        except ValueError as error:
            print(f"NBG1_visible_character_addresses=unbound:{error}")
    print("register_semantics=authentic_frame_observation")
    print("NBG0_map_registers_consumed=no_bitmap_mode" if nbg0_mode == "bitmap"
          else "NBG0_map_registers_consumed=character_mode")
    print("NBG1_map_registers_consumed=no_bitmap_mode" if nbg1_mode == "bitmap"
          else "NBG1_map_registers_consumed=character_mode")
    print("asset_consumer_identity=unbound")
    print("host_composition_admission=blocked")
    missing = sorted(set(args.require_layer) - set(layers))
    if missing:
        print("required_layers_missing=" + ",".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
