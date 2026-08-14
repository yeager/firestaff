#!/usr/bin/env python3
"""Unit checks for streaming, fail-closed VDP1 source-buffer verification."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
SPEC = importlib.util.spec_from_file_location(
    "nexus_vdp1_source_to_vram",
    ROOT / "scripts" / "analyze_nexus_vdp1_source_to_vram.py")
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def capture_frame(vram: bytes) -> bytes:
    assert len(vram) == 0x80000
    vdp1 = vram + bytes(0x80000)
    vdp2 = bytes(0x200) + bytes(0x80000) + bytes(0x1000)
    return b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V1\n" + vdp1 + b"VDP2_RAW\n" + vdp2


def main() -> int:
    with tempfile.TemporaryDirectory() as temporary:
        capture = Path(temporary) / "capture.raw"
        first = bytes(0x80000)
        second = bytearray(0x80000)
        second[0x10A00:0x10A04] = b"\x34\x12\xCD\xAB"
        capture.write_bytes(
            b"FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
            b"frame=0\n" + capture_frame(first) +
            b"frame=1\n" + capture_frame(bytes(second)))
        assert MODULE.vdp1_vram_at(capture, 2, 1)[0x10A00:0x10A04] == b"\x34\x12\xCD\xAB"
        try:
            MODULE.vdp1_vram_at(capture, 2, 2)
        except ValueError:
            pass
        else:
            raise AssertionError("out-of-range frame must be rejected")
        capture.write_bytes(capture.read_bytes() + b"trailing")
        try:
            MODULE.vdp1_vram_at(capture, 2, 1)
        except ValueError:
            pass
        else:
            raise AssertionError("trailing raw data must be rejected")
    print("nexus VDP1 source-to-VRAM analyser: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
