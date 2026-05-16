#!/usr/bin/env python3
"""Narrow source-lock gate for DM1 V1 party successful-move leave/enter sensor order."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
OUT = ROOT / 'parity-evidence' / 'verification' / 'dm1_v1_party_movement_sensor_order_source_lock.json'


def read(path: Path, enc: str = 'utf-8') -> str:
    return path.read_text(encoding=enc)


def line_no(text: str, offset: int) -> int:
    return text.count('\n', 0, offset) + 1


def require_order(text: str, needles: list[str], label: str, start: int = 0) -> list[int]:
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


def main() -> int:
    citations: list[str] = []
    moves = read(RED / 'MOVESENS.C', 'latin-1')
    clik = read(RED / 'CLIKMENU.C', 'latin-1')
    command = read(RED / 'COMMAND.C', 'latin-1')
    sensor = read(ROOT / 'src/memory/memory_sensor_execution_pc34_compat.c')
    probe = read(ROOT / 'tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c')

    move_func = moves.index('BOOLEAN F0267_MOVE_GetMoveResult_CPSCE')
    move_party_positions = require_order(moves, [
        'G0306_i_PartyMapX = P0560_i_DestinationMapX;',
        'G0307_i_PartyMapY = P0561_i_DestinationMapY;',
        'G0397_i_MoveResultMapX = P0560_i_DestinationMapX;',
        'G0398_i_MoveResultMapY = P0561_i_DestinationMapY;',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);',
        'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);',
    ], 'MOVESENS successful party movement coordinate/result/source-leave/destination-enter order', move_func)
    a, b = exact_lines(moves, move_party_positions)
    if (a, b) != (442, 818):
        raise AssertionError(f'MOVESENS audited order line range drifted: got {a}-{b}, expected 442-818')
    citations.append(f'party coords updated before leave/enter sensor calls: MOVESENS.C:{a}-{b}')
    citations.append('party real-step scent/timing block: MOVESENS.C:763-784')
    citations.append('party source leave call: MOVESENS.C:799-802')
    citations.append('party destination enter call: MOVESENS.C:810-818')

    f0276 = moves.index('void F0276_SENSOR_ProcessThingAdditionOrRemoval')
    f0276_positions = require_order(moves, [
        'void F0276_SENSOR_ProcessThingAdditionOrRemoval',
        'L0770_ui_SensorTriggeredCell = CM1_CELL_ANY;',
        'while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST)',
        'L0768_B_TriggerSensor = P0592_B_AddThing;',
        'case C003_SENSOR_FLOOR_PARTY:',
        'F0272_SENSOR_TriggerEffect(L0769_ps_Sensor, L0778_i_Effect, P0588_ui_MapX, P0589_ui_MapY, CM1_CELL_ANY);',
        'F0271_SENSOR_ProcessRotationEffect();',
    ], 'MOVESENS F0276 floor-party add/remove sensor traversal/effect order', f0276)
    a, b = exact_lines(moves, f0276_positions)
    if (a, b) != (1553, 1793):
        raise AssertionError(f'MOVESENS F0276 line range drifted: got {a}-{b}, expected 1553-1793')
    citations.append(f'F0276 floor-party add/remove traversal/effect order: MOVESENS.C:{a}-{b}')

    clik_positions = require_order(clik, [
        'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection',
        'if (L1117_B_MovementBlocked)',
        'return;',
        'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);',
        'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;',
    ], 'CLIKMENU movement blocks before successful F0267 call then timing ticks', clik.index('void F0366_COMMAND_ProcessTypes3To6_MoveParty'))
    a, b = exact_lines(clik, clik_positions)
    if (a, b) != (269, 345):
        raise AssertionError(f'CLIKMENU movement success line range drifted: got {a}-{b}, expected 269-345')
    citations.append(f'successful movement reaches F0267 only after block checks: CLIKMENU.C:{a}-{b}')

    command_positions = require_order(command, [
        'void F0380_COMMAND_ProcessQueue_CPSC',
        'L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;',
        'if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)',
        'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);',
    ], 'COMMAND queued move dispatch reaches F0366', command.index('void F0380_COMMAND_ProcessQueue_CPSC'))
    a, b = exact_lines(command, command_positions)
    if (a, b) != (2045, 2155):
        raise AssertionError(f'COMMAND dispatch line range drifted: got {a}-{b}, expected 2045-2155')
    citations.append(f'queued C003-C006 move dispatch: COMMAND.C:{a}-{b}')

    fire_positions = require_order(sensor, [
        'int F0718_SENSOR_ProcessPartyEnterLeave_Compat',
        'sensorCount = F0717_SENSOR_EnumerateOnSquare_Compat',
        'F0710_SENSOR_Execute_Compat(dungeon, things, &sensors[i]',
        'outList->effects[outList->count++] = tmp.effects[j];',
    ], 'Firestaff F0718 enumerates sensors and appends effects in source list order')
    a, b = exact_lines(sensor, fire_positions)
    citations.append(f'Firestaff enter/leave walker preserves sensor-list order: memory_sensor_execution_pc34_compat.c:{a}-{b}')

    probe_positions = require_order(probe, [
        'ok &= expect_int("movement accepted", F0702_MOVEMENT_TryMove_Compat',
        'party.mapX = moveResult.newMapX;',
        'party.mapY = moveResult.newMapY;',
        'ok &= expect_int("source leave sensors empty after party coord update", F0718_SENSOR_ProcessPartyEnterLeave_Compat',
        'SENSOR_EVENT_WALK_OFF, &leaveEffects',
        'ok &= expect_int("destination enter sensors processed", F0718_SENSOR_ProcessPartyEnterLeave_Compat',
        'SENSOR_EVENT_WALK_ON, &enterEffects',
        'ok &= expect_int("destination first effect teleport", enterEffects.effects[0].kind, SENSOR_EFFECT_TELEPORT);',
        'ok &= expect_int("destination second effect text", enterEffects.effects[1].kind, SENSOR_EFFECT_SHOW_TEXT);',
    ], 'probe models successful move -> coord update -> source leave -> destination enter -> list-order effects')
    a, b = exact_lines(probe, probe_positions)
    citations.append(f'probe successful move leave/enter order: test_dm1_v1_command_movement_sensor_timing_pc34_compat.c:{a}-{b}')

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps({
        'status': 'pass',
        'sourceLock': 'DM1 V1 party successful movement sensor order',
        'redmcsbRoot': str(RED),
        'focus': 'after a successful party movement, ReDMCSB updates party coords, processes source-square removal/leave sensors, then destination-square addition/enter sensors; F0276 traverses sensors in thing-list order',
        'citations': citations,
    }, indent=2) + '\n')
    print(f'dm1_v1_party_movement_sensor_order_source_lock=pass citations={len(citations)} output={OUT}')
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f'dm1_v1_party_movement_sensor_order_source_lock=fail {exc}', file=sys.stderr)
        raise
