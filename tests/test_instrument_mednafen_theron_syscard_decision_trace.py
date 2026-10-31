#!/usr/bin/env python3
"""Check placement and idempotence of the independent BIOS decision hook."""

from pathlib import Path
import os
import shlex
import shutil
import subprocess
import sys
import tempfile


REPO = Path(__file__).resolve().parents[1]
INSTRUMENTER = REPO / "scripts/instrument_mednafen_theron_syscard_decision_trace.py"
MARKER = "    TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);"
HARNESS = """#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
struct FakeCPU { uintptr_t FastPageR[8]; uint8 MPR[8]; uint8 S; } HuCPU = {};
uint16 TheronTraceInstructionPC = 0xE11D;
uint32 TheronTraceInstructionPhysicalPC = 0x00011D;
uint32 TheronTraceCDSharedEventSequence = 0;
#define HU_A 0
#define HU_X 0
#define HU_Y 0
#define HU_P 0
void TheronPCEFastDratorMenuRouteStep(uint16) {}
void step() {
""" + MARKER + "\n}\nint main() { step(); return 0; }\n"


def main() -> None:
    build = REPO / "build"
    build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix="theron-decision-instrument-", dir=build
    ) as name:
        source = Path(name) / "src/pce_fast/huc6280.cpp"
        source.parent.mkdir(parents=True)
        source.write_text(MARKER + "\n")
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), name], capture_output=True, text=True
        )
        assert result.returncode == 0, result.stderr
        changed = source.read_text()
        assert changed.count("FIRESTAFF_THERON_SYSTEM_CARD_DECISION_TRACE") == 1
        assert changed.index("decision_truncated cap=2048") < changed.index(MARKER)
        assert "pc == 0xE11D" in changed and "pc == 0xE209" in changed
        compiler = shlex.split(os.environ.get("CXX", "c++"))
        if compiler and shutil.which(compiler[0]):
            injected = changed[:changed.index(MARKER)]
            source.write_text(HARNESS.replace(MARKER, injected + MARKER, 1))
            result = subprocess.run(
                [*compiler, "-std=c++11", "-Werror", "-fsyntax-only", str(source)],
                capture_output=True, text=True,
            )
            assert result.returncode == 0, result.stderr
        result = subprocess.run(
            [sys.executable, str(INSTRUMENTER), name], capture_output=True, text=True
        )
        assert result.returncode != 0 and "already instrumented" in result.stderr
    print("test_instrument_mednafen_theron_syscard_decision_trace: PASS")


if __name__ == "__main__":
    main()
