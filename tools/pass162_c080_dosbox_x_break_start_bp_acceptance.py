#!/usr/bin/env python3
"""Pass162 DOSBox-X break-start command acceptance probe.

This is a debugger-control probe only. It starts DOSBox-X/dosbox-debug with an
empty autoexec and `-debug -break-start`, sends the pass162 C080 BP/BPM command
packet through an owned PTY, and records whether the debugger prompt accepts the
numeric addresses. It does not mount or execute Dungeon Master binaries.
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
PASS162_DIR = ROOT / "parity-evidence/verification/pass162_c080_queue_trace"
COMMANDS_TXT = PASS162_DIR / "pass162_c080_dosbox_debug_commands.txt"
OUT_DIR = PASS162_DIR / "dosbox_x_break_start_bp_acceptance"

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CONTROL_RE = re.compile(r"[\x00-\x08\x0b\x0c\x0e-\x1f]")
BP_SET_RE = re.compile(r"DEBUG: Set breakpoint at ([0-9A-F]{4}:[0-9A-F]{4})", re.I)
BPM_SET_RE = re.compile(r"DEBUG: Set memory breakpoint at ([0-9A-F]{4}:[0-9A-F]{4})", re.I)
BPLIST_BP_RE = re.compile(r"\b\d+\.\s+BP\s+([0-9A-F]{4}:[0-9A-F]{4})", re.I)
BPLIST_BPM_RE = re.compile(r"\b\d+\.\s+BPM\s+([0-9A-F]{4}:[0-9A-F]{4})", re.I)


def clean(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r", "\n")
    return CONTROL_RE.sub("", text)


def drain(child: pexpect.spawn, seconds: float) -> str:
    out = ""
    deadline = time.time() + seconds
    while time.time() < deadline:
        try:
            out += child.read_nonblocking(size=8192, timeout=0.05)
        except pexpect.TIMEOUT:
            pass
        except pexpect.EOF:
            out += "<EOF>"
            break
    return out


def write_empty_conf(path: Path) -> None:
    path.write_text(
        "\n".join(
            [
                "[sdl]",
                "fullscreen=false",
                "output=surface",
                "usescancodes=false",
                "[dosbox]",
                "machine=svga_paradise",
                "memsize=4",
                "[cpu]",
                "core=normal",
                "cycles=3000",
                "[mixer]",
                "nosound=true",
                "[autoexec]",
                "",
            ]
        ),
        encoding="utf-8",
    )


def load_command_packet(path: Path) -> tuple[list[str], list[str], list[str]]:
    commands: list[str] = []
    expected_bp: list[str] = []
    expected_bpm: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.split(";", 1)[0].strip()
        if not line:
            continue
        op = line.split()[0].upper()
        if op not in {"BPDEL", "BP", "BPM", "BPLIST", "CPU"}:
            continue
        commands.append(line)
        parts = line.split()
        if len(parts) >= 2 and op == "BP":
            expected_bp.append(parts[1].upper())
        if len(parts) >= 2 and op == "BPM":
            expected_bpm.append(parts[1].upper())
    return commands, sorted(set(expected_bp)), sorted(set(expected_bpm))


def start_xvfb() -> tuple[subprocess.Popen[bytes], str]:
    for display_no in range(88, 100):
        socket = Path(f"/tmp/.X11-unix/X{display_no}")
        if socket.exists():
            continue
        display = f":{display_no}"
        proc = subprocess.Popen(
            ["Xvfb", display, "-screen", "0", "1024x768x24", "-nolisten", "tcp"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        for _ in range(40):
            if socket.exists():
                return proc, display
            time.sleep(0.05)
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
    raise RuntimeError("no free Xvfb display in :88..:99")


def run_probe(seconds: int) -> dict[str, Any]:
    missing = [tool for tool in ["dosbox-debug", "Xvfb"] if not shutil.which(tool)]
    if missing:
        return {"ran": False, "blocker": "missing tools: " + ", ".join(missing)}
    if not COMMANDS_TXT.exists():
        return {"ran": False, "blocker": f"missing command packet: {COMMANDS_TXT}"}

    commands, expected_bp, expected_bpm = load_command_packet(COMMANDS_TXT)
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="firestaff-pass162-break-start-") as td:
        conf = Path(td) / "dosbox.conf"
        write_empty_conf(conf)
        xvfb, display = start_xvfb()
        transcript = ""
        command_log: list[dict[str, Any]] = []
        child: pexpect.spawn | None = None
        try:
            child = pexpect.spawn(
                "dosbox-debug",
                ["-debug", "-break-start", "-conf", str(conf), "-exit", "-o", "quit warning=false"],
                env={**os.environ, "DISPLAY": display, "TERM": "vt100"},
                encoding="utf-8",
                timeout=2,
                echo=False,
                maxread=8192,
            )
            child.delaybeforesend = 0.05
            time.sleep(2.5)
            transcript += drain(child, 1.0)
            for command in commands:
                child.sendline(command)
                chunk = drain(child, 0.55)
                transcript += chunk
                command_log.append({"cmd": command, "excerpt": clean(chunk)[-500:]})
            transcript += drain(child, max(1.0, min(4.0, seconds)))
        finally:
            if child is not None:
                try:
                    child.terminate(force=True)
                except Exception:
                    pass
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()

    cleaned = clean(transcript)
    transcript_path = OUT_DIR / "dosbox_x_break_start_bp_acceptance.clean.txt"
    transcript_path.write_text(cleaned[-200000:], encoding="utf-8")
    set_bp = sorted(set(addr.upper() for addr in BP_SET_RE.findall(cleaned)))
    set_bpm = sorted(set(addr.upper() for addr in BPM_SET_RE.findall(cleaned)))
    listed_bp = sorted(set(addr.upper() for addr in BPLIST_BP_RE.findall(cleaned)))
    listed_bpm = sorted(set(addr.upper() for addr in BPLIST_BPM_RE.findall(cleaned)))
    prompt_seen = "->" in cleaned
    accepted_bp = sorted(set(set_bp + listed_bp))
    accepted_bpm = sorted(set(set_bpm + listed_bpm))
    bp_ok = accepted_bp == expected_bp
    bpm_ok = accepted_bpm == expected_bpm
    return {
        "ran": True,
        "method": "owned PTY + Xvfb + dosbox-debug -debug -break-start; empty autoexec; no original binaries mounted or executed",
        "display": display,
        "commands": commands,
        "prompt_seen": prompt_seen,
        "expected_bp": expected_bp,
        "accepted_bp": accepted_bp,
        "bp_ok": bp_ok,
        "expected_bpm": expected_bpm,
        "accepted_bpm": accepted_bpm,
        "bpm_ok": bpm_ok,
        "set_bp": set_bp,
        "set_bpm": set_bpm,
        "listed_bp": listed_bp,
        "listed_bpm": listed_bpm,
        "command_log": command_log,
        "transcript": str(transcript_path.relative_to(ROOT)),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--seconds", type=int, default=8)
    args = parser.parse_args()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    runtime = run_probe(max(4, min(args.seconds, 30)))
    ok = bool(runtime.get("ran") and runtime.get("prompt_seen") and runtime.get("bp_ok") and runtime.get("bpm_ok"))
    status = "PASS162_DOSBOX_X_BREAK_START_BP_BPM_ACCEPTED" if ok else "BLOCKED_PASS162_DOSBOX_X_BREAK_START_BP_BPM_NOT_CONFIRMED"
    manifest = {
        "schema": "pass162_c080_dosbox_x_break_start_bp_acceptance.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": ok,
        "runtime": runtime,
        "scope": "debugger command acceptance only; no Dungeon Master binary execution and no runtime hit claim",
        "non_claims": [
            "does not prove F0359/F0380/F0377/F0275/F0280 are reached",
            "does not promote pass435 semantic original-route readiness",
            "does not replace a live stock-original transcript",
        ],
    }
    manifest_path = OUT_DIR / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "ok": ok, "manifest": str(manifest_path)}, indent=2, sort_keys=True))
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
