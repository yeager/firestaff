"""Regression coverage for raw-2352 Nexus Track 1 directory parsing."""

from __future__ import annotations

import importlib.util
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "nexus_cdb_read_trace", ROOT / "scripts" / "analyze_nexus_cdb_read_trace.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def record(lba: int, size: int, flags: int, name: bytes) -> bytes:
    result = bytearray(33 + len(name))
    result[0] = len(result)
    result[2:6] = lba.to_bytes(4, "little")
    result[10:14] = size.to_bytes(4, "little")
    result[25] = flags
    result[32] = len(name)
    result[33:] = name
    return bytes(result)


def test_raw_track_user_data_offset_is_used() -> None:
    raw = bytearray(21 * 2352)

    def put_sector(lba: int, data: bytes) -> None:
        raw[lba * 2352 + 16:lba * 2352 + 16 + len(data)] = data

    pvd = bytearray(2048)
    pvd[1:6] = b"CD001"
    pvd[156:156 + 34] = record(20, 2048, 2, b"\x00")
    put_sector(16, pvd)
    directory = record(20, 2048, 2, b"\x00") + record(42, 5, 0, b"TITLE.CG;1")
    put_sector(20, directory)
    with patch.object(Path, "read_bytes", return_value=bytes(raw)):
        assert MODULE.read_iso_files(Path("track01.bin")) == [(42, 5, "TITLE.CG")]
