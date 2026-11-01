#!/usr/bin/env python3
"""Keep the indirect-jump follow forward declaration at file scope."""

from pathlib import Path
import sys


DECLARATION = (
    "static void TheronTraceIndirectJumpFollowArm(unsigned int target, "
    "uint32 dispatch_event);\n"
)


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_indirect_jump_follow_declaration.py SOURCE"
        )
    source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
    if not source.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")
    text = source.read_text(encoding="utf-8")
    text = text.replace(DECLARATION, "", 1)
    if text.count("static void TheronTraceIndirectJump(") != 1:
        raise SystemExit("FAIL: expected one indirect-jump trace function anchor")
    if DECLARATION not in text:
        text = text.replace(
            "static void TheronTraceIndirectJump(",
            DECLARATION + "static void TheronTraceIndirectJump(",
            1,
        )
    source.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
