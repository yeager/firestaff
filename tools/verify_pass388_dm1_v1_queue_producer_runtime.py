#!/usr/bin/env python3
"""Pass388: runtime-probe DM1 V1 command-queue producer side.

This pass narrows pass387's F0380 empty-queue blocker by watching the actual
source-locked producers (mouse F0359 / pending replay F0360 / keyboard F0361)
and the queue-count storage while the original route is delivered after arming.
"""
from __future__ import annotations

import argparse
import json
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
import verify_pass386_dm1_v1_keyboard_vs_click_command_dispatch as p386

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass388_dm1_v1_queue_producer_runtime"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

ADDR = dict(p386.ADDR)
ADDR.update({
    "F0359_COMMAND_ProcessClick_CPSC": "22F7:030D",
    "F0360_COMMAND_ProcessPendingClick": "22F7:03F0",
    "G0436_B_PendingClickPresent": "2C23:3EC6",
})
ARMED_CODE = [
    "F0359_COMMAND_ProcessClick_CPSC",
    "F0360_COMMAND_ProcessPendingClick",
    "F0361_COMMAND_ProcessKeyPress",
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    "F0128_DUNGEONVIEW_Draw_CPSF",
]
ARMED_MEM = [
    "G0321_B_StopWaitingForPlayerInput",
    "G2153_i_QueuedCommandsCount",
    "G0436_B_PendingClickPresent",
]
KEY_ROUTE = p386.KEY_ROUTE
CLICK_ROUTE = p386.CLICK_ROUTE
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)
BPLIST_VAL_RE = re.compile(r"BPMEM\s+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+\((?P<value>[0-9A-F]{2})\)", re.I)


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def source_audit() -> list[dict[str, Any]]:
    rows = p386.source_audit_pass386()
    specs = [
        ("COMMAND.C", "click_producer_and_pending_replay", [
            "void F0359_COMMAND_ProcessClick_CPSC",
            "G0436_B_PendingClickPresent = C1_TRUE",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G2153_i_QueuedCommandsCount++;",
            "void F0360_COMMAND_ProcessPendingClick",
            "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus);",
        ]),
        ("COMMAND.C", "keyboard_producer_enqueue", [
            "void F0361_COMMAND_ProcessKeyPress",
            "G0444_ps_SecondaryKeyboardInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command",
            "G2153_i_QueuedCommandsCount++;",
        ]),
        ("COMMAND.C", "f0380_empty_before_pop", [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "F0360_COMMAND_ProcessPendingClick();",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
    ]
    for fn, section, needles in specs:
        f = p385.SRC / fn
        hits = {n: p385.find_line(f, n) for n in needles} if f.exists() else {}
        rows.append({
            "file": fn,
            "section": section,
            "path": str(f),
            "ok": f.exists() and all(v is not None for v in hits.values()),
            "lineHits": hits,
            "missing": [k for k, v in hits.items() if v is None],
        })
    return rows


def bplist_values(text: str) -> dict[str, int]:
    out: dict[str, int] = {}
    for m in BPLIST_VAL_RE.finditer(p385.clean(text)):
        out[m.group("addr").upper()] = int(m.group("value"), 16)
    return out


def classify_stop(post: str) -> dict[str, Any]:
    c = p385.clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    lines = p385.code_lines(c)[-16:]
    kind = "other"
    entry = None
    for name in ARMED_CODE:
        target = ADDR[name].upper()
        if any(line.upper().startswith(target) for line in lines) or target in c.upper():
            kind = name
            entry = target
            break
    if mem:
        last = mem[-1]
        a = last["addr"].upper()
        if a == ADDR["G0321_B_StopWaitingForPlayerInput"]:
            kind = "g0321_bpm"
        elif a == ADDR["G2153_i_QueuedCommandsCount"]:
            kind = "g2153_queue_count_bpm"
        elif a == ADDR["G0436_B_PendingClickPresent"]:
            kind = "g0436_pending_click_bpm"
    return {
        "addr": p385.last_code_addr(c),
        "entryAddr": entry,
        "kind": kind,
        "memoryBreakpoints": mem[-5:],
        "postRunningCodeLines": lines,
        "postRunningExcerpt": c[-2400:],
    }


def dbg(child: pexpect.spawn, cmd: str, log: list[dict[str, Any]], transcript: list[str], wait: float = .55) -> str:
    child.sendline(cmd)
    time.sleep(.22)
    out = p385.drain(child, wait)
    transcript.append(out)
    log.append({"t": time.time(), "cmd": cmd, "excerpt": p385.clean(out)[-900:]})
    return out


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
                row = {"t": time.time(), "runningMarkerSeen": True, "promptReappearedAfterRunning": True, **classify_stop(c.split("(Running)", 1)[-1])}
                bpout = dbg(child, "BPLIST", cmdlog, transcript)
                vals = bplist_values(bpout)
                row["bpmCurrentValues"] = {
                    "G2153_i_QueuedCommandsCount": vals.get(ADDR["G2153_i_QueuedCommandsCount"].upper()),
                    "G0436_B_PendingClickPresent": vals.get(ADDR["G0436_B_PendingClickPresent"].upper()),
                    "G0321_B_StopWaitingForPlayerInput": vals.get(ADDR["G0321_B_StopWaitingForPlayerInput"].upper()),
                }
                stops.append(row)
                dbg(child, "CPU", cmdlog, transcript)
                child.send("\x1bOt")
                cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after producer/queue stop", "kind": row.get("kind")})
                buf = ""
                running.set()
        time.sleep(.05)
    return saw_running


def run_route(label: str, route: str, seconds: int) -> dict[str, Any]:
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    with tempfile.TemporaryDirectory(prefix=f"firestaff-pass388-{label}-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(p385.ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        p385.write_conf(conf, stage)
        display = f":{160 + (p385.os.getpid() % 40)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**p385.os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(p385.drain(child, 1))
            win = p386.robust_find_win(display)
            if not win:
                return {"label": label, "ran": True, "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed to load/menu prefix"})
            p385.drive(display, win, p385.LOAD_PREFIX, routelog)
            if not p385.pause_to_prompt(child, display, win, cmdlog, transcript, "post-load arm point"):
                return {"label": label, "ran": True, "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "blocker": "no debugger prompt after load prefix"}
            cmds = ["BPDEL *"] + [f"BP {ADDR[n]}" for n in ARMED_CODE] + [f"BPM {ADDR[n]}" for n in ARMED_MEM] + ["BPLIST"]
            for cmd in cmds:
                dbg(child, cmd, cmdlog, transcript)
            bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained = {n: (ADDR[n] in bplist) for n in ARMED_CODE + ARMED_MEM}
            arm_time = time.time()
            running = threading.Event()
            stop = threading.Event()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": f"run after arm: {label}"})
            t = threading.Thread(target=p385.drive, args=(display, win, p385.LOAD_SUFFIX + " " + route, routelog, running, stop), daemon=True)
            t.start()
            saw = monitor(child, seconds, transcript, cmdlog, stops, running, stop)
            stop.set()
            t.join(timeout=1)
            p385.pause_to_prompt(child, display, win, cmdlog, transcript, "final sample")
            dbg(child, "BPLIST", cmdlog, transcript)
            final_bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_final = {n: (ADDR[n] in final_bplist) for n in ARMED_CODE + ARMED_MEM}
            route_inputs_after_arm = [r for r in routelog if r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:")]
            route_after_arm = bool(route_inputs_after_arm)
            route_success_after_arm = any(r.get("rc") == 0 for r in route_inputs_after_arm)
            route_failures_after_arm = [
                {
                    "route_item": r.get("route_item"),
                    "rc": r.get("rc"),
                    "out": str(r.get("out", ""))[-240:],
                }
                for r in route_inputs_after_arm
                if r.get("rc") not in (None, 0)
            ]
            return {
                "label": label,
                "ran": True,
                "durationSeconds": round(time.time() - start, 3),
                "boundedSeconds": seconds,
                "route": p385.LOAD_SUFFIX + " " + route,
                "routeInputAfterArming": route_after_arm,
                "routeInputAfterArmingSucceeded": route_success_after_arm,
                "routeInputFailuresAfterArming": route_failures_after_arm,
                "sawRunning": saw,
                "retainedAtArm": retained,
                "retainedFinal": retained_final,
                "stops": stops,
                "routeLog": routelog,
                "commandLog": cmdlog,
            }
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
            (OUT / f"pass388_{label}_runtime.clean.txt").write_text(p385.clean("".join(transcript))[-350000:] + "\n", encoding="utf-8")


def summarize(probes: list[dict[str, Any]]) -> tuple[str, dict[str, Any], str | None]:
    all_stops = [s for p in probes for s in p.get("stops", [])]
    kinds = [s.get("kind") for s in all_stops]
    f0380_samples = [s for s in all_stops if s.get("kind") == "F0380_COMMAND_ProcessQueue_CPSC"]
    f0380_counts = [s.get("bpmCurrentValues", {}).get("G2153_i_QueuedCommandsCount") for s in f0380_samples]
    route_control_ok = all(p.get("sawRunning") is True and p.get("routeInputAfterArming") is True and all(p.get("retainedAtArm", {}).values()) for p in probes)
    route_input_ok = all(p.get("routeInputAfterArmingSucceeded") is True for p in probes)
    route_ok = route_control_ok and route_input_ok
    predicates = {
        "routesRanAfterArmingWithBreakpointsRetained": route_ok,
        "routeControlReachedAfterArmingWithBreakpointsRetained": route_control_ok,
        "routeInputAfterArmingSucceeded": route_input_ok,
        "clickProducerEntryHit": "F0359_COMMAND_ProcessClick_CPSC" in kinds,
        "pendingReplayHit": "F0360_COMMAND_ProcessPendingClick" in kinds,
        "keyboardProducerEntryHit": "F0361_COMMAND_ProcessKeyPress" in kinds,
        "queueCountWriteObserved": "g2153_queue_count_bpm" in kinds,
        "pendingClickWriteObserved": "g0436_pending_click_bpm" in kinds,
        "f0380Reached": bool(f0380_samples),
        "f0380QueueCountPositiveImmediatelyBefore": any((v or 0) > 0 for v in f0380_counts),
        "f0380QueueCountSamples": f0380_counts,
        "dispatchReached": any(k in kinds for k in ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
    }
    if predicates["f0380QueueCountPositiveImmediatelyBefore"]:
        return "PASS388_A_PRODUCER_MAKES_QUEUE_COUNT_POSITIVE_BEFORE_F0380", predicates, None
    if route_control_ok and not route_input_ok:
        return "BLOCKED_PASS388_ROUTE_INPUT_NOT_DELIVERED_AFTER_ARMING", predicates, "debugger arming/control reached the post-load loop with breakpoints retained, but no keyboard route input after arming returned rc=0; this is not proof of an F0380 empty queue"
    if route_ok and predicates["f0380Reached"] and not predicates["queueCountWriteObserved"] and not predicates["dispatchReached"]:
        return "BLOCKED_PASS388_B_ORIGINAL_ROUTE_NEVER_HITS_ENQUEUE_BRANCH", predicates, "runtime reached F0380 with G2153 byte sampled as zero and no G2153 write/watchpoint under producer-side arming"
    return "FAIL_PASS388_RUNTIME_CONTROL_OR_PREDICATE_INCOMPLETE", predicates, "producer/F0380 runtime predicates incomplete"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=45)
    ap.add_argument("--only", choices=["keyboard", "click", "both"], default="both")
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 90))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    probes = []
    if args.only in {"keyboard", "both"}:
        probes.append(run_route("keyboard", KEY_ROUTE, args.seconds))
    if args.only in {"click", "both"}:
        probes.append(run_route("click", CLICK_ROUTE, args.seconds))
    status, predicates, blocker = summarize(probes)
    if not all(r["ok"] for r in source):
        status = "FAIL_PASS388_SOURCE_AUDIT_FAILED"
        blocker = "source audit failed"
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
        "runtimeArtifacts": {
            "keyboardTranscript": f"parity-evidence/verification/{PASS}/pass388_keyboard_runtime.clean.txt",
            "clickTranscript": f"parity-evidence/verification/{PASS}/pass388_click_runtime.clean.txt",
        },
        "proofPredicates": predicates,
        "blocker": blocker,
        "decisionRule": "A only if a post-arming F0380 stop samples G2153_i_QueuedCommandsCount > 0 before F0380 executes; B if routes/arms are retained, F0380 is reached, no G2153 write/watchpoint occurs, and dispatch is not reached.",
        "notPromotedBy": ["BPLIST setup echo", "route keylog alone", "source-only address binding", "forced pause without route-after-arm"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    for p in probes:
        label = str(p.get("label"))
        (OUT / f"pass388_{label}_route_keylog.json").write_text(json.dumps(p.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
        (OUT / f"pass388_{label}_command_log.json").write_text(json.dumps(p.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass388 — DM1 V1 queue producer runtime",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        blocker or "Producer path made the queue count positive before F0380.",
        "",
        "## Predicate summary",
        "",
        *[f"- `{k}`: `{v}`" for k, v in predicates.items()],
        "",
        "## Evidence",
        "",
        f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        f"- Keyboard transcript: `parity-evidence/verification/{PASS}/pass388_keyboard_runtime.clean.txt`",
        f"- Click transcript: `parity-evidence/verification/{PASS}/pass388_click_runtime.clean.txt`",
        "",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS388") or status.startswith("BLOCKED_PASS388") else 1


if __name__ == "__main__":
    raise SystemExit(main())
