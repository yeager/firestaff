#!/usr/bin/env python3
"""Compile the maintained writer-PC hook and exercise its sample-cap marker."""

from __future__ import annotations

import os
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PATCH = ROOT / "scripts/mednafen_1.32.1_theron_pce_fast_cd_ram_writer_pc_trace.patch"


def hook_source() -> str:
    lines = PATCH.read_text(encoding="utf-8").splitlines()
    start = next(
        index for index, line in enumerate(lines)
        if line.startswith("+static void TheronTraceCDRAMWriterPCWrite(")
    )
    body: list[str] = []
    brace_depth = 0
    saw_open_brace = False
    for line in lines[start:]:
        if not line.startswith("+"):
            break
        code = line[1:]
        body.append(code)
        saw_open_brace = saw_open_brace or "{" in code
        brace_depth += code.count("{") - code.count("}")
        if saw_open_brace and brace_depth == 0:
            break
    if not body or brace_depth != 0:
        raise ValueError("could not extract the complete writer-PC hook")
    return "\n".join(body)


HARNESS = r"""
#include <cstdint>
#include <cstdio>
#include <cstdlib>

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;

struct FakeHuCPU
{
 uint8 MPR[8] = {};
 uint8 A = 0, X = 0, Y = 0, S = 0, P = 0;
} HuCPU;

uint8 ROMSpace[0x200000] = {};
uint16 TheronTraceInstructionPC = 0xEA9C;
uint32 TheronTraceInstructionPhysicalPC = 0x1FEA9C;
uint32 TheronTraceCDSharedEventSequence = 0;

HOOK

int main(int argc, char **argv)
{
 if(argc != 2) return 2;
 if(setenv("FIRESTAFF_THERON_PCE_FAST_CD_RAM_WRITER_PC_TRACE", argv[1], 1) != 0) return 3;
 if(setenv("FIRESTAFF_THERON_PCE_FAST_CD_RAM_WRITER_PC_TRACE_LIMIT", "2", 1) != 0) return 4;
 TheronTraceCDRAMWriterPCWrite(0x1000D2, 0x11);
 TheronTraceCDRAMWriterPCWrite(0x1000D3, 0x22);
 TheronTraceCDRAMWriterPCWrite(0x1000D4, 0x33);
 return 0;
}
"""


def main() -> int:
    if os.name == "nt":
        print("SKIP: writer-PC hook fixture requires POSIX environment variables")
        return 0
    compiler = shlex.split(os.environ.get("CXX", "c++"))
    if not compiler or shutil.which(compiler[0]) is None:
        print("SKIP: no C++ compiler available for the writer-PC hook fixture")
        return 0

    build_directory = ROOT / "build"
    build_directory.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="theron-writer-pc-hook-", dir=build_directory) as temp:
        directory = Path(temp)
        source = directory / "writer_pc_hook.cpp"
        executable = directory / "writer_pc_hook"
        trace = directory / "writer.trace"
        source.write_text(HARNESS.replace("HOOK", hook_source()), encoding="utf-8")
        build = subprocess.run(
            compiler + ["-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
            check=False,
            capture_output=True,
            text=True,
        )
        if build.returncode != 0:
            raise AssertionError(f"writer-PC hook fixture did not compile:\n{build.stderr}")
        run = subprocess.run([str(executable), str(trace)], check=False, capture_output=True, text=True)
        if run.returncode != 0:
            raise AssertionError(f"writer-PC hook fixture exited {run.returncode}: {run.stderr}")

        rows = trace.read_text(encoding="ascii").splitlines()
        if not rows or not rows[0].startswith("source=mednafen-pce-fast-cd-ram-writer-pc-write-v1 "):
            raise AssertionError("writer-PC hook emitted an unexpected trace header")
        if sum(line.startswith("cd_ram_writer_pc_write ") for line in rows) != 2:
            raise AssertionError("writer-PC hook did not retain exactly its sample limit")
        if sum(line.startswith("cd_ram_writer_pc_truncated sequence=2 limit=2 reason=sample-cap") for line in rows) != 1:
            raise AssertionError("writer-PC hook did not mark its reached sample cap exactly once")
        if len(rows) != 4:
            raise AssertionError("writer-PC hook wrote rows after the sample-cap marker")

    print("PASS: writer-PC hook emits an explicit cap marker after exactly the bounded rows")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, AssertionError) as exc:
        raise SystemExit(f"FAIL: {exc}") from exc
