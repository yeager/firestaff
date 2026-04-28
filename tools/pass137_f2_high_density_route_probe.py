#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original
from tools.pass80_original_frame_classifier import sha256
STAGE = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"
PROGRAMS = {"pk":"DM -vv -sn -pk", "pm":"DM -vv -sn -pm"}
# pass135 identified F2 after enter_only as high-density right/spell area.
BASES = {
  "enter_f2": [("key","Return"),("wait",1.5),("key","F2"),("wait",1.0)],
  "enter_f1_f2": [("key","Return"),("wait",1.5),("key","F1"),("wait",0.8),("key","F2"),("wait",1.0)],
  "enter_f2_return": [("key","Return"),("wait",1.5),("key","F2"),("wait",1.0),("key","Return"),("wait",1.0)],
}
FOLLOWUPS = [("key","1"),("key","2"),("key","3"),("key","4"),("key","F1"),("key","F2"),("key","F3"),("key","F4"),("key","i"),("key","I"),("key","Return"),("key","Space"),("click",235,52),("click",260,62),("click",300,68),("click",260,50),("click",276,140),("click",276,158)]
SCENARIOS=[(pn+"_"+bn, prog, base) for pn,prog in PROGRAMS.items() for bn,base in BASES.items()]

def conf(out: Path, program: str):
  p=out/"dosbox-pass137.conf"
  p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\n{program}\n""")
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

def do(out,log,action,idx):
  if action[0]=="wait":
    time.sleep(float(action[1])); return {"phase":"wait","seconds":action[1],**shot(out,log,f"wait_{idx}",idx)}
  if action[0]=="key":
    safe(log,f"key-{action[1]}",lambda:tap(wait_window(log),action[1],log,delay=.9)); return {"phase":"key","value":action[1],**shot(out,log,f"key_{action[1]}_{idx}",idx)}
  _,x,y=action
  safe(log,f"click-{x}-{y}",lambda:click_original(wait_window(log),x,y,log,delay=.9)); return {"phase":"click","x":x,"y":y,**shot(out,log,f"click_{x}_{y}_{idx}",idx)}

def run_one(base,name,program,actions):
  out=base/name; out.mkdir(parents=True,exist_ok=True)
  rows=[]; log=[]; idx=1
  proc=subprocess.Popen([DOSBOX,"-conf",str(conf(out,program))],stdout=(out/"dosbox.log").open("w"),stderr=subprocess.STDOUT,text=True)
  try:
    wait_window(log); time.sleep(7.0)
    rows.append({"phase":"initial",**shot(out,log,"initial",idx)}); idx+=1
    for a in actions + FOLLOWUPS:
      rows.append(do(out,log,a,idx)); idx+=1
  finally:
    try: proc.terminate(); proc.wait(timeout=2)
    except Exception: proc.kill()
    (out/"pass137_driver.log").write_text("\n".join(log)+"\n")
  (out/"pass137_rows.json").write_text(json.dumps(rows,indent=2)+"\n")
  return {"scenario":name,"program":program,"rows":rows}

def main():
  base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True)
  results=[]; errors=[]
  for name,prog,actions in SCENARIOS:
    try: results.append(run_one(base,name,prog,actions))
    except Exception as e: errors.append({"scenario":name,"program":prog,"error":str(e)})
  interesting=[]
  known_blank={"48ed3743ab6a","fbeb1b82cd09","9fc8530431a3"}
  for r in results:
    for row in r["rows"]:
      if row.get("class") in {"spell_panel","inventory"} or row.get("sha12") not in known_blank:
        interesting.append((r["scenario"],row))
  (base/"pass137_results.json").write_text(json.dumps({"results":results,"errors":errors},indent=2)+"\n")
  md=["# Pass 137 — F2/high-density original route probe","",f"- run base: {base}",f"- scenarios: {len(SCENARIOS)}",f"- completed: {len(results)}",f"- errors: {len(errors)}",f"- interesting rows: {len(interesting)}","","## Interesting rows",""]
  if interesting:
    for sc,row in interesting[:180]:
      val=row.get("value") or row.get("label") or (f"{row.get('x')},{row.get('y')}" if row.get("x") is not None else "")
      md.append(f"- {sc}: {row.get('phase')} {val} -> {row.get('sha12')} {row.get('class')} {row.get('reason')} file {row.get('file')}")
  else:
    md.append("No F2/high-density route escaped known no-party hashes or reached spell/inventory classifier classes.")
  md += ["","## Scenario summary",""]
  for r in results:
    classes=','.join(str(row.get('class','')) for row in r['rows'])
    hashes=','.join(sorted({str(row.get('sha12','')) for row in r['rows']}))
    md.append(f"- {r['scenario']}: program={r['program']} classes={classes} hashes={hashes}")
  if errors:
    md += ["","## Errors",""]
    for e in errors: md.append(f"- {e['scenario']}: {e['error']}")
  Path("parity-evidence/pass137_f2_high_density_route_probe.md").write_text("\n".join(md)+"\n")
if __name__=="__main__": main()
