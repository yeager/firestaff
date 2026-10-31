#!/usr/bin/env python3
"""Check placement and idempotence of the bounded System Card I/O/RAM hook."""

from __future__ import annotations

import os
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
RUNTIME_MAIN = r"""
uint32 TheronTraceCDSharedEventSequence = 10;
uint16 TheronTraceInstructionPC = 0;
uint32 TheronTraceInstructionPhysicalPC = 0;
static uint8 code_page[65536];
static uint8 ReadMemory(uint32) { return 0x04; }
static void WriteMemory(uint32, uint8) {}
int main() {
 HuCPU.MPR[0] = 0xF8;
 HuCPU.MPR[1] = 0xF8;
 HuCPU.MPR[7] = 0x00;
 HuCPU.FastPageR[7] = reinterpret_cast<uintptr_t>(code_page);
 HuCPU.PCERead[0xF8] = ReadMemory;
 HuCPU.PCEWrite[0xF8] = WriteMemory;
 HuCPU.PCERead[0x00] = ReadMemory;
 HuCPU.PCEWrite[0x00] = WriteMemory;
 TheronTraceInstructionPC = 0xEA4D;
 TheronTraceInstructionPhysicalPC = 0x00EA4D;
 RdMem(0x1801);
 TheronTraceInstructionPC = 0xEA50;
 TheronTraceInstructionPhysicalPC = 0x00EA50;
 WrMem(0x2259, 0x04);
 TheronTraceInstructionPC = 0xE162;
 TheronTraceInstructionPhysicalPC = 0x00E162;
 RdMem(0x2259);
 TheronTraceInstructionPC = 0xE900;
 TheronTraceInstructionPhysicalPC = 0x00E900;
 RdMem(0x1800);
 return 0;
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
        assert "A == 0x2259" in changed
        assert "logical_ranges=1800-1803,224c-2255,2259,2273-2280,22a4" in changed
        assert "limit=8388608" in changed
        assert "FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_PC_FILTER" in changed
        assert "pc_filter=%s" in changed
        assert 'strcmp(pc_filter, "none")' in changed
        assert "pc != 0xEA4D && pc != 0xEA50 && pc != 0xE162" in changed
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
        runtime_source = root / "runtime.cpp"
        runtime_source.write_text(changed + RUNTIME_MAIN)
        executable = root / "runtime-filter-check"
        compiler = subprocess.run(
            ["c++", "-std=c++11", str(runtime_source), "-o", str(executable)],
            check=False, capture_output=True, text=True,
        )
        assert compiler.returncode == 0, compiler.stderr
        for selected_filter, expected_rows in (("none", 4), ("run-path", 3)):
            trace_path = root / f"{selected_filter}.trace"
            environment = os.environ.copy()
            environment["FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_TRACE"] = str(trace_path)
            environment["FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_PC_FILTER"] = selected_filter
            result = subprocess.run(
                [str(executable)], check=False, capture_output=True, text=True,
                env=environment,
            )
            assert result.returncode == 0, result.stderr
            rows = trace_path.read_text().splitlines()
            assert len(rows) == expected_rows + 1, rows
            assert f"pc_filter={selected_filter}" in rows[0]
            if selected_filter == "run-path":
                assert all(
                    f"logical_pc={pc:04x}" in line
                    for line, pc in zip(rows[1:], (0xEA4D, 0xEA50, 0xE162))
                ), rows
    print("test_instrument_mednafen_theron_syscard_io_ram_trace: PASS")


if __name__ == "__main__":
    main()
