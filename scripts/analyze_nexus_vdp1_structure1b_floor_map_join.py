#!/usr/bin/env python3
"""Compare authentic Nexus frame-119 floor quads with the retail LEV00 map.

This is a bounded evidence analyzer, not a scene renderer or admission gate.
It verifies the captured VDP1 texture bytes and CLUTs, groups the unowned
48x48 quads by their observed command-space Y spans, and checks whether their
numeric Structure2 IDs occur as a row pattern in the hash-verified LEV00
Structure1B map. Numeric equality alone does not establish an ID mapping.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import struct
from pathlib import Path

from analyze_nexus_vdp1_dgn_material_join import (
    dgn_materials,
    dgn_structure3_face_owners,
)
from analyze_nexus_vdp1_source_join import be16, be32, ymir_commands
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


FRAME = 119
TRACE_ROWS = 189
VDP1_VRAM_SHA256 = "12230c8142fcc33c9429bf9cb203137eafcd6fdc3c5c0aa4ca3c32913e55b77a"
VDP1_TRACE_SHA256 = "624b664cb7b64f7df5d719edbff929256462875226966d3992656946bdf6384f"
BUNDLE_SHA256 = "807274ace341cdf12280c201399c989f082c5645051443d4aa4e77349f4d1e20"

# Relative starts and material IDs are the row grouping observed in the
# complete VDP1 list.  The scanner proves whether this four-row pattern is
# unique in the authenticated Structure1B grid.
ROW_PATTERN = (
    (1, (2, 2, 2)),
    (1, (2, 1, 2)),
    (0, (2, 3, 1, 3, 2)),
    (0, (1, 1, 1, 5, 4)),
)
EXPECTED_SPANS = ((33, 40), (40, 52), (52, 73), (73, 122))


def fail(message: str) -> None:
    raise ValueError(message)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_bundle(path: Path) -> dict[str, str]:
    fields = {}
    for line in path.read_text(encoding="ascii").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            fields[key] = value
    return fields


def signed16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def structure1b_floor_grid(data: bytes) -> list[list[int]]:
    if hashlib.sha256(data).hexdigest() != DISC_HASH["LEV00.DGN"]:
        fail("LEV00.DGN is not the canonical retail file")
    structure1_block = be16(data, 0x0C)
    structure1_blocks = be16(data, 0x0E)
    useful_size = be32(data, 0x10)
    structure1_offset = structure1_block * 0x800
    structure1_size = structure1_blocks * 0x800
    if (structure1_block == 0 or structure1_blocks == 0 or
            structure1_offset + structure1_size > len(data) or
            useful_size > structure1_size or structure1_size < 0x38):
        fail("LEV00.DGN Structure1 bounds are invalid")
    if data[structure1_offset + 2:structure1_offset + 4] != b"@@":
        fail("LEV00.DGN Structure1 dimensions are not 64x64")
    relative = be32(data, structure1_offset + 0x14)
    map_size = 64 * 64 * 8
    if relative + map_size > useful_size:
        fail("LEV00.DGN Structure1B exceeds its declared payload")
    base = structure1_offset + relative
    return [
        [
            (be16(data, base + (y * 64 + x) * 8) >> 7) & 0x1F
            for x in range(64)
        ]
        for y in range(64)
    ]


def find_pattern(grid: list[list[int]]) -> list[tuple[int, int]]:
    max_width = max(dx + len(materials)
                    for dx, materials in ROW_PATTERN)
    hits = []
    for y in range(64 - len(ROW_PATTERN) + 1):
        for x in range(64 - max_width + 1):
            if all(
                tuple(grid[y + dy][x + dx:x + dx + len(materials)]) == materials
                for dy, (dx, materials) in enumerate(ROW_PATTERN)
            ):
                hits.append((x, y))
    return hits


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_dumps", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=FRAME)
    args = parser.parse_args()
    try:
        if args.frame != FRAME:
            fail(f"this authenticated capture is bound to frame {FRAME}")

        bundle_path = args.capture_dumps / "vdp1-capture-bundle.txt"
        trace_path = args.capture_dumps / "vdp1-command-trace.csv"
        vram_path = args.capture_dumps / "vdp1-vram.bin"
        for path, expected in (
            (bundle_path, BUNDLE_SHA256),
            (trace_path, VDP1_TRACE_SHA256),
            (vram_path, VDP1_VRAM_SHA256),
        ):
            if sha256(path) != expected:
                fail(f"{path.name} does not match the retained frame-{FRAME} capture")

        bundle = read_bundle(bundle_path)
        if (bundle.get("capture_frame_index") != str(FRAME) or
                bundle.get("trace_rows_for_capture_frame") != str(TRACE_ROWS) or
                bundle.get("trace_file_complete") != "1" or
                bundle.get("trace_dropped_total") != "0"):
            fail("the Ymir command-trace receipt is incomplete or mismatched")

        vram, captured_frame, commands = ymir_commands(args.capture_dumps)
        if captured_frame != FRAME or len(commands) != TRACE_ROWS:
            fail("Ymir command list does not match the retained frame-119 receipt")
        if len(vram) != 0x80000:
            fail("captured VDP1 VRAM has an unexpected size")
        materials = [material for material in dgn_materials(args.data_dir)
                     if material[0].startswith("LEV00.DGN[")]
        unowned = []
        for index, (address, words) in enumerate(commands):
            control = words[0]
            if control & 0xC000 or (control & 0x000F) != 2:
                continue
            mode = (words[2] >> 3) & 0x7
            if mode != 1:
                continue
            width = ((words[5] >> 8) & 0x003F) * 8
            height = words[5] & 0x00FF
            if width <= 0 or height <= 0:
                continue
            source_offset = words[4] * 8
            source_size = width * height * 4 // 8
            if source_offset + source_size > len(vram):
                fail(f"frame {FRAME} command 0x{address:04x} reads outside VDP1 VRAM")
            source = vram[source_offset:source_offset + source_size]
            if not (words[2] & 0x0040) and not any(source):
                continue
            matches = [
                material for material in materials
                if material[1] == 0x0008 and material[2] == source
            ]
            if not matches:
                continue
            clut_word = (words[3] & ~0x3) << 2
            palette_offset = clut_word * 2
            if palette_offset > len(vram) or 32 > len(vram) - palette_offset:
                fail(f"frame {FRAME} command 0x{address:04x} CLUT is out of bounds")
            palette = vram[palette_offset:palette_offset + 32]
            coordinates = [signed16(words[index])
                           for index in (7, 9, 11, 13)]
            y_span = (min(coordinates), max(coordinates))
            owner_matches = []
            for name, _, _, source_palette in matches:
                image_match = re.search(r"Structure2=(\d+)", name)
                if not image_match:
                    continue
                image_id = int(image_match.group(1))
                if palette != source_palette:
                    continue
                owners = dgn_structure3_face_owners(
                    args.data_dir, "LEV00.DGN", image_id
                )
                if not owners:
                    owner_matches.append(image_id)
            if owner_matches:
                if len(set(owner_matches)) != 1:
                    fail(f"ambiguous LEV00 material at VDP1 command 0x{address:04x}")
                unowned.append((
                    index, address,
                    owner_matches[0], y_span,
                ))

        bands: dict[tuple[int, int], list[tuple[int, int, int]]] = {}
        for index, address, image_id, y_span in unowned:
            bands.setdefault(y_span, []).append((index, address, image_id))
        spans = tuple(sorted(bands))
        if spans != EXPECTED_SPANS:
            fail(f"unowned LEV00 floor-quads have unexpected Y spans: {spans}")
        observed_pattern = tuple(
            tuple(item[2] for item in sorted(bands[span])) for span in spans
        )
        expected_pattern = tuple(materials for _, materials in ROW_PATTERN)
        if observed_pattern != expected_pattern:
            fail(f"captured floor-material rows differ: {observed_pattern}")
        if len(unowned) != 16:
            fail(f"found {len(unowned)} unowned LEV00 floor quads, expected 16")

        grid = structure1b_floor_grid((args.data_dir / "LEV00.DGN").read_bytes())
        hits = find_pattern(grid)
        if hits != [(30, 26)]:
            fail(f"Structure1B row pattern is not uniquely located at (30, 26): {hits}")

    except (OSError, ValueError, KeyError, struct.error) as error:
        print(f"NEXUS_VDP1_STRUCTURE1B_FLOOR_MAP_JOIN_INVALID: {error}")
        return 1

    print(f"frame={FRAME} complete_vdp1_commands={TRACE_ROWS}")
    print("capture_hashes=verified")
    print("floor_draws=16 structure2_source_and_clut_matches=verified structure3_face_owners=none")
    for span in spans:
        details = ",".join(
            f"index={index}/address=0x{address:04x}/S2={image_id}"
            for index, address, image_id in sorted(bands[span])
        )
        print(f"command_y_span={span[0]}..{span[1]} materials="
              f"{','.join(map(str, observed_pattern[spans.index(span)]))} "
              f"draws={details}")
    print("unique_numeric_structure1b_pattern=LEV00.DGN:x30..34,y26..29")
    print("structure2_to_structure1b_id_mapping=unproven")
    for y, (x_offset, values) in enumerate(ROW_PATTERN, start=26):
        print(f"map_row_y={y} x{30 + x_offset}..{30 + x_offset + len(values) - 1} "
              f"materials={','.join(map(str, values))}")
    print("active_cell_selection=unproven")
    print("camera_projection=unproven")
    print("vdp2_composition=unproven")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
