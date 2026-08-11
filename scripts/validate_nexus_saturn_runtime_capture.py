#!/usr/bin/env python3
"""Validate the raw VDP1/VDP2 witness emitted by the Saturn capture patch.

This validates transport/layout only.  It intentionally does not admit PRS3,
SLEV/SAL, HUD, viewport, tilemap, CLUT, or mesh semantics.  The optional
VDP1-activity check only separates an active raw VDP1 witness from an idle
startup frame; it does not identify the consumer or authorize a draw.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


RUNTIME_MAGIC = b"FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
MDFN_RUNTIME_MAGIC = b"MDFN_SS_SATURN_RUNTIME_CAPTURE_V1\n"
VDP1_MAGIC = b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V1\n"
VDP1_MAGIC_V2 = b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2\n"
VDP1_MAGIC_MDFN = b"VDP1_RAW\n"
VDP2_MAGIC = b"VDP2_RAW\n"
# VDP1::FirestaffCaptureRaw() writes VRAM and both framebuffers.  The
# framebuffer selector is already represented by the V2 state line; the
# emitted payload is exactly 0x100000 bytes in the retail capture patch.
VDP1_PAYLOAD_BYTES = 0x40000 * 2 + 0x20000 * 2 + 0x20000 * 2
VDP2_PAYLOAD_BYTES = 0x100 * 2 + 0x40000 * 2 + 0x800 * 2
VDP1_STATE_RE = re.compile(
    rb"^state=tvmr:[0-9a-f]+,fbcr:[0-9a-f]+,ptmr:([0-9a-f]+),"
    rb"edsr:([0-9a-f]+),lopr:[0-9a-f]+,copr:[0-9a-f]+,"
    # The current Firestaff patch appends the captured system-clip words.
    # Older operator-built Mednafen binaries emit the same V2 state without
    # those optional fields. They are not needed for transport validation;
    # source ownership and production placement remain separately gated.
    rb"ret:[0-9a-f]+,fb:[01](?:,sysclipx:[0-9a-f]+,sysclipy:[0-9a-f]+)?$"
)


def validate(blob: bytes, required_frames: int) -> tuple[int, list[int]]:
    runtime_magic = next(
        (magic for magic in (RUNTIME_MAGIC, MDFN_RUNTIME_MAGIC)
         if blob.startswith(magic)),
        None,
    )
    if runtime_magic is None:
        raise ValueError("missing runtime capture magic")

    offset = len(runtime_magic)
    frames: list[int] = []
    while offset < len(blob):
        marker = f"frame={len(frames)}\n".encode("ascii")
        if not blob.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        frames.append(offset)
        offset += len(marker)
        if not (blob.startswith(VDP1_MAGIC, offset) or
                blob.startswith(VDP1_MAGIC_V2, offset) or
                blob.startswith(VDP1_MAGIC_MDFN, offset)):
            raise ValueError(f"missing VDP1 marker for frame {len(frames) - 1}")
        if blob.startswith(VDP1_MAGIC_V2, offset):
            offset += len(VDP1_MAGIC_V2)
            state_end = blob.find(b"\n", offset)
            if state_end < 0 or not blob.startswith(b"state=", offset):
                raise ValueError(f"missing VDP1 state line for frame {len(frames) - 1}")
            offset = state_end + 1
        elif blob.startswith(VDP1_MAGIC, offset):
            offset += len(VDP1_MAGIC)
        else:
            offset += len(VDP1_MAGIC_MDFN)
        offset += VDP1_PAYLOAD_BYTES
        # Early V2 captures appended the draw-buffer selector after the fixed
        # VDP1 payload. It is redundant metadata, not part of the VDP1 image.
        if not blob.startswith(VDP2_MAGIC, offset) and \
                blob.startswith(VDP2_MAGIC, offset + 1):
            offset += 1
        if not blob.startswith(VDP2_MAGIC, offset):
            raise ValueError(f"missing VDP2 marker for frame {len(frames) - 1}")
        offset += len(VDP2_MAGIC) + VDP2_PAYLOAD_BYTES

    if offset != len(blob):
        raise ValueError("trailing bytes after final frame")
    if len(frames) != required_frames:
        raise ValueError(f"expected {required_frames} frames, found {len(frames)}")
    return len(frames), frames


def validate_vdp1_activity(blob: bytes, frame_offsets: list[int]) -> int:
    """Require an observed VDP1 execution state without admitting semantics.

    This is intentionally weaker than a menu/HUD/viewport binding.  It only
    proves that the V2 producer observed a non-idle VDP1 command engine and
    that its captured VRAM/framebuffer payload was not all zero.  The caller
    must still bind command, CLUT, source asset and destination ownership.
    """
    active_frames = 0
    for frame_index, frame_offset in enumerate(frame_offsets):
        offset = frame_offset + len(f"frame={frame_index}\n".encode("ascii"))
        if not blob.startswith(VDP1_MAGIC_V2, offset):
            raise ValueError("VDP1 activity requires the V2 state marker")
        offset += len(VDP1_MAGIC_V2)
        state_end = blob.find(b"\n", offset)
        if state_end < 0:
            raise ValueError("missing VDP1 V2 state line")
        match = VDP1_STATE_RE.fullmatch(blob[offset:state_end])
        if not match:
            raise ValueError("malformed VDP1 V2 state line")
        ptmr, edsr = (int(value, 16) for value in match.groups())
        offset = state_end + 1
        vdp1 = blob[offset : offset + VDP1_PAYLOAD_BYTES]
        vram = vdp1[: 0x40000 * 2]
        fb0 = vdp1[0x40000 * 2 : 0x40000 * 2 + 0x20000 * 2]
        fb1 = vdp1[0x40000 * 2 + 0x20000 * 2 : 0x40000 * 2 + 0x20000 * 4]
        if ptmr != 0 and edsr != 0 and (any(vram) or any(fb0) or any(fb1)):
            active_frames += 1
    if active_frames == 0:
        raise ValueError("no frame contains non-idle VDP1 state and nonzero payload")
    return active_frames


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--require-frames", type=int, default=2)
    parser.add_argument(
        "--require-vdp1-activity",
        action="store_true",
        help="require a V2 non-idle VDP1 state and nonzero captured payload",
    )
    args = parser.parse_args()
    try:
        blob = args.capture.read_bytes()
        count, offsets = validate(blob, args.require_frames)
        active_frames = (
            validate_vdp1_activity(blob, offsets) if args.require_vdp1_activity else None
        )
    except (OSError, ValueError) as error:
        print(f"NEXUS_SATURN_RUNTIME_CAPTURE_INVALID: {error}")
        return 1
    print(
        "NEXUS_SATURN_RUNTIME_CAPTURE_RAW_LAYOUT: "
        f"frames={count} bytes={len(blob)} offsets={','.join(map(str, offsets))}"
    )
    if active_frames is not None:
        print(f"vdp1_active_observation=frames:{active_frames}")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
