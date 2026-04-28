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
TARGETS = {
  "mirror_left": (82,94), "mirror_center": (112,86), "mirror_right": (145,94),
  "portrait_left": (72,64), "portrait_center": (112,60), "portrait_right": (152,64),
  "resurrect": (92,165), "reincarnate": (178,165), "right_top": (260,50), "move_cluster": (276,150),
}
ROUTES = {"baseline": [], "turn_left": ["KP_Left"], "turn_right": ["KP_Right"], "about_face": ["KP_Left", "KP_Left"]}
SCENARIOS = []
for rn, keys in ROUTES.items():
  for t in ["mirror_left", "mirror_center", "mirror_right", "portrait_left", "portrait_center", "portrait_right"]:
    SCENARIOS.append((f"{rn}_{t}_single", [("key", k) for k in keys] + [("click", t)]))
    SCENARIOS.append((f"{rn}_{t}_double_return", [("key", k) for k in keys] + [("click", t), ("click", t), ("key", "Return")]))
for rn, keys in ROUTES.items():
  SCENARIOS.append((f"{rn}_mirror_resurrect", [("key", k) for k in keys] + [("click", "mirror_left"), ("click", "resurrect"), ("key", "Return")]))
  SCENARIOS.append((f"{rn}_mirror_reincarnate", [("key", k) for k in keys] + [("click", "mirror_left"), ("click", "reincarnate"), ("key", "Return")]))

def conf(out: Path):
  p = out/"dosbox-pass129.conf"
  p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\nDM -vv -sn -pk\n""")
  return p

def safe(log, label, fn):
  last = None
  for i in range(3):
    try: return fn()
    except Exception as e:
      last = e; log.append(f"retry {label} {i+1}: {e}"); time.sleep(0.25)
  raise RuntimeError(f"failed {label}: {last}")

def shot(out, log, label, idx):
  raw = safe(log, f"capture-{label}", lambda: capture_new(wait_window(log, timeout=4.0), out, label, log))
  target = out/f"image{idx:04d}-{label}.png"
  if target.exists(): target.unlink()
  shutil.move(str(raw), target)
  cls, reason = classify_file(target)
  return {"label": label, "file": target.name, "sha12": sha256(target)[:12], "class": cls, "reason": reason}

def reach_gameplay(out, log):
  safe(log, "start-return", lambda: tap(wait_window(log), "Return", log, delay=1.0))
  rows=[]
  for i in range(1, 16):
    raw = safe(log, f"gate{i}", lambda: capture_new(wait_window(log), out, f"gate{i:02d}", log))
    cls, reason = classify_file(raw)
    dst = out/f"gate{i:02d}_{cls}.png"; shutil.move(str(raw), dst)
    rows.append({"phase":"gate", "step":i, "sha12":sha256(dst)[:12], "class":cls, "reason":reason, "file":dst.name})
    if cls == "dungeon_gameplay": return rows
    if cls == "entrance_menu": safe(log, "gate-return", lambda: tap(wait_window(log), "Return", log, delay=0.8))
    else: time.sleep(0.8)
  raise RuntimeError("no dungeon_gameplay gate")

def act(out, log, action, idx):
  kind, val = action
  if kind == "key":
    safe(log, f"key-{val}", lambda: tap(wait_window(log), val, log, delay=0.9))
    return {"phase":"action", "action":"key", "value":val, **shot(out, log, f"key_{val}_{idx}", idx)}
  x,y = TARGETS[val]
  safe(log, f"click-{val}", lambda: click_original(wait_window(log), x, y, log, delay=0.9))
  return {"phase":"action", "action":"click", "value":val, "x":x, "y":y, **shot(out, log, f"click_{val}_{idx}", idx)}

def run_one(base, name, actions):
  out = base/name; out.mkdir(parents=True, exist_ok=True)
  log=[]; rows=[]; idx=1
  proc = subprocess.Popen([DOSBOX, "-conf", str(conf(out))], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
  try:
    wait_window(log); time.sleep(7.0)
    rows.extend(reach_gameplay(out, log))
    rows.append({"phase":"baseline", **shot(out, log, "baseline", idx)}); idx += 1
    for a in actions:
      rows.append(act(out, log, a, idx)); idx += 1
    for key in ["F1", "F2", "F3", "F4", "i", "I", "Return"]:
      rows.append(act(out, log, ("key", key), idx)); idx += 1
  finally:
    try: proc.terminate(); proc.wait(timeout=2)
    except Exception: proc.kill()
    (out/"pass129_driver.log").write_text("\n".join(log)+"\n")
  (out/"pass129_rows.json").write_text(json.dumps(rows, indent=2)+"\n")
  return {"scenario": name, "rows": rows}

def main():
  base = Path(sys.argv[1]); base.mkdir(parents=True, exist_ok=True)
  results=[]; errors=[]
  for name, actions in SCENARIOS:
    try: results.append(run_one(base, name, actions))
    except Exception as e: errors.append({"scenario":name, "error":str(e)})
  (base/"pass129_results.json").write_text(json.dumps({"results":results,"errors":errors}, indent=2)+"\n")
  known = {"48ed3743ab6a", "fbeb1b82cd09", "9fc8530431a3", "6d20a6fee397"}
  interesting=[]
  for r in results:
    for row in r["rows"]:
      if row.get("phase") == "gate": continue
      if row.get("sha12") not in known or row.get("class") not in {"dungeon_gameplay", "wall_closeup", "graphics_320x200_unclassified"}:
        interesting.append((r["scenario"], row))
  md=["# Pass 129 — DM1 V1 original champion route atlas", "", f"- run base: {base}", f"- scenarios: {len(SCENARIOS)}", f"- completed: {len(results)}", f"- errors: {len(errors)}", f"- interesting rows: {len(interesting)}", "", "## Interesting rows", ""]
  if interesting:
    for scenario,row in interesting[:120]:
      md.append(f"- {scenario}: {row.get('action') or row.get('phase')} {row.get('value') or row.get('label')} -> {row.get('sha12')} {row.get('class')} {row.get('reason')} file {row.get('file')}")
  else:
    md.append("No scenario escaped the known no-party dungeon/wall/unclassified signatures. Remaining blocker is original-route semantics; next pass should test source/asset-backed champion hall coordinates or startup variants without -pk.")
  md += ["", "## Scenario summary", ""]
  for r in results:
    uniq=sorted({row.get("sha12") for row in r["rows"] if row.get("phase") != "gate"})
    md.append(f"- {r['scenario']}: unique_hashes={len(uniq)} hashes={','.join(uniq[:8])}")
  if errors:
    md += ["", "## Errors", ""]
    for e in errors: md.append(f"- {e['scenario']}: {e['error']}")
  Path("parity-evidence/pass129_original_champion_route_atlas.md").write_text("\n".join(md)+"\n")
if __name__ == "__main__": main()
