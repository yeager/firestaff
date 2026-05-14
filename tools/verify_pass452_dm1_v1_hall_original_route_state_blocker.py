#!/usr/bin/env python3
"""Pass452 DM1 V1 original Hall click-no-transition blocker gate.

Source-first diagnosis for the N2 DOSBox-X capture at
/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509.

The captured route turned away from the source-locked initial C127 mirror before
clicking the portrait point.  This gate records the exact source/data anchors and
emits the next executable transition instead of encouraging more coordinate
permutations.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = Path('/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source')
ARTIFACT = Path('/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509/manifest.json')
CORRECTED_ARTIFACT = Path('/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509')
OUT_DIR = ROOT / 'parity-evidence/verification/pass452_dm1_v1_hall_original_route_state_blocker'
OUT_JSON = OUT_DIR / 'manifest.json'
OUT_MD = ROOT / 'parity-evidence/pass452_dm1_v1_hall_original_route_state_blocker.md'

EXPECTED = {
    'DUNGEON.DAT_sha256': 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85',
    'GRAPHICS.DAT_sha256': '2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e',
    'TITLE_sha256': 'adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745',
}

SOURCE_LOCKS = [
    {
        'file': 'LOADSAVE.C',
        'lines': '1940-1944',
        'point': 'legacy Hall-route assumption came from DungeonHeader InitialPartyLocation decoding to map0 x=1 y=3 dir=South; this must not be reused as proof that normal Firestaff DM1 launch starts in Hall of Champions',
        'must': ['G0298_B_NewGame', 'G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection', 'G0309_i_PartyMapIndex = 0'],
    },
    {
        'file': 'CLIKMENU.C',
        'lines': '142-173,264-328',
        'point': 'turns stay on the same square; blocked steps do not move the party, so the N2 route turn-left, blocked-step, turn-left ends at the initial square facing North, not South',
        'must': ['F0365_COMMAND_ProcessTypes1To2_TurnParty', 'F0366_COMMAND_ProcessTypes3To6_MoveParty', 'if (L1117_B_MovementBlocked)'],
    },
    {
        'file': 'CLIKVIEW.C',
        'lines': '347-431',
        'point': 'PC C080 click subtracts viewport origin, then only C05/front-wall ornament hit calls F0372 to touch the wall sensor in front of the current facing direction',
        'must': ['P0752_i_X -= G2067_i_ViewportScreenX', 'P0753_i_Y -= G2068_i_ViewportScreenY', 'C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT', 'F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor'],
    },
    {
        'file': 'MOVESENS.C',
        'lines': '1309-1503',
        'point': 'C127 wall champion portrait is the candidate transition; it is allowed with no leader, but only for the clicked wall cell/facing precondition',
        'must': ['F0275_SENSOR_IsTriggeredByClickOnWall', 'C127_SENSOR_WALL_CHAMPION_PORTRAIT', 'F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)'],
    },
    {
        'file': 'DUNVIEW.C',
        'lines': '525,3912-3919',
        'point': 'visible source portrait box is viewport x=96..127 y=35..63; with PC viewport origin y=33, center is screen x=111 y=82',
        'must': ['G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4] = { 96, 127, 35, 63 }', 'P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT', 'C026_GRAPHIC_CHAMPION_PORTRAITS'],
    },
    {
        'file': 'COORD.C',
        'lines': '1693-1698,1748-1749',
        'point': 'PC viewport origin is x=0 y=33 and portrait is 32x29, confirming x111/y82 screen click geometry',
        'must': ['G2067_i_ViewportScreenX = 0', 'G2068_i_ViewportScreenY = 33', 'G2078_C32_PortraitWidth = 32', 'G2079_C29_PortraitHeight = 29'],
    },
    {
        'file': 'REVIVE.C',
        'lines': '63-67,272,704-789',
        'point': 'F0280 appends/marks the candidate; only after that can C160/C161/C162 panel commands in F0282 finalize/cancel candidate state',
        'must': ['F0280_CHAMPION_AddCandidateChampionToParty', 'G0299_ui_CandidateChampionOrdinal', 'F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel'],
    },
    {
        'file': 'COMMAND.C',
        'lines': '108-114,1985-1991',
        'point': 'viewport C080 and resurrect/reincarnate panel commands are separate command spaces; C160/C161 cannot create a candidate before C127/F0280',
        'must': ['C080_COMMAND_CLICK_IN_DUNGEON_VIEW', 'M568_PANEL_RESURRECT_REINCARNATE', 'G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel'],
    },
]


def source_has(file: str, needles: list[str]) -> bool:
    text = (SRC_ROOT / file).read_text(encoding='latin-1')
    flat = ' '.join(text.split())
    return all(' '.join(n.split()) in flat for n in needles)


def corrected_initial_south_transition() -> dict:
    run_dir = CORRECTED_ARTIFACT / 'probe-initial-south-corrected'
    keylog = run_dir / 'original-viewpoint-route-keys.log'
    images = sorted(run_dir.glob('image*.png'))
    import hashlib
    hashes = []
    for path in images:
        h = hashlib.sha256(path.read_bytes()).hexdigest()
        hashes.append(h)
    log = keylog.read_text(encoding='utf-8', errors='replace') if keylog.exists() else ''
    candidate_sha = 'e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e'
    terminal_sha = '7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f'
    candidate_index = hashes.index(candidate_sha) if candidate_sha in hashes else -1
    terminal_index = hashes.index(terminal_sha) if terminal_sha in hashes else -1
    ok = (
        run_dir.is_dir()
        and 'route-token click:111,82' in log
        and 'left-click-mapped 111,82 -> absolute' in log
        and 'client-relative 372,358' in log
        and 'route-token click:130,115' in log
        and 'client-relative 435,468' in log
        and candidate_index > 0
        and terminal_index > candidate_index
    )
    return {
        'artifactRoot': str(CORRECTED_ARTIFACT),
        'run': 'probe-initial-south-corrected',
        'exists': run_dir.is_dir(),
        'keyLog': str(keylog),
        'imageCount': len(images),
        'uniqueImageSha256Count': len(set(hashes)),
        'candidateSelectSha256': candidate_sha,
        'candidateIndex': candidate_index,
        'terminalHudSha256': terminal_sha,
        'terminalIndex': terminal_index,
        'ok': ok,
    }


def main() -> int:
    if not ARTIFACT.exists():
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        payload = {'schema': 'pass452_dm1_v1_hall_original_route_state_blocker.v1', 'status': 'BLOCKED_EXTERNAL_HALL_ARTIFACT_MISSING', 'artifact_manifest': str(ARTIFACT), 'blocker': 'external Hall capture artifact is not mounted on this host'}
        OUT_JSON.write_text(json.dumps(payload, indent=2) + '\n', encoding='utf-8')
        OUT_MD.write_text('# Pass452 DM1 V1 Hall original route state blocker\n\nStatus: BLOCKED_EXTERNAL_HALL_ARTIFACT_MISSING\n', encoding='utf-8')
        print(payload['status'])
        return 0
    data = json.loads(ARTIFACT.read_text())
    provenance = data.get('source_provenance', {})
    for key, want in EXPECTED.items():
        got = provenance.get(key)
        if got != want:
            raise SystemExit(f'{key} mismatch: got {got!r}, want {want!r}')
    for lock in SOURCE_LOCKS:
        if not source_has(lock['file'], lock['must']):
            raise SystemExit(f"source lock failed: {lock['file']} {lock['point']}")

    route_labels = [step[0] for step in data.get('route', [])]
    must_have = ['turn_left_east', 'step_east_blocked', 'turn_left_north_front_mirror', 'candidate portrait source center']
    missing = [label for label in must_have if label not in route_labels]
    if missing:
        raise SystemExit(f'artifact route missing expected blocker step {missing[0]!r}')

    before = data['entries'][3]
    after = data['entries'][4]
    corrected = corrected_initial_south_transition()
    status = 'PASS_PASS452_WRONG_FACING_BLOCKER_SUPERSEDED_BY_INITIAL_SOUTH_RERUN' if corrected['ok'] else 'BLOCKED_WRONG_FACING_AFTER_CAPTURE_ROUTE_TURNS_AWAY_FROM_INITIAL_C127'

    diagnosis = {
        'schema': 'pass452_dm1_v1_hall_original_route_state_blocker.v1',
        'status': status,
        'artifact_manifest': str(ARTIFACT),
        'artifact_status': data.get('status'),
        'pc34_hashes': provenance,
        'route_labels': route_labels,
        'source_locks': SOURCE_LOCKS,
        'route_state_diagnosis': {
            'legacy_hall_route_state': {'map': 0, 'x': 1, 'y': 3, 'dir': 'South'},
            'normal_launch_baseline': 'SUPERSEDED_BY_2026_05_14_WEBCHAT_SCREENSHOTS_NOT_HALL_OF_CHAMPIONS',
            'source_front_c127_at_initial_state': {'map': 0, 'x': 1, 'y': 4, 'sensor': 16, 'type': 'C127_SENSOR_WALL_CHAMPION_PORTRAIT'},
            'captured_click_state_inferred_from_manifest': {'map': 0, 'x': 1, 'y': 3, 'dir': 'North'},
            'why_no_candidate_transition': 'the source portrait point was clicked after the capture route turned left twice and the intervening east step was blocked; F0372/F0275 therefore touches the north front wall, not the initial south C127 champion portrait wall at (1,4)',
            'not_blamed': ['portrait x/y geometry', 'C160/C161 panel geometry', 'Firestaff C080 implementation', 'PC34 data identity'],
        },
        'observed_frame_hashes': {
            'panel_visible_north_front_mirror_pc320_sha256': before.get('pc320_sha256'),
            'after_click_111_82_pc320_sha256': after.get('pc320_sha256'),
            'interpretation': 'hash changed only by pointer/visual noise; candidate panel was not visible, so do not promote candidate/resurrect/HUD parity from this run',
        },
        'corrected_initial_south_transition': corrected,
        'next_state_transition': None if corrected['ok'] else {
            'name': 'fresh_new_game_initial_south_c127_click_then_c160',
            'steps': [
                'select VGA / No Sound / Mouse as before',
                'press Return Return to enter the fresh game',
                'STOP on the first Hall frame: map0 x=1 y=3 dir=South; do not turn or step',
                'click PC screen x=111 y=82 (same N2 root mapping from manifest: root x=302 y=264)',
                'wait >=1.0s and capture candidate_select; expect resurrect/reincarnate/cancel candidate panel',
                'then click C160 resurrect center PC x=130 y=115 (root x=340 y=330) or C161 reincarnate center PC x=186 y=115',
            ],
            'route_edit': 'remove the turn_left_east, step_east_blocked, and turn_left_north_front_mirror inputs before the portrait click',
            'exact_next_command': 'xvfb-run -a -s "-screen 0 800x600x24" dosbox-x -conf ~/openclaw-artifacts/dm1-hall-dosbox-20260509/dosboxx.conf -fastlaunch -nogui -nomenu -time-limit 150',
        },
    }

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(diagnosis, indent=2) + '\n')

    lines = [
        '# Pass452 — DM1 V1 original Hall route-state blocker',
        '',
        f"- status: `{diagnosis['status']}`",
        f"- artifact: `{ARTIFACT}`",
        f"- DUNGEON.DAT sha256: `{provenance['DUNGEON.DAT_sha256']}`",
        f"- GRAPHICS.DAT sha256: `{provenance['GRAPHICS.DAT_sha256']}`",
        f"- TITLE sha256: `{provenance['TITLE_sha256']}`",
        '',
        '## Diagnosis',
        '',
        'The N2 capture did prove a stock PC34 Hall/front-mirror visible artifact for a Hall-specific route, but 2026-05-14 webchat screenshots supersede using Hall of Champions as the normal Firestaff DM1 start baseline. This gate must not be cited as proof that ordinary DM1 launch starts in Hall of Champions. The stale Hall route clicked after this manifest route:',
        '',
    ]
    for label in route_labels:
        lines.append(f'- {label}')
    lines += [
        '',
        'That route is not a valid normal-start baseline. It is only a legacy Hall-specific route-state diagnosis: party map0 `(1,3)` facing **South**, facing the C127 champion portrait sensor on `(1,4)`. The capture then turned left to East, attempted an East step that was blocked, and turned left again to **North** before clicking x111/y82. Since turns do not move the party and blocked steps do not move it, the click touched the north front wall, not the south C127 wall.',
        '',
        'Correction 2026-05-14: Daniel provided webchat screenshot evidence that Firestaff DM1 does **not** begin in Hall of Champions. Treat Hall-route evidence as a bounded resurrection/champion-route lane only, never as the expected ordinary DM1 launch frame.',
        '',
        'That stale run is now superseded by the corrected initial-south rerun when `corrected_initial_south_transition.ok` is true. The diagnosis remains in the evidence record so the bad route is not reused.',
        '',
        '## Source locks',
        '',
    ]
    for lock in SOURCE_LOCKS:
        lines.append(f"- `{lock['file']}:{lock['lines']}` — {lock['point']}")
    lines += [
        '',
        '## Resolution',
        '',
        f"- corrected initial-south rerun ok: `{diagnosis['corrected_initial_south_transition']['ok']}`",
        f"- corrected artifact: `{diagnosis['corrected_initial_south_transition']['artifactRoot']}`",
        f"- images: `{diagnosis['corrected_initial_south_transition']['imageCount']}` / unique `{diagnosis['corrected_initial_south_transition']['uniqueImageSha256Count']}`",
        f"- candidate index: `{diagnosis['corrected_initial_south_transition']['candidateIndex']}`",
        f"- terminal index: `{diagnosis['corrected_initial_south_transition']['terminalIndex']}`",
    ]
    if diagnosis['next_state_transition']:
        lines += [
            '## Next executable transition',
            '',
            'Use the same stock PC34 data and DOSBox-X setup, but after entering the fresh game, do **not** turn or step.  Click the source portrait center while still at initial South-facing Hall state:',
            '',
            '1. select VGA / No Sound / Mouse as before',
            '2. press `Return Return` to enter the fresh game',
            '3. capture first Hall frame: expected map0 `(1,3,S)`',
            '4. click PC screen `x=111,y=82` (N2 root mapping from this run: `x=302,y=264`)',
            '5. wait/capture `candidate_select`; expect resurrect/reincarnate/cancel panel',
            '6. then click C160 resurrect `x=130,y=115` (root `x=340,y=330`) or C161 reincarnate `x=186,y=115`',
            '',
            f"Route edit: {diagnosis['next_state_transition']['route_edit']}.",
            '',
            'Exact command shell:',
            '',
            '```sh',
            diagnosis['next_state_transition']['exact_next_command'],
            '```',
        ]
    OUT_MD.parent.mkdir(parents=True, exist_ok=True)
    OUT_MD.write_text('\n'.join(lines) + '\n')
    print(f'PASS wrote {OUT_JSON}')
    print(f'PASS wrote {OUT_MD}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
