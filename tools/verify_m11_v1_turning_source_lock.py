#!/usr/bin/env python3
"""Verify M11 DM1 V1 turning mechanics against local ReDMCSB source."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
OUT_JSON = ROOT / 'parity-evidence/verification/m11_v1_turning_source_lock.json'
OUT_MD = ROOT / 'parity-evidence/verification/m11_v1_turning_source_lock.md'

SOURCE_RANGES = [
    {
        'file': 'COMMAND.C', 'lines': '396-405',
        'claim': 'movement arrows map click zones to turn-left/turn-right and four movement commands',
        'needles': ['C001_COMMAND_TURN_LEFT', 'C002_COMMAND_TURN_RIGHT', 'C003_COMMAND_MOVE_FORWARD', 'C006_COMMAND_MOVE_LEFT'],
    },
    {
        'file': 'COMMAND.C', 'lines': '2045-2156',
        'claim': 'queue gate blocks only movement commands C003..C006; turns dispatch to F0365',
        'needles': ['G0310_i_DisabledMovementTicks', 'C003_COMMAND_MOVE_FORWARD', 'C006_COMMAND_MOVE_LEFT', 'F0365_COMMAND_ProcessTypes1To2_TurnParty', 'F0366_COMMAND_ProcessTypes3To6_MoveParty'],
    },
    {
        'file': 'CLIKMENU.C', 'lines': '142-174',
        'claim': 'turn command sets stop-waiting, highlights arrow, stairs take-stairs, otherwise sensors wrap one normalized 90-degree F0284 direction mutation',
        'needles': ['G0321_B_StopWaitingForPlayerInput = C1_TRUE', 'F0362_COMMAND_HighlightBoxEnable(234, 261, 125, 145)', 'F0362_COMMAND_HighlightBoxEnable(291, 318, 125, 145)', 'C03_ELEMENT_STAIRS', 'F0364_COMMAND_TakeStairs', 'F0276_SENSOR_ProcessThingAdditionOrRemoval', 'F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ((P0734_i_Command == C002_COMMAND_TURN_RIGHT) ? 1 : 3)))'],
    },
    {
        'file': 'CHAMPION.C', 'lines': '117-130',
        'claim': 'F0284 computes delta, rotates champion Cell/Direction by delta, stores G0308_i_PartyDirection',
        'needles': ['if (P0600_i_Direction == G0308_i_PartyDirection)', 'L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection', 'L0834_i_Delta += 4', 'L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta)', 'L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta)', 'G0308_i_PartyDirection = P0600_i_Direction'],
    },
    {
        'file': 'GAMELOOP.C', 'lines': '90-92',
        'claim': 'main loop redraw uses current G0308_i_PartyDirection after command processing',
        'needles': ['F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)'],
    },
    {
        'file': 'GAMELOOP.C', 'lines': '215-219',
        'claim': 'command processing loop stops after G0321_B_StopWaitingForPlayerInput is set by turn',
        'needles': ['F0380_COMMAND_ProcessQueue_CPSC()', 'if (!G0321_B_StopWaitingForPlayerInput)', 'F0363_COMMAND_HighlightBoxDisable()', 'while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking)'],
    },
    {
        'file': 'DRAWVIEW.C', 'lines': '709-724',
        'claim': 'viewport presentation is a requested draw plus vertical blank wait, not a multi-frame yaw animation loop',
        'needles': ['void F0097_DUNGEONVIEW_DrawViewport', 'G0324_B_DrawViewportRequested = C1_TRUE', 'M526_WaitVerticalBlank()'],
    },
    {
        'file': 'DUNGEON.C', 'lines': '35-44',
        'claim': 'direction constants use north/east/south/west vector tables',
        'needles': ['0,    /* North */', '1,    /* East */', '-1 }; /* South */', '-1,  /* North */', '1,   /* West */'],
    },
]

FIRESTAFF_FILES = [
    {
        'path': ROOT / 'src/engine/m11_v1_turning_presentation_pc34_compat.c',
        'claim': 'guarded V1 presentation turning seam stores one-step 90-degree endpoint render semantics with no wall block check',
        'needles': ['M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL', 'COMMAND.C:2150-2152', 'CLIKMENU.C:171-173', 'result.quarterTurnSteps = 1', 'result.animationFrames = 1', 'result.intermediateFrames = 0', 'result.wallBlockCheck = 0', 'result.waitsForViewportVBlank = 1', 'poses[i].cell = m11_normalize_direction(poses[i].cell + result.delta)', 'poses[i].direction = m11_normalize_direction(poses[i].direction + result.delta)'],
    },
    {
        'path': ROOT / 'src/dm1/dm1_v1_movement_command_core_pc34_compat.c',
        'claim': 'turn dispatch uses guarded M11 V1 seam and handles stairs-before-turn source sequence',
        'needles': ['CLIKMENU.C:167-169 consumes a turn command on a stairs', 'F0705_MOVEMENT_ResolveStairsTransition_Compat', 'm11_v1_turning_apply_party_original_presentation_pc34_compat', 'M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL', 'CHAMPION.C:117-130'],
    },
    {
        'path': ROOT / 'tests/test_m11_v1_turning_presentation_pc34_compat.c',
        'claim': 'ctest asserts direction delta, one endpoint frame, no intermediate yaw frames, no wall block, V1 guard, and champion pose rotation',
        'needles': ['right one source step per ninety', 'right single rendered endpoint frame', 'right no intermediate yaw frames', 'right has no wall block check', 'non-v1 presentation guard no-op', 'champion0 direction rotated'],
    },
]


def read(path: Path, enc: str = 'latin-1') -> str:
    if not path.exists():
        raise AssertionError(f'missing required file: {path}')
    return path.read_text(encoding=enc)


def compact(text: str) -> str:
    return ' '.join(text.split())


def block(path: Path, linespec: str) -> str:
    lo, hi = [int(x) for x in linespec.split('-')]
    lines = read(path).splitlines()
    return '\n'.join(lines[lo - 1:hi])


def require_source(entry: dict) -> dict:
    text = compact(block(RED / entry['file'], entry['lines']))
    for needle in entry['needles']:
        if compact(needle) not in text:
            raise AssertionError(f"{entry['file']}:{entry['lines']} missing {needle!r}")
    return {'file': entry['file'], 'lines': entry['lines'], 'claim': entry['claim']}


def require_firestaff(entry: dict) -> dict:
    text = compact(read(entry['path'], 'utf-8'))
    for needle in entry['needles']:
        if compact(needle) not in text:
            raise AssertionError(f"{entry['path'].relative_to(ROOT)} missing {needle!r}")
    return {'file': str(entry['path'].relative_to(ROOT)), 'claim': entry['claim']}


def main() -> int:
    citations = [require_source(e) for e in SOURCE_RANGES]
    firestaff = [require_firestaff(e) for e in FIRESTAFF_FILES]
    cmake = read(ROOT / 'CMakeLists.txt', 'utf-8')
    if 'm11_v1_turning_presentation_pc34_compat' not in cmake:
        raise AssertionError('CMakeLists.txt does not register m11_v1_turning_presentation_pc34_compat')
    result = {
        'status': 'pass',
        'schema': 'firestaff.m11_v1_turning_source_lock.v1',
        'redmcsbRoot': str(RED),
        'scope': 'DM1 V1 turn left/right source mechanics, direction state, endpoint render presentation, movement/wall interaction',
        'citations': citations,
        'firestaffEvidence': firestaff,
        'answers': {
            'stepsPer90DegreeTurn': 1,
            'facingDirectionStorage': 'G0308_i_PartyDirection equivalent PartyState_Compat.direction; champion Direction/Cell rotate by delta',
            'turnAnimationFrames': '1 endpoint viewport presentation; 0 intermediate yaw frames; draw request waits for vertical blank',
            'movementInteraction': 'movement cooldown gate applies only C003..C006, so turns can dispatch while movement is disabled; command loop stops after turn sets G0321',
            'wallBlocking': 'none for turning; only stairs special-case on current square before normal turn',
        },
        'ctest': 'm11_v1_turning_presentation_pc34_compat',
    }
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2) + '\n')
    lines = ['# M11 DM1 V1 turning source lock', '', 'Status: **pass**', '', '## ReDMCSB citations', '']
    lines += [f"- `{x['file']}:{x['lines']}` — {x['claim']}" for x in citations]
    lines += ['', '## Firestaff evidence', '']
    lines += [f"- `{x['file']}` — {x['claim']}" for x in firestaff]
    lines += ['', '## Answers', '', '- Steps per 90° turn: 1', '- Facing storage: PartyState_Compat.direction mirrors G0308_i_PartyDirection; champion directions rotate by delta.', '- Frames/timing: one endpoint viewport presentation, zero intermediate yaw frames, vertical-blank wait on viewport draw.', '- Movement interaction: movement cooldown gate blocks only move commands C003..C006; turns still dispatch.', '- Wall blocking: none for turning; current-square stairs are the only turn-command special case.']
    OUT_MD.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(f'm11_v1_turning_source_lock=pass citations={len(citations)} output={OUT_JSON}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
