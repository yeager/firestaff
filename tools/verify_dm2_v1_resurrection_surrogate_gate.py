#!/usr/bin/env python3
"""Keep type-0x0D runtime dispatch fail-closed until c_hero is imported.

SKProject/SKULLWIN/c_tim_proc.cpp::DM2_PROCESS_TIMER_RESURRECTION has three
dependent phases.  The bounded Firestaff session record is not the original
263-byte c_hero, so runtime may consume this timer but must not mutate that
surrogate from the final phase alone.
"""

from pathlib import Path
import sys


def function_body(source: str, marker: str) -> str:
    start = source.find(marker)
    if start < 0:
        raise ValueError(f"missing function marker: {marker}")
    opening = source.find("{", start)
    if opening < 0:
        raise ValueError("missing function opening brace")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise ValueError("unterminated function body")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    runtime = root / "src/dm2/dm2_v1_runtime.c"
    source = runtime.read_text(encoding="utf-8")
    try:
        body = function_body(source, "static int dm2_runtime_resurrection_timer(")
    except ValueError as error:
        print(f"FAIL: {error}")
        return 1

    forbidden = (
        "DM2_ChampionRecord",
        "dm2_v1_bring_champion_to_life",
        "session_snapshot",
        "hero_flag",
        "inventory",
        "cur_hp",
        "max_hp",
    )
    present = [token for token in forbidden if token in body]
    if present:
        print("FAIL: resurrection handler mutates or reads surrogate fields:",
              ", ".join(present))
        return 1
    if "return 1;" not in body:
        print("FAIL: resurrection handler no longer consumes the ordered timer")
        return 1
    if "dispatcher.handlers[DM2_V1_TIMER_RESURRECTION] =" not in source:
        print("FAIL: runtime no longer routes type-0x0D through its ordered handler")
        return 1

    print("PASS: type-0x0D remains ordered and fail-closed for non-source c_hero state")
    return 0


if __name__ == "__main__":
    sys.exit(main())
