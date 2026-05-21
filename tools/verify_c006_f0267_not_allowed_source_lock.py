#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

checks = {
    "include/memory_dungeon_dat_pc34_compat.h": [
        "unsigned char  allowedCreatureTypes[16];",
    ],
    "src/memory/memory_dungeon_dat_pc34_compat.c": [
        "bytes used by DUNGEON.C:F0139_DUNGEON_IsCreatureAllowedOnMap",
        "fread(m->allowedCreatureTypes, 1, m->creatureTypeCount, file)",
    ],
    "src/memory/memory_tick_orchestrator_pc34_compat.c": [
        "orch_is_group_creature_allowed_on_map_compat",
        "ReDMCSB DUNGEON.C:F0139:1050-1079 reads the group creature type",
        "MOVESENS.C:F0267:656-663",
        "CM1_MAPX_NOT_ON_A_SQUARE",
        "!orch_is_group_creature_allowed_on_map_compat(world, group, destMapIndex)",
        "!orch_is_group_creature_allowed_on_map_compat(world, group, retry.mapIndex)",
        "orch_drop_group_slot_possessions_compat",
    ],
    "tests/test_m10_c006_generator_reenable_dispatch_pc34_compat.c": [
        "disallowed C006 group is not linked onto target square",
        "disallowed C006 keeps only C65 re-enable event",
        "event60 disallowed group drops carried slot on destination square",
        "event60 disallowed group does not insert or schedule retry",
    ],
}

source_checks = {
    "DUNGEON.C": [
        "BOOLEAN F0139_DUNGEON_IsCreatureAllowedOnMap",
        "L0235_ui_CreatureType = ((GROUP*)AL0236_puc_Group)->Type;",
        "AL0236_puc_AllowedCreatureType = G0279_pppuc_DungeonMapData[P0235_ui_MapIndex][L0237_ps_Map->A.Width] + L0237_ps_Map->A.Height + 1;",
        "for (L0234_i_Counter = L0237_ps_Map->C.CreatureTypeCount; L0234_i_Counter > 0; L0234_i_Counter--)",
    ],
    "MOVESENS.C": [
        "F0187_GROUP_DropMovingCreatureFixedPossessions(P0557_T_Thing, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        "F0188_GROUP_DropGroupPossessions(P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing, C02_MODE_PLAY_ONE_TICK_LATER);",
        "if (P0558_i_SourceMapX >= 0)",
        "return C1_TRUE; /* The specified group thing cannot be moved because it was killed by a fall or because it is not allowed on the destination map */",
    ],
    "GROUP.C": [
        "F0267_MOVE_GetMoveResult_CPSCE(L0349_T_GroupThing, CM1_MAPX_NOT_ON_A_SQUARE, 0, P0353_i_MapX, P0354_i_MapY)",
    ],
    "TIMELINE.C": [
        "F0267_MOVE_GetMoveResult_CPSCE(P0529_ps_Event->C.Slot, CM1_MAPX_NOT_ON_A_SQUARE, 0, L0656_ui_MapX, L0657_ui_MapY);",
    ],
}

missing = []
if not RED.exists():
    missing.append(f"missing ReDMCSB source root {RED}")

for rel, needles in checks.items():
    text = (ROOT / rel).read_text()
    for needle in needles:
        if needle not in text:
            missing.append(f"{rel}: missing {needle!r}")

for rel, needles in source_checks.items():
    text = (RED / rel).read_text(encoding="latin-1", errors="replace")
    for needle in needles:
        if needle not in text:
            missing.append(f"ReDMCSB {rel}: missing {needle!r}")

if missing:
    raise SystemExit("\n".join(missing))

print("C006_F0267_NOT_ALLOWED_SOURCE_LOCK_OK")
