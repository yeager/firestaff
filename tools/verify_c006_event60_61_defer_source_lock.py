#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM = Path.home() / ".openclaw/data/firestaff-original-games/DM"

checks = {
    'include/memory_timeline_pc34_compat.h': [
        'TIMELINE_EVENT_MOVE_GROUP_SILENT',
        'TIMELINE_EVENT_MOVE_GROUP_AUDIBLE',
    ],
    'src/memory/memory_tick_orchestrator_pc34_compat.c': [
        'ReDMCSB MOVESENS.C:F0265:169-192',
        'MOVESENS.C:F0267:830-844',
        'ReDMCSB GROUP.C:F0185:543-545',
        'ReDMCSB TIMELINE.C:F0252:1527-1567',
        'lines 1536-1555 give one 1/4-chance random adjacent insertion',
        'lines 1565-1567 retry',
        'TIMELINE_EVENT_MOVE_GROUP_SILENT',
        'TIMELINE_EVENT_MOVE_GROUP_AUDIBLE',
        'orch_schedule_deferred_group_move_compat',
        'orch_handle_deferred_group_move_event_compat',
        'orch_is_lord_chaos_allowed_square_compat',
        'orch_try_lord_chaos_random_adjacent_retry_compat',
        'orch_resolve_group_f0267_teleporter_destination_compat',
        'MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE source lock',
        'lines 453-454 choose MASK0x0001_SCOPE_CREATURES',
        'lines 474-492 require an open teleporter',
        'lines 520-524 request M560',
        'for generated/deferred insertion; group rotation remains outside it.',
    ],
    'tests/test_m10_c006_generator_reenable_dispatch_pc34_compat.c': [
        'blocked C006 generator keeps initialized group slot for event60',
        'blocked C006 generator schedules silent event60 insertion retry',
        'event60 retry links deferred group when destination clears',
        'event60 retry schedules source C37 wander after insertion',
        'Lord Chaos random adjacent retry links group to allowed east square',
        'Lord Chaos random adjacent retry schedules C37 at adjacent square',
        'C006 F0267 teleporter leaves source teleporter chain untouched',
        'C006 F0267 teleporter links generated group at target map square',
        'C006 F0267 cross-map group does not seed party-map active state',
        'C006 F0267 audible teleporter buzzes at target map square before F0185 buzzes',
    ],
}

source_checks = {
    'TIMELINE.C': [
        'REGISTER BOOLEAN L0659_B_RandomDirectionMoveRetried;',
        'L0659_B_RandomDirectionMoveRetried = C0_FALSE;',
        'if ((L0658_ps_Group->Type == C23_CREATURE_LORD_CHAOS) && !M004_RANDOM(4))',
        'switch (M004_RANDOM(4))',
        'if (F0223_GROUP_IsLordChaosAllowed(L0656_ui_MapX, L0657_ui_MapY))',
        'goto T0252001;',
        'P0529_ps_Event->Map_Time += 5;',
        'F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0529_ps_Event);',
        'F0267_MOVE_GetMoveResult_CPSCE(P0529_ps_Event->C.Slot, CM1_MAPX_NOT_ON_A_SQUARE, 0, L0656_ui_MapX, L0657_ui_MapY);',
    ],
    'GROUP.C': [
        'BOOLEAN F0223_GROUP_IsLordChaosAllowed',
        'C01_ELEMENT_CORRIDOR',
        'C05_ELEMENT_TELEPORTER',
        'C02_ELEMENT_PIT',
        'C04_ELEMENT_DOOR',
        'THING F0185_GROUP_GetGenerated',
        'F0267_MOVE_GetMoveResult_CPSCE(L0349_T_GroupThing',
        'F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, P0353_i_MapX, P0354_i_MapY, C01_MODE_PLAY_IF_PRIORITIZED);',
    ],
    'MOVESENS.C': [
        'BOOLEAN F0267_MOVE_GetMoveResult_CPSCE',
        'F0265_MOVE_CreateEvent60To61_MoveGroup',
        'L0718_i_RequiredTeleporterScope = MASK0x0001_SCOPE_CREATURES;',
        'if ((AL0709_i_DestinationSquareType = M034_SQUARE_TYPE(AL0708_i_DestinationSquare)) == C05_ELEMENT_TELEPORTER)',
        'P0560_i_DestinationMapX = L0712_ps_Teleporter->TargetMapX;',
        'F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, P0560_i_DestinationMapX, P0561_i_DestinationMapY, C01_MODE_PLAY_IF_PRIORITIZED);',
        'return C1_TRUE; /* The specified group thing cannot be moved',
    ],
}

missing = []
if not RED.exists():
    missing.append(f'missing ReDMCSB source root {RED}')
if not DM.exists():
    missing.append(f'missing N2 local DM original data root {DM}')

for rel, needles in checks.items():
    text = (ROOT / rel).read_text()
    for needle in needles:
        if needle not in text:
            missing.append(f'{rel}: missing {needle!r}')

for rel, needles in source_checks.items():
    text = (RED / rel).read_text(encoding='latin-1', errors='replace')
    for needle in needles:
        if needle not in text:
            missing.append(f'ReDMCSB {rel}: missing {needle!r}')

if missing:
    raise SystemExit('\n'.join(missing))

print('C006_EVENT60_61_DEFER_SOURCE_LOCK_OK')
