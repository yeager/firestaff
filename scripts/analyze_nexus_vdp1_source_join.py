#!/usr/bin/env python3
"""Check a real VDP1 type-2 source span against retail texture bytes.

The command/source join is an observation boundary only.  A byte match would
still require a CLUT, placement, command-order and source-owner receipt before
the production renderer could use it; an absent match is useful negative
evidence and never authorizes a fallback texture.  The DGN candidates are
only the bounded raw image spans of hash-verified retail Structure2 records;
their pixel and palette semantics remain unproven.
"""

from __future__ import annotations

import argparse
import array
import hashlib
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp1_command_sequence import find_chain, parse_copr
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


# These extracted-file identities supplement the Japanese disc manifest.
# MENU.BPK has three authenticated regional hashes; the same named resource is
# not byte-identical across the Japanese, English, and French retail discs.
# All are authenticated inputs, never filename-only admission.
STARTUP_ASSET_HASHES = {
    "MENU.BPK": {
        "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636",  # Japanese
        "f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca",  # English
        "c4e2427f54083e92cdf38f3b1f296e135bdb007de227431be690cc41381fd543",  # French
    },
    "FONT256.S2D": {"764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb"},
    "TITLE.BIN": {"a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"},
    "TITLE.CG": {"fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580"},
    "STABG.BIN": {"7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913"},
}


def accepted_retail_hashes(name: str) -> frozenset[str]:
    """Return only verified regional identities for one extracted resource."""
    accepted = set(STARTUP_ASSET_HASHES.get(name, ()))
    disc_hash = DISC_HASH.get(name)
    if disc_hash is not None:
        accepted.add(disc_hash)
    return frozenset(accepted)


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 4], "big")


def mns_surfaces(data_dir: Path) -> list[tuple[str, bytes]]:
    surfaces: list[tuple[str, bytes]] = []
    for path in sorted(data_dir.glob("*.MNS")):
        data = path.read_bytes()
        if len(data) < 0x2C or data[:4] != b"DMDF" or be32(data, 4) != len(data):
            continue
        text_offset = be32(data, 0x24)
        if (text_offset + 12 > len(data) or
                data[text_offset:text_offset + 4] != b"TEXT"):
            continue
        text_size = be32(data, text_offset + 4)
        count = be32(data, text_offset + 8)
        if text_size < 0x24 or text_offset + text_size > len(data) or count > 64:
            continue
        for index in range(count):
            descriptor = text_offset + 0x24 + index * 20
            if descriptor + 20 > text_offset + text_size:
                break
            image_id = be16(data, descriptor)
            if image_id == 0xFFFF:
                break
            width = be16(data, descriptor + 6)
            height = be16(data, descriptor + 8)
            relative = be32(data, descriptor + 12)
            size = width * height * 2
            start = text_offset + relative
            if width == 0 or height == 0 or start < text_offset:
                continue
            if relative + size > text_size or start + size > len(data):
                continue
            raw = data[start:start + size]
            surfaces.append((f"{path.name}[{index}] {width}x{height}", raw))
    return surfaces


def dgn_structure2_surfaces(data_dir: Path) -> list[tuple[str, bytes]]:
    """Return bounded raw image spans from the canonical LEV00..LEV15 set."""
    surfaces: list[tuple[str, bytes]] = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        path = data_dir / name
        data = path.read_bytes()
        expected_hash = DISC_HASH.get(name)
        if expected_hash is None or hashlib.sha256(data).hexdigest() != expected_hash:
            raise ValueError(f"{name} is not the canonical retail DGN")
        if len(data) < 0x1C:
            raise ValueError(f"{name} DGN header is truncated")
        block = be16(data, 0x14)
        blocks = be16(data, 0x16)
        useful = be32(data, 0x18)
        payload_start = block * 0x800
        payload_size = blocks * 0x800
        if (block == 0 or blocks == 0 or payload_start > len(data) or
                payload_size > len(data) - payload_start or useful > payload_size):
            raise ValueError(f"{name} Structure2 envelope is invalid")
        cursor = 0
        descriptor_index = 0
        while cursor + 20 <= useful:
            descriptor = data[payload_start + cursor:payload_start + cursor + 20]
            image_id = be16(descriptor, 0)
            if image_id == 0xFFFF:
                break
            if image_id != descriptor_index:
                raise ValueError(f"{name} Structure2 image ids are not sequential")
            encoding = be16(descriptor, 2)
            width = be16(descriptor, 6)
            height = be16(descriptor, 8)
            relative = be32(descriptor, 12)
            if width == 0 or height == 0 or encoding not in (0x0008, 0x0028):
                raise ValueError(f"{name} Structure2 descriptor is invalid")
            image_bytes = ((width * height + 1) // 2
                           if encoding == 0x0008 else width * height * 2)
            if relative > useful or image_bytes > useful - relative:
                raise ValueError(f"{name} Structure2 image span is invalid")
            start = payload_start + relative
            surfaces.append((
                f"{name}[Structure2={image_id} encoding=0x{encoding:04x} "
                f"{width}x{height}]", data[start:start + image_bytes]))
            descriptor_index += 1
            cursor += 20
        else:
            raise ValueError(f"{name} Structure2 terminator is missing")
    return surfaces


def swapped_words(data: bytes) -> bytes:
    words = array.array("H")
    even_size = len(data) & ~1
    words.frombytes(data[:even_size])
    words.byteswap()
    return words.tobytes() + data[even_size:]


def authenticated_retail_files(
        data_dir: Path) -> tuple[list[tuple[str, bytes, bytes]], int]:
    """Read and byte-swap verified retail resources once per analyzer run."""
    files: list[tuple[str, bytes, bytes]] = []
    rejected = 0
    expected_names = set(DISC_HASH) | set(STARTUP_ASSET_HASHES)
    for name in sorted(expected_names):
        path = data_dir / name
        if not path.is_file():
            continue
        data = path.read_bytes()
        actual_hash = hashlib.sha256(data).hexdigest()
        if actual_hash not in accepted_retail_hashes(name):
            rejected += 1
            continue
        swapped = swapped_words(data)
        files.append((name, data, swapped))
    return files, rejected


def retail_file_matches(
        retail_files: list[tuple[str, bytes, bytes]],
        source: bytes) -> tuple[list[str], list[str]]:
    """Find exact byte and word-swapped joins in authenticated retail files."""
    exact: list[str] = []
    word_swap: list[str] = []
    for name, data, swapped in retail_files:
        if data.find(source) >= 0:
            exact.append(name)
        if swapped and swapped.find(source) >= 0:
            word_swap.append(name)
    return exact, word_swap


def aggregate_source_join_status(draw_count: int, joined_count: int) -> str:
    """Summarize per-draw byte joins without promoting one hit to all draws."""
    if draw_count <= 0:
        return "no_draws"
    if joined_count <= 0:
        return "unbound"
    if joined_count < draw_count:
        return "partial"
    return "complete"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP1_SOURCE_JOIN_INVALID: negative frame")
        return 1
    try:
        required = args.capture_frames or args.frame + 1
        selected_frame = None
        for frame_index, frame in iter_frame_regions_file(args.capture, required):
            if frame_index == args.frame:
                selected_frame = frame
                break
        if selected_frame is None:
            raise ValueError("selected frame is absent from capture")
        commands = [
            (record.offset, record.words)
            for record in find_chain(
                selected_frame["vdp1-vram"],
                parse_copr(selected_frame["vdp1-state"]),
            )
        ]
        mns = mns_surfaces(args.data_dir)
        dgn = dgn_structure2_surfaces(args.data_dir)
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_SOURCE_JOIN_INVALID: {error}")
        return 1

    draws: list[tuple[int, int, int, bytes]] = []
    vram = selected_frame["vdp1-vram"]
    for offset, words in commands:
        control = words[0]
        command_type = control & 0x000F
        if command_type > 2 or control & 0x8000:
            continue
        colour_mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = (words[5] >> 8) & 0x00FF
        bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = (width * height * bits_per_pixel) // 8
        source_end = source_offset + source_size
        # VDP1 command lists may contain zero-sized control/system records
        # with a normal command-type nibble. They are not texture sources;
        # ignore them rather than rejecting the complete authenticated list.
        if source_size <= 0:
            continue
        if source_end > len(vram):
            print("NEXUS_VDP1_SOURCE_JOIN_INVALID: source span outside VDP1 VRAM")
            return 1
        draws.append((offset, colour_mode, source_offset, vram[source_offset:source_end]))

    print(
        f"frame={args.frame} draw_commands={len(draws)} "
        f"mns_surfaces={len(mns)} dgn_structure2_surfaces={len(dgn)}"
    )
    retail_files, rejected_files = authenticated_retail_files(args.data_dir)
    joined_draws = 0
    for offset, colour_mode, source_offset, source in draws:
        source_hash = hashlib.sha256(source).hexdigest()
        exact: list[str] = []
        swapped_exact: list[str] = []
        dgn_exact: list[str] = []
        dgn_swapped_exact: list[str] = []
        for name, surface in mns:
            if surface == source:
                exact.append(name)
            if swapped_words(surface) == source:
                swapped_exact.append(name)
        for name, surface in dgn:
            if surface == source:
                dgn_exact.append(name)
            if swapped_words(surface) == source:
                dgn_swapped_exact.append(name)
        file_exact, file_word_swap = retail_file_matches(retail_files, source)
        draw_joined = bool(
            exact or swapped_exact or dgn_exact or dgn_swapped_exact or
            file_exact or file_word_swap
        )
        if draw_joined:
            joined_draws += 1
        print(
            f"command_offset=0x{offset:05x} colour_mode={colour_mode} "
            f"source_offset=0x{source_offset:05x} source_bytes={len(source)} "
            f"source_sha256={source_hash}"
        )
        print("mns_exact=" + ("|".join(exact) if exact else "none"))
        print("mns_word_swap_exact=" +
              ("|".join(swapped_exact) if swapped_exact else "none"))
        print("dgn_structure2_exact=" +
              ("|".join(dgn_exact) if dgn_exact else "none"))
        print("dgn_structure2_word_swap_exact=" +
              ("|".join(dgn_swapped_exact) if dgn_swapped_exact else "none"))
        print("retail_file_exact=" + ("|".join(file_exact) if file_exact else "none"))
        print("retail_file_word_swap_exact=" +
              ("|".join(file_word_swap) if file_word_swap else "none"))
        print(f"retail_files_scanned={len(retail_files)}")
        print(f"retail_files_hash_rejected={rejected_files}")
    print(f"source_joined_draws={joined_draws}/{len(draws)}")
    print("source_join=" + aggregate_source_join_status(len(draws), joined_draws))
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
