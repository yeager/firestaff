#!/usr/bin/env python3
"""Pass121: build a small original DM1 keypad route atlas after pass120 found KP_Left/KP_Right visual transitions."""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap
from tools.pass80_original_frame_classifier import sha256

STAGE = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"


def current_window(log):
    return wait_window(log, timeout=4.0)

def safe(log, label, fn):
    last=None
    for i in range(3):
        try: return fn()
        except Exception as e:
            last=e; log.append(f"retry {label} {i+1}: {e}"); time.sleep(0.25)
    raise RuntimeError(f"failed {label}: {last}")

def conf(out: Path):
    p=out/"dosbox-pass121.conf"
    p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\nDM -vv -sn -pk\n""")
    return p

def run_sequence(base: Path, label: str, keys: list[str]):
    out=base/label; out.mkdir(parents=True, exist_ok=True); log=[]; rows=[]
    proc=subprocess.Popen([DOSBOX,"-conf",str(conf(out))], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wid=wait_window(log); time.sleep(7.0)
        safe(log,"start-return",lambda: tap(current_window(log),"Return",log,delay=1.0))
        got=False
        for i in range(1,16):
            p=safe(log,f"gatecap{i}",lambda: capture_new(current_window(log),out,f"gate{i:02d}",log))
            cls,reason=classify_file(p); dst=out/f"gate{i:02d}_{cls}.png"; shutil.move(str(p),dst)
            rows.append({"phase":"gate","step":i,"class":cls,"sha12":sha256(dst)[:12],"reason":reason,"file":dst.name})
            if cls=="dungeon_gameplay": got=True; break
            if cls=="entrance_menu": safe(log,"gate-return",lambda: tap(current_window(log),"Return",log,delay=0.8))
            else: time.sleep(0.8)
        if not got: raise RuntimeError("no dungeon_gameplay gate")
        # baseline then sequence; finally F1/F4 probes to see if any route changes party/spell/inventory response
        shot=1
        p=safe(log,"baseline",lambda: capture_new(current_window(log),out,"baseline",log)); target=out/f"image{shot:04d}-raw.png"; shutil.move(str(p),target)
        cls,reason=classify_file(target); rows.append({"phase":"probe","step":0,"key":"baseline","class":cls,"sha12":sha256(target)[:12],"reason":reason,"file":target.name}); shot+=1
        for key in keys + ["F1","F4"]:
            safe(log,f"key-{key}",lambda key=key: tap(current_window(log),key,log,delay=1.1))
            p=safe(log,f"capture-{key}",lambda key=key: capture_new(current_window(log),out,f"after_{key}",log)); target=out/f"image{shot:04d}-raw.png"; shutil.move(str(p),target)
            cls,reason=classify_file(target); rows.append({"phase":"probe","step":shot-1,"key":key,"class":cls,"sha12":sha256(target)[:12],"reason":reason,"file":target.name}); shot+=1
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception: proc.kill()
        (out/"pass121_driver.log").write_text("\n".join(log)+"\n")
    (out/"pass121_rows.json").write_text(json.dumps(rows,indent=2)+"\n")
    return {"label":label,"keys":keys,"rows":rows}


def main():
    base=Path(sys.argv[1]); base.mkdir(parents=True, exist_ok=True)
    seqs={
      "left_once":["KP_Left"], "right_once":["KP_Right"], "left_twice":["KP_Left","KP_Left"], "right_twice":["KP_Right","KP_Right"],
      "left_right":["KP_Left","KP_Right"], "right_left":["KP_Right","KP_Left"],
      "left_up":["KP_Left","KP_Up"], "right_up":["KP_Right","KP_Up"], "left_down":["KP_Left","KP_Down"], "right_down":["KP_Right","KP_Down"],
      "left_left_up":["KP_Left","KP_Left","KP_Up"], "right_right_up":["KP_Right","KP_Right","KP_Up"],
      "right_left_up":["KP_Right","KP_Left","KP_Up"], "left_right_up":["KP_Left","KP_Right","KP_Up"],
    }
    results=[]
    for label,keys in seqs.items():
        try: results.append(run_sequence(base,label,keys))
        except Exception as e: results.append({"label":label,"keys":keys,"error":str(e)})
    (base/"pass121_results.json").write_text(json.dumps(results,indent=2)+"\n")
    md=["# Pass 121 — original keypad route atlas", "", f"- run base: `{base}`", "- scope: use pass120 discovery (`KP_Left`/`KP_Right` change original viewport) to map short keypad sequences and F1/F4 response after each route.", "", "## Summary", ""]
    for r in results:
        if r.get("error"):
            md.append(f"- `{r['label']}` ERROR `{r['error']}`")
            continue
        probes=[x for x in r['rows'] if x.get('phase')=='probe']
        sig=" -> ".join(f"{x.get('key')}:{x.get('sha12')}:{x.get('class')}" for x in probes)
        changed=len({x.get('sha12') for x in probes})>1
        md.append(f"- `{r['label']}` keys `{r['keys']}` changed={changed}: {sig}")
    md += ["", "## Interpretation", "", "This is route-atlas evidence only. It is successful only if a sequence produces non-repeating original gameplay states and/or changes F1/F4 away from the no-party response; otherwise the champion/party-ready blocker remains upstream of overlay parity.", ""]
    Path("parity-evidence/pass121_original_keypad_route_atlas.md").write_text("\n".join(md)+"\n")

if __name__=='__main__': main()
