#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window,capture_new,classify_file,tap,click_original
from tools.pass80_original_frame_classifier import sha256
STAGE=Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX="/usr/bin/dosbox"
PROGRAM="DM -vv -sn -pm"
BASE_ROUTE=[("key","Return"),("wait",1.4),("key","F1"),("wait",0.9)]
# pass139 showed these exits reach dungeon_gameplay hash 48ed after PM Return->F1. This pass tests whether that state accepts movement/action/input as true party control.
SCENARIOS={
 "f1_key4_movement": [("key","4"),("wait",0.6),("key","Up"),("key","Left"),("key","Right"),("key","Down")],
 "f1_return_movement": [("key","Return"),("wait",0.6),("key","Up"),("key","Up"),("key","Right"),("key","Left")],
 "f1_space_movement": [("key","Space"),("wait",0.6),("key","Up"),("key","Right"),("key","Down")],
 "f1_panel_clicks_then_move": [("click",235,52),("click",276,158),("wait",0.6),("key","Up"),("key","Right"),("key","Left")],
 "f1_top_clicks_then_move": [("click",190,8),("click",244,9),("wait",0.6),("key","Up"),("key","Right"),("key","Left")],
}

def conf(out:Path):
 p=out/"dosbox-pass141.conf"
 p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\n{PROGRAM}\n""")
 return p

def safe(log,label,fn):
 last=None
 for i in range(3):
  try: return fn()
  except Exception as e:
   last=e; log.append(f"retry {label} {i+1}: {e}"); time.sleep(.25)
 raise RuntimeError(f"failed {label}: {last}")

def shot(out,log,label,idx):
 raw=safe(log,f"capture-{label}",lambda:capture_new(wait_window(log,timeout=5.0),out,label,log))
 dst=out/f"image{idx:04d}-{label}.png"
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst)
 cls,reason=classify_file(dst)
 return {"label":label,"file":dst.name,"sha12":sha256(dst)[:12],"class":cls,"reason":reason}

def do(out,log,a,idx):
 if a[0]=="wait":
  time.sleep(float(a[1])); return {"phase":"wait","seconds":a[1],**shot(out,log,f"wait_{idx}",idx)}
 if a[0]=="key":
  safe(log,f"key-{a[1]}",lambda:tap(wait_window(log),a[1],log,delay=.85)); return {"phase":"key","value":a[1],**shot(out,log,f"key_{a[1]}_{idx}",idx)}
 _,x,y=a
 safe(log,f"click-{x}-{y}",lambda:click_original(wait_window(log),x,y,log,delay=.85)); return {"phase":"click","x":x,"y":y,**shot(out,log,f"click_{x}_{y}_{idx}",idx)}

def run_one(base,name,actions):
 out=base/name; out.mkdir(parents=True,exist_ok=True)
 rows=[]; log=[]; idx=1
 proc=subprocess.Popen([DOSBOX,"-conf",str(conf(out))],stdout=(out/"dosbox.log").open("w"),stderr=subprocess.STDOUT,text=True)
 try:
  wait_window(log); time.sleep(7.0)
  rows.append({"phase":"initial",**shot(out,log,"initial",idx)}); idx+=1
  for a in BASE_ROUTE+actions:
   rows.append(do(out,log,a,idx)); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
  (out/"pass141_driver.log").write_text("\n".join(log)+"\n")
 (out/"pass141_rows.json").write_text(json.dumps(rows,indent=2)+"\n")
 return {"scenario":name,"rows":rows}

def movement_response(rows):
 moves=[r for r in rows if r.get("value") in {"Up","Down","Left","Right"}]
 hashes=[r.get("sha12") for r in moves]
 return {"movement_rows":len(moves),"movement_unique_hashes":len(set(hashes)),"movement_hashes":hashes,"party_control_candidate":len(set(hashes))>1 or any(h!="48ed3743ab6a" for h in hashes)}

def main():
 base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True)
 results=[]; errors=[]
 for name,actions in SCENARIOS.items():
  try:
   r=run_one(base,name,actions); r["movement_response"]=movement_response(r["rows"]); results.append(r)
  except Exception as e: errors.append({"scenario":name,"error":str(e)})
 (base/"pass141_results.json").write_text(json.dumps({"results":results,"errors":errors},indent=2)+"\n")
 md=["# Pass 141 — PM F1 dungeon control readiness", "", f"- run base: `{base}`", "- route base: DM `-pm`, Return → F1, then a pass139-proven dungeon exit, then movement keys", f"- scenarios: {len(SCENARIOS)}", f"- completed: {len(results)}", f"- errors: {len(errors)}", "", "## Movement response summary", ""]
 for r in results:
  mr=r["movement_response"]
  md.append(f"- `{r['scenario']}`: candidate={mr['party_control_candidate']} movement_unique_hashes={mr['movement_unique_hashes']} movement_hashes={','.join(mr['movement_hashes'])}")
 md += ["", "## Rows", ""]
 for r in results:
  md.append(f"### {r['scenario']}")
  for row in r["rows"]:
   val=row.get("value") or row.get("label") or (f"{row.get('x')},{row.get('y')}" if row.get("x") is not None else "")
   md.append(f"- {row.get('phase')} `{val}` -> `{row.get('sha12')}` `{row.get('class')}` — {row.get('reason')} — `{row.get('file')}`")
 if errors:
  md += ["", "## Errors", ""]
  for e in errors: md.append(f"- `{e['scenario']}`: {e['error']}")
 Path("parity-evidence/pass141_pm_f1_dungeon_control_readiness.md").write_text("\n".join(md)+"\n")
if __name__=="__main__": main()
