#!/usr/bin/env python3
"""Source-lock DM1 V1 Hall mirror finalization sensor-disable ordering.

This gate exists because REVIVE.C BUG0_87 is intentionally *not* "disable the
C127 portrait sensor". The original disables the first C03 sensor thing found on
the front mirror square, after skipping non-sensor things such as textstrings.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification/dm1_v1_hall_mirror_sensor_disable_source_lock.json"

SRC = {name: REDMCSB / name for name in ["REVIVE.C", "DUNGEON.C", "MOVESENS.C", "CLIKVIEW.C"]}
FIRESTAFF = {
    "include/dm1_v1_resurrection_pc34_compat.h": ROOT / "include/dm1_v1_resurrection_pc34_compat.h",
    "src/dm1/dm1_v1_resurrection_pc34_compat.c": ROOT / "src/dm1/dm1_v1_resurrection_pc34_compat.c",
    "tests/test_dm1_v1_resurrection_pc34_compat.c": ROOT / "tests/test_dm1_v1_resurrection_pc34_compat.c",
}


def block(path: Path, start: int, end: int) -> str:
    return "\n".join(path.read_text(encoding="latin-1").splitlines()[start - 1:end])


def require(citation: str, path: Path, start: int, end: int, needles: list[str], point: str) -> dict[str, Any]:
    text = block(path, start, end)
    compact = " ".join(text.split())
    missing = [n for n in needles if " ".join(n.split()) not in compact]
    if missing:
        raise SystemExit(f"{citation} missing expected text: {missing[0]}")
    return {"citation": citation, "source": str(path), "line_range": [start, end], "needles": needles, "point": point, "verified": True}


def require_text(path: Path, needles: list[str], point: str) -> dict[str, Any]:
    text = path.read_text(encoding="utf-8")
    missing = [n for n in needles if n not in text]
    if missing:
        raise SystemExit(f"{path} missing expected text: {missing[0]}")
    return {"source": str(path), "needles": needles, "point": point, "verified": True}


def main() -> int:
    for path in list(SRC.values()) + list(FIRESTAFF.values()):
        if not path.exists():
            raise SystemExit(f"missing input: {path}")

    source_checks = [
        require("REVIVE.C:785-799", SRC["REVIVE.C"], 785, 799, [
            "L0825_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0828_i_MapX, L0829_i_MapY)",
            "for (;;) { /*_Infinite loop_*/",
            "if (M012_TYPE(L0825_T_Thing) == C03_THING_TYPE_SENSOR)",
            "BUG0_87 The same champion may be resurrected/reincarnated multiple times",
            "does not check that this is the champion portrait sensor",
            "M044_SET_TYPE_DISABLED(L0827_ps_Sensor)",
            "break",
        ], "After final resurrect/reincarnate, the original scans the mirror square thing-list and disables the first C03 sensor thing, not specifically C127."),
        require("REVIVE.C:801-804", SRC["REVIVE.C"], 801, 804, [
            "L0825_T_Thing = F0159_DUNGEON_GetNextThing(L0825_T_Thing)",
        ], "If the current thing is not a sensor, REVIVE advances to the next thing-list entry."),
        require("DUNGEON.C:2568-2583", SRC["DUNGEON.C"], 2568, 2583, [
            "while ((L0314_T_Thing != C0xFFFE_THING_ENDOFLIST)",
            "L0312_i_ThingType = M012_TYPE(L0314_T_Thing)",
            "L0312_i_ThingType == C02_THING_TYPE_TEXTSTRING",
            "TEXTSTRING",
            "Visible",
        ], "Wall-square traversal includes C02 textstrings before C03 sensors; textstrings are not disabled by REVIVE's C03 sensor check."),
        require("DUNGEON.C:2608-2612", SRC["DUNGEON.C"], 2608, 2612, [
            "M039_TYPE(L0308_ps_Sensor) == C127_SENSOR_WALL_CHAMPION_PORTRAIT",
            "G0289_i_DungeonView_ChampionPortraitOrdinal = M000_INDEX_TO_ORDINAL(M040_DATA(L0308_ps_Sensor))",
        ], "Portrait rendering uses C127 sensor data, separate from finalization's first-sensor disable."),
        require("MOVESENS.C:1390-1395", SRC["MOVESENS.C"], 1390, 1395, [
            "M039_TYPE(L0755_ps_Sensor)) == C000_SENSOR_DISABLED",
            "C127_SENSOR_WALL_CHAMPION_PORTRAIT",
            "L0752_ui_Cell != P0587_ui_Cell",
        ], "Click-time wall sensor processing skips disabled sensors, permits C127 with no leader, and requires cell match."),
        require("MOVESENS.C:1501-1503", SRC["MOVESENS.C"], 1501, 1503, [
            "case C127_SENSOR_WALL_CHAMPION_PORTRAIT",
            "F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)",
        ], "The C127 click route creates the candidate; it does not define finalization disable order."),
        require("CLIKVIEW.C:21-25", SRC["CLIKVIEW.C"], 21, 25, [
            "L1135_ui_MapX = G0306_i_PartyMapX",
            "G0233_ai_Graphic559_DirectionToStepEastCount[G0308_i_PartyDirection]",
            "F0275_SENSOR_IsTriggeredByClickOnWall",
            "M018_OPPOSITE(G0308_i_PartyDirection)",
        ], "Viewport clicks target the front square/opposite wall cell that later REVIVE uses for finalization."),
    ]

    implementation_checks = [
        require_text(FIRESTAFF["include/dm1_v1_resurrection_pc34_compat.h"], [
            "MirrorThing_Compat",
            "F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat",
            "first C03 sensor thing",
            "not search for C127",
        ], "Firestaff exposes a bounded pure helper for the literal first-sensor disable semantics."),
        require_text(FIRESTAFF["src/dm1/dm1_v1_resurrection_pc34_compat.c"], [
            "if (things[i].thingType == DM1_THING_TYPE_SENSOR)",
            "out.disabledOldSensorType = things[i].sensorType",
            "out.disabledNewSensorType = DM1_SENSOR_DISABLED",
        ], "Implementation skips non-sensors and returns the first sensor's old type plus disabled result."),
        require_text(FIRESTAFF["tests/test_dm1_v1_resurrection_pc34_compat.c"], [
            "test_mirror_sensor_disable_order",
            "textstring before sensor is skipped",
            "first sensor type is disabled even when not C127",
            "C127 is disabled only when it is first sensor in thing-list order",
        ], "Regression tests pin the custom-dungeon sensor-order bug parity."),
    ]

    result = {
        "schema": "dm1_v1_hall_mirror_sensor_disable_source_lock.v1",
        "status": "PASS",
        "redmcsb_root": str(REDMCSB),
        "source_checks": source_checks,
        "implementation_checks": implementation_checks,
        "original_data_used": [],
        "original_data_hash_rule": "No original dungeon/graphics binary data was read by this gate; only ReDMCSB source and Firestaff source/tests were inspected.",
        "decision": "Literal parity should model REVIVE.C BUG0_87 as first C03 sensor thing disabled after non-sensors are skipped; original DM1 dungeons are unaffected because champion mirror squares have a single sensor, but custom-dungeon ordering is observable.",
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"PASS wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
