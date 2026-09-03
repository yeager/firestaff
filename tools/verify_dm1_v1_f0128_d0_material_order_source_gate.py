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
    d0 = function_body(source, "static void m11_dm1_f0128_replay_d0_primitives")
    for step, call in (
        ("DM1_V1_F0128_STEP_F0104_PIT", "m11_draw_dm1_floor_pits"),
        ("DM1_V1_F0128_STEP_F0104_STAIRS", "m11_draw_dm1_stairs"),
        ("DM1_V1_F0128_STEP_F0113_FIELD",
         "m11_draw_dm1_teleporter_field_at"),
    ):
        assert step in d0 and call in d0, (
            f"missing scheduler-owned D0 material pass: {call}"
        )

    effect = viewport.find("m11_draw_dm1_deferred_explosion_pass")
    final = viewport.find("m11_dm1_f0128_replay_d0_primitives")
    mirror = viewport.find("m11_draw_dm1_front_mirror_route")
    # C127 is a F0107 D1C wall overlay; it remains before F0115/effects.
    # Only the D0 pit/stair/field pass is deferred until after those effects.
    assert mirror >= 0 and effect >= 0 and mirror < effect < final
    assert viewport.count("m11_dm1_f0128_replay_d0_primitives(") >= 4
    d0_left = viewport.find("DM1_V1_F0128_VIEW_SQUARE_D0L", final)
    d0_right = viewport.find("DM1_V1_F0128_VIEW_SQUARE_D0R", d0_left)
    d0_center = viewport.find("DM1_V1_F0128_VIEW_SQUARE_D0C", d0_right)
    assert final >= 0 and d0_left < d0_right < d0_center
    print("ok: scheduler-owned DM1 D0 material follows D1 content and effects")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
