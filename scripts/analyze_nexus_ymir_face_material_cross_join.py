#!/usr/bin/env python3
"""Cross-check authentic Ymir face-to-command joins against retail materials.

This evidence analyzer does not grant renderer admission. It joins captured
SH-2 face/staging rows to VDP1 commands, then checks whether the exact source
face is among the Structure3 owners of a candidate DGN image record whose bytes
match the captured span. Direct-colour commands have no indexed Structure3
selector and remain separately classified.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path

from analyze_nexus_ymir_face_to_command import analyze as analyze_faces
from analyze_nexus_vdp1_command_sequence import Record
from analyze_nexus_vdp1_dgn_command_sequence_join import (
    command_material,
    dgn_match_index,
)
from analyze_nexus_vdp1_dgn_material_join import dgn_structure3_face_owners
from analyze_nexus_vdp1_source_join import ymir_trace_prefix


def relation_for_face_material(
    mode: int, source_face: tuple[int, int, int] | None,
    candidate_owners: set[tuple[int, int, int]],
) -> str:
    """Classify evidence without treating missing provenance as a match."""
    if source_face is None:
        return "runtime_face_unresolved"
    if mode == 1:
        if source_face in candidate_owners:
            return "indexed_face_selector_owner"
        if candidate_owners:
            return "indexed_selector_has_other_face_owners"
        return "indexed_selector_has_no_structure3_owner"
    if mode == 5:
        return "direct_colour_face_material_link_unproven"
    return "unsupported_colour_mode"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--retail-data", type=Path, required=True)
    parser.add_argument("--firestaff-repo", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=1605)
    parser.add_argument("--json-out", type=Path)
    args = parser.parse_args()

    rows, resolved, _, *_ = analyze_faces(
        args.evidence, args.retail_data, args.firestaff_repo, args.frame
    )
    dump = args.evidence / "f364-correlation/dumps"
    vram, frame, traced_commands = ymir_trace_prefix(dump)
    if frame != args.frame:
        raise ValueError(f"requested frame {args.frame}, capture contains {frame}")
    image_index, palette_index = dgn_match_index(
        args.retail_data, ymir_byte_order=True
    )
    command_by_address = {}
    for address, words in traced_commands:
        material = command_material(vram, Record(address, words))
        if material is not None:
            if address in command_by_address:
                raise ValueError(f"duplicate VDP1 command address {address:#x}")
            command_by_address[address] = material

    report_rows = []
    relationships: Counter[str] = Counter()
    matched_source_images = 0
    matched_indexed_palettes = 0
    matched_direct_colour_images = 0
    for row in rows:
        matches = row["command_unordered"]
        if len(matches) != 1:
            raise ValueError("face observation does not have one unique VDP1 command")
        command_index, address = matches[0]
        material = command_by_address.get(address)
        if material is None or not material["bounded"]:
            raise ValueError(f"command {address:#x} has no bounded material record")
        mode = material["mode"]
        image_candidates = image_index.get((mode, material["source"]), [])
        if image_candidates:
            matched_source_images += 1
        palette_candidates = (
            palette_index.get(material["palette"], []) if mode == 1 else []
        )
        if palette_candidates:
            matched_indexed_palettes += 1
        if mode == 5 and image_candidates:
            matched_direct_colour_images += 1

        owners: set[tuple[int, int, int]] = set()
        for name in image_candidates:
            match = re.fullmatch(r"LEV(\d\d)\.DGN\[Structure2=(\d+).*", name)
            if not match:
                continue
            level, image_id = int(match.group(1)), int(match.group(2))
            for owner in dgn_structure3_face_owners(
                args.retail_data, f"LEV{level:02d}.DGN", image_id
            ):
                face_match = re.fullmatch(
                    r"LEV(\d\d)\.DGN\[Structure3-entry=(\d+),face=(\d+)\]",
                    owner,
                )
                if face_match:
                    owners.add(tuple(map(int, face_match.groups())))

        key = (row["cpu"], row["face_ptr"])
        source = resolved.get(key)
        source_face = ((source[0], source[1], source[2]) if source else None)
        relation = relation_for_face_material(mode, source_face, owners)
        relationships[relation] += 1
        report_rows.append({
            "cpu": row["cpu"],
            "face_wram_address": f"0x{row['face_ptr']:08x}",
            "source_face": (None if source is None else {
                "level": source[0], "entry": source[1], "ordinal": source[2],
            }),
            "command_index": command_index,
            "command_address": f"0x{address:05x}",
            "colour_mode": mode,
            "image_candidates": image_candidates,
            "palette_candidates": palette_candidates,
            "candidate_structure3_owner_count": len(owners),
            "source_face_is_candidate_owner": source_face in owners if source_face else None,
            "relationship": relation,
        })

    summary = {
        "capture_frame": frame,
        "sh2_face_staging_observations": len(rows),
        "unique_vdp1_geometry_joins": len(rows),
        "bounded_material_records": len(command_by_address),
        "retail_image_byte_matches": matched_source_images,
        "indexed_palette_byte_matches": matched_indexed_palettes,
        "direct_colour_image_byte_matches": matched_direct_colour_images,
        "face_material_relationships": dict(sorted(relationships.items())),
        "limits": [
            "Only the retained authentic Ymir frame is covered.",
            "Repeated retail image bytes can have multiple candidate DGN owners.",
            "Direct-colour bytes have no image selector or CLUT; a face-material link is not inferred.",
            "This does not prove per-pixel contribution, full-frame VDP1+VDP2 parity, or renderer admission.",
        ],
        "rows": report_rows,
    }
    encoded = json.dumps(summary, indent=2) + "\n"
    if args.json_out:
        args.json_out.write_text(encoded, encoding="utf-8")
    print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
