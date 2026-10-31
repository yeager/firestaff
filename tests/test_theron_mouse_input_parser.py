#!/usr/bin/env python3
"""Compile and exercise the bounded mouse-script parser used by both PCE cores."""
from __future__ import annotations

import os
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    (ROOT / "build").mkdir(exist_ok=True)
    compiler = os.environ.get("CXX")
    if compiler:
        command = shlex.split(compiler)
    else:
        compiler_path = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
        if not compiler_path:
            raise SystemExit("FAIL: a C++ compiler is required for the mouse parser check")
        command = [compiler_path]

    sys_path = str(ROOT / "scripts")
    import sys

    sys.path.insert(0, sys_path)
    from instrument_mednafen_theron_mouse_trace import (
        mouse_input_parser_source,
        pointer_c1df_trace_gate_source,
    )

    source = '''#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int16_t int16;
typedef uint8_t uint8;
''' + mouse_input_parser_source() + '''
int main(int argc, char **)
{
 int16 dx = 0, dy = 0;
 uint8 buttons = 0;
 unsigned emitted = 0;
 while(emitted < 20002 && TheronNextMouseInput(&dx, &dy, &buttons))
 {
  printf("%d,%d,%u\\n", dx, dy, (unsigned)buttons);
  emitted++;
 }
 puts("end");
 return argc == 2 ? 0 : 2;
}
'''
    with tempfile.TemporaryDirectory(prefix="theron-mouse-parser.", dir=ROOT / "build") as temporary:
        cpp = Path(temporary) / "parser.cpp"
        binary = Path(temporary) / "parser"
        cpp.write_text(source)
        subprocess.run(
            command + ["-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
            check=True,
        )

        def run(script: str) -> list[str]:
            environment = os.environ.copy()
            environment["FIRESTAFF_THERON_MOUSE_INPUT_SCRIPT"] = script
            return subprocess.run(
                [str(binary), "run"], check=True, text=True, capture_output=True, env=environment
            ).stdout.splitlines()

        repeated = run("1,-2,3*3;0,0,0")
        expected = ["1,-2,3"] * 3 + ["0,0,0", "end"]
        if repeated != expected:
            raise SystemExit(f"FAIL: repeat expansion mismatch: {repeated!r}")

        bounded = run("0,0,0*20000;1,0,0")
        if len(bounded) != 20001 or bounded[0] != "0,0,0" or bounded[-1] != "end":
            raise SystemExit("FAIL: expanded mouse script did not stop at the 20000-update bound")

        for malformed in (
            "1,2,3junk",
            "+1,2,3",
            "1,2,3*+1",
            "1,2,3*0",
            "1,2,3*20001",
            "0,0,16",
            "-128,0,0",
        ):
            result = run(malformed)
            if result != ["end"]:
                raise SystemExit(f"FAIL: malformed mouse sample was accepted: {malformed!r}")

        gate_cpp = Path(temporary) / "pointer_gate.cpp"
        gate_binary = Path(temporary) / "pointer_gate"
        gate_cpp.write_text(
            '#include <stdbool.h>\n#include <stdlib.h>\n#include <stdio.h>\n'
            + pointer_c1df_trace_gate_source()
            + '\nint main(void) { puts(TheronIsPointerC1DFTraceEnabled() ? "on" : "off"); return 0; }\n'
        )
        subprocess.run(
            command + ["-std=c++11", "-Wall", "-Wextra", "-Werror", str(gate_cpp), "-o", str(gate_binary)],
            check=True,
        )
        for value, expected in (("0", "off"), ("1", "on"), ("true", "off")):
            environment = os.environ.copy()
            environment["FIRESTAFF_THERON_POINTER_C1DF_TRACE"] = value
            result = subprocess.run(
                [str(gate_binary)], check=True, text=True, capture_output=True, env=environment
            ).stdout.strip()
            if result != expected:
                raise SystemExit(f"FAIL: pointer trace flag {value!r} returned {result!r}")

    instrumenter = (ROOT / "scripts/instrument_mednafen_theron_mouse_trace.py").read_text()
    if instrumenter.count("+ mouse_input_parser_source()") != 2:
        raise SystemExit("FAIL: standard PCE and PCE Fast do not share the mouse parser")
    if instrumenter.count("+ pointer_c1df_trace_gate_source()") != 1:
        raise SystemExit("FAIL: C1DF tracing does not use the tested opt-in gate")
    if '"\\t const uint8 reader_mpr = MPR[reader_pc >> 13];\\n"' not in instrumenter:
        raise SystemExit("FAIL: mouse guest-read hook is not anchored to the current reader MPR")
    if '"\\t const uint16 reader_pc = GetRegister(GSREG_PC);\\n"\n        "\\t const uint8 reader_mpr' in instrumenter:
        raise SystemExit("FAIL: mouse guest-read hook still assumes stale adjacent reader fields")
    capture_script = (ROOT / "scripts/capture_theron_mednafen_live_trace.sh").read_text()
    if 'FIRESTAFF_THERON_POINTER_C1DF_TRACE=\"$capture_pointer_c1df_trace\"' not in capture_script:
        raise SystemExit("FAIL: capture flag is not passed to the C1DF trace gate")
    print("PASS: mouse parser bounds pass and C1DF tracing is opt-in behind the capture flag")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
