#!/usr/bin/env python3
"""Pass166: runtime route probe using ReDMCSB portrait click geometry.

Pass164/165 proved the source route/geometry: while facing a champion mirror,
click screen x=111 y=82 to trigger C127/F0280 candidate state, then use C160/C161.
This pass replays that sequence with strict pass162-style classification.
"""
from __future__ import annotations

import json
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

from PIL import Image, ImageChops, ImageStat

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original  # noqa: E402
from tools.pass80_original_frame_classifier import sha256  # noqa: E402

STAGE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"
OUT_ROOT = Path("parity-evidence/verification/pass166_source_portrait_click_route_probe")
RUN_BASE_ROOT = Path.home() / ".openclaw/data/firestaff-n2-runs"

STATIC_NO_PARTY_HASHES = {"48ed3743ab6a", "082b4d249740"}
DUNGEON_CLASSES = {"dungeon_gameplay"}
CONTROL_CLASSES = {"inventory", "spell_panel"}
UNSAFE_CLASSES = {"title_or_menu", "entrance_menu", "wall_closeup", "non_graphics_blocker"}

CROPS = {
    "viewport": (0, 0, 224, 136),
    "right_panel": (224, 0, 320, 136),
    "lower_panel": (0, 136, 320, 200),
    "top_party_strip": (0, 0, 320, 48),
    "candidate_buttons": (70, 80, 225, 148),
    "movement_panel": (224, 112, 320, 180),
}

SOURCE_LOCKS = [
    {"file": "DUNVIEW.C", "lines": "525", "point": "G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}."},
    {"file": "COORD.C", "lines": "1693,1698", "point": "PC viewport origin is x=0,y=33, making viewport center x=111,y=49 become screen x=111,y=82."},
    {"file": "CLIKVIEW.C", "lines": "348-349,407-431", "point": "F0377 subtracts viewport origin, tests C05, and C05 calls F0372."},
    {"file": "MOVESENS.C", "lines": "1392-1502", "point": "F0275 allows C127 with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty."},
    {"file": "COMMAND.C", "lines": "231-238,509-511", "point": "C160/C161 button centers remain x=130,y=115 and x=186,y=115 after candidate state."},
]

SCENARIOS: list[dict[str, Any]] = [
    {
        "name": "enter_portrait11182_then_resurrect",
        "program": "DM -vv -sn",
        "purpose": "Source route: enter dungeon, click portrait-on-wall center x=111 y=82, then C160 resurrect center x=130 y=115.",
        "actions": [
            ("click", 270, 52), ("wait", 1.5), ("shot", "after_c407_enter_click"),
            ("click", 111, 82), ("wait", 1.2), ("shot", "after_source_portrait_111_82"),
            ("click", 130, 115), ("wait", 1.2), ("shot", "after_source_c160_resurrect"),
            ("key", "Return"), ("wait", 1.2), ("shot", "after_confirm"),
            ("key", "Up"), ("wait", 1.0), ("shot", "after_move_up"),
            ("key", "Right"), ("wait", 1.0), ("shot", "after_turn_right"),
            ("key", "F1"), ("wait", 1.0), ("shot", "after_f1_readiness"),
            ("key", "F4"), ("wait", 1.0), ("shot", "after_f4_readiness"),
        ],
    },
    {
        "name": "enter_portrait11182_then_reincarnate",
        "program": "DM -vv -sn",
        "purpose": "Source route: enter dungeon, click portrait-on-wall center x=111 y=82, then C161 reincarnate center x=186 y=115.",
        "actions": [
            ("click", 270, 52), ("wait", 1.5), ("shot", "after_c407_enter_click"),
            ("click", 111, 82), ("wait", 1.2), ("shot", "after_source_portrait_111_82"),
            ("click", 186, 115), ("wait", 1.2), ("shot", "after_source_c161_reincarnate"),
            ("key", "Return"), ("wait", 1.2), ("shot", "after_confirm"),
            ("key", "Up"), ("wait", 1.0), ("shot", "after_move_up"),
            ("key", "Right"), ("wait", 1.0), ("shot", "after_turn_right"),
            ("key", "F1"), ("wait", 1.0), ("shot", "after_f1_readiness"),
            ("key", "F4"), ("wait", 1.0), ("shot", "after_f4_readiness"),
        ],
    },
]


def slug(s: str) -> str:
    return "".join(c.lower() if c.isalnum() else "_" for c in s).strip("_")


def conf(out: Path, program: str) -> Path:
    p = out / "dosbox-pass166.conf"
    p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\n{program}\n""")
    return p


def shot(out: Path, log: list[str], label: str, idx: int) -> dict[str, Any]:
    raw = capture_new(wait_window(log, timeout=5.0), out, label, log)
    dst = out / f"image{idx:04d}-{slug(label)}.png"
    if dst.exists():
        dst.unlink()
    shutil.move(str(raw), dst)
    cls, reason = classify_file(dst)
    return {"index": idx, "label": label, "file": dst.name, "path": str(dst), "sha12": sha256(dst)[:12], "class": cls, "reason": reason}


def crop_stats(path: Path) -> dict[str, Any]:
    im = Image.open(path).convert("RGB")
    stats: dict[str, Any] = {}
    for name, box in CROPS.items():
        cr = im.crop(box)
        st = ImageStat.Stat(cr)
        nonblack = sum(1 for px in cr.getdata() if px != (0, 0, 0))
        colors = cr.convert("P", palette=Image.Palette.ADAPTIVE, colors=64).getcolors(maxcolors=100000) or []
        stats[name] = {"mean_rgb": [round(x, 2) for x in st.mean], "nonblack_ratio": round(nonblack/(cr.size[0]*cr.size[1]), 6), "palette64_colors": len(colors)}
    return stats


def diff_stats(a: Path, b: Path) -> dict[str, Any]:
    ia, ib = Image.open(a).convert("RGB"), Image.open(b).convert("RGB")
    d = ImageChops.difference(ia, ib)
    bbox = d.getbbox()
    nz = sum(1 for px in d.getdata() if px != (0,0,0))
    return {"bbox": list(bbox) if bbox else None, "changed_pixels": nz, "changed_ratio": round(nz/(320*200), 6)}


def do_action(out: Path, log: list[str], action: tuple[Any, ...], idx: int) -> dict[str, Any] | None:
    kind = action[0]
    if kind == "wait":
        time.sleep(float(action[1])); return None
    if kind == "shot":
        return {"phase": "shot", **shot(out, log, str(action[1]), idx)}
    if kind == "key":
        key = str(action[1]); tap(wait_window(log, timeout=5.0), key, log, delay=0.6)
        return {"phase": "key", "value": key, **shot(out, log, f"key_{key}_{idx}", idx)}
    if kind == "click":
        x, y = int(action[1]), int(action[2]); click_original(wait_window(log, timeout=5.0), x, y, log, delay=0.6)
        return {"phase": "click", "x": x, "y": y, **shot(out, log, f"click_{x}_{y}_{idx}", idx)}
    raise ValueError(action)


def classify_route(rows: list[dict[str, Any]]) -> tuple[str, str, dict[str, Any]]:
    shots = [r for r in rows if "sha12" in r]
    hashes = [r["sha12"] for r in shots]
    static_hits = sorted(set(hashes) & STATIC_NO_PARTY_HASHES)
    diffs = [{"from": a["label"], "to": b["label"], **diff_stats(Path(a["path"]), Path(b["path"]))} for a,b in zip(shots, shots[1:])]
    dynamic_inputs = [d for d in diffs if d["changed_ratio"] > 0.01 and any(tok in d["to"] for tok in ("move", "turn", "key_Up", "key_Right", "key_Left", "f1", "f4"))]
    portrait_delta = [d for d in diffs if "source_portrait_111_82" in d["to"]]
    control = [r for r in shots if r["class"] in CONTROL_CLASSES]
    dungeon = [r for r in shots if r["class"] in DUNGEON_CLASSES]
    unsafe_tail = [r for r in shots[-5:] if r["class"] in UNSAFE_CLASSES]
    evidence = {"hashes": hashes, "unique_hashes": sorted(set(hashes)), "classes": [r["class"] for r in shots], "static_hits": static_hits, "portrait_click_delta": portrait_delta, "dynamic_inputs": dynamic_inputs, "control_count": len(control), "dungeon_count": len(dungeon), "unsafe_tail_count": len(unsafe_tail)}
    if static_hits:
        return "blocked/static-no-party", f"known static no-party hash present: {', '.join(static_hits)}", evidence
    if not dungeon:
        return "blocked/no-dungeon", "route never produced a dungeon gameplay frame", evidence
    if not portrait_delta or portrait_delta[0].get("changed_ratio", 0) < 0.001:
        return "blocked/portrait-click-no-visible-delta", "source portrait click did not visibly change candidate state", evidence
    if unsafe_tail:
        return "blocked/unsafe-tail", "tail still contains title/entrance/menu frames", evidence
    if not dynamic_inputs:
        return "blocked/no-input-delta", "post-candidate movement/control inputs did not produce non-trivial deltas", evidence
    if not control:
        return "blocked/no-party-control-marker", "dynamic dungeon exists but no inventory/spell/control marker proves recruited party", evidence
    return "party-control-ready-candidate", "non-static route with portrait delta, dynamic inputs, and party/control marker", evidence


def run_scenario(run_base: Path, scenario: dict[str, Any]) -> dict[str, Any]:
    out = run_base / slug(scenario["name"]); out.mkdir(parents=True, exist_ok=True)
    log: list[str] = []; rows: list[dict[str, Any]] = []
    proc = subprocess.Popen([DOSBOX, "-conf", str(conf(out, scenario["program"]))], stdout=(out/"dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wait_window(log, timeout=6.0); time.sleep(7.0)
        rows.append({"phase": "initial", **shot(out, log, "initial", 1)})
        idx = 2
        for action in scenario["actions"]:
            row = do_action(out, log, action, idx)
            if row:
                row["crop_stats"] = crop_stats(Path(row["path"]))
                rows.append(row); idx += 1
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception: proc.kill()
        (out/"pass166_driver.log").write_text("\n".join(log)+"\n")
    classification, reason, evidence = classify_route(rows)
    summary = {"name": scenario["name"], "program": scenario["program"], "purpose": scenario["purpose"], "classification": classification, "reason": reason, "source_locks": SOURCE_LOCKS, "route_evidence": evidence, "rows": rows, "evidence_dir": str(out)}
    (out/"summary.json").write_text(json.dumps(summary, indent=2)+"\n")
    return summary


def main() -> int:
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    run_base = RUN_BASE_ROOT / (time.strftime("%Y%m%d-%H%M%S") + "-pass166-source-portrait-click-route-probe")
    run_base.mkdir(parents=True, exist_ok=True)
    results=[]; errors=[]
    for scenario in SCENARIOS:
        try:
            result=run_scenario(run_base, scenario)
            ev=OUT_ROOT/slug(scenario["name"])
            if ev.exists(): shutil.rmtree(ev)
            shutil.copytree(Path(result["evidence_dir"]), ev)
            result["evidence_dir"] = str(ev)
            (ev/"summary.json").write_text(json.dumps(result, indent=2)+"\n")
            results.append(result)
        except Exception as exc:
            errors.append({"scenario": scenario["name"], "program": scenario["program"], "error": str(exc)})
    buckets: dict[str,int] = {}
    for r in results: buckets[r["classification"]] = buckets.get(r["classification"], 0) + 1
    manifest={"schema":"pass166_source_portrait_click_route_probe.v1","run_base":str(run_base),"evidence_root":str(OUT_ROOT),"source_locks":SOURCE_LOCKS,"completed":len(results),"errors":errors,"buckets":buckets,"results":results}
    (OUT_ROOT/"manifest.json").write_text(json.dumps(manifest, indent=2)+"\n")
    lines=["# Pass 166 — source portrait click route probe","",f"- run base: `{run_base}`",f"- evidence root: `{OUT_ROOT}`",f"- completed: {len(results)}",f"- errors: {len(errors)}",f"- buckets: {', '.join(f'{k}={v}' for k,v in sorted(buckets.items()))}","","## Source locks",""]
    lines += [f"- `{s['file']}` {s['lines']}: {s['point']}" for s in SOURCE_LOCKS]
    lines += ["", "## Result matrix", ""]
    for r in results:
        ev=r["route_evidence"]
        lines.append(f"- `{r['name']}` `{r['program']}`: **{r['classification']}** — {r['reason']} — unique hashes: {', '.join(ev['unique_hashes'])} — `{r['evidence_dir']}`")
    if errors:
        lines += ["", "## Errors", ""] + [f"- `{e['scenario']}` `{e['program']}`: {e['error']}" for e in errors]
    lines += ["", "## Interpretation", "", "This is the first runtime probe after the ReDMCSB C127/F0280 source route and portrait geometry were locked by pass164/pass165. A pass requires no static no-party hash, a visible portrait-click candidate transition, post-choice input deltas, and a party/control marker."]
    (OUT_ROOT/"README.md").write_text("\n".join(lines)+"\n")
    print(f"wrote {OUT_ROOT}/README.md")
    print(f"run_base={run_base}")
    print(f"completed={len(results)} errors={len(errors)} buckets={buckets}")
    return 1 if errors else 0

if __name__ == "__main__":
    raise SystemExit(main())
