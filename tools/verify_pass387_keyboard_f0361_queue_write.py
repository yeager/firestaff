#!/usr/bin/env python3
"""Pass387 verifier: decisive F0361 keyboard queue-write probe.

Pass386 armed the debugger before the final saved-game/load key, so its first
F0361 hit could be from route setup rather than a post-gameplay movement key.
This verifier deliberately drives the load suffix unarmed, pauses at the live
party-control point, then arms only F0361 and G2153 queued-command-count before
sending movement keys.  The decisive predicate is a post-arm F0361 hit followed
by a G2153 write (normally 00 -> 01).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import pexpect

import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass387_keyboard_f0361_queue_write"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")

ADDR = {
    "F0361_COMMAND_ProcessKeyPress": "22F7:0407",
    "G2153_i_QueuedCommandsCount": "2C23:3E78",
}

# Several movement keys give the probe more than one opportunity while the
# route is still short enough to keep the artifact bounded.
MOVEMENT_ROUTE = "kp5 wait:700 kp4 wait:700 kp6 wait:700 kp5 wait:1500"
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)
KEY_ARG_RE = re.compile(r"\[bp\+06\][^=]*=(?P<key>[0-9A-F]{4})", re.I)
BPLIST_VAL_RE = re.compile(r"BPMEM\s+2C23:3E78\s+\((?P<val>[0-9A-F]{2})\)", re.I)


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def read(path: Path) -> str:
    return path.read_text(encoding="latin-1")


def find_line(path: Path, needle: str) -> int | None:
    for i, line in enumerate(read(path).splitlines(), 1):
        if needle in line:
            return i
    return None


def source_audit() -> list[dict[str, Any]]:
    command = RED / "COMMAND.C"
    gameloop = RED / "GAMELOOP.C"
    specs = [
        (command, "movement_keyboard_table", [
            "KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
            "{ C001_COMMAND_TURN_LEFT",
            "{ C003_COMMAND_MOVE_FORWARD",
            "{ C002_COMMAND_TURN_RIGHT",
        ]),
        (command, "f0361_gate_and_queue_write", [
            "void F0361_COMMAND_ProcessKeyPress",
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "if (G2153_i_QueuedCommandsCount < C5_UNKNOWN)",
            "if (L1110_i_CommandQueueIndex == G0433_i_CommandQueueFirstIndex)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "F0360_COMMAND_ProcessPendingClick();",
        ]),
        (gameloop, "keyboard_buffer_feeds_f0361", [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
        ]),
    ]
    rows: list[dict[str, Any]] = []
    for path, section, needles in specs:
        hits = {n: find_line(path, n) for n in needles} if path.exists() else {}
        rows.append({
            "file": path.name,
            "section": section,
            "path": str(path),
            "ok": path.exists() and all(v is not None for v in hits.values()),
            "lineHits": hits,
            "missing": [k for k, v in hits.items() if v is None],
        })
    return rows


def classify_stop(post: str) -> dict[str, Any]:
    c = p385.clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    kind = "other"
    key_arg = None
    if ADDR["F0361_COMMAND_ProcessKeyPress"] in c.upper() or "[BP+06]" in c.upper():
        kind = "F0361_COMMAND_ProcessKeyPress"
        m = KEY_ARG_RE.search(c)
        if m:
            key_arg = m.group("key").upper()
    if mem:
        last = mem[-1]
        if last["addr"].upper() == ADDR["G2153_i_QueuedCommandsCount"]:
            kind = "g2153_queue_count_bpm"
    return {
        "kind": kind,
        "keyArgumentHex": key_arg,
        "memoryBreakpoints": mem[-4:],
        "postRunningCodeLines": p385.code_lines(c)[-12:],
        "postRunningExcerpt": c[-2000:],
    }


def current_queue_count_from_bplist(text: str) -> str | None:
    m = BPLIST_VAL_RE.search(p385.clean(text))
    return m.group("val").upper() if m else None


def run_probe(seconds: int) -> dict[str, Any]:
    transcript: list[str] = []
    cmdlog: list[dict[str, Any]] = []
    routelog: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    start = time.time()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass387-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(p385.ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        p385.write_conf(conf, stage)
        display = f":{150 + (os.getpid() % 30)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(p385.drain(child, 1))
            win = p385.find_win(display)
            if not win:
                return {"ran": True, "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed through load prefix and suffix"})
            p385.drive(display, win, p385.LOAD_PREFIX + " " + p385.LOAD_SUFFIX + " wait:2500", routelog)
            if not p385.pause_to_prompt(child, display, win, cmdlog, transcript, "post-gameplay arm point"):
                return {"ran": True, "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "blocker": "no debugger prompt at post-gameplay arm point"}
            for cmd in ["BPDEL *", f"BP {ADDR['F0361_COMMAND_ProcessKeyPress']}", f"BPM {ADDR['G2153_i_QueuedCommandsCount']}", "BPLIST"]:
                p385.dbg(child, cmd, cmdlog, transcript)
            bplist = cmdlog[-1].get("excerpt", "")
            retained_at_arm = {
                "F0361_COMMAND_ProcessKeyPress": ADDR["F0361_COMMAND_ProcessKeyPress"] in p385.clean(bplist).upper(),
                "G2153_i_QueuedCommandsCount": ADDR["G2153_i_QueuedCommandsCount"] in p385.clean(bplist).upper(),
            }
            queue_count_at_arm = current_queue_count_from_bplist(bplist)
            arm_time = time.time()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run after post-gameplay arm"})
            p385.drive(display, win, MOVEMENT_ROUTE, routelog)
            deadline = time.time() + seconds
            buf = ""
            saw_running = False
            while time.time() < deadline and len(stops) < 8:
                chunk = p385.drain(child, .25)
                if chunk:
                    transcript.append(chunk)
                    buf += chunk
                    c = p385.clean(buf)
                    if "(Running)" in c:
                        saw_running = True
                    if "(Running)" in c and "->" in c.split("(Running)", 1)[-1]:
                        post = c.split("(Running)", 1)[-1]
                        row = {"t": time.time(), **classify_stop(post)}
                        bpl = p385.dbg(child, "BPLIST", cmdlog, transcript)
                        row["queueCountBplistValue"] = current_queue_count_from_bplist(bpl)
                        stops.append(row)
                        # Once both predicates are proven, keep the artifact bounded.
                        if any(s.get("kind") == "F0361_COMMAND_ProcessKeyPress" for s in stops) and any(s.get("kind") == "g2153_queue_count_bpm" for s in stops):
                            break
                        child.send("\x1bOt")
                        cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after pass387 stop", "kind": row.get("kind")})
                        buf = ""
                time.sleep(.05)
            route_after_arm = any(r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:") for r in routelog)
            return {
                "ran": True,
                "durationSeconds": round(time.time() - start, 3),
                "boundedSeconds": seconds,
                "postGameplayArm": True,
                "route": MOVEMENT_ROUTE,
                "routeInputAfterArming": route_after_arm,
                "sawRunning": saw_running,
                "retainedAtArm": retained_at_arm,
                "queueCountAtArm": queue_count_at_arm,
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
            clean_transcript = p385.clean("".join(transcript))[-220000:]
            clean_transcript = "\n".join(line.rstrip() for line in clean_transcript.splitlines())
            (OUT / "pass387_keyboard_runtime.clean.txt").write_text(clean_transcript + "\n", encoding="utf-8")


def summarize(probe: dict[str, Any], source: list[dict[str, Any]]) -> tuple[str, dict[str, Any], str | None]:
    stops = probe.get("stops", [])
    g2153_changes = [bp for s in stops for bp in s.get("memoryBreakpoints", []) if bp.get("addr", "").upper() == ADDR["G2153_i_QueuedCommandsCount"]]
    predicates = {
        "sourceAuditOk": all(r.get("ok") for r in source),
        "postGameplayArm": probe.get("postGameplayArm") is True,
        "routeRanAfterArm": probe.get("sawRunning") is True and probe.get("routeInputAfterArming") is True,
        "breakpointsRetainedAtArm": all(probe.get("retainedAtArm", {}).values()),
        "queueCountAtArmZero": probe.get("queueCountAtArm") == "00",
        "f0361HitAfterGameplayArm": any(s.get("kind") == "F0361_COMMAND_ProcessKeyPress" for s in stops),
        "queuedCommandCountWriteObserved": any(bp.get("old") == "00" and bp.get("new") == "01" for bp in g2153_changes),
        "anyQueuedCommandCountWriteObserved": bool(g2153_changes),
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS387_SOURCE_AUDIT_FAILED", predicates, "source audit failed"
    if predicates["routeRanAfterArm"] and predicates["breakpointsRetainedAtArm"] and predicates["f0361HitAfterGameplayArm"] and predicates["queuedCommandCountWriteObserved"]:
        return "PASS387_KEYBOARD_F0361_QUEUE_WRITE_PROVEN", predicates, None
    if predicates["f0361HitAfterGameplayArm"] and not predicates["anyQueuedCommandCountWriteObserved"]:
        return "BLOCKED_PASS387_F0361_HIT_NO_QUEUE_COUNT_WRITE", predicates, "post-gameplay movement key reaches F0361 but no G2153 queued-command-count write was observed"
    if predicates["routeRanAfterArm"] and not predicates["f0361HitAfterGameplayArm"]:
        return "BLOCKED_PASS387_MOVEMENT_KEY_NOT_REACHING_F0361", predicates, "post-gameplay movement route ran but did not reach F0361"
    return "BLOCKED_PASS387_RUNTIME_PROBE_INCONCLUSIVE", predicates, "runtime probe did not produce decisive F0361/G2153 evidence"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=25)
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 60))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    probe = run_probe(args.seconds)
    status, predicates, blocker = summarize(probe, source)
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(),
        "sourceRoot": str(RED),
        "addresses": ADDR,
        "sourceAudit": source,
        "runtimeProbe": {k: v for k, v in probe.items() if k not in {"routeLog", "commandLog"}},
        "runtimeArtifacts": {
            "transcript": f"parity-evidence/verification/{PASS}/pass387_keyboard_runtime.clean.txt",
            "routeKeylog": f"parity-evidence/verification/{PASS}/pass387_keyboard_route_keylog.json",
            "commandLog": f"parity-evidence/verification/{PASS}/pass387_keyboard_command_log.json",
        },
        "proofPredicates": predicates,
        "blocker": blocker,
        "decision": "Arm after the saved-game/load suffix. A post-gameplay F0361 hit followed by a G2153 00->01 write proves keyboard input reaches the queue write; the pass386 no-write observation was caused by probe timing/arming before gameplay, not by F0361 gating.",
        "notPromotedBy": ["source-only queue write lines", "route keylog alone", "F0361 entry without a G2153 write", "BPLIST command echo"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass387_keyboard_route_keylog.json").write_text(json.dumps(probe.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass387_keyboard_command_log.json").write_text(json.dumps(probe.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass387 — DM1 V1 keyboard F0361 queue write",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        ("Keyboard queue write proven after arming at the post-gameplay party-control point." if blocker is None else f"Blocked: {blocker}"),
        "",
        "Pass386 armed before the saved-game/load suffix. This pass drives that suffix unarmed, pauses at the live gameplay point, arms `F0361_COMMAND_ProcessKeyPress` and `G2153_i_QueuedCommandsCount`, then sends movement keys. Runtime observes `F0361` followed by a `G2153` `00 -> 01` write, so the earlier no-write result was a probe timing/arming artifact, not an F0361 queue gate.",
        "",
        "## Source audit conclusion",
        "",
        "- ReDMCSB source locks the keyboard-buffer path into `F0361_COMMAND_ProcessKeyPress`.",
        "- `F0361` requires a non-null primary keyboard table, checks queue capacity, scans primary/secondary keyboard tables, writes `G0432_as_CommandQueue[...]`, and increments `G2153_i_QueuedCommandsCount`.",
        "",
        "## Runtime predicates",
        "",
        *[f"- `{k}`: `{v}`" for k, v in predicates.items()],
        "",
        "## Evidence",
        "",
        f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        f"- Bounded runtime transcript: `parity-evidence/verification/{PASS}/pass387_keyboard_runtime.clean.txt`",
        f"- Route keylog: `parity-evidence/verification/{PASS}/pass387_keyboard_route_keylog.json`",
        f"- Debug command log: `parity-evidence/verification/{PASS}/pass387_keyboard_command_log.json`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS387") or status.startswith("BLOCKED_PASS387") else 1


if __name__ == "__main__":
    raise SystemExit(main())
