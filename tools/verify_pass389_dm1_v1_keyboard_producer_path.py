#!/usr/bin/env python3
"""Pass389 verifier: DM1 V1 keyboard movement command producer path.

This pass narrows the pass387/pass386 blocker by arming only after the
post-load gameplay key has been delivered, then driving movement keypad keys.
It promotes only strict post-Running stops, and records whether movement keys
reach F0361 with PC34 movement key codes and whether G2153 queue count changes.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import tempfile
import shutil
import subprocess
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import pexpect

import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass389_dm1_v1_keyboard_producer_path"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = p385.SRC
ADDR = dict(p385.ADDR)
ADDR.update({
    "F0361_COMMAND_ProcessKeyPress": "22F7:0407",
    "G2153_i_QueuedCommandsCount": "2C23:3E78",
})
MOVE_ROUTE = "kp5 wait:700 kp4 wait:700 kp6 wait:700 kp5 wait:700"
ARG_RE = re.compile(r"push word \[bp\+06\]ss:\[[0-9A-F]{4}\]=(?P<arg>[0-9A-F]{4})", re.I)
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)
PC34_MOVEMENT_CODES = {"004B", "004C", "004D", "004F", "0050", "0051"}
PC34_MOVEMENT_COMMANDS = {
    "C001_COMMAND_TURN_LEFT",
    "C002_COMMAND_TURN_RIGHT",
    "C003_COMMAND_MOVE_FORWARD",
    "C004_COMMAND_MOVE_RIGHT",
    "C005_COMMAND_MOVE_BACKWARD",
    "C006_COMMAND_MOVE_LEFT",
}


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def norm(s: str) -> str:
    return " ".join(s.split())


def read_src(name: str) -> str:
    return (SRC / name).read_text(encoding="latin-1", errors="replace")


def find_line(name: str, needle: str) -> int | None:
    n = norm(needle)
    for i, line in enumerate(read_src(name).splitlines(), 1):
        if n in norm(line):
            return i
    return None


def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("GAMELOOP.C", "keyboard_drain_calls_f0361", [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
        ]),
        ("COMMAND.C", "pc34_secondary_keyboard_movement_table", [
            "KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]",
            "{ C001_COMMAND_TURN_LEFT,     0x004B }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
            "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
            "{ C006_COMMAND_MOVE_LEFT,     0x004F }",
            "{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }",
            "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
        ]),
        ("STARTUP2.C", "gameplay_keyboard_interfaces_enabled", [
            "G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;",
            "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
        ]),
        ("COMMAND.C", "f0361_exact_keyboard_producer", [
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "if (G2153_i_QueuedCommandsCount < C5_UNKNOWN)",
            "if (P0728_KeyCode == L1112_ps_KeyboardInput->Code)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "if ((L1112_ps_KeyboardInput = G0444_ps_SecondaryKeyboardInput) == NULL)",
            "goto T0361xxx;",
        ]),
        ("COMMAND.C", "f0380_queue_pop_gate", [
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
    ]
    rows = []
    for file, section, needles in specs:
        path = SRC / file
        hits = {needle: find_line(file, needle) for needle in needles} if path.exists() else {}
        rows.append({
            "file": file,
            "section": section,
            "path": str(path),
            "ok": path.exists() and all(v is not None for v in hits.values()),
            "lineHits": hits,
            "missing": [k for k, v in hits.items() if v is None],
        })
    return rows


def robust_find_win(display: str) -> str | None:
    out = p385.xdo(display, ["search", "--sync", "--class", "dosbox"]).stdout
    for line in out.splitlines():
        if line.strip().isdigit():
            return line.strip()
    return None


def classify_stop(post: str) -> dict[str, Any]:
    c = p385.clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    lines = p385.code_lines(c)[-14:]
    kind = "other"
    entry = None
    arg = None
    if ADDR["F0361_COMMAND_ProcessKeyPress"].upper() in c.upper() or any(line.upper().startswith(ADDR["F0361_COMMAND_ProcessKeyPress"].upper()) for line in lines):
        kind = "F0361_COMMAND_ProcessKeyPress"
        entry = ADDR["F0361_COMMAND_ProcessKeyPress"].upper()
        m = ARG_RE.search(c)
        arg = m.group("arg").upper() if m else None
    elif ADDR["F0380_COMMAND_ProcessQueue_CPSC"].upper() in c.upper() or any(line.upper().startswith(ADDR["F0380_COMMAND_ProcessQueue_CPSC"].upper()) for line in lines):
        kind = "F0380_COMMAND_ProcessQueue_CPSC"
        entry = ADDR["F0380_COMMAND_ProcessQueue_CPSC"].upper()
    if mem:
        last = mem[-1]
        if last["addr"].upper() == ADDR["G2153_i_QueuedCommandsCount"]:
            kind = "g2153_queue_count_bpm"
    return {
        "t": time.time(),
        "kind": kind,
        "addr": p385.last_code_addr(c),
        "entryAddr": entry,
        "f0361KeyCode": arg,
        "f0361KeyCodeIsPc34Movement": arg in PC34_MOVEMENT_CODES if arg else False,
        "memoryBreakpoints": mem[-4:],
        "postRunningCodeLines": lines,
        "postRunningExcerpt": c[-2200:],
    }


def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], running: threading.Event, stop: threading.Event) -> bool:
    deadline = time.time() + seconds
    buf = ""
    saw = False
    while time.time() < deadline and not stop.is_set():
        chunk = p385.drain(child, .25)
        if chunk:
            transcript.append(chunk)
            buf += chunk
            c = p385.clean(buf)
            if "(Running)" in c:
                saw = True
                running.set()
            if "(Running)" in c and "->" in c.split("(Running)", 1)[-1]:
                running.clear()
                post = c.split("(Running)", 1)[-1]
                row = classify_stop(post)
                stops.append(row)
                for cmd in ["MEMDUMP " + ADDR["G2153_i_QueuedCommandsCount"] + " 8", "BPLIST"]:
                    p385.dbg(child, cmd, cmdlog, transcript)
                child.send("\x1bOt")
                cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after pass389 strict stop", "kind": row.get("kind"), "f0361KeyCode": row.get("f0361KeyCode")})
                buf = ""
                running.set()
        time.sleep(.05)
    return saw


def run_probe(seconds: int, route: str) -> dict[str, Any]:
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass389-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(p385.ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        p385.write_conf(conf, stage)
        display = f":{160 + (os.getpid() % 40)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**p385.os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(p385.drain(child, 1))
            win = robust_find_win(display)
            if not win:
                return {"ran": True, "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed through load plus gameplay suffix"})
            # Important: deliver LOAD_SUFFIX before arming, so movement keys are the only armed keyboard events.
            p385.drive(display, win, p385.LOAD_PREFIX + " " + p385.LOAD_SUFFIX, routelog)
            if not p385.pause_to_prompt(child, display, win, cmdlog, transcript, "post-load+suffix arm point"):
                return {"ran": True, "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "blocker": "no debugger prompt after load+suffix"}
            cmds = ["BPDEL *", f"BP {ADDR['F0361_COMMAND_ProcessKeyPress']}", f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}", f"BPM {ADDR['G2153_i_QueuedCommandsCount']}", "BPLIST"]
            for cmd in cmds:
                p385.dbg(child, cmd, cmdlog, transcript)
            arm_time = time.time()
            bplist = p385.clean(cmdlog[-1].get("excerpt", "")).upper()
            retained = {"F0361_COMMAND_ProcessKeyPress": ADDR["F0361_COMMAND_ProcessKeyPress"] in bplist, "F0380_COMMAND_ProcessQueue_CPSC": ADDR["F0380_COMMAND_ProcessQueue_CPSC"] in bplist, "G2153_i_QueuedCommandsCount": ADDR["G2153_i_QueuedCommandsCount"] in bplist}
            running = threading.Event(); stop = threading.Event()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run after pass389 arm"})
            t = threading.Thread(target=p385.drive, args=(display, win, route, routelog, running, stop), daemon=True)
            t.start()
            saw = monitor(child, seconds, transcript, cmdlog, stops, running, stop)
            stop.set(); t.join(timeout=1)
            p385.pause_to_prompt(child, display, win, cmdlog, transcript, "final sample")
            route_after_arm = any(r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:") for r in routelog)
            return {"ran": True, "durationSeconds": round(time.time() - start, 3), "boundedSeconds": seconds, "route": route, "routeInputAfterArming": route_after_arm, "sawRunning": saw, "retainedAtArm": retained, "stops": stops, "routeLog": routelog, "commandLog": cmdlog}
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
            (OUT / "pass389_runtime.clean.txt").write_text(p385.clean("".join(transcript))[-350000:] + "\n", encoding="utf-8")


def summarize(probe: dict[str, Any], source: list[dict[str, Any]]) -> tuple[str, dict[str, Any], str]:
    stops = probe.get("stops", [])
    args = [s.get("f0361KeyCode") for s in stops if s.get("kind") == "F0361_COMMAND_ProcessKeyPress"]
    predicates = {
        "sourceAuditOk": all(r["ok"] for r in source),
        "routeRanAfterArm": probe.get("sawRunning") is True and probe.get("routeInputAfterArming") is True and all(probe.get("retainedAtArm", {}).values()),
        "movementKeyReachedF0361": bool(args),
        "movementF0361KeyCodes": args,
        "movementF0361KeyCodeMatchesPc34MovementTable": any(a in PC34_MOVEMENT_CODES for a in args),
        "queueCountChanged": any(s.get("kind") == "g2153_queue_count_bpm" for s in stops),
        "f0380Reached": any(s.get("kind") == "F0380_COMMAND_ProcessQueue_CPSC" for s in stops),
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS389_SOURCE_AUDIT_FAILED", predicates, "source audit failed"
    if predicates["queueCountChanged"]:
        return "PASS389_KEYBOARD_PRODUCER_PROVEN", predicates, "movement key reached F0361 producer and changed G2153 queued-command count"
    if predicates["movementKeyReachedF0361"] and not predicates["movementF0361KeyCodeMatchesPc34MovementTable"]:
        return "BLOCKED_PASS389_KEYCODE_MISMATCH_BEFORE_PRODUCER", predicates, "movement key entered F0361, but runtime key code did not match the PC34 movement table, so F0361 skipped the queue write/increment producer"
    if predicates["routeRanAfterArm"] and not predicates["movementKeyReachedF0361"]:
        return "BLOCKED_PASS389_MOVEMENT_KEYS_NOT_REACHING_F0361", predicates, "armed movement route ran, but no movement key reached F0361; keyboard buffer/input delivery is before the command producer"
    return "BLOCKED_PASS389_PRODUCER_NOT_PROVEN", predicates, "movement route did not prove the F0361 producer path"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=35)
    ap.add_argument("--route", default=MOVE_ROUTE)
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 90))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    probe = run_probe(args.seconds, args.route)
    status, predicates, blocker = summarize(probe, source)
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(),
        "sourceRoot": str(SRC),
        "addresses": ADDR,
        "pc34MovementCodes": sorted(PC34_MOVEMENT_CODES),
        "pc34MovementCommands": sorted(PC34_MOVEMENT_COMMANDS),
        "sourceAudit": source,
        "runtimeProbe": {k: v for k, v in probe.items() if k not in {"routeLog", "commandLog"}},
        "runtimeArtifacts": {"transcript": f"parity-evidence/verification/{PASS}/pass389_runtime.clean.txt", "routeKeylog": f"parity-evidence/verification/{PASS}/pass389_route_keylog.json", "commandLog": f"parity-evidence/verification/{PASS}/pass389_command_log.json"},
        "proofPredicates": predicates,
        "blocker": blocker,
        "promotionRule": "Promote only a strict post-Running G2153 queued-command-count memory breakpoint after armed movement-route input. F0361 entry alone is not a producer proof unless its argument matches the PC34 movement table and the queue count changes.",
        "notPromotedBy": ["source-only producer path", "route keylog alone", "BPLIST/BP/BPM command echo", "forced pause", "pre-arm LOAD_SUFFIX key"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass389_route_keylog.json").write_text(json.dumps(probe.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass389_command_log.json").write_text(json.dumps(probe.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass389 — DM1 V1 keyboard producer path",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        blocker,
        "",
        "## Source-locked producer path",
        "",
        "- `GAMELOOP.C` drains `M527_IsCharacterInKeyboardBuffer()` into `F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())`.",
        "- `STARTUP2.C` enables `G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement` during gameplay.",
        "- In PC34, `G0459` maps movement commands to key codes `0x004B`, `0x004C`, `0x004D`, `0x004F`, `0x0050`, and `0x0051`; `F0361` writes `G0432_as_CommandQueue[...]` and increments `G2153_i_QueuedCommandsCount` only after exact key-code equality.",
        "- `F0380` can dispatch only after `G2153_i_QueuedCommandsCount != 0`; otherwise it follows the empty-queue bypass already proven in pass387.",
        "",
        "## Runtime predicates",
        "",
        *[f"- `{k}`: `{v}`" for k, v in predicates.items()],
        "",
        "## Evidence",
        "",
        f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        f"- Transcript: `parity-evidence/verification/{PASS}/pass389_runtime.clean.txt`",
        f"- Route keylog: `parity-evidence/verification/{PASS}/pass389_route_keylog.json`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS389") or status.startswith("BLOCKED_PASS389") else 1


if __name__ == "__main__":
    raise SystemExit(main())
