#!/usr/bin/env python3
"""Focused DM1 V1 command-gate source lock for movement side effects.

This gate is intentionally narrow: it connects the ReDMCSB command queue lock
and dispatch path to Firestaff's command/movement integration probe, proving
that queued movement is gated before dequeue while blocked/successful movement
side effects are owned by the F0366/F0267 legality boundary.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
OUT_JSON = ROOT / 'parity-evidence' / 'verification' / 'dm1_v1_movement_command_gate_source_lock.json'
OUT_MD = ROOT / 'parity-evidence' / 'verification' / 'dm1_v1_movement_command_gate_source_lock.md'

SOURCE_RANGES = [
    {'file': 'COMMAND.C', 'lines': '6-16', 'claim': 'command queue storage, first/last indexes, lock, and pending-click fields', 'needles': ['G0432_as_CommandQueue', 'G0433_i_CommandQueueFirstIndex', 'G0434_i_CommandQueueLastIndex', 'G0435_B_CommandQueueLocked', 'G0436_B_PendingClickPresent']},
    {'file': 'COMMAND.C', 'lines': '106-121', 'claim': 'secondary mouse movement rows map visible arrows/viewport to C001..C006/C080/C083', 'needles': ['G0448_as_Graphic561_SecondaryMouseInput_Movement', 'C001_COMMAND_TURN_LEFT', 'C003_COMMAND_MOVE_FORWARD', 'C006_COMMAND_MOVE_LEFT', 'C080_COMMAND_CLICK_IN_DUNGEON_VIEW']},
    {'file': 'COMMAND.C', 'lines': '252-260', 'claim': 'legacy movement keyboard rows map keypad 4/5/6/1/2/3 to turn/step commands', 'needles': ['G0459_as_Graphic561_SecondaryKeyboardInput_Movement', 'C001_COMMAND_TURN_LEFT', '0x0088', 'C003_COMMAND_MOVE_FORWARD', '0x000B', 'C004_COMMAND_MOVE_RIGHT', '0x0015']},
    {'file': 'COMMAND.C', 'lines': '677-685', 'claim': 'PC-34 movement keyboard rows map normalized 0x004B..0x0051 codes to turn/step commands', 'needles': ['MEDIA707_I34E_I34M', 'C001_COMMAND_TURN_LEFT,     0x004B', 'C003_COMMAND_MOVE_FORWARD,  0x004C', 'C002_COMMAND_TURN_RIGHT,    0x004D', 'C006_COMMAND_MOVE_LEFT,     0x004F', 'C005_COMMAND_MOVE_BACKWARD, 0x0050', 'C004_COMMAND_MOVE_RIGHT,    0x0051']},
    {'file': 'IO2.C', 'lines': '39-61', 'claim': 'PC-34 input normalizes shifted extended arrow scancodes before command-table lookup', 'needles': ['MEDIA707_I34E_I34M', 'switch (L2944_ui_ - 0x1248)', "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'", 'return L2944_ui_']},
    {'file': 'COMMAND.C', 'lines': '1452-1662', 'claim': 'mouse click path stores pending clicks when locked and enqueues resolved commands otherwise', 'needles': ['F0359_COMMAND_ProcessClick_CPSC', 'G0436_B_PendingClickPresent = C1_TRUE', 'F0358_COMMAND_GetCommandFromMouseInput_CPSC', 'G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command']},
    {'file': 'COMMAND.C', 'lines': '1692-1707', 'claim': 'pending click replay occurs after the queue is unlocked', 'needles': ['F0360_COMMAND_ProcessPendingClick', 'if (G0436_B_PendingClickPresent)', 'G0436_B_PendingClickPresent = C0_FALSE', 'F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX']},
    {'file': 'COMMAND.C', 'lines': '1709-1813', 'claim': 'keyboard path locks the queue, checks primary then secondary tables, stores matching command, unlocks, and replays pending click', 'needles': ['F0361_COMMAND_ProcessKeyPress', 'G0435_B_CommandQueueLocked = C1_TRUE', 'while (L1111_i_Command = L1112_ps_KeyboardInput->Command)', 'P0728_KeyCode == L1112_ps_KeyboardInput->Code', 'G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command', 'G0444_ps_SecondaryKeyboardInput', 'G0435_B_CommandQueueLocked = C0_FALSE', 'F0360_COMMAND_ProcessPendingClick();']},
    {'file': 'COMMAND.C', 'lines': '2045-2156', 'claim': 'F0380 locks the queue, leaves gated movement queued, dequeues commands, replays pending clicks, then dispatches turns/moves', 'needles': ['F0380_COMMAND_ProcessQueue_CPSC', 'G0435_B_CommandQueueLocked = C1_TRUE', 'G0310_i_DisabledMovementTicks', 'G0311_i_ProjectileDisabledMovementTicks', 'L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command', 'G0435_B_CommandQueueLocked = C0_FALSE', 'F0360_COMMAND_ProcessPendingClick();', 'F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);', 'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);']},
    {'file': 'CLIKMENU.C', 'lines': '142-174', 'claim': 'turn dispatch changes direction and processes stairs/current-square sensor boundary', 'needles': ['F0365_COMMAND_ProcessTypes1To2_TurnParty', 'F0364_COMMAND_TakeStairs', 'F0284_CHAMPION_SetPartyDirection']},
    {'file': 'CLIKMENU.C', 'lines': '180-347', 'claim': 'movement dispatch maps command to relative delta, blocks before F0267, discards input on block, and applies timing only after success', 'needles': ['F0366_COMMAND_ProcessTypes3To6_MoveParty', 'G0465_ai_Graphic561_MovementArrowToStepForwardCount', 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 'L1117_B_MovementBlocked', 'F0357_COMMAND_DiscardAllInput();', 'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);', 'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;', 'G0311_i_ProjectileDisabledMovementTicks = 0;']},
    {'file': 'MOVESENS.C', 'lines': '738-783', 'claim': 'successful move records destination result and party movement timing/scent state', 'needles': ['G0397_i_MoveResultMapX = P0560_i_DestinationMapX;', 'G0398_i_MoveResultMapY = P0561_i_DestinationMapY;', 'G0362_l_LastPartyMovementTime = G0313_ul_GameTime;', 'F0317_CHAMPION_AddScentStrength']},
    {'file': 'MOVESENS.C', 'lines': '799-818', 'claim': 'successful party move processes source leave then destination enter sensors', 'needles': ['F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);']},
    {'file': 'MOVESENS.C', 'lines': '1553-1794', 'claim': 'sensor walker traverses square thing-list and triggers matching sensor effects in source order', 'needles': ['F0276_SENSOR_ProcessThingAdditionOrRemoval', 'while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST)', 'F0272_SENSOR_TriggerEffect']},
    {'file': 'DUNGEON.C', 'lines': '1371-1440', 'claim': 'relative movement math updates map coordinates and square lookup returns current map data', 'needles': ['F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 'P0253_i_Direction += 1, P0253_i_Direction &= 3', 'unsigned char F0151_DUNGEON_GetSquare', 'return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];']},
    {'file': 'CHAMPION.C', 'lines': '1180-1214', 'claim': 'champion load/wound/boots movement tick calculation feeds post-success disabled movement ticks', 'needles': ['F0310_CHAMPION_GetMovementTicks', 'F0309_CHAMPION_GetMaximumLoad', 'MASK0x0020_WOUND_FEET', 'C194_ICON_ARMOUR_BOOT_OF_SPEED', 'return L0933_ui_Ticks;']},
]

FIRESTAFF_FILES = [
    {'path': ROOT / 'dm1_v1_input_command_queue_pc34_compat.c', 'claim': 'compat queue models keyboard/mouse input rows, lock, pending replay, movement-disabled gate, and dispatch fields', 'needles': ['COMMAND.C:1709-1813 F0361 queues primary/secondary keyboard commands', 'IO2.C:27-61 F0540_INPUT_Crawcin', 'case 0x004B:', 'case 0x004C:', 'case 0x0051:', 'queue->locked = 1;', 'is_move_command(result.command)', 'result.movementDisabledGate = 1;', 'process_pending_click(queue)', 'result.dispatchedMove = 1']},
    {'path': ROOT / 'dm1_v1_movement_command_core_pc34_compat.c', 'claim': 'compat command core owns F0380->F0365/F0366 parity boundaries: turns bypass movement gates, blocked steps discard input before side effects, accepted steps process leave/enter and timing', 'needles': ['DM1_V1_InputCommandQueue_ProcessOnePc34Compat', 'dm1_v1_is_turn_command(outResult->queue.command)', 'F0718_SENSOR_ProcessPartyEnterLeave_Compat', 'dm1_v1_is_step_command(outResult->queue.command)', 'F0702_MOVEMENT_TryMove_Compat', 'outResult->inputDiscardRequested = 1;', 'DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);', 'F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat', 'DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat', 'outResult->viewportRedrawRequested = 1;']},
    {'path': ROOT / 'test_dm1_v1_input_command_queue_pc34_compat.c', 'claim': 'input queue probe covers keypad, PC-34 normalized arrow rows, shifted arrows, pending mouse replay, turn gate bypass, movement gate retention, and C5 regular-command queue capacity', 'needles': ['enqueue keypad forward', 'locked mouse becomes pending', 'movement gate set', 'turn bypasses movement disabled gate', 'pc34 table left arrow turns left', 'pc34 table up arrow moves forward', 'pc34 io2 shifted up arrow normalizes to forward', 'pc34 shifted backward arrow strafes right', 'redmcsb queue holds five commands']},
    {'path': ROOT / 'test_dm1_v1_command_movement_sensor_timing_pc34_compat.c', 'claim': 'integration probe covers successful movement side effects, blocked movement side-effect absence, command gating, and command-core turn/step/collision/redraw seams', 'needles': ['mouse movement destination sensors processed', 'blocked movement skips enter/leave sensors', 'blocked movement skips timing update', 'disabled movement leaves command queued', 'projectile same-direction movement leaves command queued', 'turn bypasses movement gate', 'core forward1 destination sensors', 'core turn bypasses movement gates', 'core east-facing right strafe moves south', 'core blocked wall clears queued followup', 'core group block clears queued followup', 'empty-party group collision bug']},
    {'path': ROOT / 'test_dm1_v1_movement_command_core_pc34_compat.c', 'claim': 'focused command-core probe proves PC34 keyboard movement rows survive dequeue into turn, successful movement, cooldown clearing, and blocked-movement queue discard', 'needles': ['pc34 core left arrow enqueues turn', 'pc34 core left arrow dispatches turn', 'pc34 core left arrow turn bypasses movement gate', 'pc34 core up arrow dispatches move', 'pc34 core up arrow applies step', 'pc34 core up arrow clears projectile cooldown', 'pc34 core blocked up arrow reports wall', 'pc34 core blocked up arrow discards followup']},
    {'path': ROOT / 'tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py', 'claim': 'broader source-lock gate already ties command queue, movement legality, sensor order, and timing to Firestaff files', 'needles': ['COMMAND queue lock/gate order', 'CLIKMENU blocked-before-successful move pipeline order', 'MOVESENS result/timing/sensor order']},
]

ORDER_CHECKS = [
    ('COMMAND.C', '2045-2156', 'queue gate before dequeue/replay/dispatch', ['G0435_B_CommandQueueLocked = C1_TRUE;', 'L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;', 'if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)', 'L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;', 'G0435_B_CommandQueueLocked = C0_FALSE;', 'F0360_COMMAND_ProcessPendingClick();', 'F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);', 'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);']),
    ('CLIKMENU.C', '180-347', 'blocked path returns before successful F0267/timing path', ['F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 'if (L1117_B_MovementBlocked)', 'F0357_COMMAND_DiscardAllInput();', 'G0321_B_StopWaitingForPlayerInput = C0_FALSE;', 'return;', 'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);', 'G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;']),
    ('MOVESENS.C', '738-818', 'successful move result/timing before leave/enter sensor side effects', ['G0397_i_MoveResultMapX = P0560_i_DestinationMapX;', 'G0362_l_LastPartyMovementTime = G0313_ul_GameTime;', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);']),
]


def read(path: Path, enc='latin-1') -> str:
    if not path.exists():
        raise AssertionError(f'missing required file: {path}')
    return path.read_text(encoding=enc)


def parse_range(spec: str) -> tuple[int, int]:
    first, last = spec.split('-')
    return int(first), int(last)


def block(path: Path, spec: str) -> str:
    lo, hi = parse_range(spec)
    ls = read(path).splitlines()
    if hi > len(ls):
        raise AssertionError(f'{path.name}:{spec} exceeds file length {len(ls)}')
    return '\n'.join(ls[lo - 1:hi])


def compact(s: str) -> str:
    return ' '.join(s.split())


def require_block(entry: dict) -> dict:
    text = block(RED / entry['file'], entry['lines'])
    flat = compact(text)
    for needle in entry['needles']:
        if compact(needle) not in flat:
            raise AssertionError(f"{entry['file']}:{entry['lines']} missing {needle!r}")
    return {'file': entry['file'], 'lines': entry['lines'], 'claim': entry['claim']}


def require_order(file_name: str, spec: str, label: str, needles: list[str]) -> dict:
    text = compact(block(RED / file_name, spec))
    pos = -1
    for needle in needles:
        hit = text.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f'{file_name}:{spec} order {label} missing {needle!r}')
        if hit <= pos:
            raise AssertionError(f'{file_name}:{spec} order {label} out of order at {needle!r}')
        pos = hit
    return {'file': file_name, 'lines': spec, 'claim': label}


def require_firestaff(entry: dict) -> dict:
    flat = compact(read(entry['path'], 'utf-8'))
    for needle in entry['needles']:
        if compact(needle) not in flat:
            raise AssertionError(f"{entry['path'].relative_to(ROOT)} missing {needle!r}")
    return {'file': str(entry['path'].relative_to(ROOT)), 'claim': entry['claim']}


def ctest_registered() -> bool:
    return 'dm1_v1_movement_command_gate_source_lock' in read(ROOT / 'CMakeLists.txt', 'utf-8')


def main() -> int:
    citations = [require_block(entry) for entry in SOURCE_RANGES]
    order = [require_order(file_name, spec, label, needles) for file_name, spec, label, needles in ORDER_CHECKS]
    firestaff = [require_firestaff(entry) for entry in FIRESTAFF_FILES]
    if not ctest_registered():
        raise AssertionError('CMakeLists.txt does not register dm1_v1_movement_command_gate_source_lock')
    result = {'status': 'pass', 'schema': 'firestaff.dm1_v1_movement_command_gate_source_lock.v1', 'redmcsbRoot': str(RED), 'scope': 'source-lock command queue -> movement dispatch -> blocked/success side effects', 'citations': citations, 'orderChecks': order, 'firestaffEvidence': firestaff, 'ctest': 'dm1_v1_movement_command_gate_source_lock'}
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2) + '\n')
    lines = ['# DM1 V1 movement command gate source lock', '', 'Status: **pass**', '', 'Scope: source-lock command queue -> movement dispatch -> blocked/success side effects.', '', '## ReDMCSB citations', '']
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in citations]
    lines += ['', '## Order checks', '']
    lines += [f"- `{item['file']}:{item['lines']}` — {item['claim']}" for item in order]
    lines += ['', '## Firestaff evidence', '']
    lines += [f"- `{item['file']}` — {item['claim']}" for item in firestaff]
    lines.append('')
    OUT_MD.write_text('\n'.join(lines), encoding='utf-8')
    print(f'dm1_v1_movement_command_gate_source_lock=pass citations={len(citations)} output={OUT_JSON}')
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f'dm1_v1_movement_command_gate_source_lock=fail {exc}', file=sys.stderr)
        raise
