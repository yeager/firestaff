#!/usr/bin/env python3
"""Validate generic Mednafen and Firestaff Saturn VDP capture streams.

This checker is deliberately game-independent.  It validates only the
transport layout emitted by either the proposed ``ss.capture.*`` settings or
the external Firestaff V2 producer; it does not infer a game, asset, menu,
HUD, or viewport meaning from the bytes.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


MDFN_MAGIC = b"MDFN_SS_SATURN_RUNTIME_CAPTURE_V1\n"
FIRESTAFF_MAGIC = b"FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
MDFN_VDP1 = b"VDP1_RAW\n"
FIRESTAFF_VDP1 = b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2\n"
VDP2 = b"VDP2_RAW\n"
VDP1_WORDS = 0x40000 + (0x20000 * 2)
VDP1_BYTES = VDP1_WORDS * 2
VDP2_BYTES = (0x100 + 0x40000 + 0x800) * 2


def read_u16_be(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 2 > len(data):
        raise ValueError("truncated 16-bit field")
    return struct.unpack_from(">H", data, offset)[0]


def validate(data: bytes, required_frames: int) -> tuple[int, list[tuple[int, int, int, int]]]:
    if data.startswith(MDFN_MAGIC):
        offset = len(MDFN_MAGIC)
        producer_v2 = False
    elif data.startswith(FIRESTAFF_MAGIC):
        offset = len(FIRESTAFF_MAGIC)
        producer_v2 = True
    else:
        raise ValueError("missing Mednafen Saturn capture magic")
    frames: list[tuple[int, int, int, int]] = []
    while offset < len(data):
        marker = f"frame={len(frames)}\n".encode("ascii")
        if not data.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        frame_offset = offset
        offset += len(marker)
        vdp1_marker = FIRESTAFF_VDP1 if producer_v2 else MDFN_VDP1
        if not data.startswith(vdp1_marker, offset):
            raise ValueError(f"missing VDP1 marker for frame {len(frames)}")
        offset += len(vdp1_marker)
        has_state = data.startswith(b"state=", offset)
        if has_state:
            state_end = data.find(b"\n", offset)
            if state_end < 0:
                raise ValueError(f"missing VDP1 state line for frame {len(frames)}")
            offset = state_end + 1
        if producer_v2 and not has_state:
            raise ValueError(f"missing VDP1 V2 state line for frame {len(frames)}")
        draw_buffer_bytes = 0 if producer_v2 else 1
        if offset + VDP1_BYTES + draw_buffer_bytes > len(data):
            raise ValueError(f"truncated VDP1 payload for frame {len(frames)}")
        offset += VDP1_BYTES
        if producer_v2:
            draw_buffer = -1
        else:
            draw_buffer = data[offset]
            if draw_buffer not in (0, 1):
                raise ValueError(f"invalid draw-buffer selector {draw_buffer}")
            offset += 1
        if not data.startswith(VDP2, offset):
            raise ValueError(f"missing VDP2 marker for frame {len(frames)}")
        offset += len(VDP2)
        if offset + VDP2_BYTES > len(data):
            raise ValueError(f"truncated VDP2 payload for frame {len(frames)}")
        # The first register is TVMD; BGON is at the Saturn VDP2 register
        # offset used by Firestaff's transport reader.  This checks the
        # candidate's explicit big-endian encoding without assigning meaning.
        tvmd = read_u16_be(data, offset)
        bgon = read_u16_be(data, offset + 0x20)
        frames.append((frame_offset, draw_buffer, tvmd, bgon))
        offset += VDP2_BYTES
    if len(frames) != required_frames:
        raise ValueError(f"expected {required_frames} frames, found {len(frames)}")
    if offset != len(data):
        raise ValueError("trailing bytes after final frame")
    return len(frames), frames


def self_test() -> None:
    registers = bytearray(0x200)
    struct.pack_into(">H", registers, 0x00, 0x8000)
    struct.pack_into(">H", registers, 0x20, 0x0002)
    vdp2 = registers + bytes(VDP2_BYTES - len(registers))
    mdfn_frame = bytearray()
    mdfn_frame += b"frame=0\n" + MDFN_VDP1
    mdfn_frame += bytes(VDP1_BYTES) + b"\x01" + VDP2 + vdp2
    count, _ = validate(MDFN_MAGIC + mdfn_frame, 1)
    assert count == 1

    firestaff_frame = bytearray()
    firestaff_frame += b"frame=0\n" + FIRESTAFF_VDP1
    firestaff_frame += b"state=tvmr:00,fbcr:00,ptmr:00,edsr:00,lopr:0000,copr:000000,ret:00000000,fb:0,sysclipx:013f,sysclipy:00df\n"
    firestaff_frame += bytes(VDP1_BYTES) + VDP2 + vdp2
    count, rows = validate(FIRESTAFF_MAGIC + firestaff_frame, 1)
    assert count == 1 and rows[0][1] == -1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, nargs="?")
    parser.add_argument("--require-frames", type=int, default=1)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    try:
        if args.self_test:
            self_test()
            print("mednafen Saturn capture layout self-test: PASS")
            return 0
        if args.capture is None:
            parser.error("capture is required unless --self-test is used")
        count, frames = validate(args.capture.read_bytes(), args.require_frames)
    except (AssertionError, OSError, ValueError) as error:
        print(f"MEDNAFEN_SATURN_CAPTURE_INVALID: {error}")
        return 1
    print(f"MEDNAFEN_SATURN_CAPTURE_LAYOUT: frames={count} offsets="
          + ",".join(str(offset) for offset, _, _, _ in frames))
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
