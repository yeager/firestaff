#!/usr/bin/env python3
"""Lock the ReDMCSB F0125..F0127 D0 material pass after D1 content."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
M11 = ROOT / "src/engine/m11_game_view.c"


def function_body(source: str, signature: str) -> str:
    start = source.rfind(signature)
    if start < 0:
        raise AssertionError(f"missing {signature}")
    brace = source.find("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated {signature}")


def main() -> int:
    source = M11.read_text(encoding="utf-8")
    viewport = function_body(source, "static void m11_draw_viewport")

    for signature in (
        "static void m11_draw_dm1_floor_pits",
        "static void m11_draw_dm1_stairs",
        "static void m11_draw_dm1_teleporter_fields",
    ):
        body = function_body(source, signature)
        assert "int minVisibleForward" in body
        assert "plan.relForward < minVisibleForward" in body
        assert "plan.relForward > maxVisibleForward" in body

    # F0128 no longer has global D3..D1 or D0 primitive batches.  The D0
    # terminal squares are replayed from their individual scheduler spans,
    # preserving F0125 (left), F0126 (right), F0127 (centre) order.
    d0 = function_body(source, "static void m11_dm1_f0128_execute_source_step")
    for step, call in (
        ("DM1_V1_F0128_STEP_F0104_PIT", "m11_draw_dm1_floor_pits"),
        ("DM1_V1_F0128_STEP_F0104_STAIRS", "m11_draw_dm1_stairs"),
        ("DM1_V1_F0128_STEP_F0113_FIELD",
         "m11_draw_dm1_teleporter_field_at"),
    ):
        assert step in d0 and call in d0, (
            f"missing scheduler-owned D0 material pass: {call}"
        )

    final = viewport.find("M11_DM1_F0128_EXECUTE_D0_BEFORE_THINGS")
    # C127 is a F0107 D1C wall overlay and now executes inside the owning
    # scheduler callback. Each D3..D1 F0115 step also completes its own C15
    # restart before the terminal D0 pit/stair/field phases begin.
    assert "m11_draw_dm1_front_mirror_route" in d0
    assert viewport.find("m11_draw_dm1_front_mirror_route") < 0
    assert d0.count("m11_draw_dm1_f0115_explosions_for_square") >= 2
    assert "m11_draw_dm1_deferred_explosion_pass(state" not in viewport
    assert final >= 0
    assert viewport.count("M11_DM1_F0128_EXECUTE_D0_BEFORE_THINGS") == 3
    after = viewport[final:]
    d0l = after.index("DM1_V1_F0128_VIEW_SQUARE_D0L")
    d0r = after.index("DM1_V1_F0128_VIEW_SQUARE_D0R")
    d0c = after.index("DM1_V1_F0128_VIEW_SQUARE_D0C")
    assert d0l < d0r < d0c
    assert "M11_DM1_F0128_EXECUTE_D0C_THINGS" in after
    assert "M11_DM1_F0128_EXECUTE_D0C_FIELD_AFTER_THINGS" in after
    assert after.index("M11_DM1_F0128_EXECUTE_D0_BEFORE_THINGS") < after.index(
        "M11_DM1_F0128_EXECUTE_D0C_THINGS") < after.index(
        "M11_DM1_F0128_EXECUTE_D0C_FIELD_AFTER_THINGS")
    assert "m11_draw_dm1_d0c_floor_item_pass" in d0
    assert viewport.find("m11_draw_dm1_d0c_floor_item_pass") < 0
    assert "m11_draw_dm1_stairs(state" not in viewport
    print("ok: scheduler-owned DM1 D0L, D0R and D0C transactions follow D1 in source order")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
