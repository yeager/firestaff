#!/usr/bin/env python3
"""Pass119: probe original PC DM1 champion/party-control readiness after the pass118 state gate."""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original


def current_window(log):
    return wait_window(log, timeout=4.0)


def robust_tap(wid, key, log, delay=0.35):
    try:
        tap(wid, key, log, delay=delay)
        return wid
    except Exception as e:
        log.append(f"tap stale-window retry key={key}: {e}")
        wid = current_window(log)
        tap(wid, key, log, delay=delay)
        return wid


def robust_click(wid, x, y, log, delay=0.25, double=False):
    for attempt in range(3):
        try:
            if double:
                click_original(wid, x, y, log, delay=0.15)
                click_original(wid, x, y, log, delay=delay)
            else:
                click_original(wid, x, y, log, delay=delay)
            return wid
        except Exception as e:
            log.append(f"click stale-window retry attempt={attempt+1} xy={x},{y}: {e}")
            wid = current_window(log)
    raise RuntimeError(f"could not click {x},{y} after stale-window retries")


def robust_capture(wid, out, label, log):
    for attempt in range(3):
        try:
            return wid, capture_new(wid, out, label, log)
        except Exception as e:
            log.append(f"capture retry attempt={attempt+1} label={label}: {e}")
            wid = current_window(log)
    raise RuntimeError(f"could not capture {label} after retries")
from tools.pass80_original_frame_classifier import sha256


def run_session(base: Path, name: str, actions: list[tuple], stage: Path, dosbox: str):
    out = base / name
    out.mkdir(parents=True, exist_ok=True)
    conf = out / "dosbox-pass119.conf"
    log: list[str] = []
    conf.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{stage}\"\nc:\nDM -vv -sn -pk\n""")
    proc = subprocess.Popen([dosbox, "-conf", str(conf)], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    rows=[]
    manifest=["index\tfilename\troute_label\troute_token"]
    try:
        wid = wait_window(log)
        time.sleep(7.0)
        wid = robust_tap(wid, "Return", log, delay=1.0)
        got = False
        for i in range(1, 16):
            wid, p = robust_capture(wid, out, f"gate{i:02d}", log)
            cls, reason = classify_file(p)
            dst = out / f"gate{i:02d}_{cls}.png"
            shutil.move(str(p), dst)
            rows.append({"phase":"gate","index":i,"class":cls,"reason":reason,"sha12":sha256(dst)[:12],"file":dst.name})
            if cls == "dungeon_gameplay":
                got = True
                break
            if cls == "entrance_menu":
                wid = robust_tap(wid, "Return", log, delay=0.8)
            else:
                time.sleep(0.8)
        if not got:
            raise RuntimeError("never reached dungeon_gameplay gate")
        shot_idx = 1
        for label, kind, *args in actions:
            if kind == "wait":
                time.sleep(float(args[0]))
            elif kind == "key":
                wid = robust_tap(wid, str(args[0]), log, delay=0.9)
            elif kind == "click":
                wid = robust_click(wid, int(args[0]), int(args[1]), log, delay=0.9)
            elif kind == "doubleclick":
                wid = robust_click(wid, int(args[0]), int(args[1]), log, delay=0.9, double=True)
            wid, raw = robust_capture(wid, out, label, log)
            target = out / f"image{shot_idx:04d}-raw.png"
            if target.exists():
                target.unlink()
            shutil.move(str(raw), target)
            cls, reason = classify_file(target)
            rows.append({"phase":"probe","index":shot_idx,"label":label,"class":cls,"reason":reason,"sha12":sha256(target)[:12],"file":target.name})
            manifest.append(f"{shot_idx:02d}\t{target.name}\t{label}\tshot:{label}")
            shot_idx += 1
    finally:
        try:
            proc.terminate(); proc.wait(timeout=2)
        except Exception:
            proc.kill()
    (out/"pass119_rows.json").write_text(json.dumps(rows, indent=2)+"\n")
    (out/"original_viewport_shot_labels.tsv").write_text("\n".join(manifest)+"\n")
    (out/"pass119_driver.log").write_text("\n".join(log)+"\n")
    return rows


SCENARIOS = {
  "right_column_reenter_controls": [
    ("after_gate", "wait", 0.5), ("right_top_260_50", "click", 260, 50), ("forward_arrow_276_140", "click", 276, 140),
    ("f1_spell_probe", "key", "F1"), ("f4_inventory_probe", "key", "F4"), ("return_confirm", "key", "Return")],
  "viewport_mirror_left_recruit": [
    ("after_gate", "wait", 0.5), ("mirror_left_55_72", "doubleclick", 55, 72), ("resurrect_left_92_165", "click", 92, 165),
    ("reincarnate_right_178_165", "click", 178, 165), ("f1_spell_probe", "key", "F1"), ("f4_inventory_probe", "key", "F4")],
  "viewport_mirror_mid_recruit": [
    ("after_gate", "wait", 0.5), ("mirror_mid_112_72", "doubleclick", 112, 72), ("resurrect_left_92_165", "click", 92, 165),
    ("reincarnate_right_178_165", "click", 178, 165), ("f1_spell_probe", "key", "F1"), ("f4_inventory_probe", "key", "F4")],
  "viewport_mirror_right_recruit": [
    ("after_gate", "wait", 0.5), ("mirror_right_170_72", "doubleclick", 170, 72), ("resurrect_left_92_165", "click", 92, 165),
    ("reincarnate_right_178_165", "click", 178, 165), ("f1_spell_probe", "key", "F1"), ("f4_inventory_probe", "key", "F4")],
  "keyboard_confirm_recruit": [
    ("after_gate", "wait", 0.5), ("return_after_gameplay", "key", "Return"), ("r_key", "key", "r"),
    ("y_key", "key", "y"), ("f1_spell_probe", "key", "F1"), ("f4_inventory_probe", "key", "F4")],
}


def main():
    base = Path(sys.argv[1])
    stage = Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
    dosbox = "/usr/bin/dosbox"
    summary = []
    for name, actions in SCENARIOS.items():
        rows = run_session(base, name, actions, stage, dosbox)
        classes = [r.get("class") for r in rows if r.get("phase") == "probe"]
        shas = [r.get("sha12") for r in rows if r.get("phase") == "probe"]
        summary.append((name, classes, shas))
    md = [
        "# Pass 119 — champion selection gate probe",
        "",
        f"- run base: `{base}`",
        "- scope: deterministic post-`dungeon_gameplay` probes for original PC DM1 champion/party-control readiness after pass118 state gate.",
        "",
        "## Scenario outcomes",
        "",
    ]
    for name, classes, shas in summary:
        md += [f"### {name}", "", f"- classes: `{', '.join(classes)}`", f"- sha12: `{', '.join(shas)}`", ""]
    md += [
        "## Interpretation",
        "",
        "This pass is evidence-only. A successful unblock requires any scenario to leave the repeated blank/no-party dungeon hash and/or make pass113 report party_control_ready=true after F1/F4 probes.",
        "",
    ]
    Path("parity-evidence/pass119_champion_selection_gate_probe.md").write_text("\n".join(md))


if __name__ == "__main__":
    main()
