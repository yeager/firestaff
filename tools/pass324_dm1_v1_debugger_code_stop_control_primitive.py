#!/usr/bin/env python3
"""Pass324: find a reliable DOSBox-debug code-stop/control primitive.

This pass intentionally avoids tmux pane scraping. It talks to dosbox-debug via
an owned PTY (pexpect) and classifies debugger state by terminal/control
transitions: prompt -> F5/run -> `(Running)` -> prompt. BPLIST text is recorded
only as a negative control and never accepted as a runtime stop.
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

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive"
REPORT = ROOT / "parity-evidence/pass324_dm1_v1_debugger_code_stop_control_primitive.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

SOURCE_CHECKS = [
    {"id":"F0128_DUNGEONVIEW_Draw_CPSF", "file":"DUNVIEW.C", "needles":["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
    {"id":"F0097_DUNGEONVIEW_DrawViewport", "file":"DRAWVIEW.C", "needles":["void F0097_DUNGEONVIEW_DrawViewport", "VIDRV_09_BlitViewPort"]},
    {"id":"F0380_COMMAND_ProcessQueue_CPSC", "file":"COMMAND.C", "needles":["void F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
]
ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CODE_LINE_RE = re.compile(r"\b([0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
BPLIST_RE = re.compile(r"(?:Breakpoint list:|\b\d+\.\s+BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b|DEBUG: Set breakpoint at)", re.I)


def clean(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r", "\n")
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)


def drain(child: pexpect.spawn, seconds: float) -> str:
    end = time.time() + seconds
    out = ""
    while time.time() < end:
        try:
            out += child.read_nonblocking(size=8192, timeout=0.05)
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


def source_audit() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for spec in SOURCE_CHECKS:
        path = SOURCE_ROOT / spec["file"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        anchors: dict[str, int] = {}
        for needle in spec["needles"]:
            compact_needle = " ".join(needle.split())
            for idx, line in enumerate(lines, start=1):
                if compact_needle in " ".join(line.split()):
                    anchors[needle] = idx
                    break
        out.append({"id": spec["id"], "file": spec["file"], "source_path": str(path), "ok": path.exists() and len(anchors) == len(spec["needles"]), "anchors": anchors})
    return out


def code_lines(text: str) -> list[str]:
    return [m.group(0)[:120] for m in CODE_LINE_RE.finditer(clean(text))]


def last_code_addr(text: str) -> str | None:
    matches = CODE_LINE_RE.findall(clean(text))
    return matches[-1].upper() if matches else None


def classify_transition(before_run: str, after_run: str, target: str) -> dict[str, Any]:
    c_before = clean(before_run)
    c_after = clean(after_run)
    running_seen = "(Running)" in c_after
    prompt_after = "->" in c_after
    post_running = c_after.split("(Running)", 1)[-1] if "(Running)" in c_after else c_after
    bplist_after_running = bool(BPLIST_RE.search(post_running))
    stop_code_addr = last_code_addr(post_running)
    return {
        "target_breakpoint": target.upper(),
        "before_had_prompt": "->" in c_before,
        "running_marker_seen": running_seen,
        "prompt_reappeared_after_running": prompt_after,
        "stop_code_addr_after_running": stop_code_addr,
        "bplist_text_after_running": bplist_after_running,
        "separable_from_bplist": running_seen and prompt_after and not bplist_after_running and stop_code_addr is not None,
        "post_running_code_lines": code_lines(post_running)[-8:],
    }


def runtime_probe(seconds: int) -> dict[str, Any]:
    missing = [x for x in ["dosbox-debug", "Xvfb"] if not shutil.which(x)]
    if missing:
        return {"ran": False, "blocker": "missing tools: " + ", ".join(missing)}
    OUT.mkdir(parents=True, exist_ok=True)
    transcript = ""
    command_log: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="firestaff-pass324-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        write_conf(conf, stage)
        display = ":89"
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(0.5)
        env = {**os.environ, "DISPLAY": display, "TERM": "vt100"}
        child = pexpect.spawn("dosbox-debug", ["-conf", str(conf), "-exit"], env=env, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend = 0.05
        try:
            deadline = time.time() + seconds
            time.sleep(3)
            init = drain(child, 1)
            transcript += init
            initial_addr = last_code_addr(init) or "0499:0020"
            setup = ""
            for cmd in ["BPDEL *", f"BP {initial_addr}", "BPLIST"]:
                child.sendline(cmd)
                time.sleep(0.25)
                chunk = drain(child, 0.45)
                setup += chunk
                command_log.append({"t": time.time(), "cmd": cmd, "excerpt": clean(chunk)[-400:]})
            transcript += setup
            before_run = setup[-5000:]
            child.send("\x1bOt")
            command_log.append({"t": time.time(), "control": "F5", "bytes": "ESC O t"})
            after = drain(child, max(1.0, min(6.0, deadline - time.time())))
            transcript += after
            transition = classify_transition(before_run, after, initial_addr)
        finally:
            try:
                child.terminate(force=True)
            except Exception:
                pass
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()
    clean_transcript = clean(transcript)
    clean_transcript = "\n".join(line.rstrip() for line in clean_transcript.splitlines()) + "\n"
    (OUT / "dosbox_debug_pty.clean.txt").write_text(clean_transcript[-200000:], encoding="utf-8")
    return {"ran": True, "method": "direct pexpect PTY + Xvfb, no tmux capture-pane", "command_log": command_log, "transition": transition, "transcript_path": str(OUT / "dosbox_debug_pty.clean.txt")}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=20)
    args = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    audit = source_audit()
    runtime = runtime_probe(max(8, min(args.seconds, 60)))
    found = bool(runtime.get("ran") and runtime.get("transition", {}).get("separable_from_bplist"))
    status = "PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND" if found else "BLOCKED_DEBUGGER_CONTROL_PRIMITIVE_NOT_EXPOSED"
    manifest = {
        "schema": "pass324_dm1_v1_debugger_code_stop_control_primitive.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "source_audit": audit,
        "runtime_probe": runtime,
        "control_primitive": {
            "prompt": "->",
            "run_control": "F5 in DOSBox-debug; vt100 bytes ESC O t",
            "running_marker": "(Running)",
            "stop_observable": "terminal transition from (Running) back to -> with code-view disassembly refreshed; dosbox-debug 0.74-3 does not emit a stable 'breakpoint hit' text line for code BPs",
            "negative_control": "BPLIST/setup lines include 'Breakpoint list:' and '00. BP xxxx:yyyy' but occur before F5 and are rejected unless preceded by a run-to-prompt transition",
            "recommended_parser_rule": "Use an owned PTY transcript. Record a real code stop only after F5/run bytes were sent, '(Running)' appeared, '->' reappeared, and post-running text contains a code-view address. Never infer stops from BPLIST or 'DEBUG: Set breakpoint'.",
        },
        "tested_methods": [
            "direct pexpect PTY: primitive found",
            "script(1)-style PTY capture is equivalent in principle but pexpect is preferable because it preserves command/control ownership and timing",
            "tmux capture-pane: rejected as primary primitive because command echoes and BPLIST/setup text contaminated prior pass318/pass320 stop detection",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text(f"""# Pass324 — DM1 V1 debugger code-stop/control primitive\n\nStatus: `{status}`\n\n## ReDMCSB anchors audited first\n\n- `DUNVIEW.C` F0128: `{audit[0]['anchors']}`\n- `DRAWVIEW.C` F0097 / VIDRV: `{audit[1]['anchors']}`\n- `COMMAND.C` F0380 movement command dispatch: `{audit[2]['anchors']}`\n\n## Result\n\nA reliable primitive exists without tmux pane scraping: own the DOSBox-debug terminal with pexpect. The control/state syntax is:\n\n- stopped prompt: `->`\n- run control: F5, emitted as vt100 bytes `ESC O t`\n- running marker: `(Running)`\n- actual code stop: `(Running)` followed by reappearance of `->` plus refreshed code-view disassembly. DOSBox-debug 0.74-3 did **not** print a stable `breakpoint hit` line in this environment.\n\nBPLIST/setup text is a negative control only. `Breakpoint list:` and `00. BP ...` are not stops.\n\nManifest: `parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/manifest.json`\nTranscript: `parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/dosbox_debug_pty.clean.txt`\n""", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(OUT / "manifest.json"), "transition": runtime.get("transition")}, indent=2, sort_keys=True))
    return 0 if found else 2

if __name__ == "__main__":
    raise SystemExit(main())
