#!/usr/bin/env python3
"""Verify DM1/V1 champion HUD/status lane source lock against ReDMCSB.

This is deliberately a source/probe gate, not an original-runtime screenshot claim.
It locks the top-row status-box zones and draw order used by Firestaff M11 to the
N2-local ReDMCSB source tree plus the recovered layout-696 zone table.
"""
from __future__ import annotations

import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ZONES = ROOT / "zones_h_reconstruction.json"
M11 = ROOT / "m11_game_view.c"
OUT = ROOT / "parity-evidence/verification/v1_hud_status_source_lock.json"

SOURCE_RANGES = [
    {"file": "DEFS.H", "start": 3779, "end": 3803, "locks": "top-row status/icon/name/bar/hand zone identifiers"},
    {"file": "CHAMDRAW.C", "start": 307, "end": 346, "locks": "F0287 type-7 bar zones C195/C199/C203 per champion"},
    {"file": "CHAMDRAW.C", "start": 771, "end": 815, "locks": "F0292 live status-box fill, shield borders, and dirty-bit scheduling"},
    {"file": "CHAMDRAW.C", "start": 833, "end": 905, "locks": "dead/name path, live name clear/text, statistics -> F0287 order"},
    {"file": "CHAMDRAW.C", "start": 1038, "end": 1052, "locks": "top-right champion icon clear/blit zones C113..C116"},
    {"file": "CHAMDRAW.C", "start": 1117, "end": 1139, "locks": "F0293 walks party champions and calls F0292"},
    {"file": "TIMELINE.C", "start": 1817, "end": 1830, "locks": "F0260 marks MASK0x1000_STATUS_BOX then refreshes all champions"},
]

ZONE_EXPECTED = {
    150: {"type": 9, "parent": 0, "d1": 67, "d2": 29},
    **{151 + i: {"type": 1, "parent": 150, "d1": i * 69, "d2": 0} for i in range(4)},
    **{155 + i: {"type": 9, "parent": 151 + i, "d1": 43, "d2": 7} for i in range(4)},
    **{159 + i: {"type": 1, "parent": 155 + i, "d1": 0, "d2": 0} for i in range(4)},
    **{163 + i: {"type": 18, "parent": 159 + i, "d1": 1, "d2": 0} for i in range(4)},
    **{183 + i: {"type": 9, "parent": 151 + i, "d1": 24, "d2": 29} for i in range(4)},
    **{187 + i: {"type": 1, "parent": 183 + i, "d1": 43, "d2": 0} for i in range(4)},
    **{191 + i: {"type": 9, "parent": 187 + i, "d1": 4, "d2": 25} for i in range(4)},
    **{195 + i: {"type": 7, "parent": 191 + i, "d1": 5, "d2": 26} for i in range(4)},
    **{199 + i: {"type": 7, "parent": 191 + i, "d1": 12, "d2": 26} for i in range(4)},
    **{203 + i: {"type": 7, "parent": 191 + i, "d1": 19, "d2": 26} for i in range(4)},
    **{207 + i: {"type": 9, "parent": 151 + i, "d1": 16, "d2": 16} for i in range(4)},
    **{211 + i * 2: {"type": 1, "parent": 207 + i, "d1": 4, "d2": 10} for i in range(4)},
    **{212 + i * 2: {"type": 1, "parent": 207 + i, "d1": 24, "d2": 10} for i in range(4)},
}

SOURCE_NEEDLES = {
    ("DEFS.H", 3779, 3803): [
        "C113_ZONE_CHAMPION_ICON_TOP_LEFT", "C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS",
        "C159_ZONE_CHAMPION_0_STATUS_BOX_NAME", "C163_ZONE_FIRST_CHAMPION_NAME",
        "C195_ZONE_FIRST_BAR_GRAPH", "C211_ZONE_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND",
    ],
    ("CHAMDRAW.C", 307, 346): [
        "L2252_i_BoxIndex = P0605_i_ChampionIndex + C195_ZONE_FIRST_BAR_GRAPH",
        "F0638_GetZone(L2252_i_BoxIndex, L2005_ai_XYZColoredBar)",
        "M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar)",
        "F0732_FillScreenArea(L2005_ai_XYZColoredBar, G0046_auc_Graphic562_ChampionColor[P0605_i_ChampionIndex])",
    ],
    ("CHAMDRAW.C", 771, 815): [
        "F0638_GetZone(P0615_ui_ChampionIndex + C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS, L2260_ai_XYZ)",
        "F0732_FillScreenArea(L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY)",
        "F0659_(L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount], L2260_ai_XYZ, C10_COLOR_FLESH)",
        "MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x2000_WOUNDS | MASK0x8000_ACTION_HAND",
    ],
    ("CHAMDRAW.C", 833, 905): [
        "F0650_PrintCenteredTextToScreenZone(P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME",
        "F0733_FillZoneByIndex(P0615_ui_ChampionIndex + C159_ZONE_CHAMPION_0_STATUS_BOX_NAME, C01_COLOR_DARK_GRAY)",
        "F0287_CHAMPION_DrawBarGraphs(P0615_ui_ChampionIndex)",
    ],
    ("CHAMDRAW.C", 1038, 1052): [
        "F0622_PrepareChampionIconBitmap((unsigned char*)L0866_pc_ChampionName, P0615_ui_ChampionIndex)",
        "F0621_ClearChampionIconBox(AL0864_i_ChampionIconIndex)",
        "C113_ZONE_CHAMPION_ICON_TOP_LEFT",
    ],
    ("CHAMDRAW.C", 1117, 1139): [
        "M516_CHAMPIONS[L0873_ui_ChampionIndex].Attributes |= P2062_ui_",
        "F0292_CHAMPION_DrawState(L0873_ui_ChampionIndex)",
    ],
    ("TIMELINE.C", 1817, 1830): [
        "M008_SET(M516_CHAMPIONS[L0679_ui_ChampionIndex].Attributes, MASK0x1000_STATUS_BOX)",
        "F0293_CHAMPION_DrawAllChampionStates()",
    ],
}

M11_NEEDLES = [
    "M11_V1_PARTY_SLOT_W    = 67",
    "M11_V1_PARTY_SLOT_STEP = 69",
    "M11_V1_BAR_GRAPH_REGION_X = 43",
    "M11_V1_BAR_CONTAINER_W   = 4",
    "M11_V1_BAR_CONTAINER_H   = 25",
    "M11_V1_BAR_HP_CX         = 5",
    "M11_V1_BAR_STAMINA_CX    = 12",
    "M11_V1_BAR_MANA_CX       = 19",
    "M11_V1_STATUS_READY_HAND_X = 4",
    "M11_V1_STATUS_ACTION_HAND_X = 24",
    "M11_V1_STATUS_HAND_Y = 10",
    "M11_V1_STATUS_NAME_CLEAR_W = 43",
    "M11_V1_STATUS_NAME_TEXT_X = 1",
    "M11_GameView_GetV1StatusBoxZoneId",
    "M11_GameView_GetV1StatusBarValueZoneId",
    "M11_GameView_GetV1StatusHandZoneId",
    "M11_GameView_GetV1StatusNameTextZoneId",
    "V1 source status-box background",
    "GRAPHICS.DAT-backed shield border overlays",
    "V1 champion name/title status text",
    "Pass 43: champion HP/stamina/mana bar graphs",
    "V1 status-box hand slots",
]


def read_lines(path: Path) -> list[str]:
    if not path.is_file():
        raise AssertionError(f"missing file: {path}")
    enc = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=enc, errors="replace").splitlines()


def excerpt(path: Path, start: int, end: int) -> str:
    lines = read_lines(path)
    if len(lines) < end:
        raise AssertionError(f"{path} has {len(lines)} lines, expected at least {end}")
    return "\n".join(lines[start - 1:end])


def main() -> int:
    problems: list[str] = []
    source_locks = []
    for spec in SOURCE_RANGES:
        rel = spec["file"]
        start = int(spec["start"])
        end = int(spec["end"])
        text = excerpt(REDMCSB / rel, start, end)
        missing = [n for n in SOURCE_NEEDLES.get((rel, start, end), []) if n not in text]
        if missing:
            problems.append(f"{rel}:{start}-{end} missing {missing}")
        source_locks.append({**spec, "path": str((REDMCSB / rel)), "missing": missing})

    records = json.loads(ZONES.read_text(encoding="utf-8"))["records"]
    checked_zones = {}
    for zone, expected in sorted(ZONE_EXPECTED.items()):
        got_raw = records.get(str(zone))
        if not got_raw:
            problems.append(f"missing zone C{zone}")
            continue
        got = {k: int(got_raw[k]) for k in ("type", "parent", "d1", "d2")}
        checked_zones[f"C{zone}"] = got
        if got != expected:
            problems.append(f"C{zone} {got} != {expected}")

    m11 = M11.read_text(encoding="utf-8", errors="replace")
    m11_missing = [needle for needle in M11_NEEDLES if needle not in m11]
    if m11_missing:
        problems.append(f"m11_game_view.c missing markers: {m11_missing}")

    result = {
        "schema": "v1_hud_status_source_lock.v1",
        "pass": not problems,
        "scope": "DM1 PC 3.4 V1 champion top-row/status panel source-lock gate; no original-runtime screenshot claim.",
        "redmcsbSourceRoot": str(REDMCSB),
        "sourceRanges": source_locks,
        "zoneSource": str(ZONES.relative_to(ROOT)),
        "checkedZones": checked_zones,
        "firestaffSource": str(M11.relative_to(ROOT)),
        "firestaffMarkersChecked": M11_NEEDLES,
        "problems": problems,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"probe=v1_hud_status_source_lock")
    print(f"evidence={OUT.relative_to(ROOT)}")
    print(f"v1HudStatusSourceLockOk={1 if result["pass"] else 0}")
    if problems:
        for problem in problems:
            print(f"FAIL {problem}", file=sys.stderr)
        return 1
    for spec in source_locks:
        print(f"sourceRange={spec["file"]}:{spec["start"]}-{spec["end"]} lock={spec["locks"]}")
    print(f"zonesChecked={len(checked_zones)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
