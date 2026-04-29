#!/usr/bin/env python3
"""Pass173/pass4: gate into dungeon gameplay before source C127 portrait click.

Pass166 pass3 clicked x=111,y=82 before the entrance gate had reached dungeon view.
This pass first proves the initial DM1 V1 party pose is map0 (1,3,S), facing C127
sensor 16 on wall square (1,4), then gates on an actual dungeon_gameplay frame and
only then clicks the ReDMCSB portrait center x=111,y=82 followed by C160/C161.
"""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
from typing import Any
from PIL import Image, ImageChops, ImageStat

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original  # noqa: E402
from tools.pass80_original_frame_classifier import sha256  # noqa: E402

STAGE = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"
OUT_ROOT = Path("parity-evidence/verification/pass173_source_portrait_route_gate_probe")
RUN_BASE_ROOT = Path.home()/".openclaw/data/firestaff-n2-runs"
STATIC_NO_PARTY_HASHES={"48ed3743ab6a","082b4d249740"}
CROPS={"viewport":(0,0,224,136),"right_panel":(224,0,320,136),"lower_panel":(0,136,320,200),"candidate_buttons":(70,80,225,148)}
SOURCE_ROOT="/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
SOURCE_LOCKS=[
 {"file":"DUNGEON.DAT via pass4 helper","lines":"n/a","point":"initial party location decodes to map0 x=1 y=3 dir=South; C127 sensor 16 is on wall square x=1 y=4, so the initial dungeon pose faces a champion portrait sensor."},
 {"file":"COMMAND.C","lines":"397-403,2322-2323","point":"left-click in C007_ZONE_VIEWPORT dispatches C080_COMMAND_CLICK_IN_DUNGEON_VIEW and calls F0377_COMMAND_ProcessType80_ClickInDungeonView."},
 {"file":"CLIKVIEW.C","lines":"348-349,407-431","point":"PC build subtracts viewport origin from screen coordinates; empty-hand click in C05 front-wall ornament/door-button zone calls F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor."},
 {"file":"CLIKVIEW.C","lines":"21-25","point":"F0372 computes the square in front of the party and calls F0275_SENSOR_IsTriggeredByClickOnWall(frontX,frontY,oppositeDirection)."},
 {"file":"MOVESENS.C","lines":"1392,1501-1502","point":"F0275 allows C127_SENSOR_WALL_CHAMPION_PORTRAIT even with no leader, then calls F0280_CHAMPION_AddCandidateChampionToParty(sensorData)."},
 {"file":"REVIVE.C","lines":"63+","point":"F0280_CHAMPION_AddCandidateChampionToParty is the candidate-state creation entrypoint before C160/C161 are meaningful."},
 {"file":"DUNGEON.C","lines":"2558,2608-2612","point":"while drawing wall square aspects, the same C127 sensor data sets G0289_i_DungeonView_ChampionPortraitOrdinal for visible champion portraits."},
 {"file":"DUNVIEW.C","lines":"525,3913-3928","point":"portrait source box is {96,127,35,63}; drawing the D1C front wall copies/uses the C05 clickable zone and blits M635_ZONE_PORTRAIT_ON_WALL."},
 {"file":"COORD.C","lines":"1693-1698","point":"PC viewport origin is x=0,y=33, so viewport portrait center (111,49) maps to screen x=111,y=82."},
 {"file":"COMMAND.C","lines":"231-237,509-510","point":"candidate panel commands are C160/C161; old PC boxes include resurrect/reincarnate ranges around centers x=130,y=115 and x=186,y=115."},
]
SCENARIOS=[("gate_click_portrait_then_resurrect",130,115,"C160 resurrect"),("gate_click_portrait_then_reincarnate",186,115,"C161 reincarnate")]

def slug(s:str)->str: return ''.join(c.lower() if c.isalnum() else '_' for c in s).strip('_')
def conf(out:Path)->Path:
 p=out/"dosbox-pass173.conf"; p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\nDM -vv -sn\n"""); return p

def shot(out:Path,log:list[str],label:str,idx:int)->dict[str,Any]:
 raw=capture_new(wait_window(log,timeout=5.0),out,label,log); dst=out/f"image{idx:04d}-{slug(label)}.png"
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst); cls,reason=classify_file(dst)
 return {"index":idx,"label":label,"file":dst.name,"path":str(dst),"sha12":sha256(dst)[:12],"class":cls,"reason":reason}

def crop_stats(path:Path)->dict[str,Any]:
 im=Image.open(path).convert('RGB'); out={}
 for name,box in CROPS.items():
  cr=im.crop(box); st=ImageStat.Stat(cr); nonblack=sum(1 for px in cr.getdata() if px!=(0,0,0))
  out[name]={"mean_rgb":[round(x,2) for x in st.mean],"nonblack_ratio":round(nonblack/(cr.size[0]*cr.size[1]),6)}
 return out

def diff_stats(a:Path,b:Path)->dict[str,Any]:
 ia,ib=Image.open(a).convert('RGB'),Image.open(b).convert('RGB'); d=ImageChops.difference(ia,ib); bbox=d.getbbox(); nz=sum(1 for px in d.getdata() if px!=(0,0,0))
 return {"bbox":list(bbox) if bbox else None,"changed_pixels":nz,"changed_ratio":round(nz/(320*200),6)}

def click(out:Path,log:list[str],x:int,y:int,label:str,idx:int)->dict[str,Any]:
 wid=wait_window(log,timeout=5.0); click_original(wid,x,y,log,delay=0.8); return {"phase":"click","x":x,"y":y,**shot(out,log,label,idx)}
def key(out:Path,log:list[str],k:str,label:str,idx:int)->dict[str,Any]:
 wid=wait_window(log,timeout=5.0); tap(wid,k,log,delay=0.8); return {"phase":"key","value":k,**shot(out,log,label,idx)}

def gate_to_gameplay(out:Path,log:list[str],rows:list[dict[str,Any]],idx:int)->int:
 wid=wait_window(log,timeout=8.0); deadline=time.time()+20; attempt=0
 while time.time()<deadline:
  attempt+=1; p=shot(out,log,f"gate{attempt:02d}",idx); p["phase"]="gate"; p["crop_stats"]=crop_stats(Path(p["path"])); rows.append(p); idx+=1
  if p["class"]=="dungeon_gameplay": return idx
  if p["class"]=="entrance_menu": tap(wid,"Return",log,delay=1.0)
  else: time.sleep(0.8)
 raise RuntimeError("state gate never observed dungeon_gameplay")

def classify(rows:list[dict[str,Any]])->tuple[str,str,dict[str,Any]]:
 shots=[r for r in rows if 'sha12' in r]; hashes=[r['sha12'] for r in shots]
 diffs=[{"from":a['label'],"to":b['label'],**diff_stats(Path(a['path']),Path(b['path']))} for a,b in zip(shots,shots[1:])]
 static=sorted(set(hashes)&STATIC_NO_PARTY_HASHES); portrait=[d for d in diffs if d['to']=='after_portrait_click']; choice=[d for d in diffs if d['to'] in ('after_c160_resurrect','after_c161_reincarnate')]
 ev={"hashes":hashes,"unique_hashes":sorted(set(hashes)),"classes":[r['class'] for r in shots],"static_hits":static,"diffs":diffs,"portrait_click_delta":portrait,"choice_delta":choice}
 if not any(r['label'].startswith('gate') and r['class']=='dungeon_gameplay' for r in shots): return 'blocked/no-gated-gameplay','never reached gated dungeon gameplay',ev
 if static: return 'blocked/static-no-party-after-gate',f"known no-party hash present after gate: {', '.join(static)}",ev
 if not portrait or portrait[0]['changed_ratio']<0.001: return 'blocked/portrait-click-no-visible-delta','gated portrait click produced no visible candidate transition',ev
 if not choice or choice[0]['changed_ratio']<0.001: return 'blocked/choice-no-visible-delta','C160/C161 choice produced no visible transition',ev
 return 'candidate-transition-visible','gated dungeon portrait click and choice both produced visible transitions',ev

def run_one(base:Path,name:str,cx:int,cy:int,choice_name:str)->dict[str,Any]:
 out=base/slug(name); out.mkdir(parents=True,exist_ok=True); log=[]; rows=[]; proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  wait_window(log,timeout=8.0); time.sleep(7.0); idx=1
  rows.append({"phase":"initial",**shot(out,log,'initial',idx)}); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  idx=gate_to_gameplay(out,log,rows,idx)
  rows.append(click(out,log,111,82,'after_portrait_click',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1; time.sleep(0.8)
  rows.append(click(out,log,cx,cy,'after_'+slug(choice_name),idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1; time.sleep(0.8)
  rows.append(key(out,log,'Return','after_confirm_return',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  rows.append(key(out,log,'F1','after_f1_probe',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  rows.append(key(out,log,'F4','after_f4_probe',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
  (out/'pass173_driver.log').write_text('\n'.join(log)+'\n')
 status,reason,ev=classify(rows); summary={"name":name,"choice":choice_name,"classification":status,"reason":reason,"source_locks":SOURCE_LOCKS,"route_precondition":{"map":0,"party":{"x":1,"y":3,"dir":"South"},"front_wall_sensor":{"x":1,"y":4,"sensor":16,"type":"C127_SENSOR_WALL_CHAMPION_PORTRAIT"},"click":{"x":111,"y":82}},"route_evidence":ev,"rows":rows,"evidence_dir":str(out)}
 (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n'); return summary

def main()->int:
 OUT_ROOT.mkdir(parents=True,exist_ok=True); run_base=RUN_BASE_ROOT/(time.strftime('%Y%m%d-%H%M%S')+'-pass173-source-portrait-route-gate-probe'); run_base.mkdir(parents=True,exist_ok=True)
 results=[]; errors=[]
 for name,cx,cy,choice in SCENARIOS:
  try:
   r=run_one(run_base,name,cx,cy,choice); ev=OUT_ROOT/slug(name)
   if ev.exists(): shutil.rmtree(ev)
   shutil.copytree(Path(r['evidence_dir']),ev); r['evidence_dir']=str(ev); (ev/'summary.json').write_text(json.dumps(r,indent=2)+'\n'); results.append(r)
  except Exception as e: errors.append({"scenario":name,"error":str(e)})
 buckets={}
 for r in results: buckets[r['classification']]=buckets.get(r['classification'],0)+1
 manifest={"schema":"pass173_source_portrait_route_gate_probe.v2","run_base":str(run_base),"evidence_root":str(OUT_ROOT),"completed":len(results),"errors":errors,"buckets":buckets,"source_root":SOURCE_ROOT,"source_locks":SOURCE_LOCKS,"results":results}
 (OUT_ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 lines=["# Pass 173 / pass 4 — gated source portrait route probe","",f"- run base: `{run_base}`",f"- evidence root: `{OUT_ROOT}`",f"- completed: {len(results)}",f"- errors: {len(errors)}",f"- buckets: {', '.join(f'{k}={v}' for k,v in sorted(buckets.items()))}",f"- ReDMCSB source root: `{SOURCE_ROOT}`","","## ReDMCSB source audit","", "This pass is source-first. The runtime clicks below are derived from these ReDMCSB anchors, not from emulator guessing.",""]
 for s in SOURCE_LOCKS: lines.append(f"- `{s['file']}:{s['lines']}` — {s['point']}")
 lines += ["","## Route precondition","","- DM1 V1 initial party: map0 x=1 y=3 dir=South.","- Front wall square: map0 x=1 y=4 contains sensor 16 type C127 wall champion portrait.","- Therefore no movement is required; only entrance gate must be passed before clicking x=111,y=82.","","## Results",""]
 for r in results: lines.append(f"- `{r['name']}`: **{r['classification']}** — {r['reason']} — `{r['evidence_dir']}`")
 if errors: lines += ["","## Errors",""]+[f"- `{e['scenario']}`: {e['error']}" for e in errors]
 (OUT_ROOT/'README.md').write_text('\n'.join(lines)+'\n')
 print(f"wrote {OUT_ROOT}/README.md"); print(f"run_base={run_base}"); print(f"completed={len(results)} errors={len(errors)} buckets={buckets}")
 return 1 if errors else 0
if __name__=='__main__': raise SystemExit(main())
