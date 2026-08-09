#!/usr/bin/env python3
"""Follow one authenticated Saturn VDP1 command-list chain.

The raw producer stores VDP1 VRAM as a little-endian word image.  CMDLINK is
an address in VDP1 command words; the corresponding byte offset in the raw
image is therefore ``CMDLINK << 3``.  This tool follows the same bounded
command-link operation used by the Mednafen VDP1 implementation and reports
only hardware framing:

* draw command records (types 0..7),
* User Clip (type 8), System Clip (type 9), and Local Coordinate (type A),
* the command-link mode and target, and
* an observed END record.

It does not assign a DGN face, MENU.BPK/FONT256 owner, palette meaning,
camera transform, or production-render permission.  A complete chain is
necessary capture evidence, not sufficient game-semantic evidence.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


COMMAND_BYTES = 32
VDP1_VRAM_BYTES = 0x40000 * 2
MAX_CHAIN_RECORDS = 256


@dataclass(frozen=True)
class Record:
    offset: int
    words: tuple[int, ...]

    @property
    def control(self) -> int:
        return self.words[0]

    @property
    def command_type(self) -> int:
        return self.control & 0x000F

    @property
    def jump_mode(self) -> int:
        return (self.control >> 12) & 0x0003

    @property
    def end(self) -> bool:
        return bool(self.control & 0x8000)

    @property
    def link_offset(self) -> int:
        return self.words[1] << 3


def records(vram: bytes) -> list[Record]:
    if len(vram) != VDP1_VRAM_BYTES:
        raise ValueError(f"unexpected VDP1 VRAM size: {len(vram)}")
    return [
        Record(offset, struct.unpack_from("<16H", vram, offset))
        for offset in range(0, len(vram), COMMAND_BYTES)
    ]


def record_metadata(vram: bytes) -> dict[int, tuple[int, int, int]]:
    """Read only CMDCTRL/CMDLINK while searching candidate chains.

    Keeping the 16-word payload out of the search index matters for the
    external 300-frame witness: a full 512 KiB record object for every frame
    is unnecessary and can obscure the actual bounded proof.
    """
    if len(vram) != VDP1_VRAM_BYTES:
        raise ValueError(f"unexpected VDP1 VRAM size: {len(vram)}")
    return {
        offset: (control, link, (control >> 12) & 0x0003,
                 int(any(vram[offset:offset + COMMAND_BYTES])))
        for offset in range(0, len(vram), COMMAND_BYTES)
        for control, link in (struct.unpack_from("<2H", vram, offset),)
    }


def record_at(index: dict[int, Record], offset: int) -> Record:
    if offset % COMMAND_BYTES or offset not in index:
        raise ValueError(f"invalid VDP1 command offset 0x{offset:05x}")
    return index[offset]


def next_offset(record: Record, stack: list[int]) -> int | None:
    if record.end:
        return None
    if record.jump_mode == 0:
        return record.offset + COMMAND_BYTES
    if record.jump_mode == 1:
        return record.link_offset
    if record.jump_mode == 2:
        stack.append(record.offset + COMMAND_BYTES)
        return record.link_offset
    if not stack:
        raise ValueError(
            f"VDP1 return without call at 0x{record.offset:05x}"
        )
    return stack.pop()


def follow(index: dict[int, Record], start_offset: int) -> list[Record]:
    chain: list[Record] = []
    visited: set[tuple[int, tuple[int, ...]]] = set()
    stack: list[int] = []
    offset: int | None = start_offset
    while offset is not None:
        record = record_at(index, offset)
        key = (record.offset, record.words)
        if key in visited:
            raise ValueError(f"VDP1 command loop at 0x{record.offset:05x}")
        visited.add(key)
        chain.append(record)
        if len(chain) > MAX_CHAIN_RECORDS:
            raise ValueError("VDP1 command chain exceeds bounded record limit")
        offset = next_offset(record, stack)
    if not chain[-1].end:
        raise ValueError("VDP1 command chain ended without END")
    return chain


def chain_score(chain: list[Record], current_offset: int) -> tuple[int, int, int, int]:
    draws = sum(not r.end and r.command_type <= 7 for r in chain)
    clips = sum(not r.end and r.command_type in (8, 9) for r in chain)
    locals_ = sum(not r.end and r.command_type == 10 for r in chain)
    current = int(any(r.offset == current_offset for r in chain))
    return current, int(draws > 0 and clips > 0 and locals_ > 0), draws, -len(chain)


def find_chain(vram: bytes, copr: int) -> list[Record]:
    metadata = record_metadata(vram)
    current_offset = copr << 3
    # COPR is a live cursor.  A snapshot taken while VDP1 is drawing can
    # point at an ordinary draw record; a vblank/idle snapshot may point at
    # the terminal END record.  In both cases the admissible chain must
    # contain that cursor and terminate at a later observed END record.
    if current_offset % COMMAND_BYTES or current_offset not in metadata:
        raise ValueError(f"invalid VDP1 COPR offset 0x{current_offset:05x}")

    def follow_offsets(start_offset: int) -> list[int]:
        chain: list[int] = []
        visited: set[tuple[int, tuple[int, int, int]]] = set()
        stack: list[int] = []
        offset: int | None = start_offset
        while offset is not None:
            if offset % COMMAND_BYTES or offset not in metadata:
                raise ValueError(f"invalid VDP1 command offset 0x{offset:05x}")
            control, link, jump_mode, nonempty = metadata[offset]
            if not nonempty:
                raise ValueError(f"empty VDP1 command record at 0x{offset:05x}")
            key = (offset, metadata[offset])
            if key in visited:
                raise ValueError(f"VDP1 command loop at 0x{offset:05x}")
            visited.add(key)
            chain.append(offset)
            if len(chain) > MAX_CHAIN_RECORDS:
                raise ValueError("VDP1 command chain exceeds bounded record limit")
            if control & 0x8000:
                offset = None
            elif jump_mode == 0:
                offset += COMMAND_BYTES
            elif jump_mode == 1:
                offset = link << 3
            elif jump_mode == 2:
                stack.append(offset + COMMAND_BYTES)
                offset = link << 3
            elif not stack:
                raise ValueError(f"VDP1 return without call at 0x{offset:05x}")
            else:
                offset = stack.pop()
        if not chain or not (metadata[chain[-1]][0] & 0x8000):
            raise ValueError("VDP1 command chain ended without END")
        return chain

    def metadata_score(chain: list[int]) -> tuple[int, int, int, int]:
        draws = sum(metadata[offset][0] & 0x000F <= 7 and
                    not (metadata[offset][0] & 0x8000) for offset in chain)
        clips = sum(metadata[offset][0] & 0x000F in (8, 9) and
                    not (metadata[offset][0] & 0x8000) for offset in chain)
        locals_ = sum(metadata[offset][0] & 0x000F == 10 and
                      not (metadata[offset][0] & 0x8000) for offset in chain)
        current = int(current_offset in chain)
        return current, int(draws > 0 and clips > 0 and locals_ > 0), draws, -len(chain)

    candidates: list[tuple[tuple[int, int, int, int], list[int]]] = []
    for start in range(0, VDP1_VRAM_BYTES, COMMAND_BYTES):
        try:
            chain = follow_offsets(start)
        except ValueError:
            continue
        score = metadata_score(chain)
        if score[0] and score[1]:
            candidates.append((score, chain))
    if not candidates:
        current_control = metadata[current_offset][0]
        if current_control & 0x8000:
            # A valid capture may contain a reset/idle frame with no active
            # command chain.  Preserve that observation separately from an
            # active chain; it is not a draw proof.
            return []
        raise ValueError("no bounded draw/clip/local/END chain reaches COPR")
    candidates.sort(key=lambda item: item[0], reverse=True)
    return [
        Record(offset, struct.unpack_from("<16H", vram, offset))
        for offset in candidates[0][1]
    ]


def signed_11(value: int) -> int:
    value &= 0x07FF
    return value - 0x800 if value & 0x400 else value


def describe(chain: list[Record], copr: int) -> dict[str, object]:
    draws = [r for r in chain if not r.end and r.command_type <= 7]
    user_clips = [r for r in chain if not r.end and r.command_type == 8]
    system_clips = [r for r in chain if not r.end and r.command_type == 9]
    locals_ = [r for r in chain if not r.end and r.command_type == 10]
    end = chain[-1]
    user_clip_bounds = [
        (r.words[6] & 0x1FFF, r.words[7] & 0x1FFF,
         r.words[10] & 0x1FFF, r.words[11] & 0x1FFF)
        for r in user_clips
    ]
    system_clip_points = [
        (r.words[10] & 0x1FFF, r.words[11] & 0x1FFF)
        for r in system_clips
    ]
    local_points = [
        (signed_11(r.words[6]), signed_11(r.words[7])) for r in locals_
    ]
    return {
        "start_offset": chain[0].offset,
        "end_offset": end.offset,
        "copr_offset": copr << 3,
        "record_count": len(chain),
        "draw_count": len(draws),
        "user_clip_count": len(user_clips),
        "system_clip_count": len(system_clips),
        "local_coordinate_count": len(locals_),
        "user_clip_bounds": user_clip_bounds,
        "system_clip_points": system_clip_points,
        "local_coordinates": local_points,
        "end_control": end.control,
        "offsets": [r.offset for r in chain],
        "types": ["END" if r.end else f"0x{r.command_type:x}" for r in chain],
    }


def parse_copr(state: str) -> int:
    for field in state.split(","):
        if field.startswith("copr:"):
            return int(field[5:], 16)
    raise ValueError("VDP1 state has no COPR")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--frame", type=int, default=None)
    parser.add_argument("--capture-frames", type=int, default=None)
    parser.add_argument(
        "--summary", action="store_true",
        help="omit per-chain offsets and state-value detail",
    )
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()
    try:
        requested = args.capture_frames or ((args.frame or 0) + 1)
        frames, states = frame_regions(args.capture.read_bytes(), requested)
        frame_numbers = [args.frame] if args.frame is not None else range(len(frames))
        complete = 0
        idle = 0
        for frame in frame_numbers:
            if frame < 0 or frame >= len(frames):
                raise ValueError(f"frame outside capture: {frame}")
            chain = find_chain(frames[frame]["vdp1-vram"], parse_copr(states[frame]))
            if not chain:
                idle += 1
                print(f"frame={frame} idle_end=1")
                continue
            info = describe(chain, parse_copr(states[frame]))
            complete += 1
            print(
                f"frame={frame} start=0x{info['start_offset']:05x} "
                f"end=0x{info['end_offset']:05x} records={info['record_count']} "
                f"draws={info['draw_count']} user_clip={info['user_clip_count']} "
                f"system_clip={info['system_clip_count']} "
                f"local_coord={info['local_coordinate_count']} "
                f"end_control=0x{info['end_control']:04x}"
            )
            if not args.summary:
                print(f"  offsets=" + ",".join(f"0x{x:05x}" for x in info["offsets"]))
                print(f"  types=" + ",".join(info["types"]))
                print(f"  user_clip_bounds={info['user_clip_bounds']}")
                print(f"  system_clip_points={info['system_clip_points']}")
                print(f"  local_coordinates={info['local_coordinates']}")
        print(f"complete_chains={complete} idle_end_frames={idle} "
              f"covered_frames={complete + idle}")
        print("semantic_admission=blocked")
        if args.require_complete and complete + idle != len(list(frame_numbers)):
            return 1
        return 0
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_COMMAND_SEQUENCE_INVALID: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
