#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window,capture_new,classify_file,tap,click_original
from tools.pass80_original_frame_classifier import sha256
STAGE=Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX="/usr/bin/dosbox"; PROGRAM="DM -vv -sn -pm"
BASE_ROUTE=[("key","Return"),("wait",1.4),("key","F1"),("wait",0.9)]
SCENARIOS={
 "key4_right_long": [("key","4"),("wait",1.2),("key","Up"),("wait",1.0),("key","Right"),("wait",2.0),("key","Right"),("wait",2.0),("key","Left"),("wait",2.0),("key","Down"),("wait",2.0)],
 "return_right_long": [("key","Return"),("wait",1.2),("key","Up"),("wait",1.0),("key","Right"),("wait",2.0),("key","Up"),("wait",2.0),("key","Left"),("wait",2.0)],
 "space_down_long": [("key","Space"),("wait",1.2),("key","Right"),("wait",1.0),("key","Down"),("wait",2.0),("key","Down"),("wait",2.0),("key","Right"),("wait",2.0)],
 "topclick_right_long": [("click",190,8),("click",244,9),("wait",1.2),("key","Up"),("wait",1.0),("key","Right"),("wait",2.0),("key","Right"),("wait",2.0),("key","Left"),("wait",2.0)],
 "panelclick_right_long": [("click",235,52),("click",276,158),("wait",1.2),("key","Up"),("wait",1.0),("key","Right"),("wait",2.0),("key","Left"),("wait",2.0),("key","Down"),("wait",2.0)],
}

def conf(out:Path):
 p=out/'dosbox-pass147.conf'
 p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\n{PROGRAM}\n""")
 return p

def safe(log,label,fn):
 last=None
 for i in range(3):
  try: return fn()
  except Exception as e: last=e; log.append(f"retry {label} {i+1}: {e}"); time.sleep(.25)
 raise RuntimeError(f"failed {label}: {last}")

def shot(out,log,label,idx):
 raw=safe(log,f"capture-{label}",lambda:capture_new(wait_window(log,timeout=5.0),out,label,log))
 dst=out/f"image{idx:04d}-{label}.png"
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst)
 cls,reason=classify_file(dst)
 return {"label":label,"file":dst.name,"sha12":sha256(dst)[:12],"class":cls,"reason":reason}

def do(out,log,a,idx):
 if a[0]=='wait': time.sleep(float(a[1])); return {"phase":"wait","seconds":a[1],**shot(out,log,f"wait_{idx}",idx)}
 if a[0]=='key': safe(log,f"key-{a[1]}",lambda:tap(wait_window(log),a[1],log,delay=.9)); return {"phase":"key","value":a[1],**shot(out,log,f"key_{a[1]}_{idx}",idx)}
 _,x,y=a; safe(log,f"click-{x}-{y}",lambda:click_original(wait_window(log),x,y,log,delay=.9)); return {"phase":"click","x":x,"y":y,**shot(out,log,f"click_{x}_{y}_{idx}",idx)}

def run_one(base,name,actions):
 out=base/name; out.mkdir(parents=True,exist_ok=True); rows=[]; log=[]; idx=1
 proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  wait_window(log); time.sleep(7.0); rows.append({"phase":"initial",**shot(out,log,'initial',idx)}); idx+=1
  for a in BASE_ROUTE+actions: rows.append(do(out,log,a,idx)); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
  (out/'pass147_driver.log').write_text('\n'.join(log)+'\n')
 (out/'pass147_rows.json').write_text(json.dumps(rows,indent=2)+'\n')
 return {"scenario":name,"rows":rows}

def main():
 base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True)
 results=[]; errors=[]
 for n,a in SCENARIOS.items():
  try: results.append(run_one(base,n,a))
  except Exception as e: errors.append({"scenario":n,"error":str(e)})
 summary=[]
 for r in results:
  movement=[x for x in r['rows'] if x.get('value') in {'Up','Down','Left','Right'} or x.get('phase')=='wait']
  hashes=[x.get('sha12') for x in movement]
  summary.append({"scenario":r['scenario'],"unique_hashes":len(set(hashes)),"hashes":hashes,"classes":[x.get('class') for x in movement]})
 (base/'pass147_results.json').write_text(json.dumps({"results":results,"errors":errors,"summary":summary},indent=2)+'\n')
 md=['# Pass 147 — nonstatic 48ed long replay','',f'- run base: `{base}`','- source: pass145 recommended replaying non-static 48ed scenarios with longer waits and movement/action sequences.',f'- completed: {len(results)}',f'- errors: {len(errors)}','','## Movement/wait hash summary','']
 for s in summary: md.append(f"- `{s['scenario']}`: unique_hashes={s['unique_hashes']} hashes={','.join(s['hashes'])} classes={','.join(s['classes'])}")
 md += ['', '## Interpretation', '', 'If these long replays keep producing multiple dungeon hashes after movement, the route is a real dynamic-control candidate; if they collapse back to static menu/dungeon hashes, pivot to Hall-of-Champions/source-backed champion setup.', '']
 if errors:
  md += ['## Errors','']+[f"- `{e['scenario']}`: {e['error']}" for e in errors]
 Path('parity-evidence/pass147_nonstatic_48ed_long_replay.md').write_text('\n'.join(md)+'\n')
if __name__=='__main__': main()
