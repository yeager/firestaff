#!/usr/bin/env python3
"""Compile and exercise the scripted-input parser embedded in the Mednafen patch."""
from __future__ import annotations

import os
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATCH = ROOT / "scripts/mednafen_1.32.1_theron_scripted_pce_input.patch"


def added_lines(lines: list[str], start: int, end: int) -> str:
    selected = [line[1:] for line in lines[start:end] if line.startswith("+")]
    return "\n".join(selected)


def extract_patch_code() -> tuple[str, str]:
    lines = PATCH.read_text().splitlines()
    key_start = next(i for i, line in enumerate(lines) if line == "+static uint16 theron_script_key_mask(const char *key, size_t key_length)")
    key_end = next(i for i in range(key_start + 1, len(lines)) if lines[i] == "+ return 0;") + 2
    parser_start = next(i for i, line in enumerate(lines) if line == "+  while(theron_script_valid && *cursor && theron_script_event_count < 32)")
    final_check = next(i for i in range(parser_start + 1, len(lines)) if lines[i] == "+  if(theron_script_valid && (!theron_script_event_count || *cursor))")
    parser_end = next(i for i in range(final_check + 1, len(lines)) if lines[i] == "+   theron_script_valid = false;") + 1
    return added_lines(lines, key_start, key_end), added_lines(lines, parser_start, parser_end)


def main() -> int:
    (ROOT / "build").mkdir(exist_ok=True)
    compiler = os.environ.get("CXX")
    if compiler:
        command = shlex.split(compiler)
    else:
        compiler_path = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
        if not compiler_path:
            raise SystemExit("FAIL: a C++ compiler is required for the scripted-input parser check")
        command = [compiler_path]
    key_function, parser = extract_patch_code()
    source = f'''#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef unsigned short uint16;
{key_function}
bool parse_script(const char *script)
{{
 const char *cursor = script;
 bool theron_script_valid = script && *script;
 unsigned theron_script_event_count = 0;
 struct {{ unsigned frame; unsigned duration; uint16 mask; bool emitted; char key[8]; }} theron_script_events[32] = {{}};
 {parser}
 return theron_script_valid;
}}
int main(int argc, char **argv)
{{
 return argc == 2 && parse_script(argv[1]) ? 0 : 1;
}}
'''
    with tempfile.TemporaryDirectory(prefix="theron-parser.", dir=ROOT / "build") as temporary:
        cpp = Path(temporary) / "parser.cpp"
        binary = Path(temporary) / "parser"
        cpp.write_text(source)
        subprocess.run(command + ["-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)], check=True)
        for value in ("i@1", "run@1:5,ii@6:90", "up@1:1,down@2:2"):
            subprocess.run([str(binary), value], check=True)
        for value in ("i@1,", "i@+1", "i@-1", "i@4294967296", "i@1:+2", "i@1:-2", "i@1:4294967296", "i@1,,ii@2"):
            result = subprocess.run([str(binary), value], check=False)
            if result.returncode == 0:
                raise SystemExit(f"FAIL: malformed scripted input was accepted: {value}")
    print("PASS: scripted PCE input parser rejects malformed, signed, overflowing, and trailing-empty events")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
