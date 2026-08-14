#!/usr/bin/env python3
"""Report region hashes and real frame changes in a Saturn raw witness.

This is deliberately a transport/observation tool.  A changed framebuffer or
register region proves that the instrumented Saturn producer observed changing
runtime state; it does not identify a menu, HUD, viewport, CLUT, or consumer.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from validate_nexus_saturn_runtime_capture import (
    MDFN_RUNTIME_MAGIC,
    RUNTIME_MAGIC,
    VDP1_MAGIC,
    VDP1_MAGIC_V2,
    VDP1_MAGIC_MDFN,
    VDP1_PAYLOAD_BYTES,
    VDP2_MAGIC,
    VDP2_PAYLOAD_BYTES,
    validate,
)


def runtime_magic_for(blob: bytes) -> bytes:
    for magic in (RUNTIME_MAGIC, MDFN_RUNTIME_MAGIC):
        if blob.startswith(magic):
            return magic
    raise ValueError("missing runtime capture magic")


VDP1_REGIONS = (
    ("vdp1-vram", 0x40000 * 2),
    ("vdp1-fb0", 0x20000 * 2),
    ("vdp1-fb1", 0x20000 * 2),
    ("vdp1-fb-draw-which", 1),
)
VDP2_REGIONS = (
    ("vdp2-regs", 0x100 * 2),
    ("vdp2-vram", 0x40000 * 2),
    ("vdp2-cram", 0x800 * 2),
)


def frame_regions(blob: bytes, required_frames: int) -> tuple[list[dict[str, bytes]], list[str]]:
    """Extract named raw regions after the shared validator has checked layout."""
    validate(blob, required_frames)
    offset = len(runtime_magic_for(blob))
    frames: list[dict[str, bytes]] = []
    states: list[str] = []
    for frame_index in range(required_frames):
        marker = f"frame={frame_index}\n".encode("ascii")
        if not blob.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        offset += len(marker)
        if not (blob.startswith(VDP1_MAGIC, offset) or
                blob.startswith(VDP1_MAGIC_V2, offset) or
                blob.startswith(VDP1_MAGIC_MDFN, offset)):
            raise ValueError(f"missing VDP1 marker for frame {frame_index}")
        if blob.startswith(VDP1_MAGIC_V2, offset):
            offset += len(VDP1_MAGIC_V2)
            state_end = blob.find(b"\n", offset)
            if state_end < 0 or not blob.startswith(b"state=", offset):
                raise ValueError(f"missing VDP1 state line for frame {frame_index}")
            states.append(blob[offset:state_end].decode("ascii"))
            offset = state_end + 1
        elif blob.startswith(VDP1_MAGIC, offset):
            offset += len(VDP1_MAGIC)
        else:
            offset += len(VDP1_MAGIC_MDFN)
            if blob.startswith(b"state=", offset):
                state_end = blob.find(b"\n", offset)
                if state_end < 0:
                    raise ValueError(f"missing VDP1 state line for frame {frame_index}")
                states.append(blob[offset:state_end].decode("ascii"))
                offset = state_end + 1
            else:
                states.append("state=legacy-v1-unavailable")
        vdp1 = blob[offset : offset + VDP1_PAYLOAD_BYTES]
        offset += VDP1_PAYLOAD_BYTES
        # Early V2 captures appended the draw-buffer selector once after the
        # fixed VDP1 payload. It is metadata already represented in `state=`;
        # skip it before consuming the VDP2 marker.
        if not blob.startswith(VDP2_MAGIC, offset) and \
                blob.startswith(VDP2_MAGIC, offset + 1):
            offset += 1
        if not blob.startswith(VDP2_MAGIC, offset):
            raise ValueError(f"missing VDP2 marker for frame {frame_index}")
        offset += len(VDP2_MAGIC)
        vdp2 = blob[offset : offset + VDP2_PAYLOAD_BYTES]
        offset += VDP2_PAYLOAD_BYTES

        regions: dict[str, bytes] = {}
        cursor = 0
        for name, size in VDP1_REGIONS:
            regions[name] = vdp1[cursor : cursor + size]
            cursor += size
        cursor = 0
        for name, size in VDP2_REGIONS:
            regions[name] = vdp2[cursor : cursor + size]
            cursor += size
        frames.append(regions)
    return frames, states


def _read_exact(handle, size: int, description: str) -> bytes:
    data = handle.read(size)
    if len(data) != size:
        raise ValueError(f"truncated {description}")
    return data


def iter_frame_regions_file(path: Path, required_frames: int):
    """Yield VDP2 regions from a raw capture without loading it into RAM.

    Full Saturn captures are several GiB.  The ordinary ``frame_regions`` API
    deliberately remains a simple in-memory fixture helper, while this
    iterator validates the same producer framing incrementally.  It yields
    VDP1 VRAM plus VDP2 registers, VRAM and CRAM needed by source-span
    analyzers.
    Transport validation is still strict: frame numbering, producer headers,
    fixed payload lengths and EOF must all agree with ``required_frames``.
    """
    if required_frames <= 0:
        raise ValueError("required frame count must be positive")
    with path.open("rb") as handle:
        prefix = handle.read(max(len(RUNTIME_MAGIC), len(MDFN_RUNTIME_MAGIC)))
        runtime_magic = next(
            (magic for magic in (RUNTIME_MAGIC, MDFN_RUNTIME_MAGIC)
             if prefix.startswith(magic)), None)
        if runtime_magic is None:
            raise ValueError("missing runtime capture magic")
        handle.seek(len(runtime_magic))
        for frame_index in range(required_frames):
            marker = _read_exact(handle, len(f"frame={frame_index}\n"),
                                 f"frame {frame_index} marker")
            if marker != f"frame={frame_index}\n".encode("ascii"):
                raise ValueError(f"invalid frame marker at frame {frame_index}")
            vdp1_magic = handle.readline()
            if vdp1_magic not in (VDP1_MAGIC, VDP1_MAGIC_V2, VDP1_MAGIC_MDFN):
                raise ValueError(f"missing VDP1 marker for frame {frame_index}")
            state_text = "state=legacy-v1-unavailable"
            if vdp1_magic == VDP1_MAGIC_V2:
                state = handle.readline()
                if not state.startswith(b"state="):
                    raise ValueError(f"missing VDP1 state line for frame {frame_index}")
                state_text = state.rstrip(b"\n").decode("ascii")
            elif vdp1_magic == VDP1_MAGIC_MDFN:
                state_offset = handle.tell()
                state = handle.readline()
                if state.startswith(b"state="):
                    state_text = state.rstrip(b"\n").decode("ascii")
                else:
                    handle.seek(state_offset)
            vdp1 = _read_exact(handle, VDP1_PAYLOAD_BYTES,
                               f"VDP1 payload for frame {frame_index}")
            vdp2_offset = handle.tell()
            vdp2_magic = _read_exact(handle, len(VDP2_MAGIC),
                                     f"VDP2 marker for frame {frame_index}")
            if vdp2_magic != VDP2_MAGIC:
                handle.seek(vdp2_offset + 1)
                vdp2_magic = _read_exact(handle, len(VDP2_MAGIC),
                                         f"VDP2 marker for frame {frame_index}")
                if vdp2_magic != VDP2_MAGIC:
                    raise ValueError(f"missing VDP2 marker for frame {frame_index}")
            vdp2 = _read_exact(handle, VDP2_PAYLOAD_BYTES,
                               f"VDP2 payload for frame {frame_index}")
            reg_bytes = VDP2_REGIONS[0][1]
            vram_bytes = VDP2_REGIONS[1][1]
            yield frame_index, {
                "vdp1-state": state_text,
                "vdp1-vram": vdp1[:VDP1_REGIONS[0][1]],
                "vdp2-regs": vdp2[:reg_bytes],
                "vdp2-vram": vdp2[reg_bytes:reg_bytes + vram_bytes],
                "vdp2-cram": vdp2[reg_bytes + vram_bytes:],
            }
        if handle.read(1):
            raise ValueError("trailing bytes after final VDP2 frame")


def frame_regions_vdp1(blob: bytes, required_frames: int) -> tuple[list[dict[str, bytes]], list[str]]:
    """Extract VDP1 regions from both legacy V1 and V2 raw witnesses.

    The first capture producer emitted only VDP1 payloads.  Keep this helper
    deliberately narrower than ``frame_regions``: callers may inspect VDP1
    command/material ownership, but must not assume VDP2 registers, VRAM or
    CRAM exist in a V1 witness.
    """
    offset = len(runtime_magic_for(blob))
    frames: list[dict[str, bytes]] = []
    states: list[str] = []
    for frame_index in range(required_frames):
        marker = f"frame={frame_index}\n".encode("ascii")
        if not blob.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        offset += len(marker)
        if blob.startswith(VDP1_MAGIC_V2, offset):
            offset += len(VDP1_MAGIC_V2)
            state_end = blob.find(b"\n", offset)
            if state_end < 0 or not blob.startswith(b"state=", offset):
                raise ValueError(f"missing VDP1 state line for frame {frame_index}")
            states.append(blob[offset:state_end].decode("ascii"))
            offset = state_end + 1
        elif blob.startswith(VDP1_MAGIC, offset):
            offset += len(VDP1_MAGIC)
            states.append("state=legacy-v1-unavailable")
        elif blob.startswith(VDP1_MAGIC_MDFN, offset):
            offset += len(VDP1_MAGIC_MDFN)
            states.append("state=mednafen-raw-unavailable")
        else:
            raise ValueError(f"missing VDP1 marker for frame {frame_index}")
        vdp1 = blob[offset:offset + VDP1_PAYLOAD_BYTES]
        if len(vdp1) != VDP1_PAYLOAD_BYTES:
            raise ValueError(f"truncated VDP1 payload for frame {frame_index}")
        offset += VDP1_PAYLOAD_BYTES
        regions: dict[str, bytes] = {}
        cursor = 0
        for name, size in VDP1_REGIONS:
            regions[name] = vdp1[cursor:cursor + size]
            cursor += size
        # Early V2 captures carried the framebuffer selector as one trailing
        # byte after the fixed VDP1 payload. It is already represented by the
        # state line and is not part of the VDP1 VRAM/framebuffer regions.
        if not blob.startswith(VDP2_MAGIC, offset) and \
                blob.startswith(VDP2_MAGIC, offset + 1):
            offset += 1
        if blob.startswith(VDP2_MAGIC, offset):
            offset += len(VDP2_MAGIC) + VDP2_PAYLOAD_BYTES
            if offset > len(blob):
                raise ValueError(f"truncated VDP2 payload for frame {frame_index}")
        frames.append(regions)
    if offset != len(blob):
        raise ValueError("trailing bytes after final VDP1 frame")
    return frames, states


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--require-frames", type=int, default=2)
    parser.add_argument(
        "--require-changing-region",
        action="append",
        choices=[name for name, _ in VDP1_REGIONS + VDP2_REGIONS],
        default=[],
        help="require this region to differ in at least one adjacent frame pair",
    )
    args = parser.parse_args()
    try:
        frames, states = frame_regions(args.capture.read_bytes(), args.require_frames)
    except (OSError, ValueError) as error:
        print(f"NEXUS_SATURN_RUNTIME_CAPTURE_ANALYSIS_INVALID: {error}")
        return 1

    changed: set[str] = set()
    for index, frame in enumerate(frames):
        print(
            f"frame={index} "
            + " ".join(f"{name}_sha256={digest(frame[name])}" for name, _ in VDP1_REGIONS + VDP2_REGIONS)
        )
        print(f"frame={index} vdp1_{states[index]}")
        if index:
            for name in frame:
                if frame[name] != frames[index - 1][name]:
                    changed.add(name)

    missing = sorted(set(args.require_changing_region) - changed)
    print("changed_regions=" + ",".join(sorted(changed)))
    print("semantic_admission=blocked")
    if missing:
        print("required_unchanged_regions=" + ",".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
