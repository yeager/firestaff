#!/usr/bin/env python3
"""Bound the reference PCE Fast party-RAM trace per offset and globally."""

from pathlib import Path
import sys


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_party_trace_budget.py SOURCE"
        )
    source = Path(sys.argv[1]) / "src/pce_fast/pce.cpp"
    if not source.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")
    text = source.read_text(encoding="utf-8")
    start = text.find("static void TheronTracePartyRAMWrite(")
    end = text.find("\nvoid TheronTraceDirectMainRAMWrite", start)
    if start < 0 or end < 0:
        raise SystemExit("FAIL: party-RAM trace function boundary is unavailable")
    function = text[start:end]

    if "static unsigned per_offset[256] = {};" not in function:
        anchor = " static unsigned count = 0;\n"
        if function.count(anchor) != 1:
            raise SystemExit("FAIL: expected one party-RAM trace count anchor")
        function = function.replace(
            anchor, anchor + " static unsigned per_offset[256] = {};\n", 1
        )
    function = function.replace(
        "count >= 4096)",
        "count >= 16384 || per_offset[offset] >= 4096)",
        1,
    )
    if "per_offset[offset]++" not in function:
        anchor = " fflush(trace);\n"
        if function.count(anchor) != 1:
            raise SystemExit("FAIL: expected one party-RAM trace flush anchor")
        function = function.replace(anchor, anchor + " per_offset[offset]++;\n", 1)

    source.write_text(text[:start] + function + text[end:], encoding="utf-8")


if __name__ == "__main__":
    main()
