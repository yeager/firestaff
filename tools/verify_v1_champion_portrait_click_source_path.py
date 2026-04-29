#!/usr/bin/env python3
"""Pass164: source-lock the original champion portrait click route.

After pass163 proved party creation starts at C127/F0280, this locks the
missing input path in ReDMCSB: viewport left-click C080 -> CLIKVIEW front wall
ornament/portrait box -> F0372 -> F0275 wall sensor -> C127 -> F0280.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = REPO / "parity-evidence/verification/pass164_champion_portrait_click_source_path"

CHECKS = [
    {
        "id": "SRC_CLICK_001",
        "file": "COMMAND.C",
        "needles": [
            "{ C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT",
            "if (L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW)",
            "F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY)",
        ],
        "claim": "Left-clicks in source viewport zone C007 dispatch as command C080 into F0377.",
    },
    {
        "id": "SRC_CLICK_002",
        "file": "CLIKVIEW.C",
        "needles": [
            "void F0377_COMMAND_ProcessType80_ClickInDungeonView",
            "P0752_i_X -= G2067_i_ViewportScreenX",
            "P0753_i_Y -= G2068_i_ViewportScreenY",
        ],
        "claim": "F0377 converts screen coordinates to viewport-relative coordinates before testing dungeon-view boxes/zones.",
    },
    {
        "id": "SRC_CLICK_003",
        "file": "CLIKVIEW.C",
        "needles": [
            "for (AL1150_ui_ViewCell = C00_VIEW_CELL_FRONT_LEFT; AL1150_ui_ViewCell < C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT + 1; AL1150_ui_ViewCell++)",
            "if (AL1150_ui_ViewCell == C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT)",
            "if (!G0286_B_FacingAlcove)",
            "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()",
        ],
        "claim": "When leader hand is empty, clicking the front wall ornament/door-button view cell C05 and not facing an alcove calls F0372.",
    },
    {
        "id": "SRC_CLICK_004",
        "file": "CLIKVIEW.C",
        "needles": [
            "STATICFUNCTION void F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor",
            "L1135_ui_MapX = G0306_i_PartyMapX",
            "L1135_ui_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[G0308_i_PartyDirection]",
            "F0275_SENSOR_IsTriggeredByClickOnWall(L1135_ui_MapX, L1136_ui_MapY, M018_OPPOSITE(G0308_i_PartyDirection))",
        ],
        "claim": "F0372 targets the square in front of the party and the opposite wall cell — exactly the wall the party is looking at.",
    },
    {
        "id": "SRC_CLICK_005",
        "file": "MOVESENS.C",
        "needles": [
            "BOOLEAN F0275_SENSOR_IsTriggeredByClickOnWall",
            "if ((G0411_i_LeaderIndex == CM1_CHAMPION_NONE) && (L0757_ui_SensorType != C127_SENSOR_WALL_CHAMPION_PORTRAIT))",
            "if (L0752_ui_Cell != P0587_ui_Cell)",
            "case C127_SENSOR_WALL_CHAMPION_PORTRAIT:",
            "F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)",
        ],
        "claim": "F0275 explicitly allows C127 even with no leader, requires the clicked wall cell, then calls F0280 with sensor data.",
    },
    {
        "id": "SRC_CLICK_006",
        "file": "DUNGEON.C",
        "needles": [
            "if (M039_TYPE(L0308_ps_Sensor) == C127_SENSOR_WALL_CHAMPION_PORTRAIT)",
            "G0289_i_DungeonView_ChampionPortraitOrdinal = M000_INDEX_TO_ORDINAL(M040_DATA(L0308_ps_Sensor))",
        ],
        "claim": "The visible champion portrait ordinal is sourced from the same C127 sensor data later used by F0280.",
    },
]


def hits(path: Path, needles: list[str]) -> list[dict[str, object]]:
    lines = path.read_text(errors="replace").splitlines()
    out = []
    for n in needles:
        for i, line in enumerate(lines, 1):
            if n in line:
                out.append({"needle": n, "line": i, "text": line.strip()})
                break
        else:
            raise AssertionError(f"missing {n!r} in {path}")
    return out


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    rows = []
    for c in CHECKS:
        rows.append({**c, "source": str(SRC / c["file"]), "hits": hits(SRC / c["file"], c["needles"]), "status": "PASS"})
    result = {"schema": "pass164_champion_portrait_click_source_path.v1", "checks": rows}
    (OUT / "source_path.json").write_text(json.dumps(result, indent=2) + "\n")
    md = [
        "# Pass 164 — champion portrait click source path",
        "",
        "This locks the ReDMCSB answer for the route blocker: to recruit a champion, the original input must click the visible champion portrait/wall ornament in the viewport, not jump straight to C160/C161 panel buttons.",
        "",
        "## Source route",
        "",
        "`COMMAND.C` C007 left-click → `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` → `CLIKVIEW.C:F0377` → C05 front wall ornament/door-button hit → `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor` → `MOVESENS.C:F0275_SENSOR_IsTriggeredByClickOnWall(front square, opposite party direction)` → `C127_SENSOR_WALL_CHAMPION_PORTRAIT` → `REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty`.",
        "",
        "## Checks",
        "",
    ]
    for r in rows:
        locs = ", ".join(str(h["line"]) for h in r["hits"])
        md.append(f"- {r['id']} PASS — `{r['file']}` lines {locs}: {r['claim']}")
    md += [
        "",
        "## Route implication",
        "",
        "Pass162 corrected C160/C161 panel coordinates but still collapsed to `48ed3743ab6a`. Pass163 proved F0280 must come first. Pass164 proves the missing original action is a source-faithful viewport click on the champion portrait/front-wall ornament cell that triggers C127/F0275/F0280. Next runtime pass should therefore drive the party to face a mirror square, click the portrait/ornament cell inside C05, verify candidate-state pixels, then click C160/C161.",
    ]
    (OUT / "README.md").write_text("\n".join(md) + "\n")
    print(f"PASS wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
