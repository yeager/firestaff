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


def find_function(text: str, name: str) -> str:
    m = re.search(r'\b(?:static\s+)?(?:int|void|BOOLEAN|struct\s+\w+)\s+' + re.escape(name) + r'\s*\(', text)
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
    moves = read(RED / 'MOVESENS.C', 'latin-1')
    fire_queue = read(ROOT / 'dm1_v1_input_command_queue_pc34_compat.c')
    fire_timing = read(ROOT / 'dm1_v1_movement_timing_pc34_compat.c')
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

    for needle, label, fname, text in [
        ('F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection', 'relative movement coordinate update', 'CLIKMENU.C', clik),
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
        'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);',
        'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;',
        'G0311_i_ProjectileDisabledMovementTicks = 0;',
    ], 'CLIKMENU command move pipeline order')
    require_order(moves, [
        'G0397_i_MoveResultMapX = P0560_i_DestinationMapX;',
        'G0362_l_LastPartyMovementTime = G0313_ul_GameTime;',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);',
    ], 'MOVESENS result/timing/sensor order')

    for body, checks, label in [
        (find_function(fire_queue, 'DM1_V1_InputCommandQueue_EnqueueEventPc34Compat'), ['event.kind == DM1_V1_INPUT_KIND_MOUSE && queue->locked', 'pendingClickPresent', 'return 0;'], 'Firestaff pending click capture'),
        (find_function(fire_queue, 'DM1_V1_InputCommandQueue_ProcessOnePc34Compat'), ['queue->locked = 1;', 'is_move_command(result.command)', 'movementDisabledGate = 1', 'process_pending_click(queue)', 'result.dequeued = 1', 'result.dispatchedMove = 1'], 'Firestaff queue gate/replay/dispatch'),
        (find_function(fire_move, 'F0702_MOVEMENT_TryMove_Compat'), ['F0701_MOVEMENT_GetStepDelta_Compat', 'MOVE_BLOCKED_DOOR', 'MOVE_BLOCKED_WALL', 'outResult->newMapX = nx', 'outResult->resultCode = MOVE_OK'], 'Firestaff movement legality core'),
        (find_function(fire_sensor, 'F0718_SENSOR_ProcessPartyEnterLeave_Compat'), ['F0717_SENSOR_EnumerateOnSquare_Compat', 'F0710_SENSOR_Execute_Compat', 'outList->effects[outList->count++]'], 'Firestaff sensor enter/leave walker'),
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
        'disabled movement leaves command queued',
    ]:
        if needle not in probe:
            raise AssertionError(f'probe coverage missing {needle!r}')

    result = {
        'status': 'pass',
        'redmcsbRoot': str(RED),
        'sourceLock': 'DM1 V1 input->command->move->sensor->timing pipeline',
        'citations': citations,
        'firestaffFiles': [
            'dm1_v1_input_command_queue_pc34_compat.c',
            'memory_movement_pc34_compat.c',
            'memory_sensor_execution_pc34_compat.c',
            'dm1_v1_movement_timing_pc34_compat.c',
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
