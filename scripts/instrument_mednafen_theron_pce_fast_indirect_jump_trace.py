#!/usr/bin/env python3
"""Install the reference indirect-jump trace at its source-owned opcode path."""

from pathlib import Path
import sys


OLD_HANDLER = """\t    unsigned int npc;

            GetAB(tmp);
\t    tmp += HU_X;

            npc=RdMem(tmp);
            npc|=RdMem(tmp + 1)<<8;

\t    SetPC(npc);
"""
NEW_HANDLER = """            unsigned int npc;

            GetAB(tmp);
            const unsigned int dispatch_base = tmp;
            tmp += HU_X;

            const uint8 dispatch_low = RdMem(tmp);
            const uint8 dispatch_high = RdMem(tmp + 1);
            npc = dispatch_low | (dispatch_high << 8);
            TheronTraceIndirectJump(dispatch_base, HU_X, dispatch_low,
                                    dispatch_high, npc);

            SetPC(npc);
"""


def extract_function(patch_text: str) -> str:
    lines = patch_text.splitlines(keepends=True)
    start = next(
        (i for i, line in enumerate(lines) if line.startswith("+/* Reference-only observer")),
        None,
    )
    if start is None:
        raise SystemExit("FAIL: indirect-jump trace function is absent from patch")
    function = []
    for line in lines[start:]:
        if line.startswith("--- ") or line.startswith("@@ "):
            break
        if not line.startswith("+"):
            if function and function[-1].rstrip("\n") == "}":
                break
            raise SystemExit("FAIL: unexpected context inside indirect-jump function")
        function.append(line[1:])
    result = "".join(function)
    if not result.endswith("}\n"):
        raise SystemExit("FAIL: incomplete indirect-jump trace function")
    return result


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise SystemExit(f"FAIL: expected one {label} block")
    return text.replace(old, new, 1)


def insert_once(text: str, anchor: str, block: str, label: str) -> str:
    if block.strip() in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, anchor + block, 1)


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_indirect_jump_trace.py SOURCE PATCH"
        )
    root = Path(sys.argv[1])
    patch = Path(sys.argv[2])
    source = root / "src/pce_fast/huc6280.cpp"
    ops = root / "src/pce_fast/huc6280_ops.inc"
    if not source.is_file() or not ops.is_file() or not patch.is_file():
        raise SystemExit("FAIL: source or indirect-jump patch is unavailable")

    patch_text = patch.read_text(encoding="utf-8")
    source_text = source.read_text(encoding="utf-8")
    ops_text = ops.read_text(encoding="utf-8")
    source_text = insert_once(
        source_text,
        "#define BMT_PREHONK(pork) HuCPU.in_block_move = IBM_##pork;\n",
        extract_function(patch_text),
        "PCE Fast BMT macro",
    )
    ops_text = replace_once(ops_text, OLD_HANDLER, NEW_HANDLER, "opcode $7C handler")
    source.write_text(source_text, encoding="utf-8")
    ops.write_text(ops_text, encoding="utf-8")


if __name__ == "__main__":
    main()
