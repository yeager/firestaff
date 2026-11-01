#!/usr/bin/env python3
"""Install bounded PCE Fast target-RAM write tracing at stable anchors."""

from pathlib import Path
import sys


PROTOTYPE = """extern void TheronTracePCEFastMainRAMTargetWrite(uint32 logical_address,
                                                 uint32 physical_address, uint8 value);
"""
CALL = " TheronTracePCEFastMainRAMTargetWrite(A, physical, V);\n"


def extract_function(patch_text: str) -> str:
    lines = patch_text.splitlines(keepends=True)
    start = next(
        (i for i, line in enumerate(lines) if line.startswith("+void TheronTracePCEFastMainRAMTargetWrite(")),
        None,
    )
    if start is None:
        raise SystemExit("FAIL: target-RAM trace function is absent from its patch")
    function = []
    for line in lines[start:]:
        if line.startswith("@@ ") or line.startswith("--- "):
            break
        if not line.startswith("+"):
            if function and function[-1].rstrip("\n") == "}":
                break
            if function:
                raise SystemExit("FAIL: unexpected context inside target-RAM trace function")
            continue
        function.append(line[1:])
    result = "".join(function)
    if not result.endswith("}\n"):
        raise SystemExit("FAIL: incomplete target-RAM trace function in patch")
    return result


def insert_once(text: str, anchor: str, block: str, label: str) -> str:
    if block.strip() in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, block + anchor, 1)


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_main_ram_target_write_trace.py SOURCE PATCH"
        )
    root = Path(sys.argv[1])
    patch = Path(sys.argv[2])
    header = root / "src/pce_fast/huc6280.cpp"
    source = root / "src/pce_fast/pce.cpp"
    if not header.is_file() or not source.is_file() or not patch.is_file():
        raise SystemExit("FAIL: source or target-RAM trace patch is unavailable")

    function = extract_function(patch.read_text(encoding="utf-8"))
    header_text = header.read_text(encoding="utf-8")
    source_text = source.read_text(encoding="utf-8")
    header_text = insert_once(
        header_text,
        "static INLINE void WrMem(unsigned int A, uint8 V)\n",
        PROTOTYPE,
        "PCE Fast WrMem declaration",
    )
    header_text = insert_once(
        header_text,
        " HuCPU.PCEWrite[wmpr](physical, V);\n",
        CALL,
        "PCE Fast memory write dispatch",
    )
    source_text = insert_once(
        source_text,
        "static void TheronTraceA1B7BMTWrite(uint32 physical_destination,\n"
        "                                    size_t backing_offset,\n"
        "                                    uint8 old_value, uint8 written_value);\n",
        function,
        "target-RAM trace implementation",
    )
    header.write_text(header_text, encoding="utf-8")
    source.write_text(source_text, encoding="utf-8")


if __name__ == "__main__":
    main()
