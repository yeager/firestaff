#!/usr/bin/env python3
"""Keep hero-owned runtime timers fail-closed until c_hero is imported.

SKProject/SKULLWIN/c_tim_proc.cpp::DM2_PROCESS_TIMER_0C and
DM2_PROCESS_TIMER_RESURRECTION mutate the original 263-byte c_hero. The
bounded Firestaff session record is not c_hero, so runtime may consume either
timer but must not mutate that surrogate.
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


def assert_surrogate_free(source: str, marker: str, timer_name: str) -> None:
    body = function_body(source, marker)
    forbidden = (
        "DM2_ChampionRecord",
        "dm2_v1_bring_champion_to_life",
        "session_snapshot",
        "hero_flag",
        "timer_index",
        "inventory",
        "cur_hp",
        "max_hp",
    )
    present = [token for token in forbidden if token in body]
    if present:
        raise ValueError(
            f"{timer_name} handler mutates or reads surrogate fields: "
            + ", ".join(present)
        )
    if "return 1;" not in body:
        raise ValueError(f"{timer_name} handler no longer consumes the ordered timer")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    runtime = root / "src/dm2/dm2_v1_runtime.c"
    source = runtime.read_text(encoding="utf-8")
    try:
        assert_surrogate_free(source, "static int dm2_runtime_process_0c_timer(",
                              "type-0x0C")
        assert_surrogate_free(source, "static int dm2_runtime_resurrection_timer(",
                              "type-0x0D")
    except ValueError as error:
        print(f"FAIL: {error}")
        return 1
    if ("dispatcher.handlers[DM2_V1_TIMER_PROCESS_0C] =" not in source or
            "dispatcher.handlers[DM2_V1_TIMER_RESURRECTION] =" not in source):
        print("FAIL: runtime no longer routes source hero timers through ordered handlers")
        return 1

    print("PASS: types 0x0C/0x0D remain ordered and fail-closed for non-source c_hero state")
    return 0


if __name__ == "__main__":
    sys.exit(main())
