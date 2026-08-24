#!/usr/bin/env python3
"""Inspect a raw Saturn VDP2 frame for exact Nexus title source spans.

This is a deliberately narrow provenance tool.  It recognises only the
authenticated English ``TITLE.BIN``/``TITLE.CG`` pair and reports raw,
word-swapped byte spans in VDP2 VRAM and CRAM.  A character-generator upload
by itself is not a title screen: an original display-list/layer-owner capture
is still required.  Therefore this tool never grants rendering admission.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from nexus_vdp2_registers import detect_byte_order, read_u16


TITLE_BIN_SHA256 = "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"
TITLE_BIN_JP_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
TITLE_CG_SHA256 = "fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580"
TITLE_MAPD_OFFSET = 0xE278
TITLE_MAP_COUNT = 5
TITLE_MAP_BYTES = 64 * 28 * 4
TITLE_PALETTE_OFFSET = 0x40 + TITLE_MAP_COUNT * 0x1C04


def wordswapped(data: bytes) -> bytes:
    """Return pair-reversed data, or reject an odd Saturn word span."""
    if len(data) % 2:
        raise ValueError("Saturn word span has odd length")
    return b"".join(data[index:index + 2][::-1]
                    for index in range(0, len(data), 2))


def title_spans(title_bin: bytes, title_cg: bytes) -> tuple[bytes, list[bytes], bytes]:
    """Validate the documented MAPD/TIBG record and retain raw source spans."""
    if hashlib.sha256(title_bin).hexdigest() not in {
            TITLE_BIN_SHA256, TITLE_BIN_JP_SHA256}:
        raise ValueError("TITLE.BIN hash mismatch")
    if hashlib.sha256(title_cg).hexdigest() != TITLE_CG_SHA256:
        raise ValueError("TITLE.CG hash mismatch")
    record = title_bin[TITLE_MAPD_OFFSET:]
    if len(record) < TITLE_PALETTE_OFFSET + 32 or \
            record[:4] != b"MAPD" or record[8:12] != b"TIBG":
        raise ValueError("TITLE.BIN MAPD/TIBG record missing")
    maps: list[bytes] = []
    for index in range(TITLE_MAP_COUNT):
        offset = 0x40 + index * 0x1C04
        if (int.from_bytes(record[offset:offset + 2], "big") != 64 or
                int.from_bytes(record[offset + 2:offset + 4], "big") != 28):
            raise ValueError(f"TITLE.BIN map {index} geometry is invalid")
        maps.append(record[offset + 4:offset + 4 + TITLE_MAP_BYTES])
    # TITLE.CG begins with a 32-byte source header.  The remaining payload is
    # the documented contiguous 4bpp character-generator span.
    if len(title_cg) <= 32 or (len(title_cg) - 32) % 32:
        raise ValueError("TITLE.CG character-generator span is invalid")
    return title_cg[32:], maps, record[TITLE_PALETTE_OFFSET:TITLE_PALETTE_OFFSET + 32]


def find_span(haystack: bytes, source: bytes) -> tuple[int, int]:
    return haystack.find(source), haystack.find(wordswapped(source))


def find_span_with_swapped(haystack: bytes, source: bytes,
                           swapped_source: bytes) -> tuple[int, int]:
    """Find a pre-normalized source span in one captured VDP2 region."""
    return haystack.find(source), haystack.find(swapped_source)


def describe_position(exact: int, swapped: int) -> str:
    return f"exact=0x{exact:x} word_swap=0x{swapped:x}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--frame", type=int,
                        help="inspect one zero-based frame instead of all frames")
    args = parser.parse_args()
    if args.capture_frames <= 0 or (args.frame is not None and
                                    (args.frame < 0 or args.frame >= args.capture_frames)):
        print("NEXUS_TITLE_VDP2_SOURCE_INVALID: invalid frame selection")
        return 1
    try:
        title_cg, maps, palette = title_spans(
            (args.data_dir / "TITLE.BIN").read_bytes(),
            (args.data_dir / "TITLE.CG").read_bytes())
    except (OSError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP2_SOURCE_INVALID: {error}")
        return 1

    cg_frames: list[int] = []
    map_frames: list[int] = []
    palette_frames: list[int] = []
    title_cg_swapped = wordswapped(title_cg)
    maps_swapped = [wordswapped(source) for source in maps]
    palette_swapped = wordswapped(palette)
    try:
        frames = iter_frame_regions_file(args.capture, args.capture_frames)
        for index, frame in frames:
            if args.frame is not None and index != args.frame:
                continue
            cg_exact, cg_swapped = find_span_with_swapped(
                frame["vdp2-vram"], title_cg, title_cg_swapped)
            map_positions = [find_span_with_swapped(frame["vdp2-vram"], source,
                                                    swapped)
                             for source, swapped in zip(maps, maps_swapped)]
            palette_exact, palette_swapped_position = find_span_with_swapped(
                frame["vdp2-cram"], palette, palette_swapped)
            if cg_exact >= 0 or cg_swapped >= 0:
                cg_frames.append(index)
            if any(exact >= 0 or swapped >= 0 for exact, swapped in map_positions):
                map_frames.append(index)
            if palette_exact >= 0 or palette_swapped_position >= 0:
                palette_frames.append(index)
            if args.frame is not None or cg_exact >= 0 or cg_swapped >= 0 or \
                    index in map_frames or index in palette_frames:
                registers = frame["vdp2-regs"]
                byte_order = detect_byte_order(registers)
                tvmd = read_u16(registers, 0x00, byte_order)
                bgon = read_u16(registers, 0x20, byte_order)
                chctla = read_u16(registers, 0x28, byte_order)
                print(f"frame={index} register_byte_order={byte_order} "
                      f"tvmd=0x{tvmd:04x} bgon=0x{bgon:04x} chctla=0x{chctla:04x}")
                print("title_cg_vram_" + describe_position(cg_exact, cg_swapped) +
                      f" bytes={len(title_cg)}")
                for map_index, (exact, swapped) in enumerate(map_positions):
                    print(f"title_map_{map_index}_vram_" +
                          describe_position(exact, swapped) +
                          f" bytes={len(maps[map_index])}")
                print("title_palette_cram_" +
                      describe_position(palette_exact, palette_swapped_position) +
                      f" bytes={len(palette)}")
    except (OSError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP2_SOURCE_INVALID: {error}")
        return 1

    print("title_cg_vram_source_join=verified" if cg_frames
          else "title_cg_vram_source_join=unbound")
    print("title_map_vram_source_join=verified" if map_frames
          else "title_map_vram_source_join=unbound")
    print("title_palette_cram_source_join=verified" if palette_frames
          else "title_palette_cram_source_join=unbound")
    print("title_consumer_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
