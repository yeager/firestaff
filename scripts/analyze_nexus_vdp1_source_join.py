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
import csv
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
    """Extract surfaces only from resources matching an authenticated disc hash."""
    surfaces: list[tuple[str, bytes]] = []
    for path in sorted(data_dir.glob("*.MNS")):
        data = path.read_bytes()
        if hashlib.sha256(data).hexdigest() not in accepted_retail_hashes(path.name):
            continue
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


def has_informative_source(source: bytes) -> bool:
    """Zero-filled VRAM can match padding in unrelated authenticated files."""
    return bool(source) and any(source)


def texture_dimensions(size_word: int) -> tuple[int, int]:
    """Decode Saturn CMDSIZE: horizontal cells in bits 13..8, height in 7..0."""
    return ((size_word >> 8) & 0x3F) * 8, size_word & 0xFF


def ymir_commands(bundle: Path) -> tuple[bytes, int, list[tuple[int, tuple[int, ...]]]]:
    """Read the last recorded VDP1 frame without relabelling its source."""
    receipt = {}
    for line in (bundle / "vdp1-capture-bundle.txt").read_text(encoding="ascii").splitlines():
        key, value = line.split("=", 1)
        if key in receipt:
            raise ValueError("duplicate Ymir receipt field")
        receipt[key] = value
    if (receipt.get("format") != "ymir-vdp1-capture-bundle-v1" or
            receipt.get("trace_file_complete") != "1" or
            receipt.get("trace_dropped_total") != "0"):
        raise ValueError("incomplete or dropped Ymir VDP1 trace")
    frame = int(receipt["capture_frame_index"])
    with (bundle / "vdp1-command-trace.csv").open(newline="", encoding="ascii") as stream:
        rows = list(csv.DictReader(stream))
    if len(rows) != int(receipt["trace_rows"]):
        raise ValueError("Ymir trace row count mismatch")
    selected = [row for row in rows if int(row["frame"]) == frame]
    if len(selected) != int(receipt["trace_rows_for_capture_frame"]):
        raise ValueError("Ymir capture frame row count mismatch")
    commands = []
    for index, row in enumerate(selected):
        if int(row["index"]) != index:
            raise ValueError("Ymir command indices are not contiguous")
        commands.append((int(row["address"], 0),
                         tuple(int(row[f"word{i}"], 0) for i in range(16))))
    if not commands:
        raise ValueError("Ymir capture frame has no commands")
    return_address = None
    for index, (address, words) in enumerate(commands):
        if address < 0 or address % 32 or address + 32 > 512 * 1024:
            raise ValueError("Ymir command address is out of bounds or unaligned")
        control, link = words[:2]
        if control & 0x8000:
            if index != len(commands) - 1:
                raise ValueError("Ymir trace terminates early")
            continue
        mode = (control >> 12) & 3
        if mode == 0:
            target = (address + 32) & 0x7FFFF
        elif mode == 1:
            target = (link << 3) & 0x7FFE0
        elif mode == 2:
            if return_address is None:
                return_address = (address + 32) & 0x7FFFF
            target = (link << 3) & 0x7FFE0
        elif return_address is not None:
            target = return_address
            return_address = None
        else:
            target = (address + 32) & 0x7FFFF
        if index + 1 >= len(commands) or commands[index + 1][0] != target:
            raise ValueError("Ymir command transition does not match CMDLINK")
    if not (commands[-1][1][0] & 0x8000):
        raise ValueError("Ymir command trace lacks END")
    vram = (bundle / "vdp1-vram.bin").read_bytes()
    if len(vram) != 512 * 1024:
        raise ValueError("Ymir VDP1 VRAM size mismatch")
    return vram, frame, commands


def ymir_trace_prefix(bundle: Path) -> tuple[bytes, int, list[tuple[int, tuple[int, ...]]]]:
    """Read a complete Ymir trace snapshot without claiming its frame ended.

    Ymir's trace file can be complete while the captured frame's command list
    is still a prefix.  In that case the final observed command need not carry
    END and may link to an address whose commands have not executed yet.
    Preserve the observed execution order and validate the capture receipt,
    indices, addresses, and dump bounds; callers must not treat the result as
    a completed display list.
    """
    with (bundle / "vdp1-capture-bundle.txt").open(encoding="ascii") as stream:
        receipt = {}
        for line in stream:
            key, value = line.rstrip("\n").split("=", 1)
            if key in receipt:
                raise ValueError("duplicate Ymir receipt field")
            receipt[key] = value
    if (receipt.get("format") != "ymir-vdp1-capture-bundle-v1" or
            receipt.get("trace_file_complete") != "1" or
            receipt.get("trace_dropped_total") != "0"):
        raise ValueError("incomplete or dropped Ymir VDP1 trace")
    frame = int(receipt["capture_frame_index"])
    if receipt.get("trace_file") != "vdp1-command-trace.csv":
        raise ValueError("unexpected Ymir VDP1 trace filename")
    with (bundle / "vdp1-command-trace.csv").open(
            newline="", encoding="ascii") as stream:
        rows = list(csv.DictReader(stream))
    if len(rows) != int(receipt["trace_rows"]):
        raise ValueError("Ymir trace row count mismatch")
    selected = [row for row in rows if int(row["frame"]) == frame]
    if len(selected) != int(receipt["trace_rows_for_capture_frame"]):
        raise ValueError("Ymir capture frame row count mismatch")
    commands = []
    for index, row in enumerate(selected):
        address = int(row["address"], 0)
        words = tuple(int(row[f"word{i}"], 0) for i in range(16))
        if int(row["index"]) != index:
            raise ValueError("Ymir command indices are not contiguous")
        if address < 0 or address % 32 or address + 32 > 512 * 1024:
            raise ValueError("Ymir command address is out of bounds or unaligned")
        commands.append((address, words))
    if not commands:
        raise ValueError("Ymir capture frame has no commands")
    if (int(receipt["capture_next_command_index"]) != len(commands) or
            int(receipt["capture_current_command_address"]) != commands[-1][0]):
        raise ValueError("Ymir capture cursor does not match its trace prefix")
    vram = (bundle / "vdp1-vram.bin").read_bytes()
    if len(vram) != 512 * 1024:
        raise ValueError("Ymir VDP1 VRAM size mismatch")
    return vram, frame, commands


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    parser.add_argument("--ymir-bundle", action="store_true",
                        help="read the captured frame from a Ymir dump directory")
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP1_SOURCE_JOIN_INVALID: negative frame")
        return 1
    try:
        if args.ymir_bundle:
            vram, frame_index, commands = ymir_commands(args.capture)
        else:
            required = args.capture_frames or args.frame + 1
            selected_frame = None
            for frame_index, frame in iter_frame_regions_file(args.capture, required):
                if frame_index == args.frame:
                    selected_frame = frame
                    break
            if selected_frame is None:
                raise ValueError("selected frame is absent from capture")
            vram = selected_frame["vdp1-vram"]
            commands = [
                (record.offset, record.words)
                for record in find_chain(vram, parse_copr(selected_frame["vdp1-state"]))
            ]
        mns = mns_surfaces(args.data_dir)
        dgn = dgn_structure2_surfaces(args.data_dir)
    except (OSError, KeyError, ValueError, struct.error, csv.Error) as error:
        print(f"NEXUS_VDP1_SOURCE_JOIN_INVALID: {error}")
        return 1

    draws: list[tuple[int, int, int, bytes]] = []
    for offset, words in commands:
        control = words[0]
        command_type = control & 0x000F
        if command_type > 2 or control & 0xC000:
            continue
        colour_mode = (words[2] >> 3) & 0x7
        width, height = texture_dimensions(words[5])
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
        f"frame={frame_index} draw_commands={len(draws)} "
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
        ) and has_informative_source(source)
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
        print(f"source_informative={int(has_informative_source(source))}")
    print(f"source_joined_draws={joined_draws}/{len(draws)}")
    print("source_join=" + aggregate_source_join_status(len(draws), joined_draws))
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
