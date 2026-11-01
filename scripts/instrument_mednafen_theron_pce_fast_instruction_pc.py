#!/usr/bin/env python3
"""Install shared PCE Fast instruction trace declarations in Mednafen 1.32.1."""

from pathlib import Path
import sys


HEADER_DECLARATIONS = """MDFN_HIDE extern uint16 TheronTraceInstructionPC;
MDFN_HIDE extern uint32 TheronTraceInstructionPhysicalPC;
MDFN_HIDE void TheronTraceDirectMainRAMWrite(uint32 offset, uint8 old_value, uint8 value);

"""

SOURCE_DEFINITIONS = """uint16 TheronTraceInstructionPC;
uint32 TheronTraceInstructionPhysicalPC;

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
            "usage: instrument_mednafen_theron_pce_fast_instruction_pc.py SOURCE"
        )

    root = Path(sys.argv[1])
    header = root / "src/pce_fast/huc6280.h"
    source = root / "src/pce_fast/huc6280.cpp"
    if not header.is_file() or not source.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")

    header_text = insert_once(
        header.read_text(encoding="utf-8"),
        "struct HuC6280\n{",
        HEADER_DECLARATIONS,
        "PCE Fast HuC6280 struct",
    )
    source_text = insert_once(
        source.read_text(encoding="utf-8"),
        "static uint8 dummy_bank[8192 + 8192];  // + 8192 for PC-as-ptr safety padding\n",
        SOURCE_DEFINITIONS,
        "PCE Fast dummy bank",
    )

    header.write_text(header_text, encoding="utf-8")
    source.write_text(source_text, encoding="utf-8")


if __name__ == "__main__":
    main()
