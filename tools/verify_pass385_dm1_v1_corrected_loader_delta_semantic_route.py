#!/usr/bin/env python3
"""Pass385 verifier: corrected-loader-delta DM1 V1 semantic route probe.

This bounded original-runtime probe fixes pass384 off-by-three loader delta
for COMMAND/CLIKMENU/data symbols, then promotes only strict post-Running
breakpoint/watchpoint stops observed after post-load arming. Setup echoes,
BPLIST text, and forced pauses are non-promotable.
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

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass385_dm1_v1_corrected_loader_delta_semantic_route"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

ADDR = {
    "F0380_COMMAND_ProcessQueue_CPSC": "22F7:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA7:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA7:01AA",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23B0:40FE",
    "F0097_VIDRV_09_BlitViewPort_indirect_call": "280C:1EFF",
    "G0321_B_StopWaitingForPlayerInput": "2C23:1A7C",
}
LOAD_PREFIX = "wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200"
ARMED_NAMES = {
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    "F0128_DUNGEONVIEW_Draw_CPSF",
    "G0321_B_StopWaitingForPlayerInput",
}
LOAD_SUFFIX = "one wait:3500"
MOVE_ROUTE = "kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900"
ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
MEM_BP_RE = re.compile(r"memory breakpoint\s*:\s*(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s*-\s*(?P<old>[0-9A-F]{2})\s*->\s*(?P<new>[0-9A-F]{2})", re.I)


def clean(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r", "\n")
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def drain(child: pexpect.spawn, seconds: float) -> str:
    end = time.time() + seconds
    out = ""
    while time.time() < end:
        try:
            out += child.read_nonblocking(8192, timeout=0.05)
        except pexpect.TIMEOUT:
            pass
        except pexpect.EOF:
            out += "<EOF>"
            break
    return out


def write_conf(path: Path, stage: Path) -> None:
    path.write_text("\n".join([
        "[sdl]", "fullscreen=false", "output=surface", "usescancodes=false",
        "[dosbox]", "machine=svga_paradise", "memsize=4",
        "[cpu]", "core=normal", "cycles=3000",
        "[mixer]", "nosound=true",
        "[autoexec]", f"mount c {stage}", "c:", "DEBUG DM.EXE -vv -sn -pk", "",
    ]), encoding="utf-8")


def norm(s: str) -> str:
    return " ".join(s.split())


def find_line(path: Path, needle: str) -> int | None:
    n = norm(needle)
    for i, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if n in norm(line):
            return i
    return None


def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("GAMELOOP.C", ["G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "F0380_COMMAND_ProcessQueue_CPSC();", "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]),
        ("COMMAND.C", ["void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]),
        ("CLIKMENU.C", ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]),
        ("DUNVIEW.C", ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]),
    ]
    rows = []
    for fn, needles in specs:
        path = SRC / fn
        hits = {needle: find_line(path, needle) for needle in needles} if path.exists() else {}
        missing = [needle for needle, line in hits.items() if line is None]
        rows.append({"file": fn, "path": str(path), "ok": path.exists() and not missing, "lineHits": hits, "missing": missing})
    return rows


def xdo(display: str, args: list[str]) -> subprocess.CompletedProcess[str]:
    return run(["xdotool", *args], env={**os.environ, "DISPLAY": display}, timeout=10)


def find_win(display: str) -> str | None:
    ids = [x.strip() for x in xdo(display, ["search", "--sync", "--class", "dosbox"]).stdout.splitlines() if x.strip()]
    return ids[0] if ids else None


def key_name(tok: str) -> str:
    return {"enter": "Return", "return": "Return", "one": "1", "1": "1", "kp4": "KP_Left", "kp5": "KP_Begin", "kp6": "KP_Right"}[tok]


def click_at(display: str, win: str, x: int, y: int) -> dict[str, Any]:
    ns: dict[str, Any] = {}
    exec(xdo(display, ["getwindowgeometry", "--shell", win]).stdout, {}, ns)
    gw, gh = float(ns["WIDTH"]), float(ns["HEIGHT"])
    aspect = 320 / 200
    cw, ch = gw, gw / aspect
    if ch > gh:
        ch, cw = gh, gh * aspect
    px = int(round((gw - cw) / 2 + ((x + .5) / 320) * cw))
    py = int(round((gh - ch) / 2 + ((y + .5) / 200) * ch))
    r = xdo(display, ["mousemove", "--window", win, str(px), str(py), "click", "1"])
    return {"screen": [px, py], "rc": r.returncode, "out": r.stdout[-160:]}


def drive(display: str, win: str, route: str, log: list[dict[str, Any]], running: threading.Event | None = None, stop: threading.Event | None = None) -> None:
    xdo(display, ["windowactivate", "--sync", win])
    xdo(display, ["windowfocus", "--sync", win])
    for item in route.split():
        if stop and stop.is_set():
            break
        low = item.lower()
        row: dict[str, Any] = {"t": time.time(), "route_item": item}
        if low.startswith("wait:"):
            end = time.time() + int(low.split(":", 1)[1]) / 1000
            while time.time() < end and not (stop and stop.is_set()):
                time.sleep(.05)
            row["slept"] = True
        else:
            if running:
                deadline = time.time() + 20
                while time.time() < deadline and not running.is_set() and not (stop and stop.is_set()):
                    time.sleep(.05)
                row["running_guard"] = running.is_set()
            if low.startswith("click:"):
                x, y = map(int, low.split(":", 1)[1].split(","))
                row.update(click_at(display, win, x, y))
            else:
                r = xdo(display, ["key", "--window", win, key_name(low)])
                row.update({"rc": r.returncode, "out": r.stdout[-160:]})
                time.sleep(.2)
        log.append(row)


def dbg(child: pexpect.spawn, cmd: str, log: list[dict[str, Any]], transcript: list[str]) -> str:
    child.sendline(cmd)
    time.sleep(.22)
    out = drain(child, .55)
    transcript.append(out)
    log.append({"t": time.time(), "cmd": cmd, "excerpt": clean(out)[-700:]})
    return out


def code_lines(text: str) -> list[str]:
    return [m.group(0)[:180] for m in CODE_LINE_RE.finditer(clean(text))]


def last_code_addr(text: str) -> str | None:
    matches = [m.group("addr").upper() for m in CODE_LINE_RE.finditer(clean(text))]
    return matches[-1] if matches else None


def pause_to_prompt(child: pexpect.spawn, display: str, win: str, cmdlog: list[dict[str, Any]], transcript: list[str], purpose: str) -> bool:
    for key in ["Alt+Pause", "Pause"]:
        xdo(display, ["key", "--window", win, key])
        cmdlog.append({"t": time.time(), "control": key, "purpose": purpose})
        out = drain(child, 5)
        transcript.append(out)
        if "->" in clean(out):
            return True
    return False


def classify_stop(post: str) -> dict[str, Any]:
    c = clean(post)
    mem = [m.groupdict() for m in MEM_BP_RE.finditer(c)]
    lines = code_lines(c)[-14:]
    addr = last_code_addr(c)
    entry_addr = None
    kind = "other"
    if mem:
        last = mem[-1]
        if last["addr"].upper() == ADDR["G0321_B_StopWaitingForPlayerInput"]:
            kind = "g0321_bpm"
    # dosbox-debug prints several disassembly lines after a code BP; the final
    # IP in that block is often a few bytes after the armed entry. Classify by
    # the printed entry address/window, not only by last_code_addr().
    for name in ["F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0128_DUNGEONVIEW_Draw_CPSF"]:
        target = ADDR[name].upper()
        if any(line.upper().startswith(target) for line in lines) or target in c.upper():
            kind = name
            entry_addr = target
            break
    return {"addr": addr, "entryAddr": entry_addr, "kind": kind, "memoryBreakpoints": mem[-4:], "postRunningCodeLines": lines, "postRunningExcerpt": c[-2500:]}


def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], running: threading.Event, stop: threading.Event) -> bool:
    deadline = time.time() + seconds
    buf = ""
    saw_running = False
    while time.time() < deadline and not stop.is_set():
        chunk = drain(child, .25)
        if chunk:
            transcript.append(chunk)
            buf += chunk
            c = clean(buf)
            if "(Running)" in c:
                saw_running = True
                running.set()
            if "(Running)" in c and "->" in c.split("(Running)", 1)[-1]:
                running.clear()
                post = c.split("(Running)", 1)[-1]
                row = {"t": time.time(), "runningMarkerSeen": True, "promptReappearedAfterRunning": True, **classify_stop(post)}
                stops.append(row)
                for ccmd in ["CPU", f"MEMDUMP {ADDR['G0321_B_StopWaitingForPlayerInput']} 8", "BPLIST"]:
                    dbg(child, ccmd, cmdlog, transcript)
                child.send("\x1bOt")
                cmdlog.append({"t": time.time(), "control": "F5", "purpose": "continue after strict stop", "addr": row.get("addr"), "kind": row.get("kind")})
                buf = ""
                running.set()
        time.sleep(.05)
    return saw_running


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
    with tempfile.TemporaryDirectory(prefix="firestaff-pass385-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        write_conf(conf, stage)
        display = f":{70 + (os.getpid() % 20)}"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env={**os.environ, "DISPLAY": display, "TERM": "vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = .05
        try:
            time.sleep(3)
            transcript.append(drain(child, 1))
            win = find_win(display)
            if not win:
                return {"ran": True, "stage": "window", "blocker": "dosbox window not found"}
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run unarmed to load/menu prefix"})
            drive(display, win, LOAD_PREFIX, routelog)
            if not pause_to_prompt(child, display, win, cmdlog, transcript, "post-load arm point"):
                return {"ran": True, "durationSeconds": round(time.time() - start, 3), "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "stage": "post-load arm point", "blocker": "no debugger prompt after load prefix"}
            for cmd in ["BPDEL *", f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}", f"BP {ADDR['F0365_COMMAND_ProcessTypes1To2_TurnParty']}", f"BP {ADDR['F0366_COMMAND_ProcessTypes3To6_MoveParty']}", f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}", f"BPM {ADDR['G0321_B_StopWaitingForPlayerInput']}", "BPLIST"]:
                dbg(child, cmd, cmdlog, transcript)
            bplist = clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_at_arm = {name: (ADDR[name] in bplist) for name in ARMED_NAMES}
            arm_time = time.time()
            running = threading.Event()
            stop = threading.Event()
            child.send("\x1bOt")
            cmdlog.append({"t": time.time(), "control": "F5", "purpose": "run after F0380/F0365/F0366/F0128/G0321 arm"})
            t = threading.Thread(target=drive, args=(display, win, LOAD_SUFFIX + " " + route, routelog, running, stop), daemon=True)
            t.start()
            saw_running = monitor(child, seconds, transcript, cmdlog, stops, running, stop)
            stop.set(); t.join(timeout=1)
            pause_to_prompt(child, display, win, cmdlog, transcript, "final retention/position sample")
            final_addr = last_code_addr("".join(transcript))
            dbg(child, "BPLIST", cmdlog, transcript)
            final_bplist = clean(cmdlog[-1].get("excerpt", "")).upper()
            retained_final = {name: (ADDR[name] in final_bplist) for name in ARMED_NAMES}
            route_after_arm = any(r.get("t", 0) > arm_time and not str(r.get("route_item", "")).startswith("wait:") for r in routelog)
            return {"ran": True, "durationSeconds": round(time.time() - start, 3), "boundedSeconds": seconds, "method": "post-load pexpect-owned PTY; arm corrected-loader-delta F0380/F0365/F0366/F0128 code breakpoints plus G0321 BPM; strict stops require (Running)->prompt", "route": LOAD_SUFFIX + " " + route, "routeLog": routelog, "commandLog": cmdlog, "stops": stops, "sawRunning": saw_running, "routeInputAfterArming": route_after_arm, "retainedAtArm": retained_at_arm, "retainedFinal": retained_final, "finalPauseCodeAddr": final_addr}
        finally:
            try:
                transcript.append(drain(child, .5))
                child.terminate(force=True)
            except Exception:
                pass
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()
            OUT.mkdir(parents=True, exist_ok=True)
            (OUT / "pass385_runtime.clean.txt").write_text(clean("".join(transcript))[-400000:] + "\n", encoding="utf-8")


def summarize(runtime: dict[str, Any]) -> tuple[str, dict[str, Any], str | None]:
    stops = runtime.get("stops", [])
    kinds = [s.get("kind") for s in stops]
    f0380 = "F0380_COMMAND_ProcessQueue_CPSC" in kinds
    f0365 = "F0365_COMMAND_ProcessTypes1To2_TurnParty" in kinds
    f0366 = "F0366_COMMAND_ProcessTypes3To6_MoveParty" in kinds
    g0321_to_true = any(s.get("kind") == "g0321_bpm" and any(m.get("addr", "").upper() == ADDR["G0321_B_StopWaitingForPlayerInput"] and m.get("new", "").upper() not in {"00"} for m in s.get("memoryBreakpoints", [])) for s in stops)
    f0128_after_stop = False
    seen_stop = False
    for s in stops:
        if s.get("kind") in {"F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "g0321_bpm"}:
            seen_stop = True
        if seen_stop and s.get("kind") == "F0128_DUNGEONVIEW_Draw_CPSF":
            f0128_after_stop = True
    predicates = {
        "sawRunning": runtime.get("sawRunning") is True,
        "routeInputAfterArming": runtime.get("routeInputAfterArming") is True,
        "breakpointsRetainedAtArm": all(runtime.get("retainedAtArm", {}).values()) if runtime.get("retainedAtArm") else False,
        "f0380Hit": f0380,
        "f0365OrF0366Hit": f0365 or f0366,
        "g0321StopWaitWriteObserved": g0321_to_true,
        "nextF0128AfterStopWaitObserved": f0128_after_stop,
    }
    if all(predicates.values()):
        return "PASS385_F0380_DEQUEUE_G0321_EXIT_NEXT_F0128_PROVEN", predicates, None
    if predicates["sawRunning"] and predicates["routeInputAfterArming"] and predicates["breakpointsRetainedAtArm"]:
        missing = [k for k, v in predicates.items() if not v]
        return "BLOCKED_PASS385_F0365_F0366_COMMAND_DISPATCH_NOT_PROVEN", predicates, "corrected loader delta proves F0380/G0321/F0128 runtime path, but semantic command dispatch still missing " + ", ".join(missing)
    return "FAIL_PASS385_RUNTIME_CONTROL_OR_ARMING_INCOMPLETE", predicates, "runtime control/arming predicates failed"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=75)
    ap.add_argument("--route", default=MOVE_ROUTE)
    ap.add_argument("--no-runtime", action="store_true")
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 105))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    runtime = {"ran": False, "blocker": "--no-runtime"} if args.no_runtime else runtime_probe(args.seconds, args.route)
    status, predicates, blocker = summarize(runtime) if runtime.get("ran") else ("BLOCKED_PASS385_RUNTIME_NOT_RUN", {}, runtime.get("blocker"))
    if not all(r["ok"] for r in source):
        status = "FAIL_PASS385_SOURCE_AUDIT_FAILED"
        blocker = "source audit failed"
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(),
        "sourceRoot": str(SRC),
        "addresses": ADDR,
        "sourceAudit": source,
        "runtimeProbe": {k: v for k, v in runtime.items() if k not in {"routeLog", "commandLog"}},
        "runtimeArtifacts": {
            "transcript": f"parity-evidence/verification/{PASS}/pass385_runtime.clean.txt",
            "routeKeylog": f"parity-evidence/verification/{PASS}/pass385_route_keylog.json",
            "commandLog": f"parity-evidence/verification/{PASS}/pass385_command_log.json",
        },
        "proofPredicates": predicates,
        "blocker": blocker,
        "promotionRule": "Promote only if strict post-Running stops prove F0380 plus F0365/F0366, G0321 writes nonzero, and a later F0128 stop in the same bounded post-load run. BPLIST/setup echoes, route keylogs, and forced pause samples are excluded. Pass385 uses FIRES.MAP plus the observed DS/CS +0x0736 loader delta from pass384/pass385 runtime, replacing pass384 +0x0733 candidates.",
        "notPromotedBy": ["BPLIST", "BP/BPM command echo", "forced pause", "route keylog alone", "source-only address binding"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass385_route_keylog.json").write_text(json.dumps(runtime.get("routeLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (OUT / "pass385_command_log.json").write_text(json.dumps(runtime.get("commandLog", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass385 — DM1 V1 corrected-loader-delta semantic route probe",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        "Corrected-loader-delta runtime chain proven." if status.startswith("PASS385") else f"Blocked: {blocker}",
        "",
        "## Runtime predicates",
        "",
        *[f"- `{k}`: `{v}`" for k, v in predicates.items()],
        "",
        "## Corrected addresses armed",
        "",
        *[f"- `{k}`: `{v}`" for k, v in ADDR.items()],
        "",
        "## Evidence",
        "",
        f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        f"- Transcript: `parity-evidence/verification/{PASS}/pass385_runtime.clean.txt`",
        f"- Route keylog: `parity-evidence/verification/{PASS}/pass385_route_keylog.json`",
        f"- Command log: `parity-evidence/verification/{PASS}/pass385_command_log.json`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "predicates": predicates, "blocker": blocker, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS385") or status.startswith("BLOCKED_PASS385") else 1


if __name__ == "__main__":
    raise SystemExit(main())
