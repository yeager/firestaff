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
    from instrument_mednafen_theron_mouse_trace import mouse_input_parser_source

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

    instrumenter = (ROOT / "scripts/instrument_mednafen_theron_mouse_trace.py").read_text()
    if instrumenter.count("+ mouse_input_parser_source()") != 2:
        raise SystemExit("FAIL: standard PCE and PCE Fast do not share the mouse parser")
    print("PASS: mouse parser expands bounded repeats, rejects malformed samples, and is shared by both PCE cores")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
