#!/usr/bin/env python3
"""Pass158: consolidated party/champion-control readiness route probe for DM1 PC 3.4.

This pass deliberately probes party/champion selection/control readiness *before* any
movement-control claim.  It carries forward the pass156/pass157 082b route evidence
but avoids spending more probes on movement from the static no-party state.
"""
from __future__ import annotations

import json
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

from PIL import Image, ImageChops, ImageDraw

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original  # noqa: E402
from tools.pass80_original_frame_classifier import sha256  # noqa: E402

STAGE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"
PROGRAM_SN = "DM -vv -sn"
PROGRAM_PM = "DM -vv -sn -pm"
OUT_ROOT = Path("parity-evidence/verification/pass158_party_control_readiness_route_probe")
RUN_BASE_ROOT = Path.home() / ".openclaw/data/firestaff-n2-runs"

STATIC_NO_PARTY_HASHES = {"48ed3743ab6a", "082b4d249740"}
CONTROL_CLASSES = {"inventory", "spell_panel"}
UNSAFE_CLASSES = {"title_or_menu", "entrance_menu", "wall_closeup", "non_graphics_blocker"}
CROPS = {
    "viewport": (0, 0, 224, 136),
    "right_panel": (224, 0, 320, 136),
    "lower_panel": (0, 136, 320, 200),
    "top_strip": (0, 0, 320, 48),
    "movement_panel": (224, 112, 320, 180),
    "candidate_buttons": (70, 145, 210, 190),
}

# Existing pass155/pass157 seed that reaches 082b/dungeon visuals; use only as a
# pre-party route seed, never as proof of movement/control readiness.
SEED_082B_ROUTE = [
    ("key", "Return"), ("wait", 1.1),
    ("click", 45, 35), ("wait", 0.8),
    ("click", 95, 35), ("wait", 0.8),
    ("click", 150, 35), ("wait", 0.8),
    ("click", 220, 35), ("wait", 0.8),
    ("click", 280, 35), ("wait", 0.8),
]

# Coordinates are from prior source/evidence lane probes (pass119/pass129) and
# ReDMCSB documentation semantics: click champion portrait, then Resurrect or
# Reincarnate, then verify F1/F4/inventory/spell readiness.
SCENARIOS: list[dict[str, Any]] = [
    {
        "name": "082b_seed_portrait_center_resurrect_confirm",
        "program": PROGRAM_SN,
        "purpose": "Try to turn the verified 082b pre-party seed into candidate/party state via center portrait + Resurrect before control probes.",
        "actions": SEED_082B_ROUTE + [("shot", "after_082b_seed"), ("click", 112, 60), ("wait", 1.2), ("shot", "after_portrait_center"), ("click", 92, 165), ("wait", 1.2), ("shot", "after_resurrect"), ("key", "Return"), ("wait", 1.2), ("shot", "after_confirm"), ("key", "F1"), ("wait", 1.2), ("shot", "f1_readiness"), ("key", "F4"), ("wait", 1.2), ("shot", "f4_readiness")],
    },
    {
        "name": "082b_seed_portrait_center_reincarnate_confirm",
        "program": PROGRAM_SN,
        "purpose": "Same 082b seed, but use Reincarnate button coordinate instead of Resurrect.",
        "actions": SEED_082B_ROUTE + [("shot", "after_082b_seed"), ("click", 112, 60), ("wait", 1.2), ("shot", "after_portrait_center"), ("click", 178, 165), ("wait", 1.2), ("shot", "after_reincarnate"), ("key", "Return"), ("wait", 1.2), ("shot", "after_confirm"), ("key", "F1"), ("wait", 1.2), ("shot", "f1_readiness"), ("key", "F4"), ("wait", 1.2), ("shot", "f4_readiness")],
    },
    {
        "name": "pm_f1_candidate_inventory_resurrect_then_readiness",
        "program": PROGRAM_PM,
        "purpose": "Follow pass137/pass139 PM F1 candidate-inventory lead, then choose Resurrect and verify control readiness.",
        "actions": [("key", "Return"), ("wait", 1.4), ("shot", "after_enter"), ("key", "F1"), ("wait", 1.4), ("shot", "candidate_inventory_lead"), ("click", 92, 165), ("wait", 1.4), ("shot", "after_resurrect"), ("key", "Return"), ("wait", 1.4), ("shot", "after_confirm"), ("key", "F1"), ("wait", 1.2), ("shot", "f1_readiness"), ("key", "F4"), ("wait", 1.2), ("shot", "f4_readiness")],
    },
    {
        "name": "pm_f1_candidate_inventory_reincarnate_then_readiness",
        "program": PROGRAM_PM,
        "purpose": "Follow PM F1 candidate-inventory lead, then choose Reincarnate and verify control readiness.",
        "actions": [("key", "Return"), ("wait", 1.4), ("shot", "after_enter"), ("key", "F1"), ("wait", 1.4), ("shot", "candidate_inventory_lead"), ("click", 178, 165), ("wait", 1.4), ("shot", "after_reincarnate"), ("key", "Return"), ("wait", 1.4), ("shot", "after_confirm"), ("key", "F1"), ("wait", 1.2), ("shot", "f1_readiness"), ("key", "F4"), ("wait", 1.2), ("shot", "f4_readiness")],
    },
    {
        "name": "enter_f2_roster_candidate_buttons_then_readiness",
        "program": PROGRAM_SN,
        "purpose": "Exercise the F2/high-density roster lead from pass137 before any dungeon movement; test both recruit buttons and readiness keys.",
        "actions": [("key", "Return"), ("wait", 1.4), ("shot", "after_enter"), ("key", "F2"), ("wait", 1.4), ("shot", "after_f2_roster_lead"), ("click", 112, 60), ("wait", 1.1), ("shot", "after_portrait_probe"), ("click", 92, 165), ("wait", 1.1), ("shot", "after_resurrect"), ("click", 178, 165), ("wait", 1.1), ("shot", "after_reincarnate"), ("key", "F1"), ("wait", 1.2), ("shot", "f1_readiness"), ("key", "F4"), ("wait", 1.2), ("shot", "f4_readiness")],
    },
]

SOURCE_NOTES = [
    "ReDMCSB Documentation/BugsAndChanges.htm BUG0_60 explicitly describes the original UI flow: start a new game, click a champion portrait, then wait for Reincarnate/Resurrect/Cancel.",
    "ReDMCSB Documentation/BugsAndChanges.htm BUG0_53/BUG0_43 describe candidate-champion inventory/spell states before adding/canceling; therefore inventory/spell-looking frames are not automatically party-control ready.",
    "pass151/pass153/pass157 evidence marks 48ed/082b dungeon frames as static no-party placeholders; this pass treats those as blocked unless recruit/readiness probes escape them.",
]


def slug(s: str) -> str:
    return "".join(c.lower() if c.isalnum() else "_" for c in s).strip("_")


def conf(out: Path, program: str) -> Path:
    p = out / "dosbox-pass158.conf"
    p.write_text(f"""[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c \"{STAGE}\"\nc:\n{program}\n""")
    return p


def safe(log: list[str], label: str, fn):
    last: Exception | None = None
    for attempt in range(3):
        try:
            return fn()
        except Exception as exc:  # pragma: no cover - runtime robustness path
            last = exc
            log.append(f"retry {label} attempt={attempt+1}: {exc}")
            time.sleep(0.25)
    raise RuntimeError(f"failed {label}: {last}")


def shot(out: Path, log: list[str], label: str, idx: int) -> dict[str, Any]:
    raw = safe(log, f"capture-{label}", lambda: capture_new(wait_window(log, timeout=5.0), out, label, log))
    dst = out / f"image{idx:04d}-{slug(label)}.png"
    if dst.exists():
        dst.unlink()
    shutil.move(str(raw), dst)
    cls, reason = classify_file(dst)
    return {"index": idx, "label": label, "file": dst.name, "path": str(dst), "sha12": sha256(dst)[:12], "class": cls, "reason": reason}


def do_action(out: Path, log: list[str], action: tuple[Any, ...], idx: int) -> dict[str, Any] | None:
    kind = action[0]
    if kind == "wait":
        time.sleep(float(action[1]))
        return None
    if kind == "shot":
        return {"phase": "shot", **shot(out, log, str(action[1]), idx)}
    if kind == "key":
        key = str(action[1])
        safe(log, f"key-{key}", lambda: tap(wait_window(log, timeout=5.0), key, log, delay=0.65))
        return {"phase": "key", "value": key, **shot(out, log, f"key_{key}_{idx}", idx)}
    if kind == "click":
        x, y = int(action[1]), int(action[2])
        safe(log, f"click-{x}-{y}", lambda: click_original(wait_window(log, timeout=5.0), x, y, log, delay=0.65))
        return {"phase": "click", "x": x, "y": y, **shot(out, log, f"click_{x}_{y}_{idx}", idx)}
    raise ValueError(f"unknown action {action!r}")


def annotate_and_crops(src: Path, dst_dir: Path, prefix: str) -> None:
    im = Image.open(src).convert("RGB")
    for name, box in CROPS.items():
        im.crop(box).save(dst_dir / f"{prefix}_{name}.png")
    ann = im.copy()
    dr = ImageDraw.Draw(ann)
    colors = {"viewport": "red", "right_panel": "yellow", "lower_panel": "cyan", "top_strip": "magenta", "movement_panel": "lime", "candidate_buttons": "white"}
    for name, box in CROPS.items():
        dr.rectangle(box, outline=colors[name], width=2)
        dr.text((box[0] + 2, box[1] + 2), name, fill=colors[name])
    ann.save(dst_dir / f"{prefix}_annotated_bbox.png")


def diff_stats(a_path: Path, b_path: Path, dst: Path, prefix: str) -> dict[str, Any]:
    a = Image.open(a_path).convert("RGB")
    b = Image.open(b_path).convert("RGB")
    d = ImageChops.difference(a, b)
    bbox = d.getbbox()
    nonzero = 0
    if bbox:
        pix = d.load()
        for y in range(d.size[1]):
            for x in range(d.size[0]):
                if pix[x, y] != (0, 0, 0):
                    nonzero += 1
        d.save(dst / f"{prefix}_diff.png")
        ann = b.copy()
        dr = ImageDraw.Draw(ann)
        dr.rectangle(bbox, outline="white", width=2)
        ann.save(dst / f"{prefix}_after_diff_bbox.png")
    return {"bbox": list(bbox) if bbox else None, "changed_pixels": nonzero, "changed_ratio": round(nonzero / (320 * 200), 6)}


def classify_scenario(rows: list[dict[str, Any]]) -> tuple[str, str]:
    shot_rows = [r for r in rows if "sha12" in r]
    if not shot_rows:
        return "error/no-frames", "no captured frames"
    readiness = [r for r in shot_rows if "readiness" in str(r.get("label", ""))]
    if not readiness:
        readiness = shot_rows[-2:]
    readiness_classes = {str(r.get("class")) for r in readiness}
    readiness_hashes = {str(r.get("sha12")) for r in readiness}
    unsafe = sorted(readiness_classes & UNSAFE_CLASSES)
    control = sorted(readiness_classes & CONTROL_CLASSES)
    if control and not unsafe and not (readiness_hashes & STATIC_NO_PARTY_HASHES):
        return "party-control-ready-candidate", "readiness probes reached control class without known static no-party hash; still needs semantic party-count confirmation"
    if control:
        return "candidate-inventory-only", "inventory/spell-like frame was seen, but unsafe/static or candidate-context evidence prevents party-control claim"
    if readiness_hashes & STATIC_NO_PARTY_HASHES:
        return "blocked/static-no-party", "readiness probes collapsed to known no-party static dungeon hash"
    if unsafe:
        return "blocked/menu-or-unsafe", f"readiness probes still unsafe: {', '.join(unsafe)}"
    uniq = sorted({r.get("sha12") for r in readiness})
    return "blocked/no-control-evidence", f"no spell/inventory/control class in readiness probes; readiness hashes={','.join(map(str, uniq))}"


def run_scenario(run_base: Path, scenario: dict[str, Any]) -> dict[str, Any]:
    out = run_base / slug(scenario["name"])
    out.mkdir(parents=True, exist_ok=True)
    log: list[str] = []
    rows: list[dict[str, Any]] = []
    idx = 1
    proc = subprocess.Popen([DOSBOX, "-conf", str(conf(out, scenario["program"]))], stdout=(out / "dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wait_window(log, timeout=6.0)
        time.sleep(7.5)
        rows.append({"phase": "initial", **shot(out, log, "initial", idx)})
        idx += 1
        for action in scenario["actions"]:
            row = do_action(out, log, action, idx)
            if row:
                rows.append(row)
                idx += 1
    finally:
        try:
            proc.terminate(); proc.wait(timeout=2)
        except Exception:
            proc.kill()
        (out / "pass158_driver.log").write_text("\n".join(log) + "\n")
    classification, reason = classify_scenario(rows)
    (out / "pass158_rows.json").write_text(json.dumps(rows, indent=2) + "\n")
    # Evidence: annotate first/last and each explicit readiness/recruit transition.
    selected = []
    if rows:
        selected.append(("initial", rows[0]))
        selected.append(("result", rows[-1]))
    for r in rows:
        lab = str(r.get("label", ""))
        if any(tok in lab for tok in ("candidate", "resurrect", "reincarnate", "confirm", "readiness", "082b_seed")):
            selected.append((slug(lab)[:40], r))
    seen: set[str] = set()
    for prefix, row in selected:
        path = out / row["file"]
        if row["file"] in seen or not path.exists():
            continue
        seen.add(row["file"])
        annotate_and_crops(path, out, prefix)
    if len(rows) >= 2:
        diff_stats(out / rows[0]["file"], out / rows[-1]["file"], out, "initial_to_result")
    summary = {"name": scenario["name"], "program": scenario["program"], "purpose": scenario["purpose"], "classification": classification, "reason": reason, "rows": rows, "evidence_dir": str(out)}
    (out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    return summary


def main() -> int:
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    run_base = RUN_BASE_ROOT / (time.strftime("%Y%m%d-%H%M%S") + "-pass158-party-control-readiness-route-probe")
    run_base.mkdir(parents=True, exist_ok=True)
    results: list[dict[str, Any]] = []
    errors: list[dict[str, Any]] = []
    for scenario in SCENARIOS:
        try:
            result = run_scenario(run_base, scenario)
            ev = OUT_ROOT / slug(scenario["name"])
            if ev.exists():
                shutil.rmtree(ev)
            shutil.copytree(Path(result["evidence_dir"]), ev)
            result["evidence_dir"] = str(ev)
            (ev / "summary.json").write_text(json.dumps(result, indent=2) + "\n")
            results.append(result)
        except Exception as exc:
            errors.append({"scenario": scenario["name"], "program": scenario["program"], "error": str(exc)})
    buckets: dict[str, int] = {}
    for r in results:
        buckets[r["classification"]] = buckets.get(r["classification"], 0) + 1
    manifest = {"schema": "pass158_party_control_readiness_route_probe.v1", "run_base": str(run_base), "evidence_root": str(OUT_ROOT), "source_notes": SOURCE_NOTES, "completed": len(results), "errors": errors, "buckets": buckets, "results": results}
    (OUT_ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    md = [
        "# Pass 158 — party/champion-control readiness route probe",
        "",
        "This consolidated pass probes champion selection / roster confirmation / party-control readiness before any dungeon movement claim.",
        "",
        f"- run base: `{run_base}`",
        f"- evidence root: `{OUT_ROOT}`",
        f"- scenarios: {len(SCENARIOS)}",
        f"- completed: {len(results)}",
        f"- errors: {len(errors)}",
        f"- buckets: {', '.join(f'{k}={v}' for k, v in sorted(buckets.items()))}",
        "",
        "## Source/evidence guidance used",
        "",
    ]
    md += [f"- {note}" for note in SOURCE_NOTES]
    md += ["", "## Action matrix", ""]
    for r in results:
        last = next((x for x in reversed(r["rows"]) if "sha12" in x), {})
        readiness = [x for x in r["rows"] if "readiness" in str(x.get("label", ""))]
        ready_desc = ", ".join(f"{x.get('label')}={x.get('sha12')}/{x.get('class')}" for x in readiness) or "none"
        md.append(f"- `{r['name']}` `{r['program']}`: **{r['classification']}** — {r['reason']} — final `{last.get('sha12')}`/`{last.get('class')}` — readiness: {ready_desc} — `{r['evidence_dir']}`")
    if errors:
        md += ["", "## Errors", ""]
        for e in errors:
            md.append(f"- `{e['scenario']}` `{e['program']}`: {e['error']}")
    md += ["", "## Evidence contents", "", "Each scenario directory contains raw PNG captures, `summary.json`, `pass158_rows.json`, `pass158_driver.log`, annotated bboxes, crops for viewport/right/lower/top/movement/candidate-button regions, and an initial→result diff when pixels changed."]
    (OUT_ROOT / "README.md").write_text("\n".join(md) + "\n")
    print(f"wrote {OUT_ROOT}/README.md")
    print(f"run_base={run_base}")
    print(f"completed={len(results)} errors={len(errors)}")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
