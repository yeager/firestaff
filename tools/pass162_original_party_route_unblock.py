#!/usr/bin/env python3
"""Pass162: original DM1 V1 party/control route unblock classifier.

This is Lane A from DM1_V1_FINISH_PLAN.md: stop treating any raw
`dungeon_gameplay` classifier hit as overlay-ready.  A frame sequence is only
party/control-ready if it has all three:

1. it is not a known static/no-party hash;
2. control inputs produce distinct dungeon/inventory/spell states; and
3. the sequence contains a party semantic marker (right-panel/status/inventory
   readiness), not just title/entrance/menu churn.

The pass reuses the strongest known pass141 candidate route as the first
consolidated gate and records exact source reasons for rejection/acceptance.
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
from tools.pass118_state_aware_original_route_driver import (  # noqa: E402
    wait_window,
    capture_new,
    classify_file,
    tap,
    click_original,
)
from tools.pass80_original_frame_classifier import sha256  # noqa: E402

STAGE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
DOSBOX = "/usr/bin/dosbox"
OUT_ROOT = Path("parity-evidence/verification/pass162_original_party_route_unblock")
RUN_BASE_ROOT = Path.home() / ".openclaw/data/firestaff-n2-runs"

STATIC_NO_PARTY_HASHES = {"48ed3743ab6a", "082b4d249740"}
CONTROL_CLASSES = {"inventory", "spell_panel"}
DUNGEON_CLASSES = {"dungeon_gameplay"}
UNSAFE_CLASSES = {"title_or_menu", "entrance_menu", "wall_closeup", "non_graphics_blocker"}

CROPS = {
    "viewport": (0, 0, 224, 136),
    "right_panel": (224, 0, 320, 136),
    "lower_panel": (0, 136, 320, 200),
    "top_party_strip": (0, 0, 320, 48),
    "movement_panel": (224, 112, 320, 180),
}

ROUTE_PRECONDITION = {
    "map": 0,
    "party": {"x": 1, "y": 3, "dir": "South", "raw": "0x0861"},
    "front_wall_sensor": {
        "x": 1,
        "y": 4,
        "sensor": 16,
        "thing": "0x0c10",
        "type": "C127_SENSOR_WALL_CHAMPION_PORTRAIT",
        "sensorData": 10,
    },
    "route": "no movement required after entrance gate; click source portrait center x=111,y=82 before C160/C161",
}

SOURCE_LOCKS = [
    {
        "file": "DUNGEON.DAT via pass173 helper",
        "lines": "n/a",
        "point": "Initial DM1 V1 party pose decodes to map0 x=1 y=3 dir=South (raw 0x0861), directly facing wall square x=1 y=4 with sensor 16 type C127 and sensorData=10; the deterministic route precondition is therefore an entrance gate only, not coordinate navigation.",
    },
    {
        "file": "ENTRANCE.C",
        "lines": "857-883",
        "point": "F0441_STARTEND_ProcessEntrance sets G0298_B_NewGame=C099_MODE_WAITING_ON_ENTRANCE and only exits after command/key changes it; keyboard/mouse input is processed before dungeon load.",
    },
    {
        "file": "COMMAND.C",
        "lines": "346-352, 557, 2438-2455",
        "point": "C200_COMMAND_ENTRANCE_ENTER_DUNGEON maps to C407_ZONE_ENTRANCE_ENTER / Return and sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON; resume maps to saved game, credits loops.",
    },
    {
        "file": "REVIVE.C",
        "lines": "63-150",
        "point": "F0280_CHAMPION_AddCandidateChampionToParty is the source transition that increments/uses G0305_ui_PartyChampionCount; inventory-looking candidate frames are not enough without this semantic transition.",
    },
    {
        "file": "COMMAND.C",
        "lines": "231-238, 509-511",
        "point": "C160/C161 Resurrect/Reincarnate boxes are around y=86-142 or y=90-138, not y=165; pass162 retests the corrected source-box centers x=130/y=115 and x=186/y=115.",
    },
    {
        "file": "COMMAND.C",
        "lines": "1-14, 108-114, 397-403, 1465-1662, 2045-2126, 2322-2323",
        "point": "The original command queue stores mouse-derived commands in G0432_as_CommandQueue; C007 viewport left-click maps to C080, F0365 enqueues nonzero mouse commands with X/Y, and F0380 dequeues them before dispatching C080 to F0377.",
    },
    {
        "file": "CLIKVIEW.C",
        "lines": "348-349, 407-431",
        "point": "PC click handling subtracts the viewport origin; an empty-hand hit in the C05 front-wall ornament zone calls F0372 to touch the front wall sensor.",
    },
    {
        "file": "MOVESENS.C",
        "lines": "1392, 1501-1502",
        "point": "C127_SENSOR_WALL_CHAMPION_PORTRAIT is allowed with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty.",
    },
    {
        "file": "DUNVIEW.C / COORD.C",
        "lines": "DUNVIEW.C 525,3913-3928; COORD.C 1693-1698",
        "point": "The portrait box is viewport x=96..127/y=35..63; PC viewport origin y=33 gives source screen click center x=111/y=82.",
    },
]

SCENARIOS: list[dict[str, Any]] = [
    {
        "name": "source_gated_portrait_then_resurrect",
        "program": "DM -vv -sn",
        "purpose": "Gate into actual dungeon gameplay, then click the source-locked front-wall champion portrait at screen x=111/y=82 before C160 resurrect at x=130/y=115.",
        "actions": [
            ("gate", "dungeon_gameplay"),
            ("shot", "after_gameplay_gate"),
            ("click", 111, 82), ("wait", 1.0), ("shot", "after_source_portrait_111_82"),
            ("click", 130, 115), ("wait", 1.0), ("shot", "after_source_c160_resurrect"),
            ("key", "Return"), ("wait", 1.0), ("shot", "after_confirm"),
            ("key", "Up"), ("wait", 1.0), ("shot", "after_move_up"),
            ("key", "Right"), ("wait", 1.0), ("shot", "after_turn_right"),
            ("key", "F1"), ("wait", 1.0), ("shot", "after_f1_readiness"),
            ("key", "F4"), ("wait", 1.0), ("shot", "after_f4_readiness"),
        ],
    },
    {
        "name": "source_gated_portrait_then_reincarnate",
        "program": "DM -vv -sn",
        "purpose": "Same source-gated portrait route, but use C161 reincarnate at x=186/y=115.",
        "actions": [
            ("gate", "dungeon_gameplay"),
            ("shot", "after_gameplay_gate"),
            ("click", 111, 82), ("wait", 1.0), ("shot", "after_source_portrait_111_82"),
            ("click", 186, 115), ("wait", 1.0), ("shot", "after_source_c161_reincarnate"),
            ("key", "Return"), ("wait", 1.0), ("shot", "after_confirm"),
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
    p = out / "dosbox-pass162.conf"
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


def do_action(out: Path, log: list[str], action: tuple[Any, ...], idx: int) -> dict[str, Any] | None:
    kind = action[0]
    if kind == "wait":
        time.sleep(float(action[1])); return None
    if kind == "shot":
        return {"phase": "shot", **shot(out, log, str(action[1]), idx)}
    if kind == "gate":
        target = str(action[1])
        wid = wait_window(log, timeout=5.0)
        deadline = time.time() + 22.0
        attempt = 0
        last: dict[str, Any] | None = None
        while time.time() < deadline:
            attempt += 1
            last = {"phase": "gate", **shot(out, log, f"gate_{target}_{attempt:02d}", idx)}
            if last["class"] == target:
                return last
            if last["class"] == "entrance_menu":
                tap(wid, "Return", log, delay=1.0)
            else:
                time.sleep(0.8)
        raise RuntimeError(f"state gate never observed {target}; last={last}")
    if kind == "key":
        key = str(action[1])
        wid = wait_window(log, timeout=5.0)
        xdotool_probe(log, wid, f"before-key-{key}")
        tap(wid, key, log, delay=0.6)
        xdotool_probe(log, wid, f"after-key-{key}")
        return {"phase": "key", "value": key, **shot(out, log, f"key_{key}_{idx}", idx)}
    if kind == "click":
        x, y = int(action[1]), int(action[2])
        wid = wait_window(log, timeout=5.0)
        xdotool_probe(log, wid, f"before-click-{x}-{y}")
        click_original(wid, x, y, log, delay=0.6)
        xdotool_probe(log, wid, f"after-click-{x}-{y}")
        return {"phase": "click", "x": x, "y": y, **shot(out, log, f"click_{x}_{y}_{idx}", idx)}
    raise ValueError(action)



def xdotool_probe(log: list[str], wid: str, label: str) -> None:
    """Append low-level X window/focus/mouse state around delivered inputs."""
    for args in (["getwindowgeometry", "--shell", wid], ["getwindowfocus"], ["getmouselocation", "--shell"]):
        p = subprocess.run(["xdotool", *args], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        body = (p.stdout or p.stderr).strip().replace("\n", ";")
        log.append(f"xdotool-probe {label} {' '.join(args)} rc={p.returncode} {body}")


def queue_source_probe() -> dict[str, Any]:
    """Source-centered queue path for deciding whether the original runtime should see C080/F0377."""
    return {
        "queue_storage": "COMMAND.C:1-14 defines G0432_as_CommandQueue plus first/last indices and pending click fields.",
        "viewport_to_c080": "COMMAND.C:108-114 and 397-403 map screen/zone C007 viewport left-clicks to C080_COMMAND_CLICK_IN_DUNGEON_VIEW.",
        "mouse_enqueue": "COMMAND.C:1465-1662 has F0365 derive L1109_i_Command from mouse input and enqueue nonzero commands with X/Y into G0432_as_CommandQueue.",
        "dequeue_dispatch": "COMMAND.C:2045-2126 dequeues L1160/L1161/L1162; COMMAND.C:2322-2323 dispatches C080 to F0377_COMMAND_ProcessType80_ClickInDungeonView.",
        "front_wall_to_f0280": "CLIKVIEW.C:348-349 subtracts viewport origin; CLIKVIEW.C:407-431 touches the front wall sensor; MOVESENS.C:1392 and 1501-1502 permit C127 with no leader and call F0280.",
    }

def crop_stats(path: Path) -> dict[str, Any]:
    im = Image.open(path).convert("RGB")
    stats: dict[str, Any] = {}
    for name, box in CROPS.items():
        cr = im.crop(box)
        st = ImageStat.Stat(cr)
        colors = cr.convert("P", palette=Image.Palette.ADAPTIVE, colors=64).getcolors(maxcolors=100000) or []
        nonblack = sum(1 for px in cr.getdata() if px != (0, 0, 0))
        stats[name] = {
            "mean_rgb": [round(x, 2) for x in st.mean],
            "nonblack_ratio": round(nonblack / (cr.size[0] * cr.size[1]), 6),
            "palette64_colors": len(colors),
        }
    return stats


def diff_stats(a: Path, b: Path) -> dict[str, Any]:
    ia, ib = Image.open(a).convert("RGB"), Image.open(b).convert("RGB")
    d = ImageChops.difference(ia, ib)
    bbox = d.getbbox()
    nz = sum(1 for px in d.getdata() if px != (0, 0, 0))
    return {"bbox": list(bbox) if bbox else None, "changed_pixels": nz, "changed_ratio": round(nz / (320 * 200), 6)}


def input_delivery_probe(log: list[str]) -> dict[str, Any]:
    """Record exact xdotool delivery evidence for the final blocker report."""
    return {
        "clicks": [line for line in log if line.startswith("click ")],
        "keys": [line for line in log if line.startswith("key ")],
        "windows": [line for line in log if line.startswith("window-found ")],
        "xdotool_probes": [line for line in log if line.startswith("xdotool-probe ")],
        "source_queue_path": queue_source_probe(),
    }


def classify_route(rows: list[dict[str, Any]], log: list[str]) -> tuple[str, str, dict[str, Any]]:
    shots = [r for r in rows if "sha12" in r]
    hashes = [r["sha12"] for r in shots]
    classes = [r["class"] for r in shots]
    static_hits = sorted(set(hashes) & STATIC_NO_PARTY_HASHES)
    dungeon = [r for r in shots if r["class"] in DUNGEON_CLASSES]
    control = [r for r in shots if r["class"] in CONTROL_CLASSES]
    unsafe_after_route = [r for r in shots[-5:] if r["class"] in UNSAFE_CLASSES]
    diffs = []
    for prev, cur in zip(shots, shots[1:]):
        diffs.append({"from": prev["label"], "to": cur["label"], **diff_stats(Path(prev["path"]), Path(cur["path"]))})
    portrait_delta = [d for d in diffs if "source_portrait_111_82" in d["to"] or d["to"].startswith("click_111_82")]
    choice_delta = [d for d in diffs if "source_c160_resurrect" in d["to"] or "source_c161_reincarnate" in d["to"] or d["to"].startswith("click_130_115") or d["to"].startswith("click_186_115")]
    dynamic_dungeon = [d for d in diffs if d["changed_ratio"] > 0.01 and any(tok in d["to"] for tok in ("move", "turn", "key_Up", "key_Right", "key_Left"))]
    evidence = {
        "hashes": hashes,
        "classes": classes,
        "unique_hashes": sorted(set(hashes)),
        "static_hits": static_hits,
        "dungeon_count": len(dungeon),
        "control_count": len(control),
        "unsafe_tail_count": len(unsafe_after_route),
        "portrait_click_delta": portrait_delta,
        "choice_delta": choice_delta,
        "dynamic_dungeon_inputs": dynamic_dungeon,
        "route_precondition": ROUTE_PRECONDITION,
        "input_delivery_probe": input_delivery_probe(log),
        "input_queue_source_probe": queue_source_probe(),
        "next_exact_input_candidate": "after the entrance gate, the source route has no movement step: the party is already at map0 (1,3,S) facing C127 sensor 16. This pass now records xdotool focus/geometry/mouse state and the source command-queue C007->C080->F0377 path; clicks/keys reach the active DOSBox window, but all post-gate captures remain hash 48ed3743ab6a with zero-pixel deltas, so the next exact blocker is an in-process original queue trace/breakpoint proving whether C080 enters F0380 or dies before F0377.",
    }
    if not dungeon:
        return "blocked/no-dungeon", "route never produced a dungeon gameplay frame", evidence
    if not portrait_delta or max(d["changed_ratio"] for d in portrait_delta) < 0.001:
        return "blocked/portrait-c080-no-visible-delta", "gated gameplay reached, but source portrait click x=111/y=82 produced no visible candidate transition; blocker is now C007/C080 mouse delivery or front-wall hit-state mismatch before F0280", evidence
    if not choice_delta or max(d["changed_ratio"] for d in choice_delta) < 0.001:
        return "blocked/choice-no-visible-delta", "portrait click changed state, but C160/C161 choice did not visibly confirm", evidence
    if unsafe_after_route:
        return "blocked/unsafe-tail", "tail still contains title/entrance/menu frames, not stable gameplay", evidence
    tail_hashes = {r["sha12"] for r in shots[-6:]}
    if tail_hashes & STATIC_NO_PARTY_HASHES:
        return "blocked/static-no-party-after-source-route", f"source route returned to known static no-party hash: {', '.join(sorted(tail_hashes & STATIC_NO_PARTY_HASHES))}", evidence
    if not dynamic_dungeon:
        return "blocked/no-input-delta", "movement/control inputs did not produce non-trivial dungeon deltas", evidence
    if not control:
        return "blocked/no-party-control-marker", "dynamic dungeon exists but no inventory/spell/control marker proves a recruited party", evidence
    return "party-control-ready-candidate", "non-static dynamic dungeon plus control marker; next pass can capture overlay quartet", evidence


def run_scenario(run_base: Path, scenario: dict[str, Any]) -> dict[str, Any]:
    out = run_base / slug(scenario["name"])
    out.mkdir(parents=True, exist_ok=True)
    log: list[str] = []
    rows: list[dict[str, Any]] = []
    proc = subprocess.Popen([DOSBOX, "-conf", str(conf(out, scenario["program"]))], stdout=(out / "dosbox.log").open("w"), stderr=subprocess.STDOUT, text=True)
    try:
        wait_window(log, timeout=6.0)
        time.sleep(7.0)
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
        (out / "pass162_driver.log").write_text("\n".join(log) + "\n")
    classification, reason, evidence = classify_route(rows, log)
    summary = {"name": scenario["name"], "program": scenario["program"], "purpose": scenario["purpose"], "classification": classification, "reason": reason, "source_locks": SOURCE_LOCKS, "route_precondition": ROUTE_PRECONDITION, "route_evidence": evidence, "rows": rows, "evidence_dir": str(out)}
    (out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    return summary


def main() -> int:
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    run_base = RUN_BASE_ROOT / (time.strftime("%Y%m%d-%H%M%S") + "-pass162-original-party-route-unblock")
    run_base.mkdir(parents=True, exist_ok=True)
    results, errors = [], []
    for scenario in SCENARIOS:
        try:
            result = run_scenario(run_base, scenario)
            ev = OUT_ROOT / slug(scenario["name"])
            if ev.exists(): shutil.rmtree(ev)
            shutil.copytree(Path(result["evidence_dir"]), ev)
            result["evidence_dir"] = str(ev)
            (ev / "summary.json").write_text(json.dumps(result, indent=2) + "\n")
            results.append(result)
        except Exception as exc:
            errors.append({"scenario": scenario["name"], "program": scenario["program"], "error": str(exc)})
    buckets: dict[str, int] = {}
    for r in results: buckets[r["classification"]] = buckets.get(r["classification"], 0) + 1
    manifest = {"schema": "pass162_original_party_route_unblock.v1", "run_base": str(run_base), "evidence_root": str(OUT_ROOT), "source_locks": SOURCE_LOCKS, "route_precondition": ROUTE_PRECONDITION, "completed": len(results), "errors": errors, "buckets": buckets, "results": results}
    (OUT_ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    lines = ["# Pass 162 — original party/control route unblock", "", "Lane A gate from `DM1_V1_FINISH_PLAN.md`.", "", f"- run base: `{run_base}`", f"- evidence root: `{OUT_ROOT}`", f"- completed: {len(results)}", f"- errors: {len(errors)}", f"- buckets: {', '.join(f'{k}={v}' for k, v in sorted(buckets.items()))}", "", "## Source locks", ""]
    lines += [f"- `{s['file']}` {s['lines']}: {s['point']}" for s in SOURCE_LOCKS]
    lines += [
        "",
        "## Route precondition",
        "",
        f"- Initial party: map{ROUTE_PRECONDITION['map']} x={ROUTE_PRECONDITION['party']['x']} y={ROUTE_PRECONDITION['party']['y']} dir={ROUTE_PRECONDITION['party']['dir']} raw={ROUTE_PRECONDITION['party']['raw']}.",
        f"- Front wall: x={ROUTE_PRECONDITION['front_wall_sensor']['x']} y={ROUTE_PRECONDITION['front_wall_sensor']['y']} sensor={ROUTE_PRECONDITION['front_wall_sensor']['sensor']} type={ROUTE_PRECONDITION['front_wall_sensor']['type']} sensorData={ROUTE_PRECONDITION['front_wall_sensor']['sensorData']}.",
        "- No movement is required after the entrance gate; the runtime mismatch is now whether the original C080/F0377 click reaches F0280, not map navigation.",
        "",
        "## Result matrix",
        "",
    ]
    for r in results:
        ev = r["route_evidence"]
        lines.append(f"- `{r['name']}` `{r['program']}`: **{r['classification']}** — {r['reason']} — unique hashes: {', '.join(ev['unique_hashes'])} — `{r['evidence_dir']}`")
    if errors:
        lines += ["", "## Errors", ""]
        lines += [f"- `{e['scenario']}` `{e['program']}`: {e['error']}" for e in errors]
    lines += ["", "## Interpretation", "", "A route is not overlay-ready merely because `pass80_original_frame_classifier` says `dungeon_gameplay`. This pass now gates into real dungeon gameplay first, records the source-proven initial C127 pose, records xdotool focus/geometry/mouse delivery and the source C007->C080->F0380->F0377 queue path, then requires a visible source portrait/C080 candidate transition before C160/C161 and party-control markers. A zero-delta x=111/y=82 portrait click is now narrowed to an in-process original queue trace/breakpoint question: does C080 reach F0380/F0377 at all, or is DOSBox/PC mouse translation dying before the original queue? It is not a request for more map or panel coordinate guessing."]
    (OUT_ROOT / "README.md").write_text("\n".join(lines) + "\n")
    print(f"wrote {OUT_ROOT}/README.md")
    print(f"run_base={run_base}")
    print(f"completed={len(results)} errors={len(errors)} buckets={buckets}")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
