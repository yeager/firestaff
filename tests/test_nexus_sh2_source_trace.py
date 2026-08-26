"""Regression coverage for width-aware SH-2 source-write receipts."""

from __future__ import annotations

import importlib.util
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
