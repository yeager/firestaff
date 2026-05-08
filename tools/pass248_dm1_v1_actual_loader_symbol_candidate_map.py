#!/usr/bin/env python3
from __future__ import annotations
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS246 = ROOT / 'parity-evidence/verification/pass246_dm1_v1_dmexe_fires_load_segment_bridge/manifest.json'
PASS237 = ROOT / 'parity-evidence/verification/pass237_dm1_v1_fires_static_csip_crosswalk/manifest.json'
OUT_DIR = ROOT / 'parity-evidence/verification/pass248_dm1_v1_actual_loader_symbol_candidate_map'
REPORT = ROOT / 'parity-evidence/pass248_dm1_v1_actual_loader_symbol_candidate_map.md'
DATA = ROOT / 'data/original_runtime/dm1_pc34_i34e_actual_loader_symbol_candidates.v1.json'
SOURCE_ROOT = str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")

SOURCE_SEAMS: dict[str, dict[str, Any]] = {
    'command_accepted': {'file': 'COMMAND.C', 'function': 'F0380_COMMAND_ProcessQueue_CPSC', 'lines': [2045, 2160], 'citations': ['COMMAND.C:2075-2081', 'COMMAND.C:2095-2096', 'COMMAND.C:2118-2126', 'COMMAND.C:2151-2155'], 'watch_targets': ['G0432_as_CommandQueue', 'G0433_i_CommandQueueFirstIndex', 'G0434_i_CommandQueueLastIndex', 'G0435_B_CommandQueueLocked', 'G0308_i_PartyDirection']},
    'turn_types_1_to_2': {'file': 'CLIKMENU.C', 'function': 'F0365_COMMAND_ProcessTypes1To2_TurnParty', 'lines': [142, 179], 'citations': ['CLIKMENU.C:167-173'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    'move_types_3_to_6': {'file': 'CLIKMENU.C', 'function': 'F0366_COMMAND_ProcessTypes3To6_MoveParty', 'lines': [180, 328], 'citations': ['CLIKMENU.C:264-269', 'CLIKMENU.C:272-275', 'CLIKMENU.C:326-328'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    'move_get_move_result': {'file': 'MOVESENS.C', 'function': 'F0267_MOVE_GetMoveResult_CPSCE', 'lines': [316, 560], 'citations': ['MOVESENS.C:432-443', 'MOVESENS.C:492-506', 'MOVESENS.C:556'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection']},
    'viewport_game_loop_draw_call_site': {'file': 'GAMELOOP.C', 'function': 'F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF', 'lines': [35, 95], 'citations': ['GAMELOOP.C:58-63', 'GAMELOOP.C:78-90'], 'watch_targets': ['G0306_i_PartyMapX', 'G0307_i_PartyMapY', 'G0308_i_PartyDirection', 'G0296_puc_Bitmap_Viewport']},
}
GLOBAL_DECLS = {
    'G0432_as_CommandQueue': 'TOWNSGLB.H:678', 'G0433_i_CommandQueueFirstIndex': 'TOWNSGLB.H:679', 'G0434_i_CommandQueueLastIndex': 'TOWNSGLB.H:680', 'G0435_B_CommandQueueLocked': 'TOWNSGLB.H:681',
    'G0296_puc_Bitmap_Viewport': 'TOWNSGLB.H:1371', 'G0308_i_PartyDirection': 'TOWNSGLB.H:1381', 'G0306_i_PartyMapX': 'TOWNSGLB.H:1382', 'G0307_i_PartyMapY': 'TOWNSGLB.H:1383',
}

def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding='utf-8'))

def parse_csip(csip: str) -> tuple[int, int]:
    cs, ip = csip.split(':', 1)
    return int(cs, 16), int(ip, 16)

def fmt_csip(cs: int, ip: int) -> str:
    return f'{cs & 0xffff:04X}:{ip & 0xffff:04X}'

def main() -> int:
    pass246, pass237 = load_json(PASS246), load_json(PASS237)
    candidates = pass246.get('match', {}).get('candidates', [])
    if pass246.get('status') != 'PASS_DMEXE_FIRES_LOAD_SEGMENT_BRIDGED_NO_RUNTIME_HIT_PROMOTION' or len(candidates) != 1:
        raise SystemExit('pass246 actual loader segment is not uniquely bridged')
    load_segment = int(candidates[0]['runtime_load_segment'], 16)
    rows = []
    for cand in pass237['candidates']:
        static_cs, static_ip = parse_csip(cand['candidate_static_cs_ip'])
        seam = SOURCE_SEAMS.get(cand['id'], {})
        rows.append({'id': cand['id'], 'classification': 'actual_loader_runtime_breakpoint_candidate_only_not_verified_hit', 'static_cs_ip': cand['candidate_static_cs_ip'].upper(), 'actual_dmexe_loader_load_segment': f'{load_segment:04X}', 'runtime_cs_ip_candidate': fmt_csip(load_segment + static_cs, static_ip), 'source_file': cand['source_file'], 'source_function': cand['source_function'], 'source_line_range': seam.get('lines'), 'source_citations': seam.get('citations', []), 'global_watch_targets': seam.get('watch_targets', []), 'confidence': cand['confidence'], 'promotion_blocker': cand['blocker_to_promote']})
    payload = {'schema': 'dm1_pc34_i34e_actual_loader_symbol_candidates.v1', 'timestamp_utc': datetime.now(timezone.utc).isoformat(), 'status': 'PASS_ACTUAL_LOADER_RUNTIME_CANDIDATES_READY_NO_PROMOTIONS', 'artifact_policy': {'text_only': True, 'no_original_binaries_committed': True, 'no_verified_runtime_hit_claims': True}, 'inputs': {'pass246_manifest': str(PASS246), 'pass237_manifest': str(PASS237), 'redmcsb_source_root': SOURCE_ROOT}, 'formula': 'runtime_cs = actual_dmexe_loader_load_segment + static_cs; runtime_ip = static_ip', 'actual_dmexe_loader_load_segment': f'{load_segment:04X}', 'candidate_breakpoints': rows, 'global_declarations': GLOBAL_DECLS, 'global_address_status': 'source_locked_names_only_numeric_data_addresses_still_require_debugger_or_map_evidence', 'promotion_rule': 'Do not copy these rows into verified runtime hooks until DOSBox debugger hits the runtime CS:IP and seam-specific global/watchpoint state is recorded.'}
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / 'manifest.json').write_text(json.dumps(payload, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    DATA.write_text(json.dumps(payload, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    lines = ['# Pass248 — DM1 V1 actual-loader FIRES symbol candidate map', '', 'Status: `PASS_ACTUAL_LOADER_RUNTIME_CANDIDATES_READY_NO_PROMOTIONS`', '', 'This pass binds the actual `DM.EXE` FIRES load segment from pass246 to pass237 static FIRES.EXENEW candidates. It promotes no runtime hits.', '', f'- Actual FIRES load segment: `{load_segment:04X}`', '- Formula: `runtime_cs = load_segment + static_cs`; `runtime_ip = static_ip`', '- Numeric global addresses remain blocked pending debugger/map evidence; source-level watch targets are listed.', '', '## Candidate breakpoints', '']
    for row in rows:
        lines.append(f"- `{row['id']}` -> `{row['runtime_cs_ip_candidate']}` from static `{row['static_cs_ip']}`; {row['source_file']} `{row['source_function']}`; citations: {', '.join(row['source_citations'])}")
    lines += ['', '## Source global declarations', '']
    for name, citation in sorted(GLOBAL_DECLS.items()):
        lines.append(f'- `{name}` — `{citation}`')
    lines += ['', '## Artifacts', '', f'- `{DATA}`', f"- `{OUT_DIR / 'manifest.json'}`"]
    REPORT.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(json.dumps({'status': payload['status'], 'load_segment': f'{load_segment:04X}', 'rows': len(rows), 'manifest': str(OUT_DIR / 'manifest.json'), 'data': str(DATA), 'report': str(REPORT)}, indent=2, sort_keys=True))
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
