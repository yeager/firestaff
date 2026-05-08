#!/usr/bin/env python3
"""Pass247: drive Linux /usr/bin/dosbox-debug breakpoint commands through a PTY.

The ncurses debugger command parser accepts Enter reliably in this environment
when DOSBox is started with TERM=vt100 inside tmux. TERM=xterm/screen/linux can
show the debugger UI but leaves Enter as an unparsed key for this build.

This tool intentionally uses a built-in DOSBox target (`DEBUG COMMAND.COM`) to
force the debugger loop active, then injects text commands with `tmux send-keys`.
It does not copy or execute Dungeon Master binaries; it only proves that numeric
breakpoint commands can reach the DOSBox debugger parser.
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

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver"
DEFAULT_BREAKPOINTS = [
    "22AF:06E9",
    "1EA4:010D",
    "1EA4:01AA",
    "1859:0516",
    "2AFF:110E",
]


def run(cmd: list[str], *, timeout: float = 10.0, check: bool = True) -> subprocess.CompletedProcess[str]:
    proc = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if check and proc.returncode != 0:
        raise RuntimeError(f"command failed ({proc.returncode}): {cmd!r}\n{proc.stdout}")
    return proc


def clean_capture(text: str) -> str:
    text = re.sub(r"\x1b\[[0-9;?]*[ -/]*[@-~]", "", text)
    text = re.sub(r"\x1b[()][A-Z0-9]", "", text)
    text = text.replace("\r", "\n")
    text = re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)
    # Keep bounded but preserve the command/output tail.
    lines = text.splitlines()
    return "\n".join(lines[-220:]) + "\n"


def write_conf(path: Path) -> None:
    path.write_text(
        "\n".join(
            [
                "[sdl]",
                "fullscreen=false",
                "output=surface",
                "[dosbox]",
                "machine=svga_paradise",
                "memsize=4",
                "[cpu]",
                "core=normal",
                "cycles=3000",
                "[mixer]",
                "nosound=true",
                "[autoexec]",
                "DEBUG COMMAND.COM",
                "",
            ]
        ),
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", type=Path, default=OUT_DIR)
    parser.add_argument("--breakpoint", action="append", dest="breakpoints", help="CS:IP breakpoint; may be repeated")
    parser.add_argument("--startup-wait", type=float, default=3.0)
    parser.add_argument("--command-wait", type=float, default=0.7)
    args = parser.parse_args()

    if not shutil.which("tmux"):
        raise SystemExit("tmux not found")
    dosbox_debug = shutil.which("dosbox-debug") or "/usr/bin/dosbox-debug"
    if not Path(dosbox_debug).exists():
        raise SystemExit("dosbox-debug not found")

    breakpoints = args.breakpoints or DEFAULT_BREAKPOINTS
    out_dir: Path = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    session = f"pass247-dosbox-debug-{os.getpid()}"
    commands = ["HELP", *[f"BP {bp}" for bp in breakpoints], "BPLIST"]

    with tempfile.TemporaryDirectory(prefix="firestaff-pass247-dosbox-debug-") as td:
        scratch = Path(td)
        conf = scratch / "dosbox-debug.conf"
        write_conf(conf)
        started = False
        try:
            run(
                [
                    "tmux",
                    "new-session",
                    "-d",
                    "-s",
                    session,
                    f"TERM=vt100 {dosbox_debug} -conf {conf} -exit",
                ],
                timeout=5,
            )
            started = True
            time.sleep(args.startup_wait)
            for command in commands:
                run(["tmux", "send-keys", "-t", session, command, "Enter"], timeout=5)
                time.sleep(args.command_wait)
            raw = run(["tmux", "capture-pane", "-p", "-S", "-500", "-t", session], timeout=5).stdout
        finally:
            if started:
                run(["tmux", "kill-session", "-t", session], timeout=5, check=False)

    clean = clean_capture(raw)
    transcript = out_dir / "dosbox_debug_pty_breakpoints.clean.txt"
    transcript.write_text(clean, encoding="utf-8")

    listed_breakpoints = sorted(set(re.findall(r"\b\d{2}\. BP ([0-9A-F]{4}:[0-9A-F]{4})", clean)))
    expected = sorted(bp.upper() for bp in breakpoints)
    accepted_all = listed_breakpoints == expected
    manifest = {
        "schema": "pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS_BREAKPOINT_COMMANDS_ACCEPTED" if accepted_all else "FAIL_BREAKPOINT_COMMANDS_NOT_CONFIRMED",
        "method": "tmux PTY with TERM=vt100; autoexec DEBUG COMMAND.COM; tmux send-keys text + Enter",
        "dosbox_debug": dosbox_debug,
        "tmux": shutil.which("tmux"),
        "commands": commands,
        "expected_breakpoints": expected,
        "listed_breakpoints": listed_breakpoints,
        "accepted_all": accepted_all,
        "transcript": str(transcript.relative_to(ROOT)),
        "notes": [
            "TERM=xterm/screen/linux showed the UI but did not reliably parse Enter on this build.",
            "DEBUG COMMAND.COM is only a built-in target to enter the debugger loop; no original game binaries are used.",
        ],
    }
    manifest_path = out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"status": manifest["status"], "manifest": str(manifest_path), "transcript": str(transcript)}, indent=2))
    return 0 if accepted_all else 1


if __name__ == "__main__":
    raise SystemExit(main())
