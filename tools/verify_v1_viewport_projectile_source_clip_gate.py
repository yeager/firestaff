#!/usr/bin/env python3
"""Verify V1 source-row projectile placement is viewport-clipped, not pane-clamped.

ReDMCSB F0115 draws projectiles through source coordinate rows into the viewport
bitmap.  This gate prevents a regression where Firestaff resolves C2900 rows but
then clamps the resulting source coordinates back inside synthetic side/center
pane rectangles, hiding off-edge DM1 projectile placement.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
DM1_PROJ = ROOT / "src/dm1/dm1_v1_projectile_explosion_render_pc34_compat.c"
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    pattern = re.compile(
        r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pattern.finditer(text):
        brace = text.find("{", m.end())
        semi = text.find(";", m.end(), brace if brace >= 0 else len(text))
        if brace < 0 or semi >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"missing function body {name}")


def require_in_order(body: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = body.find(marker)
        if pos < 0:
            raise AssertionError(f"{label}: missing {marker!r}")
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} is out of order")
        last = pos


def main() -> int:
    # The C2900 clip path is owned by the DM1 projectile blit-plan module;
    # M11 only consumes the plan. Verify the branches at their owner.
    text = DM1_PROJ.read_text(encoding="utf-8")
    start, body = find_function(text, "dm1_v1_projectile_sprite_blit_plan")
    require_in_order(body, [
        "dm1_viewport_3d_c2900_projectile_raw_zone_point(sourceZoneRow",
        "plan.draw_x = viewportX + zoneX - plan.draw_w / 2;",
        "if (sourceZoneRow >= 0)",
        "int minX = viewportX - plan.draw_w + 1;",
        "int maxX = viewportX + viewportW - 1;",
        "if (plan.draw_x > maxX) plan.draw_x = maxX;",
    ], "Firestaff C2900 projectile clip path")
    source_branch = body.split("if (sourceZoneRow >= 0)", 1)[1].split("        } else {", 1)[0]
    fallback_branch = body.split("if (sourceZoneRow >= 0)", 1)[1].split("        } else {", 1)[1]
    require_in_order(fallback_branch, [
        "if (plan.draw_x < paneX) plan.draw_x = paneX;",
        "if (plan.draw_x + plan.draw_w > paneX + paneW) {",
    ], "Firestaff fallback pane clamp path")
    if "plan.draw_x < paneX" in source_branch or "plan.draw_x + plan.draw_w > paneX + paneW" in source_branch:
        raise AssertionError("source-row branch still clamps projectile X to synthetic pane bounds")

    # M11 consumes the DM1 blit plan; it must not re-implement the clip.
    fire = SRC.read_text(encoding="utf-8")
    _, m11_body = find_function(fire, "m11_draw_projectile_sprite_ex")
    if "dm1_v1_projectile_sprite_blit_plan(" not in m11_body:
        raise AssertionError("M11 projectile path does not consume the DM1 blit plan")

    red = REDMCSB.read_text(encoding="latin-1")
    for needle in [
        "P0145_i_ViewSquareIndex = AL0147_ui_ViewSquareIndexBackup",
        "Draw only projectiles at specified cell",
        "T0115015_DrawProjectileAsObject",
        "F0132_VIDEO_Blit(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport",
    ]:
        if needle not in red:
            raise AssertionError(f"missing ReDMCSB citation marker {needle!r}")

    print("V1 viewport projectile source clip gate passed")
    print(f"- Firestaff dm1_v1_projectile_sprite_blit_plan: {DM1_PROJ}:{line_no(text, start)}")
    for needle in [
        "Draw only projectiles at specified cell",
        "P0145_i_ViewSquareIndex = AL0147_ui_ViewSquareIndexBackup",
        "T0115015_DrawProjectileAsObject",
        "F0132_VIDEO_Blit(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport",
    ]:
        print(f"- ReDMCSB marker {needle!r}: {REDMCSB}:{line_no(red, red.find(needle))}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
