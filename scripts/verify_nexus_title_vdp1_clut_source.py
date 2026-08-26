#!/usr/bin/env python3
"""Verify the observed title VDP1 CLUT's real LEV00 CD-to-VRAM route."""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_sh2_source_trace import read_rows
from analyze_nexus_title_vdp2_source import cue_track1
from verify_nexus_vdp1_ram_to_vram_copy import registers, swap_words


TITLE_COPY_PC = 0x06041FAC
CLUT_VRAM_ADDRESS = 0x19400


def raw_sector(track: bytes, lba: int) -> bytes:
    offset = lba * 2352 + 16
    result = track[offset:offset + 2048]
    if len(result) != 2048:
        raise ValueError("CUE track lacks the requested user-data sector")
    return result


def iso_member_at_lba(track: bytes, wanted_lba: int) -> str:
    pvd = raw_sector(track, 16)
    if pvd[1:6] != b"CD001":
        raise ValueError("CUE track lacks an ISO9660 primary volume descriptor")

    def walk(record: bytes, prefix: str = "") -> str | None:
        lba = int.from_bytes(record[2:6], "little")
        size = int.from_bytes(record[10:14], "little")
        directory = b"".join(raw_sector(track, lba + index)
                             for index in range((size + 2047) // 2048))[:size]
        offset = 0
        while offset < len(directory):
            length = directory[offset]
            if length == 0:
                offset = ((offset // 2048) + 1) * 2048
                continue
            child = directory[offset:offset + length]
            if len(child) < 34:
                raise ValueError("ISO9660 directory record is truncated")
            name = child[33:33 + child[32]].decode("ascii", "replace").split(";", 1)[0]
            if name not in ("\x00", "\x01"):
                child_lba = int.from_bytes(child[2:6], "little")
                child_size = int.from_bytes(child[10:14], "little")
                child_name = f"{prefix}/{name}".strip("/")
                if child[25] & 2:
                    found = walk(child, child_name)
                    if found:
                        return found
                elif child_lba <= wanted_lba < child_lba + (child_size + 2047) // 2048:
                    return child_name
            offset += length
        return None

    result = walk(pvd[156:])
    if result is None:
        raise ValueError("source LBA has no ISO9660 file owner")
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("source_trace", type=Path)
    parser.add_argument("--cue", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    args = parser.parse_args()
    try:
        pc, target, values = registers(args.registers)
        if pc != TITLE_COPY_PC or target != CLUT_VRAM_ADDRESS:
            raise ValueError("receipt is not the observed title CLUT writer")
        source, size = values[0], values[6]
        if size != 32:
            raise ValueError("title CLUT writer does not name a 32-byte source")
        rows = read_rows(args.source_trace)
        candidates = []
        for start in range(len(rows)):
            if rows[start][0] != source:
                continue
            candidate, end = [], source
            for row in rows[start:]:
                if row[0] != end:
                    break
                candidate.append(row)
                end += row[1]
                if end == source + size:
                    break
            if end != source + size or len({row[5] for row in candidate}) != 1:
                continue
            source_bytes = b"".join(
                (row[2] & ((1 << (row[1] * 8)) - 1)).to_bytes(row[1], "big")
                for row in candidate
            )
            candidates.append((candidate[0][5], source_bytes))
        if not candidates:
            raise ValueError("source trace lacks one contiguous title CLUT receipt")
        track = cue_track1(args.cue).read_bytes()
        frame = next(regions for index, regions in
                     iter_frame_regions_file(args.capture, args.frame + 1)
                     if index == args.frame)
        observed = frame["vdp1-vram"][target:target + size]
        matches = [(lba, blob) for lba, blob in candidates
                   if lba >= 0 and swap_words(blob) == observed]
        if len(matches) != 1:
            raise ValueError("source trace does not uniquely reconstruct the VDP1 CLUT")
        lba, source_bytes = matches[0]
        member = iso_member_at_lba(track, lba)
        if member.rsplit("/", 1)[-1].upper() != "LEV00.DGN":
            raise ValueError(f"title CLUT source owner is {member}, not LEV00.DGN")
        sector = raw_sector(track, lba)
        if source_bytes not in sector:
            raise ValueError("receipt bytes are absent from the authenticated source sector")
    except (OSError, StopIteration, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP1_CLUT_SOURCE_INVALID: {error}")
        return 1
    print(f"writer_pc=0x{pc:08x} ram=0x{source:08x}:0x{source + size:08x}")
    print(f"vdp1_clut=0x{target:05x}:0x{target + size:05x} source_lba={lba}")
    print(f"source_member={member}")
    print("saturn_word_byte_order=verified")
    print("title_vdp1_clut_lev00_source=verified")
    print("geometry_compositor_timing_semantics=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
