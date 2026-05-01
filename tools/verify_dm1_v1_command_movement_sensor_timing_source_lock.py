#!/usr/bin/env python3
"""Static source-lock gate for the DM1 V1 input->command->move->sensor->timing pipeline."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path('/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source')
OUT = ROOT / 'parity-evidence' / 'verification' / 'dm1_v1_command_movement_sensor_timing_source_lock.json'


def read(path: Path, enc: str = 'utf-8') -> str:
    return path.read_text(encoding=enc)


def line_no(text: str, offset: int) -> int:
    return text.count('\n', 0, offset) + 1


def require(text: str, needle: str, label: str, file_name: str, citations: list[str], start: int = 0) -> None:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f'{label}: missing {needle!r}')
    citations.append(f'{label}: {file_name}:{line_no(text, pos)}')


def require_order(text: str, needles: list[str], label: str) -> None:
    last = -1
    for needle in needles:
        pos = text.find(needle)
        if pos < 0:
            raise AssertionError(f'{label}: missing {needle!r}')
        if pos <= last:
            raise AssertionError(f'{label}: out-of-order marker {needle!r}')
        last = pos


def require_order_positions(text: str, needles: list[str], label: str, start: int = 0) -> list[int]:
    positions: list[int] = []
    last = start - 1
    for needle in needles:
        pos = text.find(needle, start)
        if pos < 0:
            raise AssertionError(f'{label}: missing {needle!r}')
        if pos <= last:
            raise AssertionError(f'{label}: out-of-order marker {needle!r}')
        positions.append(pos)
        last = pos
        start = pos + len(needle)
    return positions


def exact_lines(text: str, positions: list[int]) -> tuple[int, int]:
    return line_no(text, positions[0]), line_no(text, positions[-1])


def find_function(text: str, name: str) -> str:
    m = re.search(r'\b(?:static\s+)?(?:int|void|BOOLEAN|uint16_t|struct\s+\w+)\s+' + re.escape(name) + r'\s*\(', text)
    if not m:
        raise AssertionError(f'missing function {name}')
    brace = text.find('{', m.end())
    if brace < 0:
        raise AssertionError(f'missing body for {name}')
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return text[m.start():pos + 1]
    raise AssertionError(f'unterminated function {name}')


def main() -> int:
    citations: list[str] = []
    command = read(RED / 'COMMAND.C', 'latin-1')
    clik = read(RED / 'CLIKMENU.C', 'latin-1')
    dungeon = read(RED / 'DUNGEON.C', 'latin-1')
    champion = read(RED / 'CHAMPION.C', 'latin-1')
    moves = read(RED / 'MOVESENS.C', 'latin-1')
    fire_queue = read(ROOT / 'dm1_v1_input_command_queue_pc34_compat.c')
    fire_timing = read(ROOT / 'dm1_v1_movement_timing_pc34_compat.c')
    fire_lifecycle = read(ROOT / 'memory_champion_lifecycle_pc34_compat.c')
    fire_move = read(ROOT / 'memory_movement_pc34_compat.c')
    fire_sensor = read(ROOT / 'memory_sensor_execution_pc34_compat.c')
    probe = read(ROOT / 'test_dm1_v1_command_movement_sensor_timing_pc34_compat.c')

    command_media529_mouse = command.index('CM1_SCREEN_RELATIVE, C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS')
    command_process_queue = command.index('void F0380_COMMAND_ProcessQueue_CPSC')
    command_dequeue_tail = command.index('L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;')
    command_checks = [
        ('G0448_as_Graphic561_SecondaryMouseInput_Movement[9]', 'movement mouse table', 'COMMAND.C', command, command_media529_mouse),
        ('{ C003_COMMAND_MOVE_FORWARD,            CM1_SCREEN_RELATIVE, C070_ZONE_MOVE_FORWARD', 'forward click command zone', 'COMMAND.C', command, command_media529_mouse),
        ('G0435_B_CommandQueueLocked = C1_TRUE;', 'queue locks during processing', 'COMMAND.C', command, command_process_queue),
        ('if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)', 'movement cooldown gate checks queued command', 'COMMAND.C', command, command_process_queue),
        ('F0360_COMMAND_ProcessPendingClick();', 'pending click replay after unlock', 'COMMAND.C', command, command_dequeue_tail),
        ('F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);', 'turn dispatch', 'COMMAND.C', command, command_process_queue),
        ('F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);', 'move dispatch', 'COMMAND.C', command, command_process_queue),
    ]
    for needle, label, fname, text, start in command_checks:
        require(text, needle, label, fname, citations, start)

    dungeon_positions = require_order_positions(dungeon, [
        'void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement',
        '*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount',
        'P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */',
        '*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount',
        'unsigned char F0151_DUNGEON_GetSquare',
        'return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];',
    ], 'DUNGEON relative movement and square lookup source order', dungeon.index('void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement'))
    a, b = exact_lines(dungeon, dungeon_positions)
    if (a, b) != (1371, 1440):
        raise AssertionError(f'DUNGEON movement coordinate line range drifted: got {a}-{b}, expected 1371-1440')
    citations.append(f'relative movement coordinate math and square lookup: DUNGEON.C:{a}-{b}')

    champion_positions = require_order_positions(champion, [
        'int16_t F0310_CHAMPION_GetMovementTicks',
        'if ((L0932_ui_MaximumLoad = F0309_CHAMPION_GetMaximumLoad(P0648_ps_Champion)) > (AL0931_ui_Load = P0648_ps_Champion->Load))',
        'L0933_ui_Ticks = 2;',
        'L0933_ui_Ticks = 4 + (((AL0931_ui_Load - L0932_ui_MaximumLoad) << 2) / L0932_ui_MaximumLoad);',
        'if (M007_GET(P0648_ps_Champion->Wounds, MASK0x0020_WOUND_FEET))',
        'if (F0033_OBJECT_GetIconIndex(P0648_ps_Champion->Slots[C05_SLOT_FEET]) == C194_ICON_ARMOUR_BOOT_OF_SPEED)',
        'return L0933_ui_Ticks;',
    ], 'CHAMPION movement ticks source order', champion.index('int16_t F0310_CHAMPION_GetMovementTicks'))
    a, b = exact_lines(champion, champion_positions)
    if (a, b) != (1180, 1214):
        raise AssertionError(f'CHAMPION movement tick line range drifted: got {a}-{b}, expected 1180-1214')
    citations.append(f'champion load/wound/boots movement tick calculation: CHAMPION.C:{a}-{b}')

    for needle, label, fname, text in [
        ('F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection', 'relative movement coordinate update', 'CLIKMENU.C', clik),
        ('L1117_B_MovementBlocked = C1_TRUE;', 'wall blocks before move core', 'CLIKMENU.C', clik),
        ('L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) &&', 'closed door blocks before move core', 'CLIKMENU.C', clik),
        ('L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN)', 'closed real fakewall blocks before move core', 'CLIKMENU.C', clik),
        ('L1117_B_MovementBlocked = (F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY) != C0xFFFE_THING_ENDOFLIST)', 'group blocks before move core', 'CLIKMENU.C', clik),
        ('F0357_COMMAND_DiscardAllInput();', 'blocked movement discards input', 'CLIKMENU.C', clik),
        ('G0321_B_StopWaitingForPlayerInput = C0_FALSE;', 'blocked movement returns before move core', 'CLIKMENU.C', clik),
        ('F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);', 'successful move enters move/sensor core', 'CLIKMENU.C', clik),
        ('G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;', 'movement disabled ticks set after successful step', 'CLIKMENU.C', clik),
        ('G0311_i_ProjectileDisabledMovementTicks = 0;', 'projectile movement gate cleared after successful step', 'CLIKMENU.C', clik),
        ('G0397_i_MoveResultMapX = P0560_i_DestinationMapX;', 'move result x recorded', 'MOVESENS.C', moves),
        ('G0362_l_LastPartyMovementTime = G0313_ul_GameTime;', 'last-party-movement time updated on real step', 'MOVESENS.C', moves),
        ('F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);', 'source leave sensor processing', 'MOVESENS.C', moves),
        ('F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);', 'destination enter sensor processing', 'MOVESENS.C', moves),
        ('while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST)', 'sensor thing-list traversal', 'MOVESENS.C', moves),
        ('F0272_SENSOR_TriggerEffect(L0769_ps_Sensor, L0778_i_Effect, P0588_ui_MapX, P0589_ui_MapY, CM1_CELL_ANY);', 'sensor effect trigger', 'MOVESENS.C', moves),
    ]:
        require(text, needle, label, fname, citations)

    require_order(command, [
        'G0435_B_CommandQueueLocked = C1_TRUE;',
        'L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;',
        'if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)',
    ], 'COMMAND queue lock/gate order')
    dequeue_tail = command[command.index('L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;'):]
    require_order(dequeue_tail, [
        'L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;',
        'if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)',
        'G0435_B_CommandQueueLocked = C0_FALSE;',
        'F0360_COMMAND_ProcessPendingClick();',
        'F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);',
        'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);',
    ], 'COMMAND dequeue/replay/dispatch order')
    require_order(clik, [
        'AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;',
        'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection',
        'if (L1117_B_MovementBlocked)',
        'F0357_COMMAND_DiscardAllInput();',
        'G0321_B_StopWaitingForPlayerInput = C0_FALSE;\n                return;',
        'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);',
        'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;',
        'G0311_i_ProjectileDisabledMovementTicks = 0;',
    ], 'CLIKMENU blocked-before-successful move pipeline order')
    require_order(moves, [
        'G0397_i_MoveResultMapX = P0560_i_DestinationMapX;',
        'G0362_l_LastPartyMovementTime = G0313_ul_GameTime;',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);',
    ], 'MOVESENS result/timing/sensor order')

    for body, checks, label in [
        (find_function(fire_queue, 'DM1_V1_InputCommandQueue_EnqueueEventPc34Compat'), ['event.kind == DM1_V1_INPUT_KIND_MOUSE && queue->locked', 'pendingClickPresent', 'return 0;'], 'Firestaff pending click capture'),
        (find_function(fire_queue, 'DM1_V1_InputCommandQueue_ProcessOnePc34Compat'), ['queue->locked = 1;', 'is_move_command(result.command)', 'projectileDisabledMovementTicks', 'lastProjectileDisabledMovementDirection == normalize_dir', 'movementDisabledGate = 1', 'process_pending_click(queue)', 'result.dequeued = 1', 'result.dispatchedMove = 1'], 'Firestaff queue gate/replay/dispatch'),
        (find_function(fire_move, 'F0702_MOVEMENT_TryMove_Compat'), ['F0701_MOVEMENT_GetStepDelta_Compat', 'MOVE_BLOCKED_DOOR', 'MOVE_BLOCKED_WALL', 'outResult->newMapX = nx', 'outResult->resultCode = MOVE_OK'], 'Firestaff movement legality core'),
        (find_function(fire_move, 'F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat'), ['party->championCount <= 0', 'F0702_MOVEMENT_TryMove_Compat', 'DUNGEON_SQUARE_MASK_THING_LIST', 'THING_GET_TYPE(thing) == THING_TYPE_GROUP'], 'Firestaff party/group collision gate'),
        (find_function(fire_sensor, 'F0718_SENSOR_ProcessPartyEnterLeave_Compat'), ['F0717_SENSOR_EnumerateOnSquare_Compat', 'F0710_SENSOR_Execute_Compat', 'outList->effects[outList->count++]'], 'Firestaff sensor enter/leave walker'),
        (find_function(fire_timing, 'DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat'), ['F0841_LIFECYCLE_ComputeMoveTicks_Compat(load, maxLoad, wounds, footwearIcon)'], 'Firestaff movement timing champion tick wrapper'),
        (find_function(fire_lifecycle, 'F0841_LIFECYCLE_ComputeMoveTicks_Compat'), ['if ((int)maxLoad > (int)load)', 'ticks = 2', 'ticks = 4 +', 'LIFECYCLE_ICON_BOOT_OF_SPEED', 'ticks -= 1'], 'Firestaff champion movement ticks'),
        (find_function(fire_timing, 'DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat'), ['DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat', 'projectileDisabledMovementTicks = 0', 'scentRecorded = 1', 'lastPartyMovementTime = currentGameTick'], 'Firestaff successful-step timing'),
    ]:
        for needle in checks:
            if needle not in body:
                raise AssertionError(f'{label}: missing {needle!r}')

    for needle in [
        'locked mouse forward stored as pending',
        'pending click becomes queued command',
        'mouse forward dispatched as move',
        'mouse movement destination sensors processed',
        'successful step cadence from slowest living champion',
        'wall blocked-side-effects key queued',
        'door blocked-side-effects key queued',
        'fakewall blocked-side-effects key queued',
        'group blocked-side-effects key queued',
        'blocked movement skips enter/leave sensors',
        'blocked movement skips timing update',
        'disabled movement leaves command queued',
        'projectile same-direction movement leaves command queued',
        'projectile different-direction movement dispatched',
    ]:
        if needle not in probe:
            raise AssertionError(f'probe coverage missing {needle!r}')

    result = {
        'status': 'pass',
        'redmcsbRoot': str(RED),
        'sourceLock': 'DM1 V1 input->command->move->sensor->timing pipeline',
        'movementIntegrationGate': 'blocked vs successful movement, command queue dispatch, relative coordinate update, party coordinate update order, leave/enter sensor order, movement ticks',
        'citations': citations,
        'firestaffFiles': [
            'dm1_v1_input_command_queue_pc34_compat.c',
            'memory_movement_pc34_compat.c',
            'memory_sensor_execution_pc34_compat.c',
            'dm1_v1_movement_timing_pc34_compat.c',
            'memory_champion_lifecycle_pc34_compat.c',
            'DUNGEON.C:1371-1440',
            'CHAMPION.C:1180-1214',
            'test_dm1_v1_command_movement_sensor_timing_pc34_compat.c',
        ],
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + '\n')
    print(f"dm1_v1_command_movement_sensor_timing_source_lock=pass citations={len(citations)} output={OUT}")
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f'dm1_v1_command_movement_sensor_timing_source_lock=fail {exc}', file=sys.stderr)
        raise
