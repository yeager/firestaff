#!/usr/bin/env python3
"""Pass122: consolidated DM1 V1 original route solver.

Builds on pass118-121: after the state-aware gate, run larger route families
using the only proven responsive keys (KP_Left/KP_Right) plus candidate forward,
panel, inventory, champion, and click probes. Evidence only.
"""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original
from tools.pass80_original_frame_classifier import sha256

STAGE = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"

ROUTE_FAMILIES = {
    "baseline": [],
    "left": ["KP_Left"],
    "right": ["KP_Right"],
    "left_left": ["KP_Left", "KP_Left"],
    "right_right": ["KP_Right", "KP_Right"],
    "left_right": ["KP_Left", "KP_Right"],
    "right_left": ["KP_Right", "KP_Left"],
    "left_left_right": ["KP_Left", "KP_Left", "KP_Right"],
    "right_right_left": ["KP_Right", "KP_Right", "KP_Left"],
}

PANEL_KEYS = ["F1", "F2", "F3", "F4", "1", "2", "3", "4", "5", "6", "i", "I", "Space", "Return", "Escape"]
CLICK_PROBES = [
    ("right_top", 260, 50), ("move_mid", 276, 140), ("right_mid", 304, 140),
    ("left_mid", 248, 140), ("bottom_mid", 276, 158),
    ("viewport_left_mirror", 55, 72), ("viewport_mid_mirror", 112, 72), ("viewport_right_mirror", 170, 72),
    ("resurrect_left", 92, 165), ("reincarnate_right", 178, 165),
]


def current_window(log):
    return wait_window(log, timeout=4.0)


def safe(log, label, fn):
    last = None
    for i in range(3):
        try:
            return fn()
        except Exception as e:
            last = e
            log.append(f"retry {label} {i+1}: {e}")
            time.sleep(0.25)
    raise RuntimeError(f"failed {label}: {last}")


def conf(out: Path):
    p = out / "dosbox-pass122.conf"
    p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\nDM -vv -sn -pk\n""")
    return p


def shot(out: Path, log, label: str, idx: int):
    p = safe(log, f"capture-{label}", lambda: capture_new(current_window(log), out, label, log))
    target = out / f"image{idx:04d}-raw.png"
    if target.exists():
        target.unlink()
    shutil.move(str(p), target)
    cls, reason = classify_file(target)
    return {"label": label, "file": target.name, "sha12": sha256(target)[:12], "class": cls, "reason": reason}


def run_route(base: Path, route_name: str, route_keys: list[str]):
    out = base / route_name
    out.mkdir(parents=True, exist_ok=True)
    log=[]; rows=[]; idx=1
    proc = subprocess.Popen([DOSBOX, "-conf", str(conf(out))], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wait_window(log); time.sleep(7.0)
        safe(log, "start-return", lambda: tap(current_window(log), "Return", log, delay=1.0))
        got=False
        for i in range(1, 16):
            p = safe(log, f"gate{i}", lambda: capture_new(current_window(log), out, f"gate{i:02d}", log))
            cls, reason = classify_file(p)
            dst = out / f"gate{i:02d}_{cls}.png"
            shutil.move(str(p), dst)
            rows.append({"phase":"gate", "step": i, "sha12": sha256(dst)[:12], "class": cls, "reason": reason, "file": dst.name})
            if cls == "dungeon_gameplay":
                got=True; break
            if cls == "entrance_menu":
                safe(log, "gate-return", lambda: tap(current_window(log), "Return", log, delay=0.8))
            else:
                time.sleep(0.8)
        if not got:
            raise RuntimeError("no dungeon_gameplay gate")
        rows.append({"phase":"route", **shot(out, log, "baseline", idx)}); idx += 1
        for key in route_keys:
            safe(log, f"route-key-{key}", lambda key=key: tap(current_window(log), key, log, delay=1.0))
            rows.append({"phase":"route", "key": key, **shot(out, log, f"route_{key}_{idx}", idx)}); idx += 1
        # Probe panel keys in the routed state.
        for key in PANEL_KEYS:
            safe(log, f"panel-key-{key}", lambda key=key: tap(current_window(log), key, log, delay=0.9))
            rows.append({"phase":"panel_key", "key": key, **shot(out, log, f"panel_{key}_{idx}", idx)}); idx += 1
        # Probe clicks after the same routed state; this is stateful by design, so each row records what happened in sequence.
        for name,x,y in CLICK_PROBES:
            safe(log, f"click-{name}", lambda x=x,y=y: click_original(current_window(log), x, y, log, delay=0.9))
            rows.append({"phase":"click", "click": name, "x": x, "y": y, **shot(out, log, f"click_{name}_{idx}", idx)}); idx += 1
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception: proc.kill()
        (out/"pass122_driver.log").write_text("\n".join(log)+"\n")
    (out/"pass122_rows.json").write_text(json.dumps(rows, indent=2)+"\n")
    return {"route": route_name, "keys": route_keys, "rows": rows}


def main():
    base = Path(sys.argv[1]); base.mkdir(parents=True, exist_ok=True)
    results=[]
    for name, keys in ROUTE_FAMILIES.items():
        try:
            results.append(run_route(base, name, keys))
        except Exception as e:
            results.append({"route": name, "keys": keys, "error": str(e)})
    (base/"pass122_results.json").write_text(json.dumps(results, indent=2)+"\n")
    md=["# Pass 122 — DM1 V1 original route solver", "", f"- run base: `{base}`", "- scope: larger consolidated original-route pass for DM1 V1 priority. It combines proven keypad route changes from pass120/121 with panel keys, inventory/champion keys, and click probes after each routed state.", "", "## Route summaries", ""]
    interesting=[]
    for r in results:
        if r.get("error"):
            md.append(f"- `{r['route']}` ERROR `{r['error']}`")
            continue
        probes=[x for x in r["rows"] if x.get("phase") != "gate"]
        sigs=[f"{x.get('phase')}:{x.get('key') or x.get('click') or x.get('label')}:{x.get('sha12')}:{x.get('class')}" for x in probes]
        unique=sorted({x.get("sha12") for x in probes})
        md.append(f"- `{r['route']}` keys `{r['keys']}` unique_hashes={len(unique)}: " + " -> ".join(sigs[:12]) + (" ..." if len(sigs)>12 else ""))
        for x in probes:
            cls=x.get("class") or ""
            lab=str(x.get("key") or x.get("click") or x.get("label"))
            if cls not in {"dungeon_gameplay", "wall_closeup", "entrance_menu"} or lab in {"F1","F2","F3","F4","i","I"} and x.get("sha12") not in {"48ed3743ab6a","fbeb1b82cd09","9fc8530431a3"}:
                interesting.append((r['route'], x))
    md += ["", "## Interesting/non-baseline probes", ""]
    if interesting:
        for route,x in interesting[:80]:
            md.append(f"- `{route}` {x.get('phase')} `{x.get('key') or x.get('click') or x.get('label')}` -> `{x.get('sha12')}` `{x.get('class')}` `{x.get('reason')}`")
    else:
        md.append("No probe produced a classifier class outside the known no-party dungeon/wall-closeup family, and panel/inventory probes did not produce a party-ready signature.")
    md += ["", "## Interpretation", "", "This pass is evidence-only. If it does not discover a party/spell/inventory classifier transition, the next blocker is not basic input delivery but original route semantics: the automation can turn the no-party viewport, but still lacks the original champion/recruitment path needed for DM1 V1 overlay parity.", ""]
    Path("parity-evidence/pass122_dm1_v1_original_route_solver.md").write_text("\n".join(md)+"\n")

if __name__ == "__main__":
    main()
