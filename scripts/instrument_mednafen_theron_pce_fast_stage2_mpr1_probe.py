#!/usr/bin/env python3
"""Apply the existing bounded Stage 2 MPR1 probe at stable source anchors."""

from pathlib import Path
import sys


INCLUDES = "#include <stdio.h>\n#include <stdlib.h>\n"
CALL = (
    "\t  TheronTraceStage2MPR1(GetRealPC(), X_local, Y_local, P_local,\n"
    "\t                        HU_ZNFlags);\n"
)


def additions_from_hunk(patch_text: str, start: str) -> str:
    lines = patch_text.splitlines(keepends=True)
    try:
        index = next(i for i, line in enumerate(lines) if line.startswith(start))
    except StopIteration as error:
        raise SystemExit(f"FAIL: missing probe patch hunk {start}") from error
    additions = []
    for line in lines[index + 1 :]:
        if line.startswith("@@ ") or line.startswith("--- "):
            break
        if line.startswith("+") and not line.startswith("+++"):
            additions.append(line[1:])
        elif line.startswith("-"):
            raise SystemExit("FAIL: unexpected removal in probe patch hunk")
    return "".join(additions)


def insert_once(text: str, anchor: str, block: str, label: str) -> str:
    if block.strip() in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, block + anchor, 1)


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_stage2_mpr1_probe.py SOURCE PATCH"
        )
    root = Path(sys.argv[1])
    patch_path = Path(sys.argv[2])
    source = root / "src/pce_fast/huc6280.cpp"
    if not source.is_file() or not patch_path.is_file():
        raise SystemExit("FAIL: source or Stage 2 MPR1 probe patch is unavailable")

    patch_text = patch_path.read_text(encoding="utf-8")
    function = additions_from_hunk(patch_text, "@@ -29,1 +31,201 @@")
    if not function.startswith("\nstatic void TheronTraceStage2MPR1("):
        raise SystemExit("FAIL: Stage 2 MPR1 probe function hunk changed unexpectedly")
    source_text = source.read_text(encoding="utf-8")
    source_text = insert_once(
        source_text,
        '#include "vdc.h"\n',
        INCLUDES,
        "PCE Fast VDC include",
    )
    source_text = insert_once(
        source_text,
        "HuC6280 HuCPU;\n",
        function,
        "Stage 2 MPR1 probe function",
    )
    if CALL.strip() not in source_text:
        source_text = insert_once(
            source_text,
            "\t  b1 = RdAtPC();\n",
            CALL,
            "PCE Fast opcode-fetch call",
        )
    source.write_text(source_text, encoding="utf-8")


if __name__ == "__main__":
    main()
