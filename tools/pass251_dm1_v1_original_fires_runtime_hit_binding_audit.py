#!/usr/bin/env python3
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
CANDIDATES = ROOT / 'data/original_runtime/dm1_pc34_i34e_actual_loader_symbol_candidates.v1.json'
SYMBOL_MAP = ROOT / 'data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json'
PASS247 = ROOT / 'parity-evidence/verification/pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver/manifest.json'
OUT_DIR = ROOT / 'parity-evidence/verification/pass251_dm1_v1_original_fires_runtime_hit_binding_audit'
REPORT = ROOT / 'parity-evidence/pass251_dm1_v1_original_fires_runtime_hit_binding_audit.md'
MANIFEST = OUT_DIR / 'manifest.json'

SOURCE_AUDIT: list[dict[str, Any]] = [
    {'id': 'command_accepted', 'file': 'COMMAND.C', 'function': 'F0380_COMMAND_ProcessQueue_CPSC', 'range': '2045-2160', 'line_refs': ['2075-2081 queue lock/empty test', '2095-2096 command read from G0432/G0433', '2118-2126 X/Y read, first-index advance, unlock', '2151-2155 turn/move dispatch'], 'watch_targets': ['G0432_as_CommandQueue', 'G0433_i_CommandQueueFirstIndex', 'G0434_i_CommandQueueLastIndex', 'G0435_B_CommandQueueLocked', 'G0308_i_PartyDirection']},
    {'id': 'turn_types_1_to_2', 'file': 'CLIKMENU.C', 'function': 'F0365_COMMAND_ProcessTypes1To2_TurnParty', 'range': '142-179', 'line_refs': ['167 current square read at G0306/G0307', '171-173 remove/add party sensor state around F0284_CHAMPION_SetPartyDirection'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    {'id': 'move_types_3_to_6', 'file': 'CLIKMENU.C', 'function': 'F0366_COMMAND_ProcessTypes3To6_MoveParty', 'range': '180-328', 'line_refs': ['264 source X/Y seeded from G0306/G0307', '269 relative movement computes destination', '272-274 stairs path writes G0306/G0307', '326-328 non-stairs path calls F0267 with source/destination'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    {'id': 'move_get_move_result', 'file': 'MOVESENS.C', 'function': 'F0267_MOVE_GetMoveResult_CPSCE', 'range': '316-560', 'line_refs': ['442-443 successful destination writes G0306/G0307', '494-506 teleporter destination rewrites X/Y/direction', '556 pit-fall redraw calls F0128 with direction/destination'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    {'id': 'viewport_game_loop_draw_call_site', 'file': 'GAMELOOP.C', 'function': 'F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF', 'range': '35-95', 'line_refs': ['61 level-change movement result uses current G0306/G0307', '78 music update uses current party tuple', '90 F0128 draw call consumes G0308/G0306/G0307'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection', 'G0296_puc_Bitmap_Viewport']},
    {'id': 'viewport_buffer_composed', 'file': 'DUNVIEW.C', 'function': 'F0128_DUNGEONVIEW_Draw_CPSF', 'range': '8318-8610', 'line_refs': ['8318 function entry', '8367/8422/8431/8434 viewport-buffer floor/ceiling writes/flips', '8468-8540 relative-square walk', '8606/8610 F0097 viewport request'], 'watch_targets': ['G0296_puc_Bitmap_Viewport', 'G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection', 'G0324_B_DrawViewportRequested']},
    {'id': 'viewport_present', 'file': 'DRAWVIEW.C', 'function': 'F0097_DUNGEONVIEW_DrawViewport', 'range': '709-858', 'line_refs': ['721 sets G0324_B_DrawViewportRequested', '842 C007 blit on one PC-family path', '857 VIDRV_09_BlitViewPort with G0296_puc_Bitmap_Viewport'], 'watch_targets': ['G0296_puc_Bitmap_Viewport', 'G0324_B_DrawViewportRequested']},
]

GLOBAL_DECLS = {
    'G0432_as_CommandQueue': 'TOWNSGLB.H:678',
    'G0433_i_CommandQueueFirstIndex': 'TOWNSGLB.H:679',
    'G0434_i_CommandQueueLastIndex': 'TOWNSGLB.H:680',
    'G0435_B_CommandQueueLocked': 'TOWNSGLB.H:681',
    'G0296_puc_Bitmap_Viewport': 'TOWNSGLB.H:1371',
    'G0308_i_PartyDirection': 'TOWNSGLB.H:1381',
    'G0306_i_PartyMapX': 'TOWNSGLB.H:1382',
    'G0307_i_PartyMapY': 'TOWNSGLB.H:1383',
}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding='utf-8'))


def ensure_source_refs() -> list[dict[str, Any]]:
    checked = []
    for seam in SOURCE_AUDIT:
        src = SOURCE_ROOT / seam['file']
        checked.append({'file': str(src), 'exists': src.exists(), 'function': seam['function'], 'range': seam['range']})
        if not src.exists():
            raise SystemExit(f'missing ReDMCSB source file: {src}')
    towns = SOURCE_ROOT / 'TOWNSGLB.H'
    if not towns.exists():
        raise SystemExit(f'missing ReDMCSB globals file: {towns}')
    checked.append({'file': str(towns), 'exists': True, 'ranges': ['678-681', '1371-1383']})
    return checked


def main() -> int:
    source_files = ensure_source_refs()
    candidates = load_json(CANDIDATES)
    symbol_map = load_json(SYMBOL_MAP)
    pass247 = load_json(PASS247)

    candidate_rows = candidates.get('candidate_breakpoints', [])
    accepted_bp = set(pass247.get('listed_breakpoints', []))
    symbol_entries = symbol_map.get('entries', [])
    verified_symbol_entries = [e for e in symbol_entries if e.get('confidence') == 'verified_runtime_hit' or e.get('runtime_cs_ip')]
    global_entries_with_numeric_addresses = [e for e in symbol_entries if e.get('global_addresses')]

    runtime_candidates = []
    for row in candidate_rows:
        runtime_cs_ip = row.get('runtime_cs_ip_candidate')
        runtime_candidates.append({
            'id': row.get('id'),
            'runtime_cs_ip_candidate': runtime_cs_ip,
            'accepted_by_pass247_breakpoint_parser': runtime_cs_ip in accepted_bp,
            'classification': row.get('classification'),
            'source_file': row.get('source_file'),
            'source_function': row.get('source_function'),
            'source_citations': row.get('source_citations'),
            'global_watch_targets': row.get('global_watch_targets'),
            'promotion_blocker': row.get('promotion_blocker'),
        })

    blocked = bool(not verified_symbol_entries or not global_entries_with_numeric_addresses)
    payload = {
        'schema': 'pass251_dm1_v1_original_fires_runtime_hit_binding_audit.v1',
        'timestamp_utc': datetime.now(timezone.utc).isoformat(),
        'status': 'BLOCKED_RUNTIME_HIT_BINDING_PREREQUISITES_MISSING' if blocked else 'READY_FOR_RUNTIME_HIT_PROMOTION_REVIEW',
        'classification': 'blocked/original-fires-runtime-hit-binding-missing-global-addresses-and-hit-transcript' if blocked else 'review/possible-runtime-hit-prerequisites-present',
        'n2_only': True,
        'inputs': {'redmcsb_source_root': str(SOURCE_ROOT), 'candidate_map': str(CANDIDATES), 'symbol_map': str(SYMBOL_MAP), 'pass247_breakpoint_driver_manifest': str(PASS247)},
        'source_files_checked': source_files,
        'source_audit': SOURCE_AUDIT,
        'global_declarations': GLOBAL_DECLS,
        'runtime_candidates': runtime_candidates,
        'pass247_interpretation': {'status': pass247.get('status'), 'listed_breakpoints': sorted(accepted_bp), 'accepted_all': pass247.get('accepted_all'), 'promotion_value': 'breakpoint command parser proof only; not an original FIRES game-run hit transcript'},
        'prerequisite_gaps': {'verified_runtime_symbol_entries': len(verified_symbol_entries), 'symbol_entries_with_numeric_global_addresses': len(global_entries_with_numeric_addresses), 'missing': ['debugger transcript from an actual stock DM.EXE/FIRES gameplay run showing one or more candidate CS:IP hits', 'numeric runtime addresses for required globals such as G0432/G0433/G0306/G0307/G0308/G0296/G0324, or an equivalent debugger memory-read/watchpoint binding', 'route-correlated event order tying command_accepted -> movement/turn applied -> viewport draw/present']},
        'promotion_rule': 'Do not mark verified_runtime_hit until candidate CS:IP hits and seam-specific global/watchpoint state are captured from the same original FIRES runtime route.',
        'non_claims': ['no original binary payload committed', 'no screenshot/pixel parity claim', 'no verified runtime hit promoted'],
    }

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + '\n', encoding='utf-8')

    lines = [
        '# Pass251 — DM1 V1 original FIRES runtime-hit binding audit',
        '',
        f"Status: `{payload['status']}`",
        '',
        'This pass starts from the ReDMCSB source seams, then checks whether N2-local candidate CS:IP rows and debugger evidence are sufficient to promote original FIRES runtime hits. They are not yet sufficient.',
        '',
        '## ReDMCSB source audit',
        '',
    ]
    for seam in SOURCE_AUDIT:
        lines.append(f"- `{seam['id']}` — `{seam['file']}:{seam['range']}` `{seam['function']}`; refs: {', '.join(seam['line_refs'])}; watch targets: {', '.join(seam['watch_targets'])}")
    lines += ['', 'Global declarations:', '']
    for name, citation in sorted(GLOBAL_DECLS.items()):
        lines.append(f'- `{name}` — `{citation}`')
    lines += ['', '## Runtime candidate/binding status', '']
    for row in runtime_candidates:
        accepted = 'accepted by pass247 parser' if row['accepted_by_pass247_breakpoint_parser'] else 'not listed by pass247 parser'
        lines.append(f"- `{row['id']}` candidate `{row['runtime_cs_ip_candidate']}` — {accepted}; classification `{row['classification']}`; blocker: {row['promotion_blocker']}")
    lines += [
        '',
        '## Exact remaining missing binding',
        '',
        '- Pass247 proves `/usr/bin/dosbox-debug` accepts the candidate `BP` commands in a PTY (`TERM=vt100`), but it used `DEBUG COMMAND.COM` as a debugger-loop target and is not an actual stock FIRES gameplay hit transcript.',
        '- `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` still has no `verified_runtime_hit` entries and no numeric `global_addresses`.',
        '- Required next evidence: one original `DM.EXE`/`FIRES` gameplay route transcript that hits the candidate CS:IP breakpoints and reads/watches the corresponding globals in route order.',
        '',
        '## Artifacts',
        '',
        f'- `{MANIFEST}`',
        f'- `{REPORT}`',
    ]
    REPORT.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(json.dumps({'status': payload['status'], 'manifest': str(MANIFEST), 'report': str(REPORT), 'runtime_candidates': len(runtime_candidates)}, indent=2, sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
