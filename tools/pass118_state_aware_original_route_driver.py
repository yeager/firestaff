#!/usr/bin/env python3
"""Pass 118 state-aware original DM1 route driver.

Runs one DOSBox original-PC DM1 session, gates on pass80 frame classes before
issuing movement/control probes, and writes six final raw captures plus a label
manifest compatible with pass80/pass113.
"""
from __future__ import annotations

import argparse, os, shutil, subprocess, sys, time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass80_original_frame_classifier import load_rgb, stats_for, classify, REGIONS, png_dims, sha256


def run(cmd, **kw):
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kw)


def xdo(args, check=True):
    p = run(["xdotool", *args])
    if check and p.returncode:
        raise RuntimeError(f"xdotool {' '.join(args)} failed rc={p.returncode}: stdout={p.stdout!r} stderr={p.stderr!r}")
    return p.stdout.strip()


def wait_window(log, timeout=25.0):
    deadline = time.time() + timeout
    last = ""
    while time.time() < deadline:
        for query in (["search", "--class", "DOSBox"], ["search", "--name", "DOSBox"]):
            p = run(["xdotool", *query])
            last = (p.stdout + p.stderr).strip()
            ids = [x for x in p.stdout.split() if x.strip()]
            if ids:
                wid = ids[-1]
                xdo(["windowactivate", wid], check=False)
                log.append(f"window-found {wid} via {' '.join(query)}")
                return wid
        time.sleep(0.25)
    raise RuntimeError(f"no DOSBox window found; last search output={last!r}")


def latest_image(out_dir: Path) -> Path | None:
    imgs = sorted(out_dir.glob("*.png"), key=lambda p: p.stat().st_mtime)
    return imgs[-1] if imgs else None


def capture_new(wid: str, out_dir: Path, label: str, log, timeout=6.0) -> Path:
    before = {p.name for p in out_dir.glob("*.png")}
    xdo(["windowactivate", wid], check=False)
    xdo(["key", "ctrl+F5"])
    deadline = time.time() + timeout
    while time.time() < deadline:
        imgs = [p for p in out_dir.glob("*.png") if p.name not in before]
        if imgs:
            p = sorted(imgs, key=lambda q: q.stat().st_mtime)[-1]
            # Wait for the file size to settle.
            last = -1
            for _ in range(10):
                size = p.stat().st_size
                if size == last and size > 0:
                    break
                last = size
                time.sleep(0.1)
            log.append(f"capture {label} {p.name} sha={sha256(p)[:12]}")
            return p
        time.sleep(0.15)
    raise RuntimeError(f"timed out waiting for screenshot {label}; before={sorted(before)} latest={latest_image(out_dir)}")


def classify_file(path: Path):
    dims = png_dims(path)
    if dims != (320, 200):
        return "non_graphics_blocker", f"raw dimensions are {dims[0]}x{dims[1]}, not 320x200"
    img = load_rgb(path)
    regions = {name: stats_for(img, xywh) for name, xywh in REGIONS.items()}
    return classify(regions, dims)


def tap(wid: str, key: str, log, delay=0.35):
    xdo(["windowactivate", wid], check=False)
    xdo(["key", key])
    log.append(f"key {key}")
    time.sleep(delay)


def click_original(wid: str, x: int, y: int, log, delay=0.25):
    geom = xdo(["getwindowgeometry", "--shell", wid])
    vals = {}
    for line in geom.splitlines():
        if "=" in line:
            k, v = line.split("=", 1); vals[k] = int(v)
    gw, gh = vals.get("WIDTH", 640), vals.get("HEIGHT", 400)
    content_aspect = 320/200
    cw, ch = gw, gw/content_aspect
    if ch > gh:
        ch = gh; cw = ch*content_aspect
    left, top = (gw-cw)/2, (gh-ch)/2
    px = round(left + ((x+0.5)/320)*cw); py = round(top + ((y+0.5)/200)*ch)
    xdo(["mousemove", "--window", wid, str(px), str(py), "click", "1"])
    log.append(f"click {x},{y} mapped {px},{py} window={gw}x{gh}")
    time.sleep(delay)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--stage", type=Path, default=Path.home()/".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34")
    ap.add_argument("--dosbox", default="/usr/bin/dosbox")
    ap.add_argument("--program", default="DM -vv -sn -pk")
    ap.add_argument("--gate-timeout", type=float, default=18.0)
    args = ap.parse_args()
    out = args.out; gate_dir = out/"state_gate"
    out.mkdir(parents=True, exist_ok=True); gate_dir.mkdir(exist_ok=True)
    log=[]
    conf = out/"dosbox-pass118.conf"
    conf.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{args.stage}\"\nc:\n{args.program}\n""")
    for p in out.glob("image*.png"): p.unlink()
    proc = subprocess.Popen([args.dosbox, "-conf", str(conf)], stdout=(out/"dosbox-original-viewports.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wid = wait_window(log)
        time.sleep(7.0)
        tap(wid, "Return", log, delay=1.0)
        # State-aware gate: only progress after classifier sees entrance_menu disappear into dungeon_gameplay.
        deadline = time.time() + args.gate_timeout
        gate_rows=[]; attempt=0; got_gameplay=False
        while time.time() < deadline:
            attempt += 1
            p = capture_new(wid, out, f"gate{attempt:02d}", log)
            cls, reason = classify_file(p)
            dst = gate_dir/f"gate{attempt:02d}_{cls}.png"
            shutil.move(str(p), dst)
            gate_rows.append((attempt, cls, reason, dst.name, sha256(dst)[:12]))
            log.append(f"gate {attempt:02d} class={cls} reason={reason}")
            if cls == "dungeon_gameplay":
                got_gameplay=True
                break
            if cls == "entrance_menu":
                tap(wid, "Return", log, delay=0.8)
            else:
                time.sleep(0.8)
        if not got_gameplay:
            raise RuntimeError("state gate never observed dungeon_gameplay after entrance_menu/startup")

        final = [
            ("gate_confirmed_gameplay", None),
            ("move_up_after_gate", ("key", "Up")),
            ("turn_right_after_gate", ("key", "Right")),
            ("move_left_after_gate", ("key", "Left")),
            ("spell_or_party_key_after_gate", ("key", "F1")),
            ("inventory_or_party_key_after_gate", ("key", "F4")),
        ]
        manifest = ["index\tfilename\troute_label\troute_token"]
        for idx, (label, action) in enumerate(final, 1):
            if action:
                if action[0] == "key": tap(wid, action[1], log, delay=1.1)
                elif action[0] == "click": click_original(wid, action[1], action[2], log, delay=1.1)
            else:
                time.sleep(0.8)
            raw = capture_new(wid, out, label, log)
            target = out/f"image{idx:04d}-raw.png"
            if raw != target:
                if target.exists(): target.unlink()
                shutil.move(str(raw), target)
            manifest.append(f"{idx:02d}\t{target.name}\t{label}\tshot:{label}")
        (out/"original_viewport_shot_labels.tsv").write_text("\n".join(manifest)+"\n")
        (out/"pass118_gate.tsv").write_text("index\tclass\treason\tfile\tsha12\n" + "\n".join(f"{i:02d}\t{c}\t{r}\t{f}\t{s}" for i,c,r,f,s in gate_rows)+"\n")
        (out/"pass118_driver.log").write_text("\n".join(log)+"\n")
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception:
            proc.kill()

if __name__ == "__main__":
    main()
