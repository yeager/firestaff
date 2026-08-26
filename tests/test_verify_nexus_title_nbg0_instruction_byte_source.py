#!/usr/bin/env python3
"""Unit-check the strict instruction-byte-to-VDP2 title transport verifier.

The rows below are deliberately tiny trace-schema fixtures, not game data.
They exercise byte-lane handling and the fail-closed ordering rule; the
production verifier keeps the real 31,616-byte retail constants.
"""

from __future__ import annotations

import importlib.util
import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
SPEC = importlib.util.spec_from_file_location(
    "title_instruction_source",
    ROOT / "scripts" / "verify_nexus_title_nbg0_instruction_byte_source.py",
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def main() -> int:
    stage_root = os.environ.get("FIRESTAFF_TEST_STAGE_ROOT")
    if not stage_root:
        print("SKIP: FIRESTAFF_TEST_STAGE_ROOT is not set")
        return 77
    stage = Path(stage_root) / "title-instruction-byte-source"
    stage.mkdir(parents=True, exist_ok=True)
    instruction = stage / "instruction.trace"
    writes = stage / "vdp2.trace"
    instruction.write_text(
        "FIRESTAFF_NEXUS_SH2_INSTRUCTION_BYTE_READ_TRACE_V1\n"
        "cpu=0 addr=0x000000ff value=0x12 pc=0x00001234 r=2\n"
        "cpu=0 addr=0x00000100 value=0x34 pc=0x00001234 r=2\n",
        encoding="ascii",
    )
    writes.write_text(
        "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1\n"
        "area=vram addr=0x001008 size=1 value=0x1200 pc0=0x0602312c pc1=0x00000000\n"
        "area=vram addr=0x001009 size=1 value=0x0034 pc0=0x0602312c pc1=0x00000000\n",
        encoding="ascii",
    )
    old = (MODULE.COPY_BYTES, MODULE.ROW_BYTES, MODULE.ROW_STRIDE,
           MODULE.SOURCE_START, MODULE.DESTINATION_START)
    MODULE.COPY_BYTES = 2
    MODULE.ROW_BYTES = 2
    MODULE.ROW_STRIDE = 2
    MODULE.SOURCE_START = 0x100
    MODULE.DESTINATION_START = 0x1008
    old_argv = sys.argv
    try:
        sys.argv = [str(ROOT), str(instruction), str(writes),
                    "--instruction-pc", "0x1234",
                    "--instruction-register", "2"]
        assert MODULE.main() == 0
        instruction.write_text(
            instruction.read_text(encoding="ascii").replace("value=0x34", "value=0x35"),
            encoding="ascii",
        )
        assert MODULE.main() == 1
        instruction.write_text(
            "FIRESTAFF_NEXUS_SH2_INSTRUCTION_BYTE_READ_TRACE_V2\n"
            "frame=12596 cpu=0 addr=0x000000ff value=0x12 pc=0x00001234 r=2\n"
            "frame=12596 cpu=0 addr=0x00000100 value=0x34 pc=0x00005678 r=6\n",
            encoding="ascii",
        )
        sys.argv = [str(ROOT), str(instruction), str(writes)]
        assert MODULE.main() == 0
    finally:
        sys.argv = old_argv
        (MODULE.COPY_BYTES, MODULE.ROW_BYTES, MODULE.ROW_STRIDE,
         MODULE.SOURCE_START, MODULE.DESTINATION_START) = old
    print("title instruction-byte source verifier: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
