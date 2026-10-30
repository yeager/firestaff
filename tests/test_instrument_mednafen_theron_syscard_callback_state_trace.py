#!/usr/bin/env python3
"""Check the callback-state hook is bounded, placed, and idempotent."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
INSTRUMENTER = ROOT / "scripts/instrument_mednafen_theron_syscard_callback_state_trace.py"
SOURCE = """#include <cstdint>
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
struct FakeCPU { uint8 MPR[8]; } HuCPU;
extern uint32 TheronTraceCDSharedEventSequence;
extern uint16 TheronTraceInstructionPC;
extern uint32 TheronTraceInstructionPhysicalPC;
static INLINE uint8 RdMem(unsigned int A)
{
 uint8 wmpr = HuCPU.MPR[A >> 13];
 uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);
 uint8 value = HuCPU.PCERead[wmpr](physical_address);
 return(value);
}
static INLINE void WrMem(unsigned int A, uint8 V)
{
 uint8 wmpr = HuCPU.MPR[A >> 13];
 uint32 physical = (wmpr << 13) | (A & 0x1FFF);
 HuCPU.PCEWrite[wmpr](physical, V);
}
"""


def main() -> None:
    (ROOT / "build").mkdir(exist_ok=True)
    with TemporaryDirectory(prefix="theron-callback-state-hook-", dir=ROOT / "build") as name:
        root = Path(name)
        source = root / "src/pce_fast/huc6280.cpp"
        source.parent.mkdir(parents=True)
        source.write_text(SOURCE)
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), str(root)],
            check=False, capture_output=True, text=True,
        )
        assert result.returncode == 0, result.stderr
        changed = source.read_text()
        assert changed.count("FIRESTAFF_THERON_SYSTEM_CARD_CALLBACK_STATE_TRACE") == 1
        assert 'TheronTraceSystemCardCallbackState("read"' in changed
        assert 'TheronTraceSystemCardCallbackState("write"' in changed
        assert "physical_address=%06x" in changed
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), str(root)],
            check=False, capture_output=True, text=True,
        )
        assert result.returncode != 0
        assert "already instrumented" in result.stderr
    print("test_instrument_mednafen_theron_syscard_callback_state_trace: PASS")


if __name__ == "__main__":
    main()
