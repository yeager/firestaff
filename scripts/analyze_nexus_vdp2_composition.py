#!/usr/bin/env python3
"""Decode bounded VDP2 composition registers from an authentic raw witness.

The register values are an original Saturn observation.  The external
Mednafen capture stores its native ``uint16`` words in host order; on the
supported capture host this is little-endian.  They identify the
enabled VDP2 layer and hardware configuration for the captured frame, but do
not identify the retail asset that filled VRAM or authorize host composition.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


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


def read_u16(registers: bytes, offset: int) -> int:
    return int.from_bytes(registers[offset : offset + 2], "little")


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
    values = {
        name: read_u16(registers, offset)
        for offset, name in REGISTERS.items()
        if offset + 2 <= len(registers)
    }
    bgon = values["BGON"]
    layers = enabled_layers(bgon)
    nbg1_mode, nbg1_colour, nbg1_size, _ = nbg_mode(values["CHCTLA"], 1)
    nbg1_palette = (values["BMPNA"] >> 8) & 7
    print(f"frame={args.frame}")
    print("registers=" + ",".join(f"{name}=0x{value:04x}" for name, value in values.items()))
    print("enabled_layers=" + (",".join(layers) if layers else "none"))
    print(
        f"NBG1_mode={nbg1_mode} colour_code={nbg1_colour} "
        f"bitmap_size_code={nbg1_size} bitmap_palette={nbg1_palette}"
    )
    print(
        "nbg_map_offsets="
        + ",".join(f"NBG{index}=0x{(values['MPOFN'] >> (index * 4)) & 7:x}" for index in range(4))
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
    print("register_semantics=authentic_frame_observation")
    print("NBG1_map_registers_consumed=no_bitmap_mode")
    print("asset_consumer_identity=unbound")
    print("host_composition_admission=blocked")
    missing = sorted(set(args.require_layer) - set(layers))
    if missing:
        print("required_layers_missing=" + ",".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
