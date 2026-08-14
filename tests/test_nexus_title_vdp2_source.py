#!/usr/bin/env python3
"""Unit checks for the fail-closed Nexus title VDP2 span analyser."""

from __future__ import annotations

import hashlib
import importlib.util
from pathlib import Path
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
SPEC = importlib.util.spec_from_file_location(
    "nexus_title_vdp2_source", ROOT / "scripts" / "analyze_nexus_title_vdp2_source.py")
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def source_fixture() -> tuple[bytes, bytes]:
    cg = bytes(32) + bytes((index * 17) & 0xFF for index in range(5249 * 32))
    record_size = MODULE.TITLE_PALETTE_OFFSET + 32
    title = bytearray(MODULE.TITLE_MAPD_OFFSET + record_size)
    record = memoryview(title)[MODULE.TITLE_MAPD_OFFSET:]
    record[:4] = b"MAPD"
    record[8:12] = b"TIBG"
    for index in range(MODULE.TITLE_MAP_COUNT):
        offset = 0x40 + index * 0x1C04
        record[offset:offset + 2] = (64).to_bytes(2, "big")
        record[offset + 2:offset + 4] = (28).to_bytes(2, "big")
        record[offset + 4:offset + 4 + MODULE.TITLE_MAP_BYTES] = bytes(
            (cell + index * 17) & 0xFF for cell in range(MODULE.TITLE_MAP_BYTES))
    record[MODULE.TITLE_PALETTE_OFFSET:MODULE.TITLE_PALETTE_OFFSET + 32] = bytes(range(32))
    return bytes(title), cg


def main() -> int:
    title, cg = source_fixture()
    original_bin, original_cg = MODULE.TITLE_BIN_SHA256, MODULE.TITLE_CG_SHA256
    MODULE.TITLE_BIN_SHA256 = hashlib.sha256(title).hexdigest()
    MODULE.TITLE_CG_SHA256 = hashlib.sha256(cg).hexdigest()
    try:
        payload, maps, palette = MODULE.title_spans(title, cg)
        assert payload == cg[32:]
        assert len(maps) == 5 and all(len(item) == MODULE.TITLE_MAP_BYTES for item in maps)
        assert palette == bytes(range(32))
        assert MODULE.wordswapped(b"\x12\x34\xab\xcd") == b"\x34\x12\xcd\xab"
        assert MODULE.find_span(b"xx" + MODULE.wordswapped(maps[2]) + b"yy", maps[2]) == (-1, 2)
        swapped_map = MODULE.wordswapped(maps[2])
        assert MODULE.find_span_with_swapped(b"xx" + swapped_map + b"yy",
                                             maps[2], swapped_map) == (-1, 2)
        try:
            MODULE.wordswapped(b"\0")
        except ValueError:
            pass
        else:
            raise AssertionError("odd source span must be rejected")
        try:
            MODULE.title_spans(title[:-1], cg)
        except ValueError:
            pass
        else:
            raise AssertionError("short TITLE.BIN must be rejected")
        with tempfile.TemporaryDirectory() as temporary:
            capture = Path(temporary) / "capture.raw"
            vdp1 = bytes(0x100000)
            vdp2 = bytes(0x200) + bytes(0x80000) + bytes(0x1000)
            capture.write_bytes(
                b"FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
                b"frame=0\nFIRESTAFF_NEXUS_SATURN_VDP1_RAW_V1\n" + vdp1 +
                b"VDP2_RAW\n" + vdp2)
            streamed = list(MODULE.iter_frame_regions_file(capture, 1))
            assert len(streamed) == 1 and streamed[0][0] == 0
            assert len(streamed[0][1]["vdp2-vram"]) == 0x80000
    finally:
        MODULE.TITLE_BIN_SHA256, MODULE.TITLE_CG_SHA256 = original_bin, original_cg
    print("nexus title VDP2 source analyser: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
