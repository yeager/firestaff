#!/usr/bin/env python3
"""Check bounded C852 opcode and runtime code-window instrumentation."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
INSTRUMENTER = ROOT / "scripts/instrument_mednafen_theron_pce_fast_c852_fetch_trace.py"
SOURCE = """+void Run()
{
\t  TheronTraceInstructionPC = GetRealPC();
\t  b1 = RdAtPC();
\t  ADDCYC(CycTable[b1]);
}
"""


def main() -> None:
    with TemporaryDirectory(prefix="theron-c852-fetch-hook-") as name:
        root = Path(name)
        source = root / "src/pce_fast/huc6280.cpp"
        source.parent.mkdir(parents=True)
        source.write_text(SOURCE)
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), str(root)],
            check=False,
            capture_output=True,
            text=True,
        )
        assert result.returncode == 0, result.stderr
        changed = source.read_text()
        assert changed.count("source=mednafen-pce-fast-c852-opcode-fetch") == 1
        assert "code_bytes[1025]" in changed
        assert "HuCPU.FastPageR[" in changed
        assert "<= 0x1E00" in changed
        assert "i < 512" in changed
        assert "code_bytes=%s" in changed
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), str(root)],
            check=False,
            capture_output=True,
            text=True,
        )
        assert result.returncode != 0
        assert "already installed" in result.stderr
    print("test_instrument_mednafen_theron_pce_fast_c852_fetch_trace: PASS")


if __name__ == "__main__":
    main()
