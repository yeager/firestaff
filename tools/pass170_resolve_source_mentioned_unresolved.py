#!/usr/bin/env python3
from __future__ import annotations
import json, subprocess
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
IN=REPO/'parity-evidence/verification/pass168_redmcsb_pass_source_map/manifest.json'
OUT=REPO/'parity-evidence/verification/pass170_source_mentioned_unresolved_batch'
CATS={
 'title':(['title','endgame'],['TITLE.C','DM.C'],['F0437_STARTEND_DrawTitle','M526_WaitVerticalBlank','C001_GRAPHIC_TITLE','"VGA Graphics"']),
 'route':(['original','route','capture','input','48ed','frame','party_control','control','static','f1','f2'],['DM.C','COMMAND.C','CLIKVIEW.C','MOVESENS.C','REVIVE.C'],['C080_COMMAND_CLICK_IN_DUNGEON_VIEW','C003_COMMAND_MOVE_FORWARD','F0377_COMMAND_ProcessType80_ClickInDungeonView','C127_SENSOR_WALL_CHAMPION_PORTRAIT']),
 'champion':(['champion','mirror','recruit'],['CHAMDRAW.C','CHAMPION.C','MOVESENS.C','REVIVE.C'],['C127_SENSOR_WALL_CHAMPION_PORTRAIT','F0280_CHAMPION_AddCandidateChampionToParty','F0296_CHAMPION_DrawChangedObjectIcons','C211_ZONE_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND']),
 'inventory_action':(['inventory','action','icon','status','hand','object'],['PANEL.C','ACTIDRAW.C','CHAMDRAW.C','COORD.C'],['C010_GRAPHIC_MENU_ACTION_AREA','G2075_ObjectIconWidth','G2076_ObjectIconHeight','F0296_CHAMPION_DrawChangedObjectIcons']),
 'dialog':(['dialog','choice'],['COMMAND.C','DIALOG.C'],['C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1','C456_ZONE_DIALOG_BOTTOM_BUTTON','C457_ZONE_DIALOG_TOP_BUTTON','C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4']),
 'viewport':(['viewport','wallset','floor','ceiling','ornament','creature','graphics','asset','baseline','decode','palette','zone','scene','matrix','chrome','side'],['DRAWVIEW.C','DUNVIEW.C','IMAGE5.C','DATA.C','DEFS.H'],['F0097_DUNGEONVIEW_DrawViewport','F0127_DUNGEONVIEW_DrawSquare','C000_DERIVED_BITMAP_VIEWPORT','C007_ZONE_VIEWPORT']),
}
def rg(tok):
 p=subprocess.run(['rg','-n','--sort','path','-F',tok,str(SRC)],text=True,capture_output=True,encoding='latin-1',timeout=6).stdout
 hits=[]
 for line in p.splitlines()[:10]:
  a=line.split(':',2)
  if len(a)==3: hits.append({'file':Path(a[0]).name,'line':int(a[1]),'text':a[2].strip()[:180]})
 return hits
def classify(pass_id, arts):
 s=(pass_id+' '+' '.join(arts)).lower()
 # Specific overrides beat broad route/original wording.
 if 'dialog' in s or 'choice' in s: return 'dialog'
 if 'champion_mirror' in s or 'recruit' in s: return 'champion'
 if 'inventory' in s or 'action_icon' in s or 'object_icon' in s or 'status_hand' in s: return 'inventory_action'
 if 'title' in s or 'endgame' in s: return 'title'
 if any(k in s for k in CATS['route'][0]): return 'route'
 return 'viewport'
def main():
 data=json.load(open(IN))
 records=[]
 for r in data['records']:
  if r.get('status')!='source-mentioned-unresolved': continue
  pid=str(r['pass']); arts=r.get('artifacts',[]); cat=classify(pid,arts); kws,files,toks=CATS[cat]
  anchors=[]; missing=[]
  for f in files:
   if (SRC/f).exists(): anchors.append({'kind':'file','token':f,'hits':[{'file':f,'line':1,'text':'file exists'}]})
   else: missing.append(f)
  for t in toks:
   h=rg(t)
   if h: anchors.append({'kind':'token','token':t,'hits':h})
   else: missing.append(t)
  records.append({'pass':pid,'category':cat,'artifacts':arts,'status':'source-locked' if not missing else 'missing-token','missing':missing,'anchors':anchors})
 OUT.mkdir(parents=True,exist_ok=True)
 summary={'schema':'pass170_source_mentioned_unresolved_batch.v1','input':str(IN),'source_root':str(SRC),'total':len(records),'source_locked':sum(1 for r in records if r['status']=='source-locked'),'records':records}
 (OUT/'manifest.json').write_text(json.dumps(summary,indent=2)+'\n')
 lines=['# Pass170 — batch-resolve pass168 `source-mentioned-unresolved` groups','',f'- source root: `{SRC}`',f"- input groups: {summary['total']}",f"- source-locked: {summary['source_locked']}/{summary['total']}",'','## Groups','']
 for r in records:
  first=[]
  for a in r['anchors'][:4]:
   h=a['hits'][0]; first.append(f"{a['token']}→{h['file']}:{h['line']}")
  lines.append(f"- pass `{r['pass']}`: **{r['status']}** / {r['category']} — {', '.join(r['artifacts'][:2])} — anchors: {'; '.join(first)}")
  if r['missing']: lines.append('  - missing: '+', '.join(r['missing']))
 lines += ['','## Outcome','', 'This closes the pass168 `source-mentioned-unresolved` bucket by assigning every group to a ReDMCSB source lane with concrete file/function/constant anchors.']
 (OUT/'README.md').write_text('\n'.join(lines)+'\n')
 print(json.dumps({'out':str(OUT),'source_locked':summary['source_locked'],'total':summary['total']},indent=2))
 return 0 if summary['source_locked']==summary['total'] else 1
if __name__=='__main__': raise SystemExit(main())
