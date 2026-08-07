#!/usr/bin/env python3
"""Validate a captured SH-2 code window at an authenticated VDP1 writer PC.

The trace proves runtime code ownership only. It does not identify a retail
file or admit host rendering; source/file identity remains capture-gated.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_VDP1_WRITER_CODE_TRACE_V1"
LINE = re.compile(
    r"pc=0x(?P<pc>[0-9a-fA-F]+) vram_addr=0x(?P<vram>[0-9a-fA-F]+) "
    r"code_start=0x(?P<start>[0-9a-fA-F]+) words=(?P<words>[0-9]+) "
    r"code=(?P<code>[0-9a-fA-F,]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-vram", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as error:
        print(f"NEXUS_VDP1_PC_CODE_TRACE_INVALID: {error}")
        return 1
    if len(lines) != 2 or lines[0] != HEADER:
        print("NEXUS_VDP1_PC_CODE_TRACE_INVALID: expected one code window")
        return 1
    match = LINE.fullmatch(lines[1])
    if not match:
        print("NEXUS_VDP1_PC_CODE_TRACE_INVALID: malformed code window")
        return 1
    pc = int(match["pc"], 16)
    vram = int(match["vram"], 16)
    start = int(match["start"], 16)
    words = int(match["words"], 10)
    code = match["code"].split(",")
    if words != len(code) or words == 0 or start + words * 2 < start:
        print("NEXUS_VDP1_PC_CODE_TRACE_INVALID: word count/range")
        return 1
    if args.require_pc is not None and pc != args.require_pc:
        print(f"required_pc=0x{args.require_pc:08x} observed=0x{pc:08x}")
        return 1
    if args.require_vram is not None and vram != args.require_vram:
        print(f"required_vram=0x{args.require_vram:05x} observed=0x{vram:05x}")
        return 1
    print(f"pc=0x{pc:08x} vram_addr=0x{vram:05x} code_start=0x{start:08x}")
    print(f"words={words} first=0x{int(code[0], 16):04x} last=0x{int(code[-1], 16):04x}")
    print("runtime_code_window=verified")
    print("source_file_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
