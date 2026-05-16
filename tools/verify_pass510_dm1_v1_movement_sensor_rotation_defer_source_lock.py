#!/usr/bin/env python3
"""Pass510: source-lock DM1 V1 movement sensor local-rotation deferral."""
from __future__ import annotations

import hashlib
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass510_dm1_v1_movement_sensor_rotation_defer_source_lock"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path, encoding: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing/out-of-order {needle!r}")
        pos = hit


def function_body(text: str, name: str) -> tuple[int, int, str]:
    pattern = rf"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+{re.escape(name)}\s*\("
    m = re.search(pattern, text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, i) + 1, text[m.start() : i + 1]
    next_m = re.search(r"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+F\d+_[A-Za-z0-9_]+\s*\(", text[m.start() + 1 :], re.M)
    if next_m:
        end = m.start() + 1 + next_m.start()
        return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, end), text[m.start() : end]
    raise AssertionError(f"unterminated function {name}")


def git_out(*args: str) -> str:
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def source_audit() -> dict[str, list[int]]:
    movesens = read(RED / "MOVESENS.C")
    f0267_s, f0267_e, f0267 = function_body(movesens, "F0267_MOVE_GetMoveResult_CPSCE")
    f0270_s, f0270_e, f0270 = function_body(movesens, "F0270_SENSOR_TriggerLocalEffect")
    f0271_s, f0271_e, f0271 = function_body(movesens, "F0271_SENSOR_ProcessRotationEffect")
    f0272_s, f0272_e, f0272 = function_body(movesens, "F0272_SENSOR_TriggerEffect")
    f0275_s, f0275_e, f0275 = function_body(movesens, "F0275_SENSOR_IsTriggeredByClickOnWall")
    f0276_s, f0276_e, f0276 = function_body(movesens, "F0276_SENSOR_ProcessThingAdditionOrRemoval")

    require_order(f0267, [
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);",
    ], "F0267 movement calls source leave before destination enter sensor side effects")

    require_order(f0270, [
        "if (P0571_i_SensorLocalEffect == C10_EFFECT_ADD_300XP_STEAL_SKILL)",
        "G0403_i_SensorRotationEffect = P0571_i_SensorLocalEffect;",
        "G0404_i_SensorRotationEffectMapX = P0572_i_SensorLocalEffectX;",
        "G0405_i_SensorRotationEffectMapY = P0573_i_SensorLocalEffectY;",
        "G0406_i_SensorRotationEffectCell = P0574_i_SensorLocalEffectCell;",
    ], "F0270 stores the latest non-XP local rotation request in globals")

    require_order(f0271, [
        "if (G0403_i_SensorRotationEffect == CM1_EFFECT_NONE)",
        "case C01_EFFECT_CLEAR:",
        "case C02_EFFECT_TOGGLE:",
        "L0732_T_FirstSensorThing = F0161_DUNGEON_GetSquareFirstThing",
        "F0164_DUNGEON_UnlinkThingFromList(L0732_T_FirstSensorThing",
        "L0735_ps_LastSensor->Remote.Next = L0732_T_FirstSensorThing;",
        "G0403_i_SensorRotationEffect = CM1_EFFECT_NONE;",
    ], "F0271 performs one deferred sensor-list rotation and clears the pending global")

    require_order(f0272, [
        "if (P0575_ps_Sensor->Remote.OnceOnly)",
        "L0738_l_Time = G0313_ul_GameTime + P0575_ps_Sensor->Remote.Value;",
        "if (P0575_ps_Sensor->Remote.LocalEffect)",
        "F0270_SENSOR_TriggerLocalEffect(M049_LOCAL_EFFECT(P0575_ps_Sensor), P0577_i_MapX, P0578_i_MapY, P0579_i_Cell);",
        "F0268_SENSOR_AddEvent",
    ], "F0272 dispatches local effects immediately to F0270 instead of queueing remote events")

    require_order(f0276, [
        "L0766_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0588_ui_MapX, P0589_ui_MapY);",
        "while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST)",
        "F0272_SENSOR_TriggerEffect(L0769_ps_Sensor, L0778_i_Effect, P0588_ui_MapX, P0589_ui_MapY, CM1_CELL_ANY);",
        "T0276079:",
        "L0766_T_Thing = F0159_DUNGEON_GetNextThing(L0766_T_Thing);",
        "F0271_SENSOR_ProcessRotationEffect();",
    ], "F0276 defers rotation until after the whole floor-sensor iteration")

    require_order(f0275, [
        "F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell);",
        "F0272_SENSOR_TriggerEffect(L0755_ps_Sensor, L0756_i_SensorEffect, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell);",
        "F0271_SENSOR_ProcessRotationEffect();",
    ], "F0275 wall-click path also defers local rotations until after sensor iteration")

    return {
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": [f0267_s, f0267_e],
        "MOVESENS.C:F0270_SENSOR_TriggerLocalEffect": [f0270_s, f0270_e],
        "MOVESENS.C:F0271_SENSOR_ProcessRotationEffect": [f0271_s, f0271_e],
        "MOVESENS.C:F0272_SENSOR_TriggerEffect": [f0272_s, f0272_e],
        "MOVESENS.C:F0275_SENSOR_IsTriggeredByClickOnWall": [f0275_s, f0275_e],
        "MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval": [f0276_s, f0276_e],
    }


def firestaff_audit() -> dict[str, str]:
    trigger_h = read(ROOT / "include/dm1_v1_sensor_trigger_pc34_compat.h", "utf-8")
    trigger_c = read(ROOT / "src/dm1/dm1_v1_sensor_trigger_pc34_compat.c", "utf-8")
    sensor_c = read(ROOT / "src/memory/memory_sensor_execution_pc34_compat.c", "utf-8")
    movement_core = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c", "utf-8")
    trigger_test = read(ROOT / "tests/test_dm1_v1_sensor_trigger_pc34_compat.c", "utf-8")
    timing_test = read(ROOT / "tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "utf-8")
    cmake = read(ROOT / "CMakeLists.txt", "utf-8")

    for needle in [
        "int rotationPending;",
        "int rotationEffect;",
        "int rotationMapX;",
        "int rotationMapY;",
        "int rotationCell;",
    ]:
        require(trigger_h, needle, f"sensor trigger result carries {needle}")

    require_order(trigger_c, [
        "outList->rotationPending = 0;",
        "outList->rotationEffect = DM1_EFFECT_NONE;",
        "if (result.isLocal && result.effectKind == SENSOR_EFFECT_ROTATION)",
        "outList->rotationPending = 1;",
        "outList->rotationEffect = result.localEffectValue;",
        "outList->results[outList->count++] = result;",
    ], "F0725 records local floor rotations as deferred list metadata")

    require_order(trigger_c, [
        "remainingByCell[cell]++;",
        "remainingByCell[localCtx.cell]--;",
        "localCtx.sensorCountInCell = remainingByCell[localCtx.cell];",
        "if (result.isLocal && result.effectKind == SENSOR_EFFECT_ROTATION)",
        "outList->rotationPending = 1;",
        "if (sensorData[sIdx].sensorType == DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE",
        "outList->rotationEffect = DM1_EFFECT_TOGGLE;",
        "outList->results[outList->count++] = result;",
    ], "F0726 records wall-click rotations after per-cell last-sensor gating")

    require_order(sensor_c, [
        "sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat",
        "for (i = 0; i < sensorCount && i < SENSOR_ENUM_CAPACITY; ++i)",
        "F0710_SENSOR_Execute_Compat",
        "outList->effects[outList->count++] = tmp.effects[j];",
    ], "party enter/leave sensor execution walks sensors in source order")

    require_order(movement_core, [
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat(",
        "party->mapIndex = outResult->movement.newMapIndex;",
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat(",
        "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
    ], "movement core keeps sensor side effects on the success-only path before timing")

    for needle in [
        "Local: rotation effect",
        "Floor square: 2 of 3 sensors triggered",
        "Wall click: only 1 of 2 sensors triggered",
        "dm1_v1_sensor_trigger_pc34_compat",
        "dm1_v1_command_movement_sensor_timing_pc34_compat",
    ]:
        require(trigger_test + "\n" + timing_test + "\n" + cmake, needle, f"focused local coverage {needle}")

    return {
        "sensorTrigger": "F0725/F0726 expose deferred rotationPending metadata instead of mutating sensor lists in-place",
        "movementCore": "successful steps call party leave/enter sensors before successful-step timing; blocked steps return before this path",
        "focusedCTests": "dm1_v1_sensor_trigger_pc34_compat and dm1_v1_command_movement_sensor_timing_pc34_compat",
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "PASS510_DM1_V1_MOVEMENT_SENSOR_ROTATION_DEFER_SOURCE_LOCKED",
        "branch": git_out("branch", "--show-current"),
        "head": git_out("rev-parse", "HEAD"),
        "redmcsbRoot": str(RED),
        "redmcsbSha256": {
            "MOVESENS.C": sha256(RED / "MOVESENS.C"),
        },
        "sourceAudit": source,
        "firestaffAudit": firestaff_audit(),
        "closedGap": "movement-triggered local sensor rotation is deferred: F0270 records the latest local rotation request, F0276/F0275 finish the whole sensor iteration, then F0271 rotates the sensor list once and clears the pending global",
        "notClaimed": [
            "new runtime sensor-list mutation implementation",
            "original DOS breakpoint trace",
            "unsupported sensor effect materialization beyond the existing source-locked trigger metadata",
        ],
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass510 - DM1 V1 movement sensor rotation defer source lock",
        "",
        f"Status: {manifest['status']}",
        "",
        "## ReDMCSB Evidence",
    ]
    for name, span in source.items():
        lines.append(f"- {name}: lines {span[0]}-{span[1]}")
    lines += [
        "",
        "## Closed Gap",
        manifest["closedGap"],
        "",
        "## Local Gate",
        "- CTest: pass510_dm1_v1_movement_sensor_rotation_defer_source_lock",
        "- Focused tests audited: dm1_v1_sensor_trigger_pc34_compat, dm1_v1_command_movement_sensor_timing_pc34_compat",
        "",
        "## Not Claimed",
    ]
    lines.extend(f"- {item}" for item in manifest["notClaimed"])
    lines.append("")
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(manifest["status"])
    print(f"manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
