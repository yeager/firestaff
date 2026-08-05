#!/usr/bin/env python3
"""Reject truncated c_hero substitutions in production spell timers.

SKProject/SKULLWIN/c_tim_proc.cpp types 0x47, 0x48 and 0x4b update fields of
the 263-byte c_hero.  Firestaff's old 261-byte session surrogate used unrelated
byte fields as stand-ins.  Ordered consumption is allowed while the real party
owner is absent; reads or writes of those stand-ins are not.
"""

from pathlib import Path
import sys


def function_body(source: str, marker: str) -> str:
    start = source.find(marker)
    if start < 0:
        raise ValueError(f"missing function marker: {marker}")
    opening = source.find("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise ValueError("unterminated function body")


def assert_gated(source: str, marker: str, timer: str) -> None:
    body = function_body(source, marker)
    forbidden = (
        "ctx->champions[",
        "ctx->hero_ench_countdown--",
        "ctx->ench_power[",
        "ctx->poison_strength[",
    )
    present = [token for token in forbidden if token in body]
    if present:
        raise ValueError(f"{timer} still mutates a surrogate: {', '.join(present)}")
    if "ctx->receipt.hero_state_owner_missing = 1;" not in body:
        raise ValueError(f"{timer} lacks its c_hero ownership gate")
    if "return 1;" not in body:
        raise ValueError(f"{timer} no longer consumes the source-ordered timer")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    source = (root / "src/dm2/dm2_v1_spell_timer_handlers_pc34_compat.c").read_text(
        encoding="utf-8"
    )
    try:
        assert_gated(source, "static int dm2_v1_spell_timer_handle_hero_ench_flag(", "0x47")
        assert_gated(source, "static int dm2_v1_spell_timer_handle_ench_power(", "0x48")
        assert_gated(source, "static int dm2_v1_spell_timer_handle_poison(", "0x4b")
    except ValueError as error:
        print(f"FAIL: {error}")
        return 1
    print("PASS: spell timers 0x47/0x48/0x4b do not mutate a truncated c_hero surrogate")
    return 0


if __name__ == "__main__":
    sys.exit(main())
