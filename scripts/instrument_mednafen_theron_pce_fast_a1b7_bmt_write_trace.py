#!/usr/bin/env python3
"""Install the bounded PCE Fast A1B7 BMT write probe at stable anchors."""

from pathlib import Path
import sys


PROTOTYPE = """static void TheronTraceA1B7BMTWrite(uint32 physical_destination,
                                    size_t backing_offset,
                                    uint8 old_value, uint8 written_value);
"""

OLD_BASE_WRITE = """static DECLFW(BaseRAMWrite)
{
 TheronTracePartyRAMWrite(A, V);
 BaseRAM[(size_t)A - (0xF8 * 8192)] = V;
}
"""
NEW_BASE_WRITE = """static DECLFW(BaseRAMWrite)
{
 TheronTracePartyRAMWrite(A, V);
 const size_t offset = (size_t)A - (0xF8 * 8192);
 TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);
 BaseRAM[offset] = V;
}
"""
OLD_MIRRORED_WRITE = """static DECLFW(BaseRAMWrite_Mirrored)
{
 TheronTracePartyRAMWrite(A, V);
 BaseRAM[A & 0x1FFF] = V;
}
"""
NEW_MIRRORED_WRITE = """static DECLFW(BaseRAMWrite_Mirrored)
{
 TheronTracePartyRAMWrite(A, V);
 const size_t offset = A & 0x1FFF;
 TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);
 BaseRAM[offset] = V;
}
"""


def extract_function(patch_text: str) -> str:
    signatures = [
        i
        for i, line in enumerate(patch_text.splitlines(keepends=True))
        if line.startswith("+static void TheronTraceA1B7BMTWrite(")
    ]
    if len(signatures) != 2:
        raise SystemExit("FAIL: expected prototype and implementation in A1B7 patch")
    lines = patch_text.splitlines(keepends=True)
    start = signatures[1]
    function = []
    for line in lines[start:]:
        if line.startswith("@@ ") or line.startswith("--- "):
            break
        if not line.startswith("+"):
            if function:
                raise SystemExit("FAIL: unexpected context inside A1B7 trace function")
            continue
        function.append(line[1:])
    result = "".join(function)
    if not result.endswith("}\n"):
        raise SystemExit("FAIL: incomplete A1B7 trace function in patch")
    return result


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise SystemExit(f"FAIL: expected one {label} block")
    return text.replace(old, new, 1)


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_a1b7_bmt_write_trace.py SOURCE PATCH"
        )
    root = Path(sys.argv[1])
    patch = Path(sys.argv[2])
    source = root / "src/pce_fast/pce.cpp"
    if not source.is_file() or not patch.is_file():
        raise SystemExit("FAIL: source or A1B7 trace patch is unavailable")
    text = source.read_text(encoding="utf-8")
    implementation = extract_function(patch.read_text(encoding="utf-8"))

    text = replace_once(
        text,
        OLD_BASE_WRITE,
        NEW_BASE_WRITE,
        "PCE Fast BaseRAM writer",
    )
    text = replace_once(
        text,
        OLD_MIRRORED_WRITE,
        NEW_MIRRORED_WRITE,
        "PCE Fast mirrored BaseRAM writer",
    )
    if PROTOTYPE not in text:
        anchor = "static DECLFW(BaseRAMWrite)\n"
        if text.count(anchor) != 1:
            raise SystemExit("FAIL: expected one BaseRAM declaration anchor")
        text = text.replace(anchor, PROTOTYPE + anchor, 1)
    if implementation.strip() not in text:
        anchor = "\nstatic DECLFR(IORead)\n"
        if text.count(anchor) != 1:
            raise SystemExit("FAIL: expected one post-BaseRAM IORead anchor")
        text = text.replace(anchor, "\n" + implementation + anchor, 1)
    source.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
