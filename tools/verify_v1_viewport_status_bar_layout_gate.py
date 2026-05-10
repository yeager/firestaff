#!/usr/bin/env python3
"""Source-lock gate for DM1 V1 viewport status/status-bar layout."""
from __future__ import annotations
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
HEADER = ROOT / "src/test/dm1_v1_viewport_status_bar_layout_pc34_compat.h"
TEST = ROOT / "src/test/test_dm1_v1_viewport_status_bar_layout_pc34_compat.c"
M11 = ROOT / "m11_game_view.c"
ZONES = ROOT / "data/zones_h_reconstruction.json"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def line_no(text: str, needle: str) -> int:
    idx = text.find(needle)
    if idx < 0:
        raise AssertionError(f"missing source marker: {needle}")
    return text.count("\n", 0, idx) + 1


def require(text: str, needles: list[str], label: str) -> None:
    for needle in needles:
        if needle not in text:
            raise AssertionError(f"{label}: missing {needle!r}")


def function_body(text: str, name: str) -> str:
    start = text.find(name)
    if start < 0:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated function {name}")


def define_value(header: str, name: str) -> int:
    m = re.search(rf"^#define\s+{re.escape(name)}\s+(-?\d+)\b", header, re.M)
    if not m:
        raise AssertionError(f"missing define {name}")
    return int(m.group(1))


def main() -> int:
    cham = read(REDMCSB_ROOT / "CHAMDRAW.C")
    panel = read(REDMCSB_ROOT / "PANEL.C")
    defs = read(REDMCSB_ROOT / "DEFS.H")
    header = read(HEADER)
    test = read(TEST)
    m11 = read(M11)
    zones = json.loads(read(ZONES))["records"]

    f0287 = function_body(cham, "F0287_CHAMPION_DrawBarGraphs")
    require(f0287, [
        "L2254_i_Bars[0][0] = AL0845_ps_Champion->CurrentHealth",
        "L2254_i_Bars[1][0] = AL0845_ps_Champion->CurrentStamina",
        "L2254_i_Bars[2][0] = AL0845_ps_Champion->CurrentMana",
        "L2252_i_BoxIndex = P0605_i_ChampionIndex + C195_ZONE_FIRST_BAR_GRAPH",
        "L2251_i_Counter < 3",
        "L2252_i_BoxIndex += 4",
        "F0732_FillScreenArea(L2004_ai_XYZBlankBar, C12_COLOR_DARKEST_GRAY)",
        "F0732_FillScreenArea(L2005_ai_XYZColoredBar, G0046_auc_Graphic562_ChampionColor[P0605_i_ChampionIndex])",
    ], "ReDMCSB F0287")

    f0290 = function_body(cham, "F0290_CHAMPION_DrawHealthStaminaManaValues")
    require(f0290, [
        "C550_ZONE_HEALTH_VALUE",
        "C551_ZONE_MANA_VALUE",
        "CurrentStamina / 10",
        "C552_ZONE_STAMINA_VALUE",
        "CurrentMana",
    ], "ReDMCSB F0290")

    f0351 = function_body(panel, "F0351_INVENTORY_DrawChampionSkillsAndStatistics")
    require(f0351, [
        "C557_ZONE_SKILL_VALUE",
        "F0303_CHAMPION_GetSkillLevel",
        "MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE",
        "if (AL1092_i_SkillLevel == 1)",
        "G2088_C7_TextLineHeight",
        "G2016_ai_SkillRecentlyUpgraded",
    ], "ReDMCSB F0351")
    # F0351 references IGNORE_TEMPORARY_EXPERIENCE only as an input flag to
    # F0303_CHAMPION_GetSkillLevel; it has no C195-style XP/progress-bar path.
    if "C195_ZONE_FIRST_BAR_GRAPH" in f0351:
        raise AssertionError("ReDMCSB F0351 unexpectedly draws a status/XP bar")

    require(defs, [
        "#define C550_ZONE_HEALTH_VALUE",
        "#define C557_ZONE_SKILL_VALUE",
        "#define C195_ZONE_FIRST_BAR_GRAPH",
    ], "ReDMCSB DEFS")

    # Zone reconstruction: layout-696 status bars and inventory values.
    expected_zones = {
        "195": (7, 191, 5, 26), "199": (7, 191, 12, 26), "203": (7, 191, 19, 26),
        "550": (3, 4, 95, 116), "551": (3, 4, 95, 124), "552": (3, 4, 95, 132),
        "557": (4, 101, 28, 6),
    }
    for zid, expected in expected_zones.items():
        rec = zones[zid]
        got = (rec["type"], rec["parent"], rec["d1"], rec["d2"])
        if got != expected:
            raise AssertionError(f"zone {zid}: {got} != {expected}")

    # Header/test lock the same invariants, including no XP bar.
    constants = {
        "DM1_V1_STATUS_BOX_SPACING": 69,
        "DM1_V1_STATUS_BAR_WIDTH": 4,
        "DM1_V1_STATUS_BAR_HEIGHT": 25,
        "DM1_V1_STATUS_BAR_FIRST_X": 46,
        "DM1_V1_STATUS_BAR_STAT_SPACING": 7,
        "DM1_V1_HEALTH_VALUE_ZONE": 550,
        "DM1_V1_SKILL_VALUE_ZONE": 557,
        "DM1_V1_SKILL_LINE_HEIGHT": 7,
        "DM1_V1_XP_BAR_PRESENT": 0,
        "DM1_V1_XP_BAR_WIDTH": 0,
        "DM1_V1_XP_BAR_HEIGHT": 0,
    }
    for name, value in constants.items():
        if define_value(header, name) != value:
            raise AssertionError(f"{name} does not source-lock to {value}")
    require(test, [
        "dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_HEALTH) == 195 + c",
        "dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_STAMINA) == 199 + c",
        "dm1_v1_status_bar_value_zone_id(c, DM1_V1_STATUS_BAR_MANA) == 203 + c",
        "DM1_V1_XP_BAR_PRESENT == 0",
    ], "C test")

    # Active Firestaff V1 helpers must continue to expose source zone ids and geometry.
    require(m11, [
        "return 187 + championSlot;",
        "return 195 + statIndex * 4;",
        "return M11_GameView_GetV1StatusBarZoneId(statIndex) + championSlot;",
        "*outW = M11_V1_BAR_CONTAINER_W;",
        "*outH = M11_V1_BAR_CONTAINER_H;",
        "Pass 43: champion HP/stamina/mana bar graphs",
    ], "Firestaff m11_game_view")

    result = {
        "schema": "dm1_v1_viewport_status_bar_layout_gate.v1",
        "pass": True,
        "redmcsbEvidence": [
            {"file": "CHAMDRAW.C", "line": line_no(cham, "L2252_i_BoxIndex = P0605_i_ChampionIndex + C195_ZONE_FIRST_BAR_GRAPH"), "finding": "bar value zones start at C195 plus champion, then stride +4 per stat"},
            {"file": "CHAMDRAW.C", "line": line_no(cham, "F0732_FillScreenArea(L2005_ai_XYZColoredBar, G0046_auc_Graphic562_ChampionColor"), "finding": "colored fill uses champion color after blank portion"},
            {"file": "CHAMDRAW.C", "line": line_no(cham, "F0289_CHAMPION_DrawHealthOrStaminaOrManaValue(C550_ZONE_HEALTH_VALUE"), "finding": "health/stamina/mana value rows use zones C550/C551/C552"},
            {"file": "PANEL.C", "line": line_no(panel, "F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C557_ZONE_SKILL_VALUE"), "finding": "skill-level text starts at C557"},
            {"file": "PANEL.C", "line": line_no(panel, "F0303_CHAMPION_GetSkillLevel(L1093_ui_ChampionIndex"), "finding": "experience is converted to skill level for text; F0351 has no XP bar draw"},
        ],
    }
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
