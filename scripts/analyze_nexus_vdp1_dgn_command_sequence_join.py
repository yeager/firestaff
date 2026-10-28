#!/usr/bin/env python3
"""Join one complete VDP1 command chain to canonical Nexus DGN materials.

This is a capture/source receipt, not renderer permission. It follows either
an authenticated VDP1 CMDLINK chain or a complete Ymir debug trace, matches
indexed image/CLUT pairs and direct-colour pixels against hash-verified DGN
Structure2 records, and reports canonical Structure3 face rows that own each
matched selector. It does not infer the SH-2 face-selection call, camera
transform, culling, display origin, HUD/menu ownership, or VDP2 composition.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_sequence import Record, find_chain
from analyze_nexus_vdp1_source_join import be16, be32, swapped_words, ymir_commands
from analyze_nexus_vdp1_dgn_material_join import (
    dgn_materials,
    dgn_structure3_face_owners,
)


def state_copr(state: str) -> int:
    for field in state.split(","):
        if field.startswith("copr:"):
            return int(field[5:], 16)
    raise ValueError("VDP1 state has no COPR")


def dgn_match_index(data_dir: Path, *, ymir_byte_order: bool = False):
    index = {}
    for name, image, palette in dgn_materials(data_dir):
        normalized_image = image if ymir_byte_order else swapped_words(image)
        normalized_palette = (palette if ymir_byte_order else swapped_words(palette)) if palette else b""
        index.setdefault((normalized_image, normalized_palette), []).append(name)
    return index


def command_material(vram: bytes, record):
    control = record.words[0]
    if control & 0xC000 or (control & 0x000F) != 2:
        return None
    mode = (record.words[2] >> 3) & 0x7
    width = ((record.words[5] >> 8) & 0x003F) * 8
    height = record.words[5] & 0x00FF
    bits = 4 if mode <= 1 else 8 if mode <= 4 else 16
    source_offset = record.words[4] * 8
    source_size = width * height * bits // 8
    clut_word = (record.words[3] & ~0x3) << 2
    if mode not in (1, 5) or source_size <= 0:
        return {
            "offset": record.offset,
            "mode": mode,
            "source": b"",
            "palette": b"",
            "source_offset": source_offset,
            "source_size": source_size,
            "clut_word": clut_word,
            "bounded": 0,
        }
    if (source_offset > len(vram) or source_size > len(vram) - source_offset or
            (mode == 1 and
             (clut_word * 2 > len(vram) or 32 > len(vram) - clut_word * 2))):
        return {
            "offset": record.offset,
            "mode": mode,
            "source": b"",
            "palette": b"",
            "source_offset": source_offset,
            "source_size": source_size,
            "clut_word": clut_word,
            "bounded": 0,
        }
    return {
        "offset": record.offset,
        "mode": mode,
        "source": vram[source_offset:source_offset + source_size],
        "palette": (vram[clut_word * 2:clut_word * 2 + 32]
                    if mode == 1 else b""),
        "source_offset": source_offset,
        "source_size": source_size,
        "clut_word": clut_word,
        "bounded": 1,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int)
    parser.add_argument("--ymir-bundle", action="store_true",
                        help="read the authenticated command trace and VDP1 VRAM from a Ymir dump directory")
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()
    try:
        if args.ymir_bundle:
            vram, captured_frame, traced_commands = ymir_commands(args.capture)
            if captured_frame != args.frame:
                raise ValueError(
                    f"requested frame {args.frame} differs from Ymir bundle frame {captured_frame}")
            chain = [Record(offset, words) for offset, words in traced_commands]
            ymir_byte_order = True
        else:
            if args.capture_frames is None:
                raise ValueError("--capture-frames is required for a runtime capture")
            frames, states = frame_regions(args.capture.read_bytes(), args.capture_frames)
            if args.frame < 0 or args.frame >= len(frames):
                raise ValueError("requested frame is outside capture")
            vram = frames[args.frame]["vdp1-vram"]
            chain = find_chain(vram, state_copr(states[args.frame]))
            ymir_byte_order = False
        matches = dgn_match_index(args.data_dir, ymir_byte_order=ymir_byte_order)
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_DGN_SEQUENCE_JOIN_INVALID: {error}")
        return 1

    textured = 0
    source_matches = 0
    palette_matches = 0
    direct_color_matches = 0
    # This receipt proves only that a matched Structure2 image has at least
    # one canonical Structure3 owner.  It does not prove the Saturn SH-2
    # face-selection call or its selected face index.
    face_owner_matches = 0
    bounded_failures = 0
    unmatched_offsets = []
    rows = []
    for record in chain:
        material = command_material(vram, record)
        if material is None:
            continue
        textured += 1
        if not material["bounded"]:
            bounded_failures += 1
            unmatched_offsets.append(record.offset)
            continue
        # DGN bytes are source-file big-endian words; the capture is the
        # VDP1 little-endian word image.  The index is already normalized to
        # the capture representation, so compare the raw captured spans.
        names = matches.get((material["source"], material["palette"]), [])
        if not names:
            unmatched_offsets.append(record.offset)
            continue
        source_matches += 1
        if material["mode"] == 1:
            palette_matches += 1
        elif material["mode"] == 5:
            direct_color_matches += 1
        owners = []
        for name in names:
            marker = name.split("[", 1)[0]
            image_id = int(name.split("Structure2=", 1)[1].split(" ", 1)[0])
            owners.extend(dgn_structure3_face_owners(args.data_dir, marker, image_id))
        if owners:
            face_owner_matches += 1
        rows.append((record.offset, names, owners, material["source_offset"],
                     material["source_size"], material["clut_word"],
                     material["mode"]))

    complete = (textured > 0 and source_matches == textured and
                palette_matches + direct_color_matches == textured and
                face_owner_matches == textured and
                bounded_failures == 0)
    print(f"frame={args.frame} chain_records={len(chain)}")
    print(f"textured_draws={textured} source_matches={source_matches} "
          f"clut_matches={palette_matches} direct_color_matches={direct_color_matches} "
          f"face_owner_matches={face_owner_matches}")
    print(f"bounded_failures={bounded_failures} unmatched_offsets=" +
          (",".join(f"0x{x:05x}" for x in unmatched_offsets) or "none"))
    for offset, names, owners, source_offset, source_size, clut_word, mode in rows:
        print(f"draw=0x{offset:05x} source=0x{source_offset:05x} "
              f"bytes={source_size} color_mode={mode} "
              f"clut_word={'n/a' if mode == 5 else f'0x{clut_word:05x}'} "
              f"dgn={'|'.join(names)} faces={'|'.join(owners)}")
    print(f"sequence_dgn_material_join={'verified' if complete else 'unbound'}")
    print("transform_culling=unproven")
    print("display_origin=unproven")
    print("vdp2_composition=unproven")
    print("semantic_admission=blocked")
    return 0 if (complete or not args.require_complete) else 1


if __name__ == "__main__":
    raise SystemExit(main())
