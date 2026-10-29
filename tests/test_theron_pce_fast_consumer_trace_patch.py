#!/usr/bin/env python3
"""Exercise the PCE Fast consumer hook extracted from its maintained patch."""

from __future__ import annotations

import os
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PATCH = ROOT / "scripts/mednafen_1.32.1_theron_pce_fast_main_ram_consumer_read.patch"


def hook_source() -> str:
    lines = PATCH.read_text(encoding="utf-8").splitlines()
    start = next(
        index
        for index, line in enumerate(lines)
        if line.startswith("+void TheronTraceMainRAMConsumerRead(")
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
        raise ValueError("could not extract the complete consumer hook")
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
 uintptr_t FastPageR[8] = {};
 uint8 A = 0, X = 0, Y = 0, S = 0, P = 0;
} HuCPU;

static uint8 reader_code[131072] = {};
uint16 TheronTraceInstructionPC = 0xEA80;
uint32 TheronTraceInstructionPhysicalPC = 0x1FEA80;
uint32 TheronTraceCDSharedEventSequence = 0;

HOOK

int main(int argc, char **argv)
{
 if(argc != 2) return 2;
 for(unsigned i = 0; i < 8; i++) HuCPU.FastPageR[i] = reinterpret_cast<uintptr_t>(reader_code);
 if(argv[1][0] == 'p')
 {
  unsetenv("FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE");
  TheronTraceInstructionPC = 0x1000;
  TheronTraceInstructionPhysicalPC = 0x1F1000;
  for(unsigned i = 0; i < 17; i++)
   TheronTraceMainRAMConsumerRead(0x2000, 0x1F0000, 0x5A);
  return 0;
 }
 for(unsigned i = 0; i < 4096; i++)
 {
  TheronTraceMainRAMConsumerRead(0x2000, 0x1F0000 + (i & 0x1FFF), 0x5A);
  TheronTraceMainRAMConsumerRead(0x3000, 0x100000, 0xA5);
 }
 if(argv[1][0] == 'o')
 {
  TheronTraceMainRAMConsumerRead(0x2000, 0x1F0000, 0x5A);
  TheronTraceMainRAMConsumerRead(0x3000, 0x100000, 0xA5);
 }
 return 0;
}
"""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def run(executable: Path, mode: str, trace: Path, *, bad_path: bool = False) -> str:
    env = os.environ.copy()
    env["FIRESTAFF_THERON_MAIN_RAM_CONSUMER_SAMPLE_LIMIT"] = "4096"
    env["FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE"] = str(trace / "missing" / "trace") if bad_path else str(trace)
    env["FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE"] = "100000-100000"
    return subprocess.run(
        [str(executable), mode], check=False, capture_output=True, text=True, env=env
    ).stderr


def main() -> int:
    if os.name == "nt":
        print("SKIP: the extracted-hook fixture requires a POSIX C++ compiler")
        return 0
    compiler = shlex.split(os.environ.get("CXX", "c++"))
    if not compiler or shutil.which(compiler[0]) is None:
        print("SKIP: no C++ compiler available for the consumer-hook fixture")
        return 0

    build_directory = ROOT / "build"
    build_directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="theron-consumer-hook-", dir=build_directory) as temp:
        directory = Path(temp)
        source = directory / "consumer_hook.cpp"
        executable = directory / "consumer_hook"
        source.write_text(HARNESS.replace("HOOK", hook_source()), encoding="utf-8")
        build = subprocess.run(
            compiler + ["-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
            check=False,
            capture_output=True,
            text=True,
        )
        require(build.returncode == 0, f"consumer hook fixture did not compile:\n{build.stderr}")

        exact_trace = directory / "exact.trace"
        run(executable, "e", exact_trace)
        exact_lines = exact_trace.read_text(encoding="utf-8").splitlines()
        require(sum(line.startswith("main_ram_consumer_read ") and "sample_class=main_ram " in line for line in exact_lines) == 4096,
                "default reads were not retained through their exact sample limit")
        require(sum(line.startswith("main_ram_consumer_read ") and "sample_class=target_range " in line for line in exact_lines) == 4096,
                "target-range reads did not retain an independent exact-limit budget")
        require(not any(line.startswith("main_ram_consumer_truncated=") for line in exact_lines),
                "reaching an exact sample limit incorrectly marked the stream truncated")

        overflow_trace = directory / "overflow.trace"
        run(executable, "o", overflow_trace)
        overflow_lines = overflow_trace.read_text(encoding="utf-8").splitlines()
        require(sum(line.startswith("main_ram_consumer_truncated=main_ram_sample_limit ") for line in overflow_lines) == 1,
                "a dropped default read did not emit exactly one truncation marker")
        require(sum(line.startswith("main_ram_consumer_truncated=target_range_sample_limit ") for line in overflow_lines) == 1,
                "a dropped target-range read did not emit exactly one truncation marker")

        per_offset_trace = directory / "per-offset.trace"
        run(executable, "p", per_offset_trace)
        per_offset_lines = per_offset_trace.read_text(encoding="utf-8").splitlines()
        require(sum(line.startswith("main_ram_consumer_read ") for line in per_offset_lines) == 16,
                "per-address sample limit retained an unexpected number of reads")
        require(sum("reason=per_offset_sample_limit" in line for line in per_offset_lines) == 1,
                "a dropped per-address read did not produce a truncation marker")

        open_error = run(executable, "e", directory / "not-created", bad_path=True)
        require("FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_TRACE_ERROR error=open" in open_error,
                "trace-open failure was not reported to the capture process")

    print("PASS: PCE Fast consumer hook preserves independent budgets and reports actual truncation")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, AssertionError) as exc:
        raise SystemExit(f"FAIL: {exc}") from exc
