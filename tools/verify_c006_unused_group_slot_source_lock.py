#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

checks = {
    "src/memory/memory_tick_orchestrator_pc34_compat.c": [
        "orch_find_unused_group_slot_compat",
        "ReDMCSB DUNGEON.C:F0166:2077-2137",
        "GROUP.C:F0185:512-521",
        "reuse an unused source slot instead of growing the group array",
    ],
    "tests/test_m10_c006_generator_reenable_dispatch_pc34_compat.c": [
        "C006 generator keeps fixed source group-slot capacity",
        "C006 generator reuses first Next=THING_NONE group slot",
        "C006 generator does not append beyond source group-slot capacity",
        "C006 no-slot path keeps only delayed C65 re-enable",
    ],
}

source_checks = {
    "GROUP.C": [
        "THING F0185_GROUP_GetGenerated",
        "F0166_DUNGEON_GetUnusedThing(C04_THING_TYPE_GROUP)",
        "L0353_ps_Group->Slot = C0xFFFE_THING_ENDOFLIST;",
        "F0267_MOVE_GetMoveResult_CPSCE(L0349_T_GroupThing",
    ],
    "DUNGEON.C": [
        "THING F0166_DUNGEON_GetUnusedThing",
        "L0288_i_ThingIndex = L0290_i_ThingCount;",
        "if (L0291_ps_Generic->Next == C0xFFFF_THING_NONE)",
        "L0292_T_Thing = (P0296_ui_ThingType << 10) | (L0290_i_ThingCount - L0288_i_ThingIndex);",
        "L0291_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;",
    ],
    "TIMELINE.C": [
        "if (M039_TYPE(L0614_ps_Sensor) == C006_SENSOR_FLOOR_GROUP_GENERATOR)",
        "F0185_GROUP_GetGenerated(M040_DATA(L0614_ps_Sensor)",
        "if (L0614_ps_Sensor->Remote.Audible)",
        "M044_SET_TYPE_DISABLED(L0614_ps_Sensor);",
        "C65_EVENT_ENABLE_GROUP_GENERATOR",
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

print("C006_UNUSED_GROUP_SLOT_SOURCE_LOCK_OK")
