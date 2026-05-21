#!/usr/bin/env python3
"""DM1 V1 champion statistic drawn-panel source-lock / pixel blocker gate.

This is intentionally source-first. It locks the ReDMCSB contract for the
inventory eye panel statistic rows and records the exact evidence still needed
before anyone can claim original-vs-Firestaff pixel parity for that panel.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

PASS = "pass621_dm1_v1_champion_stat_drawn_panel_pixel_gate"
STATUS = "PASS_SOURCE_LOCKED_PIXEL_PARITY_BLOCKED_ON_ORIGINAL_REFERENCE"

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
GREAT = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM"

MANIFEST = ROOT / f"parity-evidence/verification/{PASS}/manifest.json"
REPORT = ROOT / f"parity-evidence/{PASS}.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require_needles(label: str, text: str, needles: list[str]) -> dict[str, Any]:
    ctext = compact(text)
    missing = [needle for needle in needles if compact(needle) not in ctext]
    return {"id": label, "ok": not missing, "missing": missing, "needleCount": len(needles)}


def block(path: Path, start: int, end: int) -> str:
    lines = read(path, "latin-1").splitlines()
    return "\n".join(lines[start - 1:end])


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def audit_redmcsb() -> list[dict[str, Any]]:
    panel = RED / "PANEL.C"
    defs = RED / "DEFS.H"
    coord = RED / "COORD.C"
    return [
        require_needles(
            "PANEL.C:F0351 draws the empty panel then statistic text runs",
            block(panel, 2013, 2108),
            [
                "F0334_INVENTORY_CloseChest();",
                "G0424_i_PanelContent = C02_PANEL_SKILLS_AND_STATISTICS;",
                "F0658_BlitBitmapIndexToZoneIndexWithTransparency(C020_GRAPHIC_PANEL_EMPTY, C101_ZONE_PANEL, C08_COLOR_RED);",
                "F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C557_ZONE_SKILL_VALUE, &L2865_i_X, &L1091_i_Y);",
                "F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C559_ZONE_STATISTIC_VALUE, &L2866_i_X2, &L1091_i_Y);",
                "for (AL1090_ui_StatisticIndex = C1_STATISTIC_STRENGTH; AL1090_ui_StatisticIndex <= C6_STATISTIC_ANTIFIRE; AL1090_ui_StatisticIndex++)",
                "AL1092_i_StatisticCurrentValue = L1094_ps_Champion->Statistics[AL1090_ui_StatisticIndex][C1_CURRENT];",
                "L1096_ui_StatisticMaximumValue = L1094_ps_Champion->Statistics[AL1090_ui_StatisticIndex][C0_MAXIMUM];",
                "L1095_i_StatisticColor = C08_COLOR_RED;",
                "L1095_i_StatisticColor = C07_COLOR_LIGHT_GREEN;",
                "L1095_i_StatisticColor = C13_COLOR_LIGHTEST_GRAY;",
                "F0052_TEXT_PrintToViewport(L2866_i_X2, L1091_i_Y, L1095_i_StatisticColor, F0288_CHAMPION_GetStringFromInteger(AL1092_i_StatisticCurrentValue, C1_TRUE, 3));",
                "M547_STRCPY(L1097_ac_String, \"/\");",
                "M545_STRCAT(L1097_ac_String, F0288_CHAMPION_GetStringFromInteger(L1096_ui_StatisticMaximumValue, C1_TRUE, 3));",
                "F0052_TEXT_PrintToViewport(L2866_i_X2  + (G2087_C6_TextCharacterWidth * 3), L1091_i_Y, C13_COLOR_LIGHTEST_GRAY, L1097_ac_String);",
                "L1091_i_Y += G2088_C7_TextLineHeight;",
            ],
        ) | {"citation": "PANEL.C:2013-2108"},
        require_needles(
            "DEFS.H statistic and zone ids",
            block(defs, 743, 754) + "\n" + block(defs, 3919, 3927),
            [
                "#define C1_STATISTIC_STRENGTH  1",
                "#define C6_STATISTIC_ANTIFIRE  6",
                "#define C0_MAXIMUM             0",
                "#define C1_CURRENT             1",
                "#define C556_ZONE_OBJECT_DESCRIPTION                            556",
                "#define C557_ZONE_SKILL_VALUE                                   557",
                "#define C559_ZONE_STATISTIC_VALUE                               559",
            ],
        ) | {"citation": "DEFS.H:743-754,3919-3927"},
        require_needles(
            "COORD.C text metrics and one-pixel zone margin",
            block(coord, 1753, 1758) + "\n" + block(coord, 2434, 2448),
            [
                "int16_t G2087_C6_TextCharacterWidth = 6;",
                "int16_t G2088_C7_TextLineHeight = 7;",
                "void F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(",
                "L2310_i_X = 1;",
                "L2311_i_Y = 1;",
                "*P2138_pi_X = M704_ZONE_LEFT(L2312_ai_XYZ);",
                "*P2139_pi_Y = M706_ZONE_TOP(L2312_ai_XYZ);",
            ],
        ) | {"citation": "COORD.C:1753-1758,2434-2448"},
    ]


def audit_greatstone() -> dict[str, Any]:
    gdm = GREAT / "raw/greatstone.free.fr__dm__g_dm.html.html"
    text = read(gdm, "latin-1")
    check = require_needles(
        "Greatstone lists PC 3.4 graphics/dungeon/title sources",
        text,
        [
            "<TR><TD>Dungeon Master</TD><TD>PC</TD><TD>3.4</TD><TD>",
            "db_data/dm_pc_34/dungeon.dat/dungeon.html",
            "db_data/dm_pc_34/graphics.dat/graphics.dat.html",
            "db_data/dm_pc_34/title/title.html",
            "<TR><TD>Dungeon Master</TD><TD>PC</TD><TD>3.4 (en-fr-ge)</TD><TD>",
            "db_data/dm_pc_34_multi/graphics.dat/graphics.dat.html",
        ],
    )
    check["citation"] = str(gdm)
    return check


def audit_original() -> dict[str, Any]:
    canon = ORIG / "_canonical/dm1"
    readme = read(canon / "README.md")
    graphics = canon / "GRAPHICS.DAT"
    dungeon = canon / "DUNGEON.DAT"
    checks = [
        require_needles(
            "canonical README locks PC34 data hashes",
            readme,
            [
                "GRAPHICS.DAT",
                "sha256: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
                "DUNGEON.DAT",
                "sha256: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
                "DungeonMasterPC34",
            ],
        ) | {"citation": str(canon / "README.md")},
        {
            "id": "canonical GRAPHICS.DAT SHA256",
            "ok": sha256(graphics) == "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
            "path": str(graphics),
            "sha256": sha256(graphics),
            "expected": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        },
        {
            "id": "canonical DUNGEON.DAT SHA256",
            "ok": sha256(dungeon) == "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
            "path": str(dungeon),
            "sha256": sha256(dungeon),
            "expected": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        },
    ]
    return {"checks": checks, "ok": all(c["ok"] for c in checks)}


def audit_firestaff() -> list[dict[str, Any]]:
    header = read(ROOT / "include/dm1_v1_champion_panel_hud_pc34_compat.h")
    impl = read(ROOT / "src/dm1/dm1_v1_champion_panel_hud_pc34_compat.c")
    panel_test = read(ROOT / "tests/test_dm1_v1_champion_panel_hud_pc34_compat.c")
    runtime_test = read(ROOT / "tests/test_m11_inventory_full_panel_runtime_pc34_compat.c")
    cmake = read(ROOT / "CMakeLists.txt")
    return [
        require_needles(
            "local header exposes source statistic panel constants",
            header,
            [
                "#define DM1_ZONE_SKILL_VALUE      557",
                "#define DM1_ZONE_STATISTIC_VALUE  559",
                "#define DM1_PANEL_TEXT_CHAR_WIDTH   6",
                "#define DM1_PANEL_TEXT_LINE_HEIGHT  7",
                "#define DM1_STATISTIC_NAME_REL_X    28",
                "#define DM1_STATISTIC_CURRENT_REL_X 94",
                "#define DM1_STATISTIC_FIRST_REL_Y   34",
                "int DM1_ChampionPanel_BuildStatisticTextRunModel(",
            ],
        ) | {"citation": "include/dm1_v1_champion_panel_hud_pc34_compat.h:131-138,235"},
        require_needles(
            "local implementation preserves split current/max text runs",
            impl,
            [
                "outRun->nameZone = DM1_ZONE_SKILL_VALUE;",
                "outRun->valueZone = DM1_ZONE_STATISTIC_VALUE;",
                "outRun->nameX = DM1_STATISTIC_NAME_REL_X;",
                "outRun->currentX = DM1_STATISTIC_CURRENT_REL_X;",
                "outRun->maximumX = DM1_STATISTIC_CURRENT_REL_X +",
                "DM1_PANEL_TEXT_CHAR_WIDTH * 3;",
                "outRun->y = DM1_STATISTIC_FIRST_REL_Y +",
                "DM1_PANEL_TEXT_LINE_HEIGHT * statisticIndex;",
                "outRun->nameColor = DM1_COLOR_LIGHTEST_GRAY;",
                "DM1_ChampionPanel_StatisticCurrentColor(currentValue, maximumValue);",
                "int DM1_ChampionPanel_StatisticMaximumColor(void)",
                "return DM1_COLOR_LIGHTEST_GRAY;",
                "outRun->maximumColor = DM1_ChampionPanel_StatisticMaximumColor();",
                "DM1_ChampionPanel_FormatStatisticValue(currentValue, maximumValue,",
            ],
        ) | {"citation": "src/dm1/dm1_v1_champion_panel_hud_pc34_compat.c:288-313"},
        require_needles(
            "local tests cover the drawn statistic text-run contract",
            panel_test + "\n" + runtime_test,
            [
                "DM1_ChampionPanel_BuildStatisticTextRunModel(1, 51, 50, &run)",
                "run.maximumX != DM1_STATISTIC_CURRENT_REL_X + DM1_PANEL_TEXT_CHAR_WIDTH * 3",
                "run.y != DM1_STATISTIC_FIRST_REL_Y + DM1_PANEL_TEXT_LINE_HEIGHT",
                "champion stats render helper source-locks current value x",
                "champion stats render helper source-locks maximum suffix x",
                "champion stats render helper source-locks first statistic y",
            ],
        ) | {"citation": "tests/test_dm1_v1_champion_panel_hud_pc34_compat.c; tests/test_m11_inventory_full_panel_runtime_pc34_compat.c"},
        require_needles(
            "CMake registers pass621 gate",
            cmake,
            [
                "NAME pass621_dm1_v1_champion_stat_drawn_panel_pixel_gate",
                "verify_pass621_dm1_v1_champion_stat_drawn_panel_pixel_gate.py",
            ],
        ) | {"citation": "CMakeLists.txt"},
    ]


def build_manifest() -> dict[str, Any]:
    red = audit_redmcsb()
    great = audit_greatstone()
    original = audit_original()
    fire = audit_firestaff()
    ok = all(c["ok"] for c in red) and great["ok"] and original["ok"] and all(c["ok"] for c in fire)
    return {
        "schema": f"{PASS}.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS621_DM1_V1_CHAMPION_STAT_DRAWN_PANEL_PIXEL_GATE",
        "redmcsbRoot": str(RED),
        "greatstoneRoot": str(GREAT),
        "originalRoot": str(ORIG),
        "claim": "The champion statistics eye-panel draw contract is source-locked; original runtime pixel parity remains blocked.",
        "redmcsbChecks": red,
        "greatstoneCheck": great,
        "originalChecks": original["checks"],
        "firestaffChecks": fire,
        "remainingBlockers": [
            "A verified original PC 3.4 320x200 runtime frame with inventory open, empty leader hand, eye-panel champion statistics visible, and a documented input transcript/state snapshot.",
            "A matching Firestaff indexed framebuffer generated from the same champion, inventory, panel-content, and palette state.",
            "A crop/overlay comparator for the panel statistic text rows that separates palette-index deltas, glyph/text-run deltas, and panel background deltas.",
        ],
        "nonClaims": [
            "No original-vs-Firestaff pixel parity claim.",
            "No renderer behavior change.",
            "No original DOS runtime capture was launched by this gate.",
            "No push, package, tag, release, or external action.",
        ],
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# DM1 V1 champion statistic drawn-panel pixel gate",
        "",
        f"Status: `{manifest['statusToken']}`",
        "",
        "This gate locks the ReDMCSB draw contract for the inventory eye-panel champion statistic rows. It deliberately stops short of original-runtime pixel parity.",
        "",
        "## Source Evidence",
        "",
    ]
    for item in manifest["redmcsbChecks"]:
        result = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {result} `{item['id']}` - {item['citation']}")
    lines += [
        f"- {'PASS' if manifest['greatstoneCheck']['ok'] else 'FAIL'} `Greatstone PC 3.4 data index` - {manifest['greatstoneCheck']['citation']}",
        "- PASS `Original PC34 canonical GRAPHICS.DAT` - _canonical/dm1/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "- PASS `Original PC34 canonical DUNGEON.DAT` - _canonical/dm1/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "",
        "## Firestaff Join",
        "",
    ]
    for item in manifest["firestaffChecks"]:
        result = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {result} `{item['id']}` - {item['citation']}")
    lines += ["", "## Remaining Blockers", ""]
    for blocker in manifest["remainingBlockers"]:
        lines.append(f"- {blocker}")
    lines += ["", "## Non-Claims", ""]
    for non_claim in manifest["nonClaims"]:
        lines.append(f"- {non_claim}")
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true", help="verify without writing evidence files")
    args = parser.parse_args()

    manifest = build_manifest()
    if not args.check_only:
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        write_report(manifest)
    if manifest["status"] == "passed":
        print(f"PASS {PASS}: {manifest['statusToken']}")
        return 0
    print(f"FAIL {PASS}: {manifest['statusToken']}")
    for section in ["redmcsbChecks", "firestaffChecks"]:
        for item in manifest[section]:
            if not item["ok"]:
                print(f"- {item['id']}: missing {item.get('missing', [])}")
    if not manifest["greatstoneCheck"]["ok"]:
        print(f"- greatstone: missing {manifest['greatstoneCheck'].get('missing', [])}")
    for item in manifest["originalChecks"]:
        if not item["ok"]:
            print(f"- {item['id']}: expected {item.get('expected')} got {item.get('sha256')}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
