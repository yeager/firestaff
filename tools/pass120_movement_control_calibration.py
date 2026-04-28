#!/usr/bin/env python3
"""Pass120: calibrate original DM1 no-party movement controls after state-aware gate."""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original
from tools.pass80_original_frame_classifier import sha256

STAGE = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"


def current_window(log):
    return wait_window(log, timeout=4.0)


def safe_call(log, what, fn):
    last = None
    for attempt in range(3):
        try:
            return fn()
        except Exception as e:
            last = e
            log.append(f"retry {what} attempt={attempt+1}: {e}")
            time.sleep(0.25)
    raise RuntimeError(f"failed {what}: {last}")


def make_conf(out: Path):
    (out/"dosbox-pass120.conf").write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\nDM -vv -sn -pk\n""")
    return out/"dosbox-pass120.conf"


def reach_gate(out: Path, log):
    proc = subprocess.Popen([DOSBOX, "-conf", str(make_conf(out))], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    wid = None
    try:
        wid = wait_window(log)
        time.sleep(7.0)
        safe_call(log, "return", lambda: tap(current_window(log), "Return", log, delay=1.0))
        for i in range(1, 16):
            wid = current_window(log)
            p = safe_call(log, f"gate_capture_{i}", lambda wid=wid: capture_new(wid, out, f"gate{i:02d}", log))
            cls, reason = classify_file(p)
            dst = out / f"gate{i:02d}_{cls}.png"
            shutil.move(str(p), dst)
            log.append(f"gate {i:02d} {cls} {sha256(dst)[:12]} {reason}")
            if cls == "dungeon_gameplay":
                return proc, current_window(log), dst
            if cls == "entrance_menu":
                safe_call(log, "return_gate", lambda: tap(current_window(log), "Return", log, delay=0.8))
            else:
                time.sleep(0.8)
        raise RuntimeError("no dungeon_gameplay gate")
    except Exception:
        try:
            proc.terminate(); proc.wait(timeout=2)
        except Exception:
            proc.kill()
        raise


def run_probe(base: Path, label: str, action):
    out = base / label
    out.mkdir(parents=True, exist_ok=True)
    log=[]
    proc, wid, gate = reach_gate(out, log)
    rows=[]
    try:
        before = safe_call(log, "before_capture", lambda: capture_new(current_window(log), out, "before", log))
        before_target = out/"image0001-raw.png"; shutil.move(str(before), before_target)
        before_hash = sha256(before_target)[:12]
        kind = action[0]
        if kind == "click":
            _, x, y = action
            safe_call(log, f"click_{x}_{y}", lambda: click_original(current_window(log), x, y, log, delay=1.4))
        elif kind == "key":
            _, key = action
            safe_call(log, f"key_{key}", lambda: tap(current_window(log), key, log, delay=1.4))
        after = safe_call(log, "after_capture", lambda: capture_new(current_window(log), out, "after", log))
        after_target = out/"image0002-raw.png"; shutil.move(str(after), after_target)
        after_hash = sha256(after_target)[:12]
        cls1, r1 = classify_file(before_target); cls2, r2 = classify_file(after_target)
        rows.append({"label": label, "action": action, "before_sha12": before_hash, "after_sha12": after_hash, "changed": before_hash != after_hash, "before_class": cls1, "after_class": cls2, "before_reason": r1, "after_reason": r2})
    finally:
        try:
            proc.terminate(); proc.wait(timeout=2)
        except Exception:
            proc.kill()
        (out/"pass120_driver.log").write_text("\n".join(log)+"\n")
    (out/"pass120_result.json").write_text(json.dumps(rows[0], indent=2)+"\n")
    return rows[0]


def main():
    base=Path(sys.argv[1]); base.mkdir(parents=True, exist_ok=True)
    probes=[]
    # visible movement-control area from pass119: x~220-319, y~120-168. Sweep centers and arrow-key fallbacks.
    for y in [124,132,140,148,156,164,172]:
        for x in [232,244,256,268,280,292,304,316]:
            probes.append((f"click_{x}_{y}", ("click", x, y)))
    for key in ["Up", "Down", "Left", "Right", "KP_Up", "KP_Down", "KP_Left", "KP_Right", "Home", "Prior"]:
        probes.append((f"key_{key.lower().replace('_','-')}", ("key", key)))
    results=[]
    for label, action in probes:
        try:
            results.append(run_probe(base, label, action))
        except Exception as e:
            results.append({"label": label, "action": action, "error": str(e)})
    (base/"pass120_results.json").write_text(json.dumps(results, indent=2)+"\n")
    winners=[r for r in results if r.get("changed")]
    md=["# Pass 120 — original movement-control calibration", "", f"- run base: `{base}`", "- scope: after pass118/pass119 state-aware gate reaches no-party dungeon_gameplay, sweep visible movement-control click/key inputs to find any deterministic visual-state transition.", "", "## Summary", "", f"- probes: `{len(results)}`", f"- changed hash probes: `{len(winners)}`", ""]
    if winners:
        md += ["## Changed probes", ""]
        for r in winners:
            md.append(f"- `{r['label']}` action `{r['action']}`: `{r['before_sha12']}` -> `{r['after_sha12']}` classes `{r['before_class']}` -> `{r['after_class']}`")
    else:
        md += ["No click/key probe in the swept movement-control area changed the repeated no-party dungeon frame."]
    md += ["", "## All results", ""]
    for r in results:
        if 'error' in r:
            md.append(f"- `{r['label']}` ERROR `{r['error']}`")
        else:
            md.append(f"- `{r['label']}` changed={r['changed']} `{r['before_sha12']}` -> `{r['after_sha12']}`")
    Path("parity-evidence/pass120_original_movement_control_calibration.md").write_text("\n".join(md)+"\n")

if __name__ == "__main__":
    main()
