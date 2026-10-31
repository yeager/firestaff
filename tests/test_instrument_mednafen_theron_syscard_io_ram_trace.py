#!/usr/bin/env python3
"""Check placement and idempotence of the bounded System Card I/O/RAM hook."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
INSTRUMENTER = ROOT / "scripts/instrument_mednafen_theron_syscard_io_ram_trace.py"
SOURCE = """#include <cstdint>
#include <cstdio>
#include <cstdlib>
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uintptr_t = unsigned long;
struct FakeCPU {
 uint8 MPR[8];
 uintptr_t FastPageR[8];
 uint8 (*PCERead[256])(uint32);
 void (*PCEWrite[256])(uint32, uint8);
} HuCPU;
extern uint32 TheronTraceCDSharedEventSequence;
extern uint16 TheronTraceInstructionPC;
extern uint32 TheronTraceInstructionPhysicalPC;
#define INLINE inline
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
    with TemporaryDirectory(prefix="theron-syscard-io-ram-hook-") as name:
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
        assert changed.count("FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_TRACE") == 1
        assert 'TheronTraceSystemCardIoRam("read"' in changed
        assert 'TheronTraceSystemCardIoRam("write"' in changed
        assert "logical_ranges=1800-1803,224c-2255,2273-2280,22a4" in changed
        compiler = subprocess.run(
            ["c++", "-std=c++11", "-fsyntax-only", str(source)],
            check=False, capture_output=True, text=True,
        )
        assert compiler.returncode == 0, compiler.stderr
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), str(root)],
            check=False, capture_output=True, text=True,
        )
        assert result.returncode != 0
        assert "already instrumented" in result.stderr
    print("test_instrument_mednafen_theron_syscard_io_ram_trace: PASS")


if __name__ == "__main__":
    main()
