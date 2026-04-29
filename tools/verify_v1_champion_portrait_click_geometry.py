#!/usr/bin/env python3
"""Pass165: source-lock the viewport click geometry for champion portraits.

This follows pass164 by proving where the source places the clickable/drawn
front portrait and how runtime coordinates are normalized.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = REPO / "parity-evidence/verification/pass165_champion_portrait_click_geometry"

CHECKS = [
    {
        "id": "SRC_GEOM_001",
        "file": "DUNVIEW.C",
        "needles": [
            "unsigned char G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4] = { 96, 127, 35, 63 };",
            "C026_GRAPHIC_CHAMPION_PORTRAITS",
            "A portrait is 32x29 pixels",
        ],
        "claim": "Legacy/byte-box portrait-on-wall area is viewport-relative x=96..127 y=35..63 (32x29).",
    },
    {
        "id": "SRC_GEOM_002",
        "file": "COORD.C",
        "needles": [
            "int16_t G2067_i_ViewportScreenX = 0;",
            "int16_t G2068_i_ViewportScreenY = 33;",
            "int16_t G2078_C32_PortraitWidth = 32;",
            "int16_t G2079_C29_PortraitHeight = 29;",
        ],
        "claim": "PC viewport origin is screen x=0 y=33; champion portrait dimensions are 32x29.",
    },
    {
        "id": "SRC_GEOM_003",
        "file": "DUNVIEW.C",
        "needles": [
            "if (P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT)",
            "F0007_MAIN_CopyBytes((char*)G2032_ai_XYZ, (char*)G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]",
            "M635_ZONE_PORTRAIT_ON_WALL",
            "F0654_Call_F0132_VIDEO_Blit",
        ],
        "claim": "For the front wall, the source copies the drawn wall/portrait zone into C05 clickable storage before drawing the portrait.",
    },
    {
        "id": "SRC_GEOM_004",
        "file": "CLIKVIEW.C",
        "needles": [
            "P0752_i_X -= G2067_i_ViewportScreenX;",
            "P0753_i_Y -= G2068_i_ViewportScreenY;",
            "if (F0798_COMMAND_IsPointInZone(G2210_aai_XYZ_DungeonViewClickable[AL1150_ui_ViewCell], P0752_i_X, P0753_i_Y))",
            "if (AL1150_ui_ViewCell == C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT)",
            "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor();",
        ],
        "claim": "Runtime clicks are converted to viewport-relative coordinates, then tested against C05; C05 triggers F0372.",
    },
]


def find(path: Path, needles: list[str]) -> list[dict[str, object]]:
    lines = path.read_text(errors="replace").splitlines()
    result = []
    for needle in needles:
        for lineno, text in enumerate(lines, 1):
            if needle in text:
                result.append({"needle": needle, "line": lineno, "text": text.strip()})
                break
        else:
            raise AssertionError(f"missing {needle!r} in {path}")
    return result


def extract_box() -> dict[str, int]:
    text = (SRC / "DUNVIEW.C").read_text(errors="replace")
    m = re.search(r"G0109_auc_Graphic558_Box_ChampionPortraitOnWall\[4\]\s*=\s*\{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\s*\}", text)
    if not m:
        raise AssertionError("portrait box not found")
    x1, x2, y1, y2 = map(int, m.groups())
    return {"viewport_x1": x1, "viewport_x2": x2, "viewport_y1": y1, "viewport_y2": y2, "viewport_center_x": (x1 + x2) // 2, "viewport_center_y": (y1 + y2) // 2}


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    rows = []
    for check in CHECKS:
        rows.append({**check, "source": str(SRC / check["file"]), "hits": find(SRC / check["file"], check["needles"]), "status": "PASS"})
    box = extract_box()
    # Source constants: viewport screen origin x=0, y=33 for PC; F0377 subtracts it.
    click = {**box, "screen_center_x": box["viewport_center_x"], "screen_center_y": box["viewport_center_y"] + 33, "viewport_origin_x": 0, "viewport_origin_y": 33}
    result = {"schema": "pass165_champion_portrait_click_geometry.v1", "recommended_click": click, "checks": rows}
    (OUT / "click_geometry.json").write_text(json.dumps(result, indent=2) + "\n")
    md = [
        "# Pass 165 — champion portrait click geometry",
        "",
        "Source-first answer for the next runtime route attempt.",
        "",
        "## Recommended click",
        "",
        f"- Viewport-relative portrait box: x={box['viewport_x1']}..{box['viewport_x2']}, y={box['viewport_y1']}..{box['viewport_y2']}",
        f"- Safe center viewport click: x={box['viewport_center_x']}, y={box['viewport_center_y']}",
        f"- PC screen click after viewport origin y=33: x={click['screen_center_x']}, y={click['screen_center_y']}",
        "",
        "Use this as the first action while facing a champion mirror/front wall. Only after candidate state is visible should C160/C161 be clicked.",
        "",
        "## Checks",
        "",
    ]
    for row in rows:
        locs = ", ".join(str(h["line"]) for h in row["hits"])
        md.append(f"- {row['id']} PASS — `{row['file']}` lines {locs}: {row['claim']}")
    md.append("")
    (OUT / "README.md").write_text("\n".join(md))
    print(f"PASS wrote {OUT}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
