#!/usr/bin/env python3
"""Regression test for Mednafen's frame-before-writes VDP1 trace contract."""

from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ANALYZER = ROOT / "scripts" / "analyze_nexus_vdp1_write_trace.py"
TRACE = """FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2
frame=0
frame=1
addr=0x63e00 size=2 value=0x0598 pc0=0x0601307c pc1=0x00000000
frame=2
addr=0x63e02 size=2 value=0xfcb4 pc0=0x0601307c pc1=0x00000000
addr=0x63e04 size=2 value=0xf987 pc0=0x0601307c pc1=0x00000000
"""


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="nexus-vdp1-trace-test-") as tmp:
        trace = Path(tmp) / "vdp1.trace"
        trace.write_text(TRACE, encoding="ascii")
        selected = subprocess.run(
            [sys.executable, str(ANALYZER), str(trace), "--frame", "1",
             "--require-address", "0x63e00"],
            check=False, capture_output=True, text=True,
        )
        if selected.returncode != 0 or "records=1" not in selected.stdout or \
                "required_matches=1" not in selected.stdout:
            print(selected.stdout, end="")
            print(selected.stderr, end="", file=sys.stderr)
            return 1
        aggregate = subprocess.run(
            [sys.executable, str(ANALYZER), str(trace)],
            check=False, capture_output=True, text=True,
        )
        if aggregate.returncode != 0 or "records=3" not in aggregate.stdout:
            print(aggregate.stdout, end="")
            print(aggregate.stderr, end="", file=sys.stderr)
            return 1
    print("NEXUS_VDP1_WRITE_TRACE_FRAME_CONTRACT=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
