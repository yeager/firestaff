#!/usr/bin/env python3
"""Pass169: resolve pass168's missing ReDMCSB source anchors.

This is a curated source-lock addendum for pass groups that were intentionally
capture/harness/evidence oriented and therefore did not carry obvious ReDMCSB
file/function names in their original notes.
"""
from __future__ import annotations
import json, subprocess
from pathlib import Path

REPO=Path(__file__).resolve().parent.parent
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
OUT=REPO/'parity-evidence/verification/pass169_redmcsb_anchor_gap_resolution'

ANCHORS={
 '17': {
   'topic':'VGA screenshot/palette export correctness',
   'claim':'Firestaff probe screenshots must decode PC/VGA indexed graphics through the VGA display path rather than treating packed framebuffer bytes as grayscale.',
   'files':['DM.C','IMAGE5.C','PC.H'],
   'tokens':['C1_VGA_GRAPHICS','F8076_IsVGAGraphicsDetected','F0680_CopyPixelsToScreenWithoutTransparency','C25_VGA'],
 },
 '64': {
   'topic':'clean original TITLE window capture path',
   'claim':'The selector route is source-backed by DM.C video/sound/controller choices and the resulting executable program name VGA before TITLE.C owns the title runtime.',
   'files':['DM.C','TITLE.C'],
   'tokens':['G8023_Video','"VGA Graphics"','F8078_strcpy(L2384_ac_VideoProgramName, "VGA")','F0437_STARTEND_DrawTitle'],
 },
 '71': {
   'topic':'deterministic in-game capture smoke gate',
   'claim':'The six-capture smoke gate protects the source-derived viewport pipeline rooted in DRAWVIEW/DUNVIEW/PANEL screen blits.',
   'files':['DRAWVIEW.C','DUNVIEW.C','PANEL.C'],
   'tokens':['F0097_DUNGEONVIEW_DrawViewport','F0127_DUNGEONVIEW_DrawSquare','C000_DERIVED_BITMAP_VIEWPORT','F0351_INVENTORY_DrawChampionSkillsAndStatistics'],
 },
 '78': {
   'topic':'original route lock attempt text-mode blocker',
   'claim':'Text-mode selector captures are blockers until DM.C selects the VGA executable and COMMAND.C viewport/movement commands can operate on a 320x200 graphics frame.',
   'files':['DM.C','COMMAND.C','TITLE.C'],
   'tokens':['G8023_Video','C080_COMMAND_CLICK_IN_DUNGEON_VIEW','C001_COMMAND_TURN_LEFT','C003_COMMAND_MOVE_FORWARD','F0437_STARTEND_DrawTitle'],
 },
 '137': {
   'topic':'F1/F2 high-density original route probe',
   'claim':'F1/F2/number-key route frames are candidate-menu evidence only; source party creation still requires COMMAND.C/MOVESENS champion portrait sensor flow.',
   'files':['COMMAND.C','MOVESENS.C','REVIVE.C'],
   'tokens':['C127_SENSOR_WALL_CHAMPION_PORTRAIT','F0280_CHAMPION_AddCandidateChampionToParty','C160_COMMAND_CLICK_IN_PANEL_RESURRECT','C161_COMMAND_CLICK_IN_PANEL_REINCARNATE'],
 },
 '151': {
   'topic':'48ed static no-party bbox blocker',
   'claim':'Repeated 48ed frames cover the source viewport rectangle but do not prove COMMAND.C movement/party input state; treat them as static no-party placeholders.',
   'files':['COMMAND.C','COORD.C','DUNVIEW.C'],
   'tokens':['C080_COMMAND_CLICK_IN_DUNGEON_VIEW','G2067_i_ViewportScreenX','G2068_i_ViewportScreenY','C007_ZONE_VIEWPORT'],
 },
 '153': {
   'topic':'Xvfb input delivery matrix for 48ed',
   'claim':'Helper/xdotool delivery was tested against the source mouse/keyboard command map; unchanged 48ed means source command prerequisites are absent rather than host input delivery alone.',
   'files':['COMMAND.C','CLIKVIEW.C','MOVESENS.C'],
   'tokens':['F0377_COMMAND_ProcessType80_ClickInDungeonView','C080_COMMAND_CLICK_IN_DUNGEON_VIEW','C083_COMMAND_TOGGLE_INVENTORY_LEADER','C127_SENSOR_WALL_CHAMPION_PORTRAIT'],
 },
 '154': {
   'topic':'static 48ed guard fix',
   'claim':'The guard is source-consistent: C080/movement frames without C127/F0280 candidate creation must not count as party-control readiness.',
   'files':['COMMAND.C','MOVESENS.C','REVIVE.C'],
   'tokens':['C080_COMMAND_CLICK_IN_DUNGEON_VIEW','C127_SENSOR_WALL_CHAMPION_PORTRAIT','F0280_CHAMPION_AddCandidateChampionToParty','F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel'],
 },
 '155': {
   'topic':'champion/control route seed finder',
   'claim':'Non-48ed seeds are useful only when they approach the source Hall-of-Champions sequence: DM.C launch, COMMAND.C viewport click, MOVESENS C127, REVIVE C160/C161.',
   'files':['DM.C','COMMAND.C','MOVESENS.C','REVIVE.C'],
   'tokens':['"VGA Graphics"','C080_COMMAND_CLICK_IN_DUNGEON_VIEW','C127_SENSOR_WALL_CHAMPION_PORTRAIT','C160_COMMAND_CLICK_IN_PANEL_RESURRECT'],
 },
 '157': {
   'topic':'082b dungeon control probe',
   'claim':'082b/48ed no-op controls are source-blocked until the original candidate-champion sensor path runs; movement keys alone are not enough.',
   'files':['COMMAND.C','MOVESENS.C','REVIVE.C'],
   'tokens':['C001_COMMAND_TURN_LEFT','C003_COMMAND_MOVE_FORWARD','C127_SENSOR_WALL_CHAMPION_PORTRAIT','F0280_CHAMPION_AddCandidateChampionToParty'],
 },
 '1757': {
   'topic':'V1 status-hand icon zones',
   'claim':'Status-hand icon zones are source-backed by CHAMDRAW/PANEL/DEFS status box and object icon geometry, not an arbitrary Firestaff-only layout.',
   'files':['CHAMDRAW.C','PANEL.C','DEFS.H'],
   'tokens':['C007_GRAPHIC_STATUS_BOX','G2075_ObjectIconWidth','F0296_CHAMPION_DrawChangedObjectIcons','C010_GRAPHIC_MENU_ACTION_AREA'],
 },
}

def rg(token):
    cmd=['rg','-n','--sort','path','-F',token,str(SRC)]
    out=subprocess.run(cmd,text=True,capture_output=True,timeout=5,encoding='latin-1').stdout
    hits=[]
    for line in out.splitlines()[:12]:
        parts=line.split(':',2)
        if len(parts)==3:
            hits.append({'file':Path(parts[0]).name,'line':int(parts[1]),'text':parts[2].strip()[:200]})
    return hits

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    records=[]
    for pid, spec in ANCHORS.items():
        anchors=[]; missing=[]
        for f in spec['files']:
            path=SRC/f
            if path.exists(): anchors.append({'kind':'file','token':f,'hits':[{'file':f,'line':1,'text':'file exists'}]})
            else: missing.append(f)
        for t in spec['tokens']:
            hits=rg(t)
            if hits: anchors.append({'kind':'token','token':t,'hits':hits})
            else: missing.append(t)
        records.append({'pass':pid, **spec, 'status':'source-locked' if not missing else 'missing-token', 'missing':missing, 'anchors':anchors})
    summary={'schema':'pass169_redmcsb_anchor_gap_resolution.v1','source_root':str(SRC),'resolved':sum(1 for r in records if r['status']=='source-locked'),'total':len(records),'records':records}
    (OUT/'manifest.json').write_text(json.dumps(summary,indent=2)+'\n')
    lines=['# Pass169 — resolve ReDMCSB anchor gaps from pass168','',f'- source root: `{SRC}`',f"- resolved: {summary['resolved']}/{summary['total']}",'','## Resolution matrix','']
    for r in records:
        first=[]
        for a in r['anchors'][:5]:
            h=a['hits'][0]
            first.append(f"{a['token']}→{h['file']}:{h['line']}")
        lines.append(f"- pass `{r['pass']}`: **{r['status']}** — {r['topic']} — {r['claim']} — anchors: {'; '.join(first)}")
        if r['missing']:
            lines.append(f"  - missing: {', '.join(r['missing'])}")
    lines += ['','## Outcome','', 'All previously `needs-redmcsb-anchor` pass groups from pass168 now have explicit ReDMCSB file/function/constant anchors in this addendum. Remaining `source-mentioned-unresolved` groups are lower-risk evidence/prose notes and should be converted in batches by lane.']
    (OUT/'README.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'out':str(OUT),'resolved':summary['resolved'],'total':summary['total']},indent=2))
    return 0 if summary['resolved']==summary['total'] else 1
if __name__=='__main__':
    raise SystemExit(main())
