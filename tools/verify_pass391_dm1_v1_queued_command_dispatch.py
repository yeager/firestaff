#!/usr/bin/env python3
"""Pass391 verifier: keyboard queue write through F0380 semantic dispatch."""
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
PASS = "pass391_dm1_v1_queued_command_dispatch"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")

ADDR = {
    "F0361_COMMAND_ProcessKeyPress": "22F7:0407",
    "F0380_COMMAND_ProcessQueue_CPSC": "22F7:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA7:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA7:01AA",
    "G2153_i_QueuedCommandsCount": "2C23:3E78",
    "G0310_i_DisabledMovementTicks": "2C23:3C9A",
    "G0311_i_ProjectileDisabledMovementTicks": "2C23:3D28",
}
INITIAL_ARMED_NAMES = {"F0361_COMMAND_ProcessKeyPress", "G2153_i_QueuedCommandsCount", "G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks"}
CONSUMER_ARMED_NAMES = {"F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"}
ARMED_NAMES = INITIAL_ARMED_NAMES | CONSUMER_ARMED_NAMES
ROUTE = "kp4 wait:900 kp6 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:1200"
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)
KEY_ARG_RE = re.compile(r"\[bp\+06\][^=]*=(?P<key>[0-9A-F]{4})", re.I)
BPM_VAL_RE = re.compile(r"BPMEM\s+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+\((?P<val>[0-9A-F]{2})\)", re.I)


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required path: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def norm(text: str) -> str:
    return " ".join(text.split())


def find_line(path: Path, needle: str) -> int | None:
    n = norm(needle)
    for i, line in enumerate(read(path).splitlines(), 1):
        if n in norm(line):
            return i
    return None


def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("GAMELOOP.C", "keyboard_drain_then_queue_process", [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ]),
        ("COMMAND.C", "f0361_keyboard_queue_write", [
            "void F0361_COMMAND_ProcessKeyPress",
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
        ]),
        ("COMMAND.C", "f0380_pop_load_dispatch", [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
        ("CLIKMENU.C", "semantic_turn_move_handlers", [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0284_CHAMPION_SetPartyDirection",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ]),
        ("PC.H", "pc34_queue_count_alias", ["#define G2153_i_QueuedCommandsCount"]),
    ]
    rows: list[dict[str, Any]] = []
    for fn, section, needles in specs:
        path = RED / fn
        hits = {needle: find_line(path, needle) for needle in needles} if path.exists() else {}
        missing = [needle for needle, line in hits.items() if line is None]
        rows.append({"file": fn, "section": section, "path": str(path), "ok": path.exists() and not missing, "lineHits": hits, "missing": missing})
    return rows


def bplist_values(text: str) -> dict[str, str]:
    return {m.group("addr").upper(): m.group("val").upper() for m in BPM_VAL_RE.finditer(p385.clean(text))}


def classify_stop(post: str) -> dict[str, Any]:
    c = p385.clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    lines = p385.code_lines(c)[-16:]
    kind = "other"
    entry_addr = None
    key_arg = None
    if mem:
        last = mem[-1]
        addr = last["addr"].upper()
        if addr == ADDR["G2153_i_QueuedCommandsCount"]:
            kind = "g2153_queue_count_bpm"
        elif addr == ADDR["G0310_i_DisabledMovementTicks"]:
            kind = "g0310_disabled_movement_ticks_bpm"
        elif addr == ADDR["G0311_i_ProjectileDisabledMovementTicks"]:
            kind = "g0311_projectile_disabled_movement_ticks_bpm"
    for name in ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]:
        target = ADDR[name].upper()
        if any(line.upper().startswith(target) for line in lines) or target in c.upper():
            kind = name
            entry_addr = target
            break
    if kind == "F0361_COMMAND_ProcessKeyPress":
        m = KEY_ARG_RE.search(c)
        if m:
            key_arg = m.group("key").upper()
    return {"kind": kind, "entryAddr": entry_addr, "addr": p385.last_code_addr(c), "keyArgumentHex": key_arg, "memoryBreakpoints": mem[-6:], "postRunningCodeLines": lines, "postRunningExcerpt": c[-2600:]}


def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], running: threading.Event, stop: threading.Event) -> tuple[bool, bool]:
    deadline = time.time() + seconds
    buf = ""
    saw_running = False
    consumer_armed_after_increment = False
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
                bpl = p385.dbg(child, "BPLIST", cmdlog, transcript)
                row["bplistValues"] = bplist_values(bpl)
                stops.append(row)
                kinds = [s.get("kind") for s in stops]
                g2153 = [bp for s in stops for bp in s.get("memoryBreakpoints", []) if bp.get("addr", "").upper() == ADDR["G2153_i_QueuedCommandsCount"]]
                increment = any(int(bp.get("new", "00"), 16) > int(bp.get("old", "00"), 16) for bp in g2153)
                decrement = any(int(bp.get("old", "00"), 16) > int(bp.get("new", "00"), 16) for bp in g2153)
                if increment and not consumer_armed_after_increment:
                    for ccmd in [f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}", f"BP {ADDR['F0365_COMMAND_ProcessTypes1To2_TurnParty']}", f"BP {ADDR['F0366_COMMAND_ProcessTypes3To6_MoveParty']}", "BPLIST"]:
                        p385.dbg(child, ccmd, cmdlog, transcript)
                    row["consumerBreakpointsArmedAfterIncrement"] = True
                    consumer_armed_after_increment = True
                if increment and decrement and any(k in {"F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"} for k in kinds):
                    stop.set()
                    break
                child.send("\x1bOt")
                cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after pass391 strict stop", "kind": row.get("kind")})
                buf = ""
                running.set()
        time.sleep(.05)
    return saw_running, consumer_armed_after_increment


def runtime_probe(seconds: int, route: str) -> dict[str, Any]:
    missing = [x for x in ["dosbox-debug", "Xvfb", "xdotool"] if not shutil.which(x)]
    if missing:
        return {"ran": False, "missingTools": missing, "blocker": "missing tools: " + ", ".join(missing)}
    OUT.mkdir(parents=True, exist_ok=True)
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    arm_time = None
    with tempfile.TemporaryDirectory(prefix="firestaff-pass391-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(p385.ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        p385.write_conf(conf, stage)
        display = f":{180 + (os.getpid() % 40)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(p385.drain(child, 1))
            win = p385.find_win(display)
            if not win:
                return {"ran": True, "stage": "window", "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed through load prefix and suffix"})
            p385.drive(display, win, p385.LOAD_PREFIX + " " + p385.LOAD_SUFFIX + " wait:2500", routelog)
            if not p385.pause_to_prompt(child, display, win, cmdlog, transcript, "post-gameplay arm point"):
                return {"ran": True, "durationSeconds": round(time.time() - start, 3), "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "stage": "post-gameplay arm point", "blocker": "no debugger prompt at post-gameplay arm point"}
            arm_cmds = ["BPDEL *", f"BP {ADDR['F0361_COMMAND_ProcessKeyPress']}", f"BPM {ADDR['G2153_i_QueuedCommandsCount']}", f"BPM {ADDR['G0310_i_DisabledMovementTicks']}", f"BPM {ADDR['G0311_i_ProjectileDisabledMovementTicks']}", "BPLIST"]
            for cmd in arm_cmds:
                p385.dbg(child, cmd, cmdlog, transcript)
            bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_at_arm = {name: (ADDR[name] in bplist) for name in INITIAL_ARMED_NAMES}
            queue_count_at_arm = bplist_values(cmdlog[-1].get("excerpt", "")).get(ADDR["G2153_i_QueuedCommandsCount"])
            arm_time = time.time()
            running = threading.Event()
            stop = threading.Event()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run after pass391 post-gameplay arm"})
            t = threading.Thread(target=p385.drive, args=(display, win, route, routelog, running, stop), daemon=True)
            t.start()
            saw_running, consumer_armed_after_increment = monitor(child, seconds, transcript, cmdlog, stops, running, stop)
            stop.set(); t.join(timeout=1)
            p385.pause_to_prompt(child, display, win, cmdlog, transcript, "final retention/position sample")
            p385.dbg(child, "BPLIST", cmdlog, transcript)
            final_bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_final = {name: (ADDR[name] in final_bplist) for name in ARMED_NAMES}
            route_after_arm = any(r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:") for r in routelog)
            return {"ran": True, "durationSeconds": round(time.time() - start, 3), "boundedSeconds": seconds, "method": "post-gameplay pexpect-owned PTY; arm F0361 plus G2153/G0310/G0311 BPM, then arm F0380/F0365/F0366 after first G2153 increment; strict stops require (Running)->prompt", "route": route, "routeInputAfterArming": route_after_arm, "sawRunning": saw_running, "retainedAtArm": retained_at_arm, "retainedFinal": retained_final, "consumerBreakpointsArmedAfterIncrement": consumer_armed_after_increment, "queueCountAtArm": queue_count_at_arm, "stops": stops, "routeLog": routelog, "commandLog": cmdlog}
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
            (OUT / "pass391_runtime.clean.txt").write_text(p385.clean("".join(transcript))[-450000:] + "\n", encoding="utf-8")


def summarize(runtime: dict[str, Any], source: list[dict[str, Any]]) -> tuple[str, dict[str, Any], str | None]:
    stops = runtime.get("stops", [])
    kinds = [s.get("kind") for s in stops]
    g2153 = [bp for s in stops for bp in s.get("memoryBreakpoints", []) if bp.get("addr", "").upper() == ADDR["G2153_i_QueuedCommandsCount"]]
    increments = [bp for bp in g2153 if int(bp.get("new", "00"), 16) > int(bp.get("old", "00"), 16)]
    decrements = [bp for bp in g2153 if int(bp.get("old", "00"), 16) > int(bp.get("new", "00"), 16)]
    dispatch = any(k in {"F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"} for k in kinds)
    source_ok = all(r.get("ok") for r in source)
    predicates = {
        "sourceAuditOk": source_ok,
        "postGameplayRuntimeRan": runtime.get("sawRunning") is True,
        "routeInputAfterArming": runtime.get("routeInputAfterArming") is True,
        "initialBreakpointsRetainedAtArm": all(runtime.get("retainedAtArm", {}).values()) if runtime.get("retainedAtArm") else False,
        "queueCountAtArmZero": runtime.get("queueCountAtArm") == "00",
        "f0361HitAfterArm": "F0361_COMMAND_ProcessKeyPress" in kinds,
        "g2153IncrementObserved": bool(increments),
        "consumerBreakpointsArmedAfterIncrement": runtime.get("consumerBreakpointsArmedAfterIncrement") is True,
        "f0380EntryBreakpointObservedAfterQueueWrite": "F0380_COMMAND_ProcessQueue_CPSC" in kinds and bool(increments),
        "f0380PopLoadAfterQueueWriteObserved": bool(decrements),
        "g2153DecrementPopLoadObserved": bool(decrements),
        "f0365OrF0366DispatchObserved": dispatch,
    }
    if not source_ok:
        return "FAIL_PASS391_SOURCE_AUDIT_FAILED", predicates, "source audit failed"
    required = [k for k in predicates if k != "f0380EntryBreakpointObservedAfterQueueWrite"]
    if all(predicates[k] for k in required):
        return "PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN", predicates, None
    if predicates["g2153IncrementObserved"] and not predicates["f0380PopLoadAfterQueueWriteObserved"]:
        return "BLOCKED_PASS391_QUEUE_WRITE_NO_F0380_POP_LOAD", predicates, "after G2153 was incremented above zero, no strict post-Running G2153 decrement/pop-load stop was observed in the bounded window"
    if predicates["f0380EntryBreakpointObservedAfterQueueWrite"] and not predicates["g2153DecrementPopLoadObserved"]:
        return "BLOCKED_PASS391_F0380_POST_WRITE_NO_POP_LOAD", predicates, "after queue write, F0380 reached but did not decrement G2153; exact source predicate is COMMAND.C:2096 movement-disabled gate (G0310_i_DisabledMovementTicks or matching G0311/G0312 projectile cooldown) when the queued command is C003..C006, otherwise the queue should pop"
    if predicates["g2153DecrementPopLoadObserved"] and not predicates["f0365OrF0366DispatchObserved"]:
        return "BLOCKED_PASS391_POP_LOAD_NO_SEMANTIC_DISPATCH", predicates, "F0380 popped/loaded a command (G2153 decrement) but no F0365/F0366 strict stop followed; exact post-pop predicate is COMMAND.C:2150/2154 command classification did not match C001/C002 turn or C003..C006 move in the observed bounded window"
    if predicates["postGameplayRuntimeRan"] and predicates["routeInputAfterArming"] and not predicates["f0361HitAfterArm"]:
        return "BLOCKED_PASS391_ROUTE_KEY_NOT_REACHING_F0361", predicates, "post-gameplay route input ran but did not reach F0361"
    return "BLOCKED_PASS391_RUNTIME_INCONCLUSIVE", predicates, "runtime probe did not produce decisive post-Running queue/dispatch evidence"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=60)
    ap.add_argument("--route", default=ROUTE)
    args = ap.parse_args()
    args.seconds = max(20, min(args.seconds, 120))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    runtime = runtime_probe(args.seconds, args.route)
    status, predicates, blocker = summarize(runtime, source) if runtime.get("ran") else ("BLOCKED_PASS391_RUNTIME_NOT_RUN", {}, runtime.get("blocker"))
    manifest = {"schema": PASS + ".v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(), "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(), "sourceRoot": str(RED), "addresses": ADDR, "sourceAudit": source, "runtimeProbe": {k: v for k, v in runtime.items() if k not in {"routeLog", "commandLog"}}, "runtimeArtifacts": {"transcript": f"parity-evidence/verification/{PASS}/pass391_runtime.clean.txt", "routeKeylog": f"parity-evidence/verification/{PASS}/pass391_route_keylog.json", "commandLog": f"parity-evidence/verification/{PASS}/pass391_command_log.json"}, "proofPredicates": predicates, "blocker": blocker, "promotionRule": "Promote only if the same bounded post-gameplay run has strict post-Running stops proving F0361, G2153 increment, F0380-scoped G2153 decrement/pop-load, and F0365/F0366 dispatch. The F0380 entry breakpoint is diagnostic only because consumer breakpoints are armed after the first queue-count increment to avoid F0380 idle-loop starvation.", "notPromotedBy": ["BPLIST", "BP/BPM command echo", "forced pause", "route keylog alone", "source-only address binding", "F0380 entry without G2153 decrement"]}
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass391_route_keylog.json").write_text(json.dumps(runtime.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass391_command_log.json").write_text(json.dumps(runtime.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join(["# Pass391 — DM1 V1 queued command dispatch", "", f"Status: `{status}`", "", "## Decision", "", "Keyboard queued-command dispatch chain proven." if blocker is None else f"Blocked: {blocker}", "", "## Runtime predicates", "", *[f"- `{k}`: `{v}`" for k, v in predicates.items()], "", "## Evidence", "", f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`", f"- Transcript: `parity-evidence/verification/{PASS}/pass391_runtime.clean.txt`", f"- Route keylog: `parity-evidence/verification/{PASS}/pass391_route_keylog.json`", f"- Command log: `parity-evidence/verification/{PASS}/pass391_command_log.json`"]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS391") or status.startswith("BLOCKED_PASS391") else 1


if __name__ == "__main__":
    raise SystemExit(main())
