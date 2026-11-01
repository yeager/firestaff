#!/usr/bin/env python3
"""Check Mednafen state structure parsing and optional authentic state joins."""

from __future__ import annotations

import gzip
import hashlib
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import inspect_theron_mednafen_pce_fast_state as inspector  # noqa: E402


def _variable(name: str, data: bytes) -> bytes:
    encoded = name.encode("ascii")
    return bytes([len(encoded)]) + encoded + struct.pack("<I", len(data)) + data


def _section(name: str, variables: list[tuple[str, bytes]]) -> bytes:
    encoded = name.encode("ascii")
    header = encoded.ljust(32, b"\0") + struct.pack(
        "<I", sum(len(_variable(key, data)) for key, data in variables)
    )
    return header + b"".join(_variable(key, data) for key, data in variables)


def _structural_state() -> bytes:
    preview = bytes([0x10, 0x20, 0x30] * 2)
    sections = b"".join(
        (
            _section("MAIN", [("BaseRAM", bytes(0x2000))]),
            _section("CPU", [("MPR", bytes(8))]),
            _section("VCE", [("color_table", bytes(0x400))]),
            _section("VDC0", [("SAT", bytes(0x200)), ("VRAM", bytes(0x10000))]),
            _section("HuC", [(inspector.CDRAM_VARIABLE, bytes(inspector.CDRAM_BYTES))]),
        )
    )
    payload = bytearray(32)
    payload[:8] = b"MDFNSVST"
    struct.pack_into("<I", payload, 16, inspector.MEDNAFEN_STATE_VERSION)
    struct.pack_into("<I", payload, 24, 2)
    struct.pack_into("<I", payload, 28, 1)
    payload.extend(preview)
    payload.extend(sections)
    struct.pack_into("<I", payload, 20, len(payload))
    return bytes(payload)


def test_parses_named_pce_fast_sections() -> None:
    state = inspector.parse_state_payload(_structural_state())
    assert state.preview_width == 2
    assert state.preview_height == 1
    assert set(state.sections) == {"MAIN", "CPU", "VCE", "VDC0", "HuC"}
    assert state.sections["MAIN"].variables["BaseRAM"].size == 0x2000
    assert state.sections["CPU"].variables["MPR"].size == 8
    assert state.sections["HuC"].variables[inspector.CDRAM_VARIABLE].size == 0x40000


def test_rejects_truncation_duplicate_sections_and_wrong_versions() -> None:
    valid = _structural_state()
    bad_name = bytearray(valid)
    bad_name[32 + 6] = 0xFF
    for malformed in (valid[:-1], bytes(bad_name)):
        try:
            inspector.parse_state_payload(malformed)
        except inspector.StateFormatError:
            pass
        else:
            raise AssertionError("malformed state payload was accepted")

    duplicate = bytearray(valid)
    section_start = 32 + 6
    duplicate.extend(valid[section_start:])
    struct.pack_into("<I", duplicate, 20, len(duplicate))
    try:
        inspector.parse_state_payload(bytes(duplicate))
    except inspector.StateFormatError as exc:
        assert "duplicate section" in str(exc)
    else:
        raise AssertionError("duplicate state sections were accepted")

    wrong_version = bytearray(valid)
    struct.pack_into("<I", wrong_version, 16, inspector.MEDNAFEN_STATE_VERSION + 1)
    try:
        inspector.parse_state_payload(bytes(wrong_version))
    except inspector.StateFormatError as exc:
        assert "source-locked to 1.32.1" in str(exc)
    else:
        raise AssertionError("unsupported state version was accepted")


def test_authentic_state_source_and_mpr_join(state_path: Path, track02_path: Path) -> None:
    compressed, state = inspector.read_state(state_path)
    raw_track02 = track02_path.read_bytes()
    report = inspector.inspect_authentic_state(state, raw_track02)
    assert hashlib.sha256(compressed).hexdigest() == (
        "b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068"
    )
    assert report["cd_ram_relative_offset"] == 0x1891D
    assert report["physical_page"] == 0x74
    assert report["physical_page_offset"] == 0x091D
    assert report["physical_address"] == 0x0E891D
    assert report["mapped_mpr_slots"] == []
    assert report["base_ram_map_copy_offsets"] == [[], [], [], []]
    assert report["preview_width"] == 341
    assert report["preview_height"] == 232


def main() -> int:
    test_parses_named_pce_fast_sections()
    test_rejects_truncation_duplicate_sections_and_wrong_versions()
    if len(sys.argv) == 3:
        test_authentic_state_source_and_mpr_join(Path(sys.argv[1]), Path(sys.argv[2]))
        print("test_inspect_theron_mednafen_pce_fast_state: PASS (including authentic join)")
    elif len(sys.argv) == 1:
        print("test_inspect_theron_mednafen_pce_fast_state: PASS (structure; real state not supplied)")
    else:
        raise SystemExit("usage: test_inspect_theron_mednafen_pce_fast_state.py [STATE TRACK02]")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
