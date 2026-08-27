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

WRITE_TRACE_SPEC = importlib.util.spec_from_file_location(
    "nexus_vdp2_write_trace", ROOT / "scripts" / "analyze_nexus_vdp2_write_trace.py")
assert WRITE_TRACE_SPEC and WRITE_TRACE_SPEC.loader
WRITE_TRACE = importlib.util.module_from_spec(WRITE_TRACE_SPEC)
WRITE_TRACE_SPEC.loader.exec_module(WRITE_TRACE)

from analyze_nexus_vdp2_composition import visible_character_spans


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


def logobg_fixture() -> bytes:
    return (b"PP" + (320).to_bytes(2, "big") + (224).to_bytes(2, "big") +
            bytes((index * 3) & 0xFF for index in range(512 + 320 * 224)))


def main() -> int:
    title, cg = source_fixture()
    logobg = logobg_fixture()
    original_bin, original_cg, original_logobg = (MODULE.TITLE_BIN_SHA256,
                                                   MODULE.TITLE_CG_SHA256,
                                                   MODULE.LOGOBG_SHA256)
    MODULE.TITLE_BIN_SHA256 = hashlib.sha256(title).hexdigest()
    MODULE.TITLE_CG_SHA256 = hashlib.sha256(cg).hexdigest()
    MODULE.LOGOBG_SHA256 = hashlib.sha256(logobg).hexdigest()
    try:
        payload, maps, palette = MODULE.title_spans(title, cg)
        assert payload == cg[32:]
        assert len(maps) == 5 and all(len(item) == MODULE.TITLE_MAP_BYTES for item in maps)
        assert palette == bytes(range(32))
        logobg_pixels, logobg_palette = MODULE.logobg_spans(logobg)
        assert len(logobg_pixels) == 320 * 224 and len(logobg_palette) == 512
        assert MODULE.wordswapped(b"\x12\x34\xab\xcd") == b"\x34\x12\xcd\xab"
        assert MODULE.find_span(b"xx" + MODULE.wordswapped(maps[2]) + b"yy", maps[2]) == (-1, 2)
        swapped_map = MODULE.wordswapped(maps[2])
        assert MODULE.find_span_with_swapped(b"xx" + swapped_map + b"yy",
                                             maps[2], swapped_map) == (-1, 2)
        row = maps[2][256:512]
        assert (0, 1, -1, 24) in MODULE.map_row_hits(
            b"x" * 24 + MODULE.wordswapped(row) + b"y" * 24, [maps[2]])
        assert MODULE.complete_disc_member_hits(
            b"x" * 32 + MODULE.wordswapped(maps[2]) + b"y" * 32,
            {"ODD": b"x", "MAP": maps[2]}) == [("MAP", -1, 32)]
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
            trace = Path(temporary) / "writes.trace"
            trace.write_text(
                "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1\n"
                "area=vram addr=0x000020 size=1 value=0xab11 pc0=0x1 pc1=0x2\n"
                "area=vram addr=0x000021 size=1 value=0xcd22 pc0=0x1 pc1=0x2\n"
                "area=vram addr=0x000022 size=2 value=0x3344 pc0=0x1 pc1=0x2\n",
                encoding="ascii")
            replay, writes = WRITE_TRACE.replay_vram_bus_writes(trace)
            assert writes == 3 and replay[0x20:0x24] == b"\xab\x22\x33\x44"
            writer_registers = Path(temporary) / "writer-registers.trace"
            source_base = 0x00100000
            destination_base = 0x25E24000
            writer_rows = [MODULE.WRITER_REGISTER_HEADER]
            for index in range(8):
                offset = len(cg) - 1 - index
                writer_rows.append(
                    f"frame=12551 addr=0x{(destination_base + offset) & 0xfffff:06x} "
                    f"pc=0x{MODULE.TITLE_COPY_PC:08x} "
                    f"r0=0x{source_base + offset:08x} "
                    f"r1=0x{destination_base + offset:08x} "
                    f"r3=0x{cg[offset]:08x} r4=0x{destination_base:08x} "
                    f"r6=0x{len(cg):08x} r14=0x{source_base:08x} "
                    "pr=0x06023182")
            writer_registers.write_text("\n".join(writer_rows) + "\n", encoding="ascii")
            assert MODULE.title_sh2_copy_plan(writer_registers, cg) == (
                12551, source_base, destination_base, 8)
            registers = bytearray(0x200)
            registers[0x44:0x46] = (0x1717).to_bytes(2, "little")
            vram = bytearray(0x80000)
            vram[0x5C000:0x5C002] = (0).to_bytes(2, "little")
            vram[0x5C002:0x5C004] = (0x1000).to_bytes(2, "little")
            names, characters = visible_character_spans(bytes(vram), bytes(registers),
                                                         "little", 1)
            assert 0x5C000 in names and 0x20000 in characters
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
            assert len(streamed[0][1]["vdp1-vram"]) == 0x80000
            assert streamed[0][1]["vdp1-state"] == "state=legacy-v1-unavailable"
    finally:
        (MODULE.TITLE_BIN_SHA256, MODULE.TITLE_CG_SHA256,
         MODULE.LOGOBG_SHA256) = original_bin, original_cg, original_logobg
    print("nexus title VDP2 source analyser: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
