#!/usr/bin/env python3
from __future__ import annotations
import json, re, subprocess
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206'
SROOT=SRC/'Toolchains/Common/Source'
OUT=REPO/'parity-evidence/verification/pass172_redmcsb_patch_gap_inventory'
PRIORITY_BUGS={
 'BUG0_71':('timing','fast-computer timing too short; title/entrance/endgame need enforced VBlank/delay parity'),
 'BUG0_73':('input','mouse click command queue race with keyboard command can drop click'),
 'BUG0_75':('viewport','champion portrait ordinal not reset when no wall square in view'),
 'BUG0_76':('viewport','single global inscription thing makes same text draw on multiple wall sides'),
 'BUG0_86':('assets','champion portrait/resurrect/reincarnate graphics missing/garbage on constrained-memory PC/custom mirror maps'),
 'BUG0_35':('inventory','open chest icon not updated in action hand while inventory open'),
 'BUG2_00':('inventory','scroll open/closed icon regression caused by chest fix'),
 'BUG0_36':('hud','dead champion cell can erase another champion icon after load/rebirth'),
 'BUG0_39':('inventory','leader Food/Water/Poisoned panel flashes while swapping scroll/chest'),
 'BUG0_42':('hud','action icons not redrawn after unfreeze'),
 'BUG0_43':('candidate','game may not end when last living champion killed while viewing candidate portrait'),
 'BUG0_52':('input','keyboard commands while pressing eye/mouth can leave pointer/input/menu state broken'),
 'BUG0_53':('candidate','rest possible with candidate inventory open, can duplicate champion/items'),
 'BUG0_64':('viewport','floor ornaments drawn over open pits'),
 'BUG0_74':('viewport','creatures drawn with wrong colors through Thieves Eye wall hole'),
 'BUG0_83':('viewport','Thieves Eye hole moves with opening doors'),
 'BUG0_84':('endgame','ending screen palette may be uninitialized'),
 'BUG0_85':('movement','empty party can move over group in custom dungeon and kill it'),
}

def grep(term):
    p=subprocess.run(['rg','-n','--sort','path','-F',term,str(SROOT)],text=True,capture_output=True,encoding='latin-1')
    hits=[]
    for line in p.stdout.splitlines():
        parts=line.split(':',2)
        if len(parts)==3: hits.append({'file':str(Path(parts[0]).relative_to(SROOT)),'line':int(parts[1]),'text':parts[2].strip()})
    return hits

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    bugs=[]
    for bug,(lane,summary) in PRIORITY_BUGS.items():
        hits=grep(bug)
        files=sorted(set(h['file'] for h in hits))
        bugs.append({'id':bug,'lane':lane,'summary':summary,'hit_count':len(hits),'files':files,'sample_hits':hits[:6]})
    # Structural gaps that are not normal BUG0_xx items.
    structural=[]
    for pat,desc in [
        ('BUGX_XX','unresolved/unnumbered source bug markers'),
        ('ANOMALY_','port/compiler/anomaly markers'),
        ('missing','explicit missing support/code/comment markers'),
        ('PC_FIX_CODE_SIZE','PC code-size parity macro comments warn of additional/missing instructions'),
        ('SU1E missing','known missing media/version gate in declarations'),
    ]:
        hits=grep(pat)
        structural.append({'pattern':pat,'summary':desc,'hit_count':len(hits),'files':sorted(set(h['file'] for h in hits))[:30],'sample_hits':hits[:12]})
    # Firestaff-supported patch candidates: source facts we already verified by current pass artifacts.
    firestaff_supported=[
        {'area':'DM1/V1 title+entrance timing','redmcsb':['TITLE.C','ENTRANCE.C','ENDGAME.C'],'firestaff_evidence':['pass160_redmcsb_source_lock','pass171_redmcsb_source_lock_closure'],'patch_shape':'make BUG0_71 timing waits explicit/portable; document expected VBlank counts for PC-fast hosts'},
        {'area':'C007 viewport → C080 click routing','redmcsb':['COMMAND.C','CLIKVIEW.C','MOVESENS.C'],'firestaff_evidence':['pass161_c080_viewport_click_source','pass164_champion_portrait_click_source_path'],'patch_shape':'add comments/tests or helper documenting C007/C080/front-wall sensor route into C127 portrait sensor'},
        {'area':'champion portrait geometry','redmcsb':['DUNVIEW.C','COORD.C'],'firestaff_evidence':['pass165_champion_portrait_click_geometry'],'patch_shape':'document PC viewport origin + portrait bbox `{96,127,35,63}` → screen click center `(111,82)`'},
        {'area':'candidate champion recruit path','redmcsb':['MOVESENS.C','REVIVE.C','COMMAND.C'],'firestaff_evidence':['pass163_champion_recruit_source_path_lock'],'patch_shape':'clarify that C160/C161 only works after C127/F0280 candidate state; avoid misleading panel-first route assumptions'},
        {'area':'object/status/action icon geometry','redmcsb':['CHAMDRAW.C','ACTIDRAW.C','PANEL.C','COORD.C'],'firestaff_evidence':['pass169_redmcsb_anchor_gap_resolution','pass170_source_mentioned_unresolved_batch'],'patch_shape':'extract/label source constants for 16x16 object icons and status/action hand zones'},
    ]
    manifest={'schema':'pass172_redmcsb_patch_gap_inventory.v1','source_root':str(SRC),'priority_bug_count':len(bugs),'structural_gap_count':len(structural),'priority_bugs':bugs,'structural_gaps':structural,'firestaff_supported_patch_candidates':firestaff_supported}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    lines=['# Pass172 — ReDMCSB patch gap inventory for Christophe','','## Executive take','', 'ReDMCSB is not “missing answers” for DM1/V1 parity; it is missing a clean patch queue around known documented bugs/anomalies and a few source facts that Firestaff has now independently gated. Highest value is to send small, source-local patches with repro/evidence, not a giant rewrite.','','## Priority patch candidates','']
    for b in bugs:
        loc=', '.join(f"`{f}`" for f in b['files'][:5]) or '-'
        lines.append(f"- **{b['id']}** ({b['lane']}): {b['summary']} — files: {loc}; hits: {b['hit_count']}")
    lines += ['','## Structural gaps / grep buckets','']
    for s in structural:
        files=', '.join(f"`{f}`" for f in s['files'][:8]) or '-'
        lines.append(f"- `{s['pattern']}`: {s['summary']} — hits: {s['hit_count']}; files: {files}")
    lines += ['','## Firestaff-backed patches we can prepare','']
    for c in firestaff_supported:
        lines.append(f"- **{c['area']}** — ReDMCSB files: {', '.join('`'+x+'`' for x in c['redmcsb'])}; evidence: {', '.join('`'+x+'`' for x in c['firestaff_evidence'])}; patch shape: {c['patch_shape']}")
    lines += ['','## Recommendation','', 'Start with 5 small Christophe-friendly patches:', '1. Documentation/test note for DM1/V1 recruit route: C007→C080→C127→F0280 before C160/C161.', '2. Portrait click geometry comment/table in `DUNVIEW.C`/`COORD.C`.', '3. BUG0_71 timing notes for TITLE/ENTRANCE/ENDGAME with exact waits.', '4. BUG0_73 input queue race explanation with a minimal safe fix candidate or at least a regression note.', '5. Inventory icon correctness: BUG0_35 + BUG2_00 split so chest fix does not regress scroll icons.', '', 'Do **not** lead with huge gameplay-behavior changes. Lead with source comments, tests, and narrowly-scoped bug fixes where ReDMCSB already documents the bug.']
    (OUT/'README.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'out':str(OUT),'priority_bugs':len(bugs),'structural_gaps':len(structural)},indent=2))
if __name__=='__main__': main()
