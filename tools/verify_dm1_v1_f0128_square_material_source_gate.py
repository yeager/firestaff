#!/usr/bin/env python3
"""Lock F0128 material dispatch and its final D0 source order."""

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
    for signature, source_name in (
        ("static void m11_draw_dm1_floor_pits", "F0104 pit"),
        ("static void m11_draw_dm1_floor_ornaments", "F0108 floor ornament"),
        ("static void m11_draw_dm1_stairs", "F0104 stairs"),
    ):
        body = function_body(source, signature)
        assert "m11_dm1_side_lane_clear_for_rel" not in body, (
            f"{source_name} must remain a F0128 square-material route")
        assert "plan.relForward > maxVisibleForward" in body or (
            "plan.relForward < minVisibleForward" in body
        )

    for signature, source_name in (
        ("static void m11_draw_dm1_floor_pits", "F0104 pit"),
        ("static void m11_draw_dm1_floor_ornaments", "F0108 floor ornament"),
        ("static void m11_draw_dm1_stairs", "F0104 stairs"),
    ):
        body = function_body(source, signature)
        assert "plan.relForward < minVisibleForward" in body, (
            f"{source_name} must support the final D0-only pass")

    for source_name, call in (
        ("F0104 pit", "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n                             1, 3, cells, -1, -2);"),
        ("F0108 floor ornament", "m11_draw_dm1_floor_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  1, 3, cells, -1, -2);"),
        ("F0104 stairs", "m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight,\n                        1, maxVisibleForward, cells, -1, -2);"),
    ):
        assert call in viewport, (
            f"{source_name} viewport call must retain full D3..D1 source dispatch"
        )

    # F0113 is no longer a global D3..D1 batch: F0128 invokes it after the
    # current square's F0115 route.  The scheduler-owned replay is the
    # authoritative D3..D1 dispatch, while the direct call below remains D0.
    assert "m11_dm1_f0128_replay_foreground_square" in viewport
    assert "DM1_V1_F0128_STEP_F0113_FIELD" in viewport

    # F0115's first door partition must be consumed inside the completed
    # source-square route, after its wall/ornament envelope and before F0111.
    # A global content walk cannot preserve that relation for overlapping
    # side lanes or a closed center door.
    pass1 = function_body(source, "static void m11_dm1_f0128_replay_door_pass1_square")
    assert "DM1_V1_F0128_STEP_F0115_DOOR_PASS1" in pass1
    assert "step->cellOrderWord" in pass1
    assert "DM1_V1_F0128_STEP_F0115_MAIN" not in pass1
    side_start = viewport.index("/* F0128 completes DnL's structural route")
    side_replay = viewport[side_start:]
    side_ornament = side_replay.index("m11_draw_dm1_wall_ornaments")
    side_pass1 = side_replay.index("m11_dm1_f0128_replay_door_pass1_square")
    side_door = side_replay.index("m11_draw_dm1_side_doors")
    assert side_ornament < side_pass1 < side_door
    center_replay = viewport[viewport.index("m11_draw_dm1_front_walls", side_start):]
    center_ornament = center_replay.index("m11_draw_dm1_wall_ornaments")
    center_pass1 = center_replay.index("m11_dm1_f0128_replay_door_pass1_square")
    center_door = center_replay.index("m11_draw_dm1_center_doors")
    assert center_ornament < center_pass1 < center_door

    effect = viewport.find("m11_draw_dm1_deferred_explosion_pass")
    d0 = viewport.find("m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n                             0, 0, cells, -1, -2);")
    mirror = viewport.find("m11_draw_dm1_front_mirror_route")
    assert mirror >= 0 and effect >= 0 and mirror < effect < d0
    print("ok: F0128 D3..D1 material is not side-culled and D0 is final")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
