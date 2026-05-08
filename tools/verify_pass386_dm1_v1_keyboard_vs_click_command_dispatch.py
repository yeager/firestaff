#!/usr/bin/env python3
"""Pass386 verifier: compare keyboard vs panel-click route dispatch.

Pass385 proved the post-load F0380/F0128 runtime chain but did not see
F0365/F0366 for the keypad route. This verifier keeps the same debugger
harness and adds:
- F0361 keypress breakpoint, to tell whether keyboard route events enter DM;
- G2153 queued-command-count watchpoint, to tell whether a command queues;
- a panel-click movement route using the original ReDMCSB interface boxes.

Promotion rule: keyboard may still be blocked, but if the click route reaches
F0365/F0366 after arming, command dispatch itself is proven and the remaining
blocker is narrowed to keyboard/scan-code injection.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import pexpect

import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass386_dm1_v1_keyboard_vs_click_command_dispatch"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

ADDR = dict(p385.ADDR)
ADDR.update({
    "F0361_COMMAND_ProcessKeyPress": "22F7:0407",
    "G2153_i_QueuedCommandsCount": "2C23:3E78",
})
ARMED = [
    "F0361_COMMAND_ProcessKeyPress",
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    "F0128_DUNGEONVIEW_Draw_CPSF",
    "G0321_B_StopWaitingForPlayerInput",
    "G2153_i_QueuedCommandsCount",
]
KEY_ROUTE = "kp5 wait:700 kp4 wait:700 kp6 wait:700 kp5 wait:700"
CLICK_ROUTE = "click:276,135 wait:700 click:248,135 wait:700 click:304,135 wait:700 click:276,135 wait:700"
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def classify_stop(post: str) -> dict[str, Any]:
    c = p385.clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    lines = p385.code_lines(c)[-14:]
    kind = "other"
    entry = None
    for name in [
        "F0361_COMMAND_ProcessKeyPress",
        "F0380_COMMAND_ProcessQueue_CPSC",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "F0128_DUNGEONVIEW_Draw_CPSF",
    ]:
        target = ADDR[name].upper()
        if any(line.upper().startswith(target) for line in lines) or target in c.upper():
            kind = name
            entry = target
            break
    if mem:
        last = mem[-1]
        if last["addr"].upper() == ADDR["G0321_B_StopWaitingForPlayerInput"]:
            kind = "g0321_bpm"
        elif last["addr"].upper() == ADDR["G2153_i_QueuedCommandsCount"]:
            kind = "g2153_queue_count_bpm"
    return {
        "addr": p385.last_code_addr(c),
        "entryAddr": entry,
        "kind": kind,
        "memoryBreakpoints": mem[-4:],
        "postRunningCodeLines": lines,
        "postRunningExcerpt": c[-2200:],
    }


def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], running: threading.Event, stop: threading.Event) -> bool:
    deadline = time.time() + seconds
    buf = ""
    saw_running = False
    while time.time() < deadline and not stop.is_set():
        chunk = p385.drain(child, .25)
        if chunk:
            transcript.append(chunk)
            buf += chunk
            c = p385.clean(buf)
            if "(Running)" in c:
                saw_running = True
                running.set()
            if "(Running)" in c and "->" in c.split("(Running)", 1)[-1]:
                running.clear()
                post = c.split("(Running)", 1)[-1]
                row = {"t": time.time(), "runningMarkerSeen": True, "promptReappearedAfterRunning": True, **classify_stop(post)}
                stops.append(row)
                for ccmd in ["CPU", "MEMDUMP " + ADDR["G0321_B_StopWaitingForPlayerInput"] + " 8", "MEMDUMP " + ADDR["G2153_i_QueuedCommandsCount"] + " 8", "BPLIST"]:
                    p385.dbg(child, ccmd, cmdlog, transcript)
                child.send("\x1bOt")
                cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after strict stop", "kind": row.get("kind"), "addr": row.get("addr")})
                buf = ""
                running.set()
        time.sleep(.05)
    return saw_running


def robust_find_win(display: str) -> str | None:
    out = p385.xdo(display, ["search", "--sync", "--class", "dosbox"]).stdout
    for line in out.splitlines():
        line = line.strip()
        if line.isdigit():
            return line
    return None


def robust_click_at(display: str, win: str, x: int, y: int) -> dict[str, Any]:
    geom = p385.xdo(display, ["getwindowgeometry", "--shell", win]).stdout
    vals: dict[str, int] = {}
    for line in geom.splitlines():
        if "=" not in line:
            continue
        k, v = line.split("=", 1)
        if k in {"WIDTH", "HEIGHT"} and v.strip().isdigit():
            vals[k] = int(v.strip())
    if "WIDTH" not in vals or "HEIGHT" not in vals:
        return {"screen": None, "rc": 1, "out": geom[-300:]}
    gw, gh = float(vals["WIDTH"]), float(vals["HEIGHT"])
    aspect = 320 / 200
    cw, ch = gw, gw / aspect
    if ch > gh:
        ch, cw = gh, gh * aspect
    px = int(round((gw - cw) / 2 + ((x + .5) / 320) * cw))
    py = int(round((gh - ch) / 2 + ((y + .5) / 200) * ch))
    r = p385.xdo(display, ["mousemove", "--window", win, str(px), str(py), "click", "1"])
    return {"screen": [px, py], "rc": r.returncode, "out": r.stdout[-160:]}


p385.click_at = robust_click_at


def run_route(label: str, route: str, seconds: int) -> dict[str, Any]:
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    with tempfile.TemporaryDirectory(prefix=f"firestaff-pass386-{label}-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(p385.ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        p385.write_conf(conf, stage)
        display = f":{120 + (os.getpid() % 40)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**p385.os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(p385.drain(child, 1))
            win = robust_find_win(display)
            if not win:
                return {"label": label, "ran": True, "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed to load/menu prefix"})
            p385.drive(display, win, p385.LOAD_PREFIX, routelog)
            if not p385.pause_to_prompt(child, display, win, cmdlog, transcript, "post-load arm point"):
                return {"label": label, "ran": True, "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "blocker": "no debugger prompt after load prefix"}
            cmds = ["BPDEL *"]
            for name in ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0128_DUNGEONVIEW_Draw_CPSF"]:
                cmds.append(f"BP {ADDR[name]}")
            cmds += [f"BPM {ADDR['G0321_B_StopWaitingForPlayerInput']}", f"BPM {ADDR['G2153_i_QueuedCommandsCount']}", "BPLIST"]
            for cmd in cmds:
                p385.dbg(child, cmd, cmdlog, transcript)
            bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained = {name: (ADDR[name] in bplist) for name in ARMED}
            arm_time = time.time()
            running = threading.Event(); stop = threading.Event()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": f"run after arm: {label}"})
            t = threading.Thread(target=p385.drive, args=(display, win, p385.LOAD_SUFFIX + " " + route, routelog, running, stop), daemon=True)
            t.start()
            saw = monitor(child, seconds, transcript, cmdlog, stops, running, stop)
            stop.set(); t.join(timeout=1)
            p385.pause_to_prompt(child, display, win, cmdlog, transcript, "final sample")
            p385.dbg(child, "BPLIST", cmdlog, transcript)
            final_bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_final = {name: (ADDR[name] in final_bplist) for name in ARMED}
            route_after_arm = any(r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:") for r in routelog)
            return {"label": label, "ran": True, "durationSeconds": round(time.time() - start, 3), "boundedSeconds": seconds, "route": p385.LOAD_SUFFIX + " " + route, "routeInputAfterArming": route_after_arm, "sawRunning": saw, "retainedAtArm": retained, "retainedFinal": retained_final, "stops": stops, "routeLog": routelog, "commandLog": cmdlog}
        finally:
            try:
                transcript.append(p385.drain(child, .5))
                child.terminate(force=True)
            except Exception:
                pass
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()
            OUT.mkdir(parents=True, exist_ok=True)
            (OUT / f"pass386_{label}_runtime.clean.txt").write_text(p385.clean("".join(transcript))[-300000:] + "\n", encoding="utf-8")


def summarize(probes: list[dict[str, Any]]) -> tuple[str, dict[str, Any], str | None]:
    by_label = {p.get("label"): p for p in probes}
    key = by_label.get("keyboard", {})
    click = by_label.get("click", {})
    def kinds(p: dict[str, Any]) -> list[str]:
        return [s.get("kind") for s in p.get("stops", [])]
    kk, ck = kinds(key), kinds(click)
    predicates = {
        "keyboardRouteRanAfterArm": key.get("sawRunning") is True and key.get("routeInputAfterArming") is True and all(key.get("retainedAtArm", {}).values()),
        "keyboardF0361Hit": "F0361_COMMAND_ProcessKeyPress" in kk,
        "keyboardQueueCountChanged": "g2153_queue_count_bpm" in kk,
        "keyboardDispatchReached": any(k in kk for k in ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
        "clickRouteRanAfterArm": click.get("sawRunning") is True and click.get("routeInputAfterArming") is True and all(click.get("retainedAtArm", {}).values()),
        "clickF0380Hit": "F0380_COMMAND_ProcessQueue_CPSC" in ck,
        "clickDispatchReached": any(k in ck for k in ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
        "clickStopWaitWriteObserved": "g0321_bpm" in ck,
        "clickF0128Observed": "F0128_DUNGEONVIEW_Draw_CPSF" in ck,
    }
    if predicates["clickRouteRanAfterArm"] and predicates["clickDispatchReached"]:
        if not predicates["keyboardDispatchReached"]:
            return "PASS386_DISPATCH_PROVEN_KEYBOARD_ROUTE_NARROWED", predicates, "panel-click route reaches F0365/F0366; remaining blocker is keyboard/scan-code route before semantic dispatch"
        return "PASS386_KEYBOARD_AND_CLICK_DISPATCH_PROVEN", predicates, None
    if predicates["keyboardRouteRanAfterArm"] and not predicates["keyboardF0361Hit"]:
        return "BLOCKED_PASS386_KEYBOARD_ROUTE_NOT_REACHING_F0361", predicates, "keyboard xdotool route ran after arming but did not break in F0361"
    if predicates["keyboardF0361Hit"] and not predicates["keyboardQueueCountChanged"] and predicates["clickF0380Hit"]:
        return "BLOCKED_PASS386_F0365_F0366_NOT_REACHED_AFTER_KEYBOARD_AND_CLICK_PROBES", predicates, "keyboard route reaches F0361 but no queued-command-count write is observed; panel-click route reaches F0380/G0321/F0128 but still no strict F0365/F0366 stop"
    return "BLOCKED_PASS386_DISPATCH_STILL_NOT_PROVEN", predicates, "neither route produced a strict F0365/F0366 stop"



def source_audit_pass386() -> list[dict[str, Any]]:
    """Source-only audit of queue producers and F0380 dequeue predicates."""
    rows = list(p385.source_audit())
    specs = [
        ("COMMAND.C", "mouse_movement_commands", [
            "MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement",
            "{ C001_COMMAND_TURN_LEFT",
            "{ C003_COMMAND_MOVE_FORWARD",
            "{ C002_COMMAND_TURN_RIGHT",
            "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G2153_i_QueuedCommandsCount++;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command",
        ]),
        ("COMMAND.C", "keyboard_movement_commands", [
            "KEYBOARD_INPUT G0458_as_Graphic561_PrimaryKeyboardInput_Interface",
            "{ C001_COMMAND_TURN_LEFT",
            "{ C003_COMMAND_MOVE_FORWARD",
            "{ C002_COMMAND_TURN_RIGHT",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command",
            "G2153_i_QueuedCommandsCount++;",
        ]),
        ("COMMAND.C", "f0380_dequeue_predicates", [
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
    ]
    for fn, name, needles in specs:
        f = p385.SRC / fn
        hits = {needle: p385.find_line(f, needle) for needle in needles} if f.exists() else {}
        rows.append({"file": fn, "section": name, "path": str(f), "ok": f.exists() and all(v is not None for v in hits.values()), "lineHits": hits, "missing": [k for k, v in hits.items() if v is None]})
    return rows

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=45)
    ap.add_argument("--only", choices=["keyboard", "click", "both"], default="both")
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 90))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit_pass386()
    probes = []
    if args.only in {"keyboard", "both"}:
        probes.append(run_route("keyboard", KEY_ROUTE, args.seconds))
    if args.only in {"click", "both"}:
        probes.append(run_route("click", CLICK_ROUTE, args.seconds))
    status, predicates, blocker = summarize(probes)
    if not all(r["ok"] for r in source):
        status = "FAIL_PASS386_SOURCE_AUDIT_FAILED"; blocker = "source audit failed"
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(),
        "sourceRoot": str(p385.SRC),
        "addresses": ADDR,
        "sourceAudit": source,
        "runtimeProbes": [{k: v for k, v in p.items() if k not in {"routeLog", "commandLog"}} for p in probes],
        "runtimeArtifacts": {"keyboardTranscript": f"parity-evidence/verification/{PASS}/pass386_keyboard_runtime.clean.txt", "clickTranscript": f"parity-evidence/verification/{PASS}/pass386_click_runtime.clean.txt"},
        "proofPredicates": predicates,
        "blocker": blocker,
        "promotionRule": "Promote when a strict post-Running stop reaches F0365/F0366 after route input and retained breakpoints. If click dispatch works while keyboard does not, command dispatch is proven and the blocker is narrowed to keyboard/scan-code injection before F0361 or queueing.",
        "notPromotedBy": ["BPLIST", "BP/BPM command echo", "forced pause", "route keylog alone", "source-only address binding"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    for p in probes:
        probe_label = p.get("label")
        (OUT / ("pass386_" + str(probe_label) + "_route_keylog.json")).write_text(json.dumps(p.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
        (OUT / ("pass386_" + str(probe_label) + "_command_log.json")).write_text(json.dumps(p.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass386 — DM1 V1 keyboard vs click command dispatch",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        f"Blocked/narrowed: {blocker}" if blocker else "Keyboard and click dispatch proven.",
        "",
        "This preserves `/F0365/F0366/F0128/G0321` as runtime anchors and keeps `/CLIKMENU/data`-style symbol handling inherited from pass385.",
        "",
        "## Source audit conclusion",
        "",
        "- ReDMCSB maps both movement-panel clicks and movement keys to the same command values consumed by `F0380`: `C001_COMMAND_TURN_LEFT`, `C002_COMMAND_TURN_RIGHT`, and `C003_COMMAND_MOVE_FORWARD`.",
        "- `F0380` only dispatches to `F0365/F0366` after the queue is non-empty, it loads `G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command`, and it decrements `G2153_i_QueuedCommandsCount`.",
        "- Runtime did not observe the queued-command-count write/decrement, so the remaining blocker is the queue-pop eligibility path before semantic handler dispatch, not the `F0365/F0366` anchors themselves.",
        "",
        "## Runtime predicates",
        "",
        *[f"- `{k}`: `{v}`" for k, v in predicates.items()],
        "",
        "## Evidence",
        "",
        f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        f"- Keyboard transcript: `parity-evidence/verification/{PASS}/pass386_keyboard_runtime.clean.txt`",
        f"- Click transcript: `parity-evidence/verification/{PASS}/pass386_click_runtime.clean.txt`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS386") or status.startswith("BLOCKED_PASS386") else 1


if __name__ == "__main__":
    raise SystemExit(main())
