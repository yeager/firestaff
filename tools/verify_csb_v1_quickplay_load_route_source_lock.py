#!/usr/bin/env python3
from __future__ import annotations
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'parity-evidence/verification/csb_v1_quickplay_load_route_source_lock.json'
DOC = ROOT / 'docs/parity/PARITY_MATRIX_CSB_V1.md'
COMPLETION_DOC = ROOT / 'docs/parity/COMPLETION_MATRIX.md'
COMPLETION = ROOT / 'parity-evidence/verification/firestaff_completion_matrix.json'
REDMCSB = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
CSB_SRC = Path.home() / '.openclaw/data/firestaff-csb-source/CSB/src'
CSBWIN = Path.home() / '.openclaw/data/firestaff-csbwin-source/CSBWin'

EXPECTED_GIT = {
    'csb_lineage': (CSB_SRC, 'dda570585abb4c8113a3298d21c0b599e6cac4f9'),
    'csbwin_lineage': (CSBWIN, '2f63d10d9b8c155e0be17888271d394255ce1bac'),
}

ANCHORS = [
    {'id':'redmcsb_save_file_router_csbgame','role':'primary','path':REDMCSB/'CEDTINC8.C','lines':'101-118','needles':['M746_FILE_ID_SAVE_CSBGAME_DAT','C13_DUNGEON_CSB_GAME','C12_DUNGEON_CSB_PRISON','M745_FILE_ID_SAVE_DMSAVE_DAT']},
    {'id':'redmcsb_make_new_adventure_requires_csb_game','role':'primary','path':REDMCSB/'CEDTINCH.C','lines':'49-64','needles':['F1996_','C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK','C13_DUNGEON_CSB_GAME','C1_TRUE']},
    {'id':'csb_lineage_quickplay_playfile_gate','role':'secondary','path':CSB_SRC/'CSBwin.cpp','lines':'502-508','needles':['IDC_QuickPlay','PlayfileIsOpen','UIM_SETOPTION','OPT_QUICKPLAY']},
    {'id':'csb_lineage_quickplay_menu_enable','role':'secondary','path':CSB_SRC/'CSBwin.cpp','lines':'1207-1216','needles':['PlayfileIsOpen','EnableMenuItem','IDC_QuickPlay','MF_ENABLED']},
    {'id':'csbwin_quickplay_playfile_gate','role':'secondary','path':CSBWIN/'CSBwin.cpp','lines':'536-542','needles':['IDC_QuickPlay','PlayfileIsOpen','UIM_SETOPTION','OPT_QUICKPLAY']},
    {'id':'csbwin_quickplay_menu_enable','role':'secondary','path':CSBWIN/'CSBwin.cpp','lines':'1236-1242','needles':['PlayfileIsOpen','EnableMenuItem','IDC_QuickPlay','MF_ENABLED']},
    {'id':'csb_game_state_constants','role':'secondary','path':CSB_SRC/'CSB.h','lines':'42-48','needles':['GAMESTATE_ResumeSavedGame','GAMESTATE_AtPrisonDoor','GAMESTATE_EnterPrison']},
    {'id':'csb_load_dungeon_signature','role':'secondary','path':CSB_SRC/'SaveGame.cpp','lines':'1591-1607','needles':['GAMESTATE_ResumeSavedGame','InsertDisk','dungeonName','Signature']},
    {'id':'csb_load_graphics_timers_mouse','role':'secondary','path':CSB_SRC/'SaveGame.cpp','lines':'2046-2075','needles':['openGraphicsFile','InitializeTimers','InitializeItem16','HandleMouseEvents']},
]

NON_CLAIMS = [
    'QuickPlay is source-locked as replay playback setup, not as a Firestaff production CSB launch route.',
    'No Firestaff CSB runtime, capture, gameplay, save compatibility, or pixel parity is enabled or claimed.',
    'The remaining launch blocker is true CSB runtime/capture handling after the guarded M12 front door.',
]

def line_window(path, span):
    start, end = [int(part) for part in span.split('-')]
    try:
        lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
    except OSError:
        return ''
    return '\n'.join(lines[start - 1:end])

def git_head(path):
    try:
        return subprocess.check_output(['git','-C',str(path),'rev-parse','HEAD'], text=True).strip()
    except Exception:
        return None

def main():
    failures = []
    git_rows = []
    for ident, (path, expected) in EXPECTED_GIT.items():
        actual = git_head(path)
        ok = actual == expected
        if not ok:
            failures.append(f'{ident} HEAD mismatch: expected {expected}, actual {actual}')
        git_rows.append({'id':ident,'path':str(path),'expected_head':expected,'actual_head':actual,'ok':ok})
    source_rows = []
    for anchor in ANCHORS:
        path = anchor['path']
        haystack = line_window(path, anchor['lines'])
        missing = [needle for needle in anchor['needles'] if needle not in haystack]
        ok = path.exists() and not missing
        if not ok:
            failures.append(f"{anchor['id']} missing {missing} path_exists={path.exists()}")
        source_rows.append({'id':anchor['id'],'role':anchor['role'],'path':str(path),'lines':anchor['lines'],'needles':anchor['needles'],'missing':missing,'ok':ok})
    completion = json.loads(COMPLETION.read_text(encoding='utf-8'))
    rows = {row.get('target'): row for row in completion.get('rows', [])}
    csb = rows.get('CSB V1', {})
    if csb.get('completionPercent') != 20 or csb.get('points') != 20:
        failures.append(f"CSB V1 completion must be 20/100 after this evidence item, got {csb.get('completionPercent')}/{csb.get('points')}")
    launch_score = csb.get('scores', {}).get('launch_smoke', [None])[0]
    if launch_score != 2:
        failures.append(f'CSB V1 launch_smoke score must be 2/10, got {launch_score}')
    doc = DOC.read_text(encoding='utf-8')
    completion_doc = COMPLETION_DOC.read_text(encoding='utf-8')
    required_doc_needles = ['csb_v1_quickplay_load_route_source_lock', chr(96)+'launch_smoke'+chr(96)+' | 2/10', 'QuickPlay/load-route boundaries', 'No CSB runtime, launch, render, gameplay, save compatibility, or pixel parity is claimed by this matrix.']
    for needle in required_doc_needles:
        if needle not in doc:
            failures.append(f'CSB V1 parity matrix missing {needle!r}')
    if '| CSB V1 | 20% | 20/100 |' not in completion_doc:
        failures.append('completion matrix doc missing CSB V1 20% row')
    if '| '+chr(96)+'launch_smoke'+chr(96)+' | 2/10 |' not in completion_doc:
        failures.append('completion matrix doc missing CSB V1 launch_smoke 2/10 detail')
    result = {
        'schema':'firestaff.csb_v1_quickplay_load_route_source_lock.v1',
        'pass':not failures,
        'scope':'CSB V1 launch-surface source-lock evidence only; no Firestaff runtime launch claim.',
        'source_anchors':source_rows,
        'git_refs':git_rows,
        'completion_impact':{'target':'CSB V1','criterion':'launch_smoke','before_score':1,'after_score':2,'before_completion_percent':19,'after_completion_percent':20},
        'non_claims':NON_CLAIMS,
        'remaining_blocker':'Production CSB runtime/capture handling remains blocked after the guarded M12 front door.',
        'failures':failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    print(('PASS' if result['pass'] else 'FAIL') + f' csb v1 quickplay/load source lock: anchors={len(source_rows)} completion=20/100')
    for failure in failures:
        print('- ' + failure)
    return 0 if result['pass'] else 1

if __name__ == '__main__':
    raise SystemExit(main())
