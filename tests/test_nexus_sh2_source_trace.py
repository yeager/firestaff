"""Regression coverage for width-aware SH-2 source-write receipts."""

from __future__ import annotations

import importlib.util
import struct
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "nexus_sh2_source_trace", ROOT / "scripts" / "analyze_nexus_sh2_source_trace.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def read_trace(text: str):
    with patch.object(Path, "read_text", return_value=text):
        return MODULE.read_rows(Path("receipt.trace"))


def test_legacy_v1_keeps_four_byte_records() -> None:
    rows = read_trace(
        "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V1\n"
        "addr=0x06000000 value=0x11223344 source=0x05818000 "
        "source_value=0x11223344 pc0=0x06010000 pc1=0x00000000\n"
    )
    assert rows == [(0x06000000, 4, 0x11223344, 0x05818000,
                     0x11223344, -1, 0x06010000, 0)]


def test_v2_preserves_two_byte_contiguity_and_value_width() -> None:
    rows = read_trace(
        "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V2\n"
        "addr=0x0025daf0 size=2 value=0x00001234 source=0x05818000 "
        "source_value=0x00005678 pc0=0x06090d04 pc1=0x0608d2f2\n"
        "addr=0x0025daf2 size=2 value=0x0000abcd source=0x05818002 "
        "source_value=0x0000dcba pc0=0x06090d04 pc1=0x0608d2f2\n"
    )
    assert MODULE.chunks(rows) == [rows]
    assert b"".join(row[2].to_bytes(row[1], "big") for row in rows) == b"\x12\x34\xab\xcd"


def test_v3_retains_cdb_lba() -> None:
    rows = read_trace(
        "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V3\n"
        "addr=0x0025daf0 size=2 value=0x00001234 source=0x05818000 "
        "source_value=0x00001234 source_lba=0x000017ca "
        "pc0=0x06090d04 pc1=0x0608d2f2\n"
    )
    assert rows[0][5] == 6090


def test_v4_accepts_cdb_fifo_word_without_changing_row_contract() -> None:
    rows = read_trace(
        "FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V4\n"
        "addr=0x0025daf0 size=2 value=0x00001234 source=0x05818000 "
        "source_value=0x00001234 source_lba=0x000017ca source_word=0x00000008 "
        "pc0=0x06090d04 pc1=0x0608d2f2\n"
    )
    assert rows[0] == (0x0025DAF0, 2, 0x1234, 0x05818000,
                       0x1234, 6090, 0x06090D04, 0x0608D2F2)


def _directory_record(lba: int, size: int, flags: int, name: bytes) -> bytes:
    length = 33 + len(name) + (1 if len(name) % 2 == 0 else 0)
    record = bytearray(length)
    record[0] = length
    record[2:6] = struct.pack("<I", lba)
    record[6:10] = struct.pack(">I", lba)
    record[10:14] = struct.pack("<I", size)
    record[14:18] = struct.pack(">I", size)
    record[25] = flags
    record[28:30] = struct.pack("<H", 1)
    record[30:32] = struct.pack(">H", 1)
    record[32] = len(name)
    record[33:33 + len(name)] = name
    return bytes(record)


def _small_iso() -> bytes:
    image = bytearray(19 * 2048)
    pvd = memoryview(image)[16 * 2048:17 * 2048]
    pvd[0:7] = b"\x01CD001\x01"
    pvd[156:190] = _directory_record(17, 2048, 2, b"\x00")
    directory = memoryview(image)[17 * 2048:18 * 2048]
    directory[0:34] = _directory_record(17, 2048, 2, b"\x00")
    directory[34:68] = _directory_record(17, 2048, 2, b"\x01")
    file_record = _directory_record(18, 4, 0, b"FOO;1")
    directory[68:68 + len(file_record)] = file_record
    image[18 * 2048:18 * 2048 + 4] = b"DATA"
    return bytes(image)


def test_cue_track1_reads_raw2352_user_data_in_memory(tmp_path: Path) -> None:
    iso = _small_iso()
    raw = b"".join(
        b"\x00" * 16 + iso[offset:offset + 2048] + b"\xff" * 288
        for offset in range(0, len(iso), 2048)
    )
    track = tmp_path / "track.bin"
    track.write_bytes(raw)
    cue = tmp_path / "disc.cue"
    cue.write_text('FILE "track.bin" BINARY\n  TRACK 01 MODE1/2352\n',
                   encoding="utf-8")

    image = MODULE.cue_track1_bytes(cue)

    assert image == iso
    assert MODULE.read_iso_files(image) == [(18 * 2048, 4, "FOO")]
