#!/usr/bin/env python3
"""Add shared PCE opcode-fetch context declarations to an isolated Mednafen tree."""

from pathlib import Path
import sys


HEADER_DECLARATIONS = """/* HuC6280::RunSub dispatches from PC before PC++.
 * This is diagnostic context for the reference capture hooks. */
MDFN_HIDE extern uint16 TheronPCEVDCInstructionPC;
MDFN_HIDE extern uint32 TheronPCEVDCInstructionPhysicalPC;
MDFN_HIDE extern uint8 TheronPCEVDCInstructionOpcode;
MDFN_HIDE extern bool TheronPCEVDCInstructionFetchInProgress;
"""

SOURCE_DEFINITIONS = """uint16 TheronPCEVDCInstructionPC;
uint32 TheronPCEVDCInstructionPhysicalPC;
uint8 TheronPCEVDCInstructionOpcode;
bool TheronPCEVDCInstructionFetchInProgress;

"""


def insert_once(text: str, anchor: str, block: str, label: str) -> str:
    if block.strip() in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, block + anchor, 1)


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_vdc_instruction_pc.py SOURCE"
        )

    root = Path(sys.argv[1])
    header = root / "src/pce/huc6280.h"
    source = root / "src/pce/huc6280.cpp"
    if not header.is_file() or not source.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE source is unavailable")

    header_text = insert_once(
        header.read_text(encoding="utf-8"),
        "class alignas(128) HuC6280\n{",
        HEADER_DECLARATIONS,
        "PCE HuC6280 class",
    )
    source_text = insert_once(
        source.read_text(encoding="utf-8"),
        "static FILE *TheronMainRAMLoaderTrace;\n",
        SOURCE_DEFINITIONS,
        "PCE main-RAM trace state",
    )

    header.write_text(header_text, encoding="utf-8")
    source.write_text(source_text, encoding="utf-8")


if __name__ == "__main__":
    main()
