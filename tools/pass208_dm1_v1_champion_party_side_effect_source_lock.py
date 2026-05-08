#!/usr/bin/env python3
"""Pass 208 source-lock gate for DM1 V1 champion/party movement side effects.

This gate is intentionally read-only.  It ties Firestaff's current pure movement
helpers to the original ReDMCSB movement side-effect contract and records the
remaining blocker: the compat champion state has no per-champion Cell field, so
F0284_CHAMPION_SetPartyDirection's Cell rotation cannot yet be represented by
existing helpers without a schema change.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REDMCSB = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", DEFAULT_REDMCSB))

CHECKS = [
    {
        "id": "champion_set_party_direction_rotates_champion_cell_direction_and_party_direction",
        "file": "CHAMPION.C",
        "lines": "118-130",
        "needles": [
            "if (P0600_i_Direction == G0308_i_PartyDirection)",
            "L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection",
            "L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta);",
            "L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta);",
            "G0308_i_PartyDirection = P0600_i_Direction;",
            "F0296_CHAMPION_DrawChangedObjectIcons();",
        ],
    },
    {
        "id": "command_tables_bind_movement_commands_to_mouse_and_keys",
        "file": "COMMAND.C",
        "lines": "106-113,252-260",
        "needles": [
            "{ C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145, MASK0x0002_MOUSE_LEFT_BUTTON }",
            "{ C006_COMMAND_MOVE_LEFT,             234, 261, 147, 167, MASK0x0002_MOUSE_LEFT_BUTTON }",
            "{ C004_COMMAND_MOVE_RIGHT,            291, 318, 147, 167, MASK0x0002_MOUSE_LEFT_BUTTON }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x000B }",
            "{ C006_COMMAND_MOVE_LEFT,     0x0008 }",
            "{ C004_COMMAND_MOVE_RIGHT,    0x0015 }",
        ],
    },
    {
        "id": "dungeon_direction_step_arrays_and_active_group_accessors",
        "file": "DUNGEON.C",
        "lines": "35-42,1264-1327",
        "needles": [
            "int16_t G0233_ai_Graphic559_DirectionToStepEastCount[4]",
            "0,    /* North */",
            "1,    /* East */",
            "-1 }; /* South */",
            "int16_t G0234_ai_Graphic559_DirectionToStepNorthCount[4]",
            "if (P0243_ui_MapIndex == G0309_i_PartyMapIndex)",
            "G0375_ps_ActiveGroups[P0244_ps_Group->ActiveGroupIndex].Cells = P0245_ui_Cells;",
            "return G0258_auc_Graphic559_GroupDirections[P0247_ps_Group->Direction];",
            "P0249_ps_Group->Direction = M021_NORMALIZE(P0250_ui_Directions);",
        ],
    },
    {
        "id": "movesens_projectile_intermediary_party_cells",
        "file": "MOVESENS.C",
        "lines": "232-313",
        "needles": [
            "L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_Cell] = M000_INDEX_TO_ORDINAL(AL0699_ui_Cell);",
            "AL0699_ui_PrimaryDirection = F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource",
            "L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[M019_PREVIOUS(AL0699_ui_PrimaryDirection)]",
            "F0217_PROJECTILE_HasImpactOccured(L0702_i_ImpactType",
            "F0007_MAIN_CopyBytes(M772_CAST_PC(L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell)",
        ],
    },
    {
        "id": "movesens_party_position_rotation_scent_sensor_handoff",
        "file": "MOVESENS.C",
        "lines": "441-451,493-518,738-821,892-898",
        "needles": [
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "L0716_ui_Direction = G0308_i_PartyDirection;",
            "F0284_CHAMPION_SetPartyDirection(L0712_ps_Teleporter->Rotation);",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + L0712_ps_Teleporter->Rotation));",
            "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
            "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;",
            "G0401_ui_MoveResultCell = M011_CELL(P0557_T_Thing);",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);",
            "G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing",
        ],
    },
]

LOCAL_CHECKS = [
    {
        "id": "movement_timing_records_square_change_scent_and_cooldown",
        "path": "dm1_v1_movement_timing_pc34_compat.c",
        "needles": [
            "MOVESENS.C:752-775 updates G0362_l_LastPartyMovementTime and scent timing",
            "result.disabledMovementTicks = DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat",
            "result.projectileDisabledMovementTicks = 0;",
            "result.scentDelayTicks = (int)(currentGameTick - previousLastPartyMovementTime);",
            "result.lastPartyMovementTime = currentGameTick;",
        ],
    },
    {
        "id": "movement_core_keeps_pure_step_direction_helpers",
        "path": "memory_movement_pc34_compat.c",
        "needles": [
            "static const int s_dx[4] = {  0,  1,  0, -1 };",
            "static const int s_dy[4] = { -1,  0,  1,  0 };",
            "case MOVE_RIGHT:    stepDir = (direction + 1) & 3; break;",
            "case MOVE_BACKWARD: stepDir = (direction + 2) & 3; break;",
            "case MOVE_LEFT:     stepDir = (direction + 3) & 3; break;",
        ],
    },
]


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return ""


def run() -> tuple[int, dict]:
    failures: list[str] = []
    redmcsb_results = []
    for check in CHECKS:
        path = REDMCSB / check["file"]
        text = read(path)
        missing = [needle for needle in check["needles"] if needle not in text]
        redmcsb_results.append({
            "id": check["id"],
            "file": str(path),
            "lines": check["lines"],
            "ok": not missing,
            "missing": missing,
        })
        if missing:
            failures.append(f"{check['id']} missing {len(missing)} source snippet(s)")

    local_results = []
    for check in LOCAL_CHECKS:
        path = ROOT / check["path"]
        text = read(path)
        missing = [needle for needle in check["needles"] if needle not in text]
        local_results.append({
            "id": check["id"],
            "file": str(path),
            "ok": not missing,
            "missing": missing,
        })
        if missing:
            failures.append(f"{check['id']} missing {len(missing)} local snippet(s)")

    champ_header = read(ROOT / "memory_champion_state_pc34_compat.h")
    champion_struct = champ_header.split("struct ChampionState_Compat", 1)[1].split("};", 1)[0] if "struct ChampionState_Compat" in champ_header else ""
    has_champion_cell = any(token in champion_struct for token in [" cell;", " Cell;", "championCell", "partyCell"])
    has_champion_direction = "unsigned char  direction;" in champion_struct

    result = {
        "pass": 208,
        "name": "dm1_v1_champion_party_side_effect_source_lock",
        "ok": not failures,
        "redmcsbSourceRoot": str(REDMCSB),
        "redmcsbChecks": redmcsb_results,
        "localChecks": local_results,
        "blockers": [
            {
                "id": "champion_cell_side_effect_schema_gap",
                "status": "BLOCKED" if not has_champion_cell else "CLEAR",
                "detail": "ReDMCSB F0284 rotates each champion Cell and Direction on party direction changes, but Firestaff ChampionState_Compat currently exposes direction without a cell/party-cell field. Full SetPartyDirection parity needs a schema/API owner before implementation.",
                "hasChampionDirectionField": has_champion_direction,
                "hasChampionCellField": has_champion_cell,
            }
        ],
        "failures": failures,
    }
    return (0 if not failures else 1), result


if __name__ == "__main__":
    code, payload = run()
    print(json.dumps(payload, indent=2, sort_keys=True))
    sys.exit(code)
