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
PROGRAM = "DM -vv -sn -pm"
# Pass137 found pm_enter_f1_f2 reaches an inventory-class frame immediately on F1.
BASE_ROUTE = [("key","Return"),("wait",1.5),("key","F1"),("wait",1.0)]
BRANCHES = {
  "f1_inventory_keys": [("key","1"),("key","2"),("key","3"),("key","4"),("key","Return"),("key","Space"),("key","i"),("key","I")],
  "f1_inventory_fkeys": [("key","F2"),("key","F3"),("key","F4"),("key","F1"),("key","Return")],
  "f1_inventory_clicks_champ_top": [("click",25,8),("click",80,8),("click",135,8),("click",190,8),("click",244,9),("click",285,9)],
  "f1_inventory_clicks_panel": [("click",235,52),("click",260,62),("click",300,68),("click",246,93),("click",275,122),("click",276,158),("click",300,180)],
  "f1_inventory_viewport": [("click",110,68),("key","F2"),("key","Return"),("click",235,52),("click",276,158)],
}

def conf(out: Path):
    p=out/"dosbox-pass139.conf"
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

def do(out,log,action,idx):
    if action[0]=="wait":
        time.sleep(float(action[1])); return {"phase":"wait","seconds":action[1],**shot(out,log,f"wait_{idx}",idx)}
    if action[0]=="key":
        safe(log,f"key-{action[1]}",lambda:tap(wait_window(log),action[1],log,delay=.9)); return {"phase":"key","value":action[1],**shot(out,log,f"key_{action[1]}_{idx}",idx)}
    _,x,y=action
    safe(log,f"click-{x}-{y}",lambda:click_original(wait_window(log),x,y,log,delay=.9)); return {"phase":"click","x":x,"y":y,**shot(out,log,f"click_{x}_{y}_{idx}",idx)}

def run_branch(base,name,actions):
    out=base/name; out.mkdir(parents=True,exist_ok=True)
    rows=[]; log=[]; idx=1
    proc=subprocess.Popen([DOSBOX,"-conf",str(conf(out))],stdout=(out/"dosbox.log").open("w"),stderr=subprocess.STDOUT,text=True)
    try:
        wait_window(log); time.sleep(7.0)
        rows.append({"phase":"initial",**shot(out,log,"initial",idx)}); idx+=1
        for a in BASE_ROUTE + actions:
            rows.append(do(out,log,a,idx)); idx+=1
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception: proc.kill()
        (out/"pass139_driver.log").write_text("\n".join(log)+"\n")
    (out/"pass139_rows.json").write_text(json.dumps(rows,indent=2)+"\n")
    return {"scenario":name,"program":PROGRAM,"rows":rows}

def main():
    base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True)
    results=[]; errors=[]
    for name,actions in BRANCHES.items():
        try: results.append(run_branch(base,name,actions))
        except Exception as e: errors.append({"scenario":name,"error":str(e)})
    interesting=[]
    for r in results:
        for row in r["rows"]:
            cls=row.get("class")
            if cls in {"inventory","dungeon_gameplay","spell_panel","graphics_320x200_unclassified"}:
                interesting.append((r["scenario"],row))
    (base/"pass139_results.json").write_text(json.dumps({"results":results,"errors":errors},indent=2)+"\n")
    md=["# Pass 139 — PM F1 inventory/control route", "", f"- run base: `{base}`", f"- base route: Return → F1 (pass137 inventory hit)", f"- scenarios: {len(BRANCHES)}", f"- completed: {len(results)}", f"- errors: {len(errors)}", f"- interesting rows: {len(interesting)}", "", "## Interesting rows", ""]
    for sc,row in interesting[:220]:
        val=row.get("value") or row.get("label") or (f"{row.get('x')},{row.get('y')}" if row.get("x") is not None else "")
        md.append(f"- `{sc}`: {row.get('phase')} `{val}` -> `{row.get('sha12')}` `{row.get('class')}` — {row.get('reason')} — `{row.get('file')}`")
    md += ["", "## Scenario summary", ""]
    for r in results:
        classes=','.join(str(row.get('class','')) for row in r['rows'])
        hashes=','.join(sorted({str(row.get('sha12','')) for row in r['rows']}))
        md.append(f"- `{r['scenario']}`: classes={classes} hashes={hashes}")
    if errors:
        md += ["", "## Errors", ""]
        for e in errors: md.append(f"- `{e['scenario']}`: {e['error']}")
    Path("parity-evidence/pass139_pm_f1_inventory_control_route.md").write_text("\n".join(md)+"\n")
if __name__ == "__main__": main()
