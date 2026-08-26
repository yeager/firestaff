#!/usr/bin/env python3
"""Unit-check title-value matching with a VDP2 trace prefix.

The rows are intentionally tiny trace-schema fixtures, never synthetic game
data.  A real trace may include earlier writes by the same SH-2 copy routine;
the verifier must select the observed title destination corridor.
"""

from __future__ import annotations

import importlib.util
import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
SPEC = importlib.util.spec_from_file_location(
    "title_copy_values", ROOT / "scripts" / "verify_nexus_title_nbg0_copy_values.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def main() -> int:
    stage_root = os.environ.get("FIRESTAFF_TEST_STAGE_ROOT")
    if not stage_root:
        print("SKIP: FIRESTAFF_TEST_STAGE_ROOT is not set")
        return 77
    stage = Path(stage_root) / "title-copy-values"
    stage.mkdir(parents=True, exist_ok=True)
    registers = stage / "registers.trace"
    writes = stage / "writes.trace"
    registers.write_text(
        "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1\n"
        "frame=12596 addr=0x001008 pc=0x0602312c r4=0x25e01008 r5=0x060ac2a7\n"
        "frame=12596 addr=0x001009 pc=0x0602312c r4=0x25e01009 r5=0x060ac2a8\n",
        encoding="ascii",
    )
    writes.write_text(
        "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1\n"
        "area=vram addr=0x001234 size=1 value=0xab00 pc0=0x0602312c pc1=0x00000000\n"
        "area=vram addr=0x001008 size=1 value=0x1200 pc0=0x0602312c pc1=0x00000000\n"
        "area=vram addr=0x001009 size=1 value=0x0034 pc0=0x0602312c pc1=0x00000000\n",
        encoding="ascii",
    )
    old = MODULE.COPY_BYTES
    old_argv = sys.argv
    try:
        MODULE.COPY_BYTES = 2
        sys.argv = [str(ROOT), str(registers), str(writes)]
        assert MODULE.main() == 0
        writes.write_text(writes.read_text(encoding="ascii").replace("addr=0x001009", "addr=0x00100a"), encoding="ascii")
        assert MODULE.main() == 1
    finally:
        MODULE.COPY_BYTES = old
        sys.argv = old_argv
    print("title copy-value verifier: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
