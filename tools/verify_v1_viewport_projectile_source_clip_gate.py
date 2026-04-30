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
SRC = ROOT / "m11_game_view.c"
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C")


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated {name}")


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
    text = SRC.read_text(encoding="utf-8")
    start, body = find_function(text, "m11_draw_projectile_sprite")
    require_in_order(body, [
        "m11_c2900_projectile_raw_zone_point(sourceZoneRow",
        "drawX = M11_VIEWPORT_X + zoneX - drawW / 2;",
        "if (sourceZoneRow >= 0)",
        "int minX = M11_VIEWPORT_X - drawW + 1;",
        "int maxX = M11_VIEWPORT_X + M11_VIEWPORT_W - 1;",
        "if (drawX > maxX) drawX = maxX;",
    ], "Firestaff C2900 projectile clip path")
    source_branch = body.split("if (sourceZoneRow >= 0)", 1)[1].split("        } else {", 1)[0]
    fallback_branch = body.split("if (sourceZoneRow >= 0)", 1)[1].split("        } else {", 1)[1]
    require_in_order(fallback_branch, [
        "if (drawX < x) drawX = x;",
        "if (drawX + drawW > x + w) drawX = x + w - drawW;",
    ], "Firestaff fallback pane clamp path")
    if "drawX < x" in source_branch or "drawX + drawW > x + w" in source_branch:
        raise AssertionError("source-row branch still clamps projectile X to synthetic pane bounds")

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
    print(f"- Firestaff m11_draw_projectile_sprite: {SRC}:{line_no(text, start)}")
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
