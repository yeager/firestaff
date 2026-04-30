#!/usr/bin/env python3
"""Attempt the DM2 INT 21h/AH=3Dh file-open runtime trace on N2.

This is intentionally conservative: DOSBox-X file-I/O logging may prove that a
DOS file open happened, but it is not accepted as the dynamic caller-path proof
unless a trace frame also includes AH plus DS:DX and DS:DX resolves to one of the
canonical member names.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from zipfile import ZipFile

REPO = Path(__file__).resolve().parents[1]
ARCHIVE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"
SKULL_ASM = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm2/SKULL.ASM"
PREV_EVIDENCE = REPO / "parity-evidence/verification/dm2_fileopen_trace_boundary_20260430/evidence.json"
TARGETS = ("DUNGEON.DAT", "GRAPHICS.DAT")
FILE_OPEN_RE = re.compile(r"FILES:file open command (?P<mode>[0-9A-Fa-f]+) file (?P<path>.+)$")
DSDX_RE = re.compile(r"AH\s*=\s*3D[hH]?|AH:3D[hH]?", re.IGNORECASE)
SEG_RE = re.compile(r"DS:DX\s*=\s*(?P<seg>[0-9A-Fa-f]{1,4}):(?P<off>[0-9A-Fa-f]{1,4})")


def load_boundary() -> dict:
    with PREV_EVIDENCE.open("r", encoding="utf-8") as f:
        return json.load(f)


def run_dosbox(timeout_s: int, keep_workdir: bool) -> tuple[dict, str]:
    dosbox_x = shutil.which("dosbox-x")
    dosbox = shutil.which("dosbox")
    gdb = shutil.which("gdb")
    tools = {"dosbox-x": dosbox_x, "dosbox": dosbox, "gdb": gdb, "xvfb-run": shutil.which("xvfb-run")}
    if not dosbox_x:
        return {"status": "blocked_missing_dosbox_x", "candidate_tools_on_n2": tools}, ""

    tmp_obj = tempfile.TemporaryDirectory(prefix="firestaff-dm2-int21-")
    workdir = Path(tmp_obj.name)
    with ZipFile(ARCHIVE) as zf:
        zf.extractall(workdir)

    cmd = [
        dosbox_x,
        "-debug",
        "-silent",
        "-nogui",
        "-exit",
        "-log-int21",
        "-log-fileio",
        "-c", f"mount c {workdir}",
        "-c", "c:",
        "-c", "ibmiop skull.exe",
    ]
    env = os.environ.copy()
    env.setdefault("SDL_VIDEODRIVER", "dummy")
    env.setdefault("SDL_AUDIODRIVER", "dummy")
    try:
        proc = subprocess.run(
            cmd,
            cwd=workdir,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=timeout_s,
            check=False,
        )
        returncode: int | str = proc.returncode
        raw = proc.stdout
        timed_out = False
    except subprocess.TimeoutExpired as exc:
        raw = (exc.stdout or "") if isinstance(exc.stdout, str) else (exc.stdout or b"").decode("utf-8", "replace")
        returncode = "timeout"
        timed_out = True

    opens = []
    target_opens = []
    register_frames = []
    for line in raw.splitlines():
        match = FILE_OPEN_RE.search(line)
        if match:
            rec = {"mode": match.group("mode"), "path": match.group("path").strip(), "line": line.strip()}
            opens.append(rec)
            upper = rec["path"].upper().replace("/", "\\")
            if any(target in upper for target in TARGETS):
                target_opens.append(rec)
        if DSDX_RE.search(line) and SEG_RE.search(line):
            register_frames.append(line.strip())

    accepted = []
    for frame in register_frames:
        upper = frame.upper().replace("/", "\\")
        if any(target in upper for target in TARGETS):
            accepted.append(frame)

    if accepted:
        status = "passed_ds_dx_target_resolved"
    elif target_opens:
        status = "blocked_fileio_only_no_ds_dx_register_frame"
    else:
        status = "blocked_no_target_open_observed"

    result = {
        "schema": "firestaff.dm2_int21_file_open_trace_next.v1",
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "pass_condition": "Accept only a trace frame at INT 21h/AH=3Dh that captures AH plus DS:DX and resolves DS:DX to DUNGEON.DAT or GRAPHICS.DAT.",
        "command": cmd,
        "timeout_seconds": timeout_s,
        "returncode": returncode,
        "timed_out": timed_out,
        "workdir_retained": str(workdir) if keep_workdir else None,
        "candidate_tools_on_n2": tools,
        "source_policy": {
            "redmcsb": "/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/ is DM/CSB comparison only; not DM2 C source.",
            "dm2": "canonical DM2 SKULL.EXE and SKULL.ASM anchors only.",
        },
        "anchors_from_473d17c": load_boundary()["dm2_exe"]["anchors"],
        "skull_asm_anchor": str(SKULL_ASM),
        "observed_file_open_count": len(opens),
        "observed_target_file_opens": target_opens[:20],
        "observed_register_frames": register_frames[:20],
        "accepted_frames": accepted,
        "log_excerpt_target_context": [ln for ln in raw.splitlines() if any(t in ln.upper() for t in TARGETS)][:80],
        "blocker": None if accepted else "N2 DOSBox-X -log-int21/-log-fileio exposes DOS file-open names but not an AH/DS:DX register frame; debugger CLI automation for a breakpoint/watch at the 473d17c open_int21_ah3d anchor remains unavailable from this headless run.",
    }
    if keep_workdir:
        tmp_obj = None  # type: ignore[assignment]
    else:
        tmp_obj.cleanup()
    return result, raw


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--timeout", type=int, default=75)
    ap.add_argument("--output", type=Path, default=REPO / "parity-evidence/verification/dm2_int21_trace_next_20260430/evidence.json")
    ap.add_argument("--raw-log", type=Path, default=REPO / "parity-evidence/verification/dm2_int21_trace_next_20260430/dosbox-x-trace.log")
    ap.add_argument("--keep-workdir", action="store_true")
    args = ap.parse_args()

    result, raw = run_dosbox(args.timeout, args.keep_workdir)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    args.raw_log.write_text(raw, encoding="utf-8", errors="replace")
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["status"] == "passed_ds_dx_target_resolved" else 2


if __name__ == "__main__":
    raise SystemExit(main())
