#!/usr/bin/env python3
"""Pass173/pass4: gate into dungeon gameplay before a source C127 probe.

The old pass clicked x=111,y=82 before the entrance gate had reached dungeon view.
The static ReDMCSB map decode predicted an initial C127 heading, but the retail PC
3.4 first frame contradicts that prediction.  This pass gates on a real dungeon
frame, then tests each source turn heading without moving before it considers a
C160/C161 choice.
"""
from __future__ import annotations
import json, os, shutil, subprocess, sys, tempfile, time
from pathlib import Path
from typing import Any
from PIL import Image, ImageChops, ImageStat

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, classify_file, tap, click_original  # noqa: E402
from tools.pass80_original_frame_classifier import sha256  # noqa: E402

DATA_ROOT = Path(os.environ.get("FIRESTAFF_DATA_ROOT", str(Path.home() / ".firestaff" / "data")))
DM1_DOS_ARCHIVE = Path(os.environ.get(
    "FIRESTAFF_DM1_DOS_ARCHIVE",
    str(DATA_ROOT / "dm1" / "Dungeon-Master_DOS_EN_Version-34.zip"),
))
# Original DOS evidence uses DOSBox-X deliberately.  Generic DOSBox builds do
# not have equivalent startup/input behaviour for this route.
DOSBOX = os.environ.get("FIRESTAFF_DOSBOX", shutil.which("dosbox-x") or "/usr/bin/dosbox-x")
# Original captures and their summaries are local evidence only.  Never default
# to a tracked documentation tree: retail frames must not be copied to Git.
OUT_ROOT = Path(os.environ.get(
    "FIRESTAFF_PASS173_OUTPUT_ROOT",
    str(REPO / ".codex-scratch" / "pass173-source-portrait-route-probe"),
))
DEFAULT_RUN_BASE_ROOT = REPO / ".codex-scratch" / "pass173-runs"
RUN_BASE_ROOT = Path(os.environ.get("FIRESTAFF_PASS173_RUN_BASE", os.environ.get("FIRESTAFF_ARTIFACT_ROOT", str(DEFAULT_RUN_BASE_ROOT))))
STATIC_NO_PARTY_HASHES={"48ed3743ab6a","082b4d249740"}
PANEL_TRANSITION_MIN_CHANGED_RATIO = 0.02
CROPS={"viewport":(0,0,224,136),"right_panel":(224,0,320,136),"lower_panel":(0,136,320,200),"candidate_buttons":(70,80,225,148)}
SOURCE_ROOT="reference/redmcsb-20210206/Toolchains/Common/Source"
SOURCE_LOCKS=[
 {"file":"DUNGEON.DAT via pass4 helper","lines":"n/a","point":"the static decode predicts map0 x=1 y=3 dir=South and C127 sensor 16 at x=1 y=4. The retail first frame is the authority when it disagrees, so all four turn headings are probed before C160/C161."},
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
# The retail first dungeon frame establishes that the old fixed-heading
# precondition was wrong for this particular PC 3.4 archive.  These are the
# original lower-panel turn controls (not host key bindings); probing all four
# headings changes neither square nor party state.
PORTRAIT_PROBE_POINT=(111,82)
# C069 is the top-right curved arrow.  The lower-right arrow is C004 MOVE
# RIGHT; using it was an accidental strafe probe, not a heading probe.
TURN_RIGHT_POINT=(307,136)

def slug(s:str)->str: return ''.join(c.lower() if c.isalnum() else '_' for c in s).strip('_')

def published_path(path: str | Path) -> str:
 """Return a non-host-specific local evidence label."""
 p = Path(path)
 try:
  return p.resolve().relative_to(REPO.resolve()).as_posix()
 except ValueError:
  return f"<ephemeral>/{p.name}"

def published_result(result: dict[str, Any], evidence_dir: Path) -> dict[str, Any]:
 """Strip capture-host paths before retaining a local evidence receipt."""
 public = json.loads(json.dumps(result))
 public["evidence_dir"] = published_path(evidence_dir)
 for row in public.get("rows", []):
  if "path" in row:
   row["path"] = f"{public['evidence_dir']}/{row['file']}"
 return public

def capture_new(wid: str, out_dir: Path, label: str, log: list[str]) -> Path:
 """Capture the visible DOSBox canvas without relying on emulator hotkeys.

 DOSBox-X does not expose the Ctrl+F5 binding used by the historic driver.
 Host capture is development-only and crops menu chrome before reducing the
 native 4:3 content to the original 320x200 coordinate space.
 """
 raw = out_dir / f"host-window-{time.monotonic_ns()}.png"
 captured = out_dir / f"host-canvas-{time.monotonic_ns()}.png"
 subprocess.run(["scrot", "--window", wid, "--overwrite", "--silent", str(raw)], check=True)
 im = Image.open(raw).convert("RGB")
 width, height = im.size
 content_width = width
 content_height = round(content_width * 200 / 320)
 if content_height > height:
  content_height = height
  content_width = round(content_height * 320 / 200)
 left = (width - content_width) // 2
 top = height - content_height
 resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
 im.crop((left, top, left + content_width, top + content_height)).resize(
  (320, 200), resample).save(captured)
 raw.unlink()
 log.append(f"host-capture {label} {captured.name} sha={sha256(captured)[:12]}")
 return captured

def wait_process_window(pid: int, log: list[str], timeout: float = 8.0) -> str:
 """Find the window owned by this run, not another active DOSBox session."""
 deadline = time.time() + timeout
 while time.time() < deadline:
  probe = subprocess.run(["xdotool", "search", "--pid", str(pid)],
                         capture_output=True, text=True)
  ids = [item for item in probe.stdout.split() if item]
  if ids:
   wid = ids[-1]
   subprocess.run(["xdotool", "windowactivate", wid], check=False,
                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
   log.append(f"window-found {wid} for-pid {pid}")
   return wid
  time.sleep(0.2)
 raise RuntimeError(f"no DOSBox window found for capture process {pid}")
def stage_original_media(stage_root: Path)->Path:
 if not DM1_DOS_ARCHIVE.is_file(): raise RuntimeError(f"missing original archive: {DM1_DOS_ARCHIVE}")
 stage_root.mkdir(parents=True, exist_ok=True)
 stage=Path(tempfile.mkdtemp(prefix="firestaff-pass173-dm1-",dir=stage_root))
 result=subprocess.run(["unzip","-qq",str(DM1_DOS_ARCHIVE),"-d",str(stage)],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 if not (stage/"DM.EXE").is_file() or not (stage/"DATA/GRAPHICS.DAT").is_file():
  shutil.rmtree(stage,ignore_errors=True)
  raise RuntimeError("unable to stage complete original PC34 archive in the selected scratch root")
 return stage

def conf(out:Path,stage:Path)->Path:
 # -pm is source program mode for the mouse route below.  Without it an
 # Entrance transition can be visible while later original-space clicks are
 # not consumed by the dungeon loop.
 p=out/"dosbox-pass173.conf"; p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nquit warning=false\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{stage}\"\nc:\nDM -vv -sn -pm\n"""); return p

def shot(out:Path,log:list[str],wid:str,label:str,idx:int)->dict[str,Any]:
 raw=capture_new(wid,out,label,log); dst=out/f"image{idx:04d}-{slug(label)}.png"
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

def click(out:Path,log:list[str],wid:str,x:int,y:int,label:str,idx:int)->dict[str,Any]:
 click_original(wid,x,y,log,delay=0.8); return {"phase":"click","x":x,"y":y,**shot(out,log,wid,label,idx)}
def key(out:Path,log:list[str],wid:str,k:str,label:str,idx:int)->dict[str,Any]:
 tap(wid,k,log,delay=0.8); return {"phase":"key","value":k,**shot(out,log,wid,label,idx)}

def gate_to_gameplay(out:Path,log:list[str],wid:str,rows:list[dict[str,Any]],idx:int)->int:
 deadline=time.time()+24; attempt=0
 while time.time()<deadline:
  attempt+=1; p=shot(out,log,wid,f"gate{attempt:02d}",idx); p["phase"]="gate"; p["crop_stats"]=crop_stats(Path(p["path"])); rows.append(p); idx+=1
  if p["class"]=="dungeon_gameplay": return idx
  if attempt == 1:
   # The title frame is classified conservatively by the generic classifier;
   # advance it exactly once before touching the Entrance control.
   tap(wid,"Return",log,delay=6.0)
  elif attempt == 2:
   # PC 3.4 consumes the green Enter glyph (not the adjacent label) as the
   # Entrance control.  The label point 260,50 is a real no-op on the retail
   # surface; use the centre of the source button at 245,51.
   # Do not send further key input after this handoff: it could move the
   # party away from the C127 route.
   click_original(wid,245,51,log,delay=6.0)
  else: time.sleep(0.8)
 raise RuntimeError("state gate never observed dungeon_gameplay")

def classify(rows:list[dict[str,Any]])->tuple[str,str,dict[str,Any]]:
 shots=[r for r in rows if 'sha12' in r]; hashes=[r['sha12'] for r in shots]
 diffs=[{"from":a['label'],"to":b['label'],**diff_stats(Path(a['path']),Path(b['path']))} for a,b in zip(shots,shots[1:])]
 static=sorted(set(hashes)&STATIC_NO_PARTY_HASHES); portrait=[d for d in diffs if d['to'].startswith('after_portrait_click')]; choice=[d for d in diffs if d['to'] in ('after_c160_resurrect','after_c161_reincarnate')]
 ev={"hashes":hashes,"unique_hashes":sorted(set(hashes)),"classes":[r['class'] for r in shots],"static_hits":static,"diffs":diffs,"portrait_click_delta":portrait,"choice_delta":choice}
 if not any(r['label'].startswith('gate') and r['class']=='dungeon_gameplay' for r in shots): return 'blocked/no-gated-gameplay','never reached gated dungeon gameplay',ev
 if static: return 'blocked/static-no-party-after-gate',f"known no-party hash present after gate: {', '.join(static)}",ev
 # The DOS pointer is part of the original framebuffer, so its movement can
 # change roughly one hundred pixels.  A candidate panel must change far more
 # than that; otherwise a cursor-only delta would become false evidence.
 if not portrait or max(d['changed_ratio'] for d in portrait)<PANEL_TRANSITION_MIN_CHANGED_RATIO: return 'blocked/portrait-click-no-visible-delta','no gated heading produced a panel-scale candidate transition',ev
 if not choice or choice[0]['changed_ratio']<PANEL_TRANSITION_MIN_CHANGED_RATIO: return 'blocked/choice-no-visible-delta','C160/C161 choice did not produce a panel-scale visible transition',ev
 return 'candidate-transition-visible','gated dungeon portrait click and choice both produced visible transitions',ev

def run_one(base:Path,stage:Path,name:str,cx:int,cy:int,choice_name:str)->dict[str,Any]:
 out=base/slug(name); out.mkdir(parents=True,exist_ok=True); log=[]; rows=[]; proc=subprocess.Popen([DOSBOX,'-exit','-set','dosbox quit warning=false','-conf',str(conf(out,stage))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  wid=wait_process_window(proc.pid,log,timeout=8.0); time.sleep(7.0); idx=1
  rows.append({"phase":"initial",**shot(out,log,wid,'initial',idx)}); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  idx=gate_to_gameplay(out,log,wid,rows,idx)
  candidate_visible=False
  for heading in range(4):
   before=Path(rows[-1]['path'])
   rows.append(click(out,log,wid,*PORTRAIT_PROBE_POINT,
                     f'after_portrait_click_heading_{heading}',idx))
   rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
   if diff_stats(before,Path(rows[-1]['path']))['changed_ratio'] >= PANEL_TRANSITION_MIN_CHANGED_RATIO:
    candidate_visible=True
    break
   if heading < 3:
    rows.append(click(out,log,wid,*TURN_RIGHT_POINT,
                      f'after_turn_right_{heading}',idx))
    rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  if candidate_visible:
   rows.append(click(out,log,wid,cx,cy,'after_'+slug(choice_name),idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1; time.sleep(0.8)
  rows.append(key(out,log,wid,'Return','after_confirm_return',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  rows.append(key(out,log,wid,'F1','after_f1_probe',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
  rows.append(key(out,log,wid,'F4','after_f4_probe',idx)); rows[-1]['crop_stats']=crop_stats(Path(rows[-1]['path'])); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
  (out/'pass173_driver.log').write_text('\n'.join(log)+'\n')
 status,reason,ev=classify(rows); summary={"name":name,"choice":choice_name,"classification":status,"reason":reason,"source_locks":SOURCE_LOCKS,"route_precondition":{"map":0,"party":{"x":1,"y":3,"dir":"South"},"front_wall_sensor":{"x":1,"y":4,"sensor":16,"type":"C127_SENSOR_WALL_CHAMPION_PORTRAIT"},"click":{"x":111,"y":82}},"route_evidence":ev,"rows":rows,"evidence_dir":str(out)}
 (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n'); return summary

def main()->int:
 OUT_ROOT.mkdir(parents=True,exist_ok=True); RUN_BASE_ROOT.mkdir(parents=True,exist_ok=True); run_base=RUN_BASE_ROOT/(time.strftime('%Y%m%d-%H%M%S')+'-pass173-source-portrait-route-gate-probe'); run_base.mkdir(parents=True,exist_ok=True); stage=stage_original_media(RUN_BASE_ROOT / "staging")
 results=[]; errors=[]
 for name,cx,cy,choice in SCENARIOS:
  try:
   r=run_one(run_base,stage,name,cx,cy,choice)
   # Keep the sanitised receipt adjacent to the original local captures.  It
   # deliberately does not copy any retail frame into a tracked tree.
   ev=Path(r['evidence_dir'])
   public=published_result(r, ev)
   (ev/'summary.json').write_text(json.dumps(public,indent=2)+'\n')
   results.append(public)
  except Exception as e: errors.append({"scenario":name,"error":str(e)})
 buckets={}
 for r in results: buckets[r['classification']]=buckets.get(r['classification'],0)+1
 shutil.rmtree(stage,ignore_errors=True)
 manifest={"schema":"pass173_source_portrait_route_gate_probe.v3","originalArchive":"caller-supplied original media","staging":"caller-selected scratch root (removed after capture)","evidence_root":published_path(OUT_ROOT),"completed":len(results),"errors":errors,"buckets":buckets,"source_root":SOURCE_ROOT,"source_locks":SOURCE_LOCKS,"results":results}
 (OUT_ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 bucket_text=', '.join(f'{k}={v}' for k,v in sorted(buckets.items())) or 'none'
 lines=["# Pass 173 / pass 4 — gated source portrait route probe","",f"- evidence root: `{published_path(OUT_ROOT)}`",f"- completed: {len(results)}",f"- errors: {len(errors)}",f"- buckets: {bucket_text}",f"- ReDMCSB source root: `{SOURCE_ROOT}`","","## ReDMCSB source audit","", "This pass is source-first. The runtime clicks below are derived from these ReDMCSB anchors, not from emulator guessing.",""]
 for s in SOURCE_LOCKS: lines.append(f"- `{s['file']}:{s['lines']}` — {s['point']}")
 lines += ["","## Route precondition","","- Static decode: map0 x=1 y=3 dir=South; front square x=1 y=4 contains C127.","- Retail observation takes priority over that static hypothesis.","- After the entrance gate, this probe clicks x=111,y=82 once per heading, turning only with the original lower-panel control. It never moves the party.","","## Results",""]
 for r in results: lines.append(f"- `{r['name']}`: **{r['classification']}** — {r['reason']} — `{r['evidence_dir']}`")
 if errors: lines += ["","## Errors",""]+[f"- `{e['scenario']}`: {e['error']}" for e in errors]
 (OUT_ROOT/'README.md').write_text('\n'.join(lines)+'\n')
 print(f"wrote {OUT_ROOT}/README.md"); print(f"run_base={run_base}"); print(f"completed={len(results)} errors={len(errors)} buckets={buckets}")
 return 1 if errors else 0
if __name__=='__main__': raise SystemExit(main())
