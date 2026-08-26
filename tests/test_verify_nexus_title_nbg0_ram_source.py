#!/usr/bin/env python3
"""Schema-only regression checks for the title CDB-to-WorkRAM verifier."""

from __future__ import annotations

import importlib.util
import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "title_ram_source", ROOT / "scripts" / "verify_nexus_title_nbg0_ram_source.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def main() -> int:
    stage_root = os.environ.get("FIRESTAFF_TEST_STAGE_ROOT")
    if not stage_root:
        print("SKIP: FIRESTAFF_TEST_STAGE_ROOT is not set")
        return 77
    trace = Path(stage_root) / "title-ram-source.trace"
    trace.parent.mkdir(parents=True, exist_ok=True)
    old_argv = sys.argv
    try:
        def row(lba: int) -> str:
            return (
                "addr=0x060ac2a8 size=4 value=0x00000000 source=0x05818000 "
                f"source_value=0x00000000 source_lba=0x{lba:08x} source_word=0x00000008 "
                "pc0=0x06090d04 pc1=0x00000000\n"
            )
        rows = [row(lba) for lba in range(6039, 6056)]
        rows.extend([row(6039)] * (7904 - len(rows)))
        trace.write_text("FIRESTAFF_NEXUS_SH2_RAM_SOURCE_TRACE_V4\n" +
                         "".join(rows), encoding="ascii")
        sys.argv = [str(ROOT), str(trace)]
        assert MODULE.main() == 0
        trace.write_text(trace.read_text(encoding="ascii").replace(
            "source=0x05818000", "source=0x05818002"), encoding="ascii")
        assert MODULE.main() == 1
    finally:
        sys.argv = old_argv
    print("title CDB-to-WorkRAM verifier: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
