#!/usr/bin/env python3
"""Exercise fail-closed Drator instruction-window source verification."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock


ROOT = Path(__file__).resolve().parents[1]
VERIFIER = ROOT / "scripts/verify_theron_pce_fast_code_fetch_source.py"
EXPECTED_MD5 = "f23601102138f87c33025877767ebf76"
HEADER = (
    "source=mednafen-pce-fast-c852-opcode-fetch "
    "event_sequence_source=TheronTraceCDSharedEventSequence"
)

spec = importlib.util.spec_from_file_location("code_fetch_verifier", VERIFIER)
assert spec and spec.loader
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


def code_window(seed: int) -> bytes:
    return bytes((index * seed + seed) & 0xFF for index in range(512))


def row(pc: int, sequence: int, event_sequence: int, window: bytes) -> str:
    mprs = [0] * 8
    slot = pc >> 13
    mprs[slot] = 0x12 if pc == 0xCBC6 else 0x34
    physical_pc = (mprs[slot] << 13) | (pc & 0x1FFF)
    fields = [
        f"sequence={sequence}",
        f"event_sequence={event_sequence}",
        f"logical_pc={pc:04x}",
        f"physical_pc={physical_pc:06x}",
        f"opcode={window[0]:02x}",
        f"code_bytes={window.hex()}",
    ]
    fields.extend(f"mpr{index}={value:02x}" for index, value in enumerate(mprs))
    return "row_reader_fetch " + " ".join(fields)


def run(trace_lines: list[str], source: bytes) -> tuple[int, str]:
    with TemporaryDirectory(prefix="theron-code-fetch-verifier-") as name:
        root = Path(name)
        trace_path = root / "fetch.trace"
        source_path = root / "track02.bin"
        trace_path.write_text("\n".join([HEADER, *trace_lines, ""]), encoding="ascii")
        source_path.write_bytes(source)
        output = io.StringIO()
        with (
            mock.patch.object(sys, "argv", [str(VERIFIER), str(trace_path), str(source_path)]),
            mock.patch.object(
                verifier.hashlib,
                "md5",
                return_value=mock.Mock(hexdigest=lambda: EXPECTED_MD5),
            ),
            contextlib.redirect_stdout(output),
            contextlib.redirect_stderr(output),
        ):
            result = verifier.main()
        return result, output.getvalue()


def main() -> None:
    window_a = code_window(17)
    window_b = code_window(29)
    source = bytearray(2048)
    source[128 : 128 + len(window_a)] = window_a
    source[1024 : 1024 + len(window_b)] = window_b
    valid = [
        row(0xCBC6, 0, 100, window_a),
        row(0x45C3, 0, 105, window_b),
    ]

    result, output = run(valid, bytes(source))
    assert result == 0, output
    assert "pc=$45C3 status=bounded matching_raw_offsets=0x0000400" in output
    assert "pc=$CBC6 status=bounded matching_raw_offsets=0x0000080" in output

    result, output = run(valid[:1], bytes(source))
    assert result != 0 and "missing instruction fetches" in output

    result, output = run(
        [*valid, "row_reader_fetch_truncated logical_pc=cbc6 limit=4096"],
        bytes(source),
    )
    assert result != 0 and "incomplete trace" in output

    result, output = run(
        [row(0xCBC6, 0, 100, window_a), row(0x45C3, 0, 100, window_b)],
        bytes(source),
    )
    assert result != 0 and "invalid execution provenance" in output

    result, output = run(
        [row(0xCBC6, 1, 100, window_a), row(0x45C3, 0, 105, window_b)],
        bytes(source),
    )
    assert result != 0 and "non-contiguous reader sequence" in output

    unmatched = bytearray(source)
    unmatched[128 : 128 + len(window_a)] = bytes([0xEE]) * len(window_a)
    result, output = run(valid, bytes(unmatched))
    assert result != 0 and "no instruction window" in output
    print("test_verify_theron_pce_fast_code_fetch_source: PASS")


if __name__ == "__main__":
    main()
