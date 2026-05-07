#!/usr/bin/env python3
"""Pass235: N2-local DOSBox debugger workflow for DM1 PC34 FIRES.EXENEW.

Text-only evidence. FIRES.EXENEW is regenerated in a temporary directory from
N2-local original data and is never written into the repo. The automated part
proves DOSBox debugger entry CS:IP. ReDMCSB source seam CS:IP remains blocked
until a symbol/global-address bridge exists.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shlex
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ORIG_DIR = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
PACKED_FIRES = ORIG_DIR / "FIRES"
UNLZEXE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Base/UNLZEXE/unlzexe.exe"
OUT_DIR = ROOT / "parity-evidence/verification/pass235_dm1_v1_dosbox_debugger_workflow"
REPORT = ROOT / "parity-evidence/pass235_dm1_v1_dosbox_debugger_workflow.md"
EXPECTED_EXENEW_SHA256 = "fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94"
EXPECTED_EXENEW_SIZE = 178224

SEAMS: list[dict[str, Any]] = [
    {"id": "command_accepted", "file": "COMMAND.C", "function": "F0380_COMMAND_ProcessQueue_CPSC", "line_range": [2045, 2156], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked = C1_TRUE;", "L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX = G0432_as_CommandQueue", "L1162_i_CommandY = G0432_as_CommandQueue", "G0435_B_CommandQueueLocked = C0_FALSE;", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "turn_handler", "file": "CLIKMENU.C", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "line_range": [142, 179], "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0308_i_PartyDirection", "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0284_CHAMPION_SetPartyDirection"]},
    {"id": "move_handler", "file": "CLIKMENU.C", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "line_range": [180, 347], "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0306_i_PartyMapX", "G0307_i_PartyMapY", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks"]},
    {"id": "move_result", "file": "MOVESENS.C", "function": "F0267_MOVE_GetMoveResult_CPSCE", "line_range": [316, 850], "needles": ["BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX", "G0307_i_PartyMapY", "C0xFFFF_THING_PARTY", "C02_ELEMENT_PIT", "C05_ELEMENT_TELEPORTER"]},
    {"id": "draw_uses_mutated_tuple", "file": "GAMELOOP.C", "function": "F0002_MAIN_GameLoop_CPSDF", "line_range": [55, 95], "needles": ["for (;;) { /*_Infinite loop_*/", "F0261_TIMELINE_Process_CPSEF();", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"]},
    {"id": "viewport_buffer_composed", "file": "DUNVIEW.C", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "line_range": [8318, 8611], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0127_DUNGEONVIEW_DrawSquareD0C", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]},
    {"id": "viewport_present", "file": "DRAWVIEW.C", "function": "F0097_DUNGEONVIEW_DrawViewport", "line_range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"]},
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_mz(path: Path) -> dict[str, Any]:
    data = path.read_bytes()[:64]
    if len(data) < 0x1C or data[:2] != b"MZ":
        raise ValueError(f"not MZ: {path}")
    vals = struct.unpack_from("<HHHHHHHHHHHH", data, 2)
    e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc, e_ss, e_sp, _sum, e_ip, e_cs, e_lfarlc = vals
    return {"relative_cs_ip": f"{e_cs:04X}:{e_ip:04X}", "relative_ss_sp": f"{e_ss:04X}:{e_sp:04X}", "header_bytes": e_cparhdr * 16, "relocations": e_crlc, "e_lfarlc": f"0x{e_lfarlc:04X}", "first_64_hex": data.hex()}


def compact(s: str) -> str:
    return " ".join(s.split())


def source_audit() -> list[dict[str, Any]]:
    audited: list[dict[str, Any]] = []
    for seam in SEAMS:
        path = SOURCE_ROOT / seam["file"]
        start, end = seam["line_range"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        found: dict[str, int] = {}
        missing: list[str] = []
        for needle in seam["needles"]:
            hit = None
            for idx in range(start - 1, min(end, len(lines))):
                if compact(needle) in compact(lines[idx]):
                    hit = idx + 1
                    break
            if hit is None:
                missing.append(needle)
            else:
                found[needle] = hit
        audited.append({**seam, "source_path": str(path), "needle_lines": found, "missing_needles": missing, "ok": path.exists() and not missing})
    return audited


def normalize_temp_paths(text: str) -> str:
    text = re.sub(r"Z:/tmp/firestaff-pass235-[^\s\"]+", "Z:<TMP>", text)
    return re.sub(r"/tmp/firestaff-pass235-[^\s\"]+", "<TMP>", text)


def clean_ansi(raw: bytes) -> str:
    text = raw.decode("latin-1", errors="replace")
    text = re.sub(r"\x1b\[[0-9;?]*[ -/]*[@-~]", "", text)
    text = text.replace("\r", "\n")
    text = re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)
    return normalize_temp_paths(text)


def make_fires_exenew(stage: Path) -> tuple[Path, dict[str, Any]]:
    work_exe = stage / "FIRES.EXE"
    shutil.copy2(PACKED_FIRES, work_exe)
    proc = subprocess.run(["wine", str(UNLZEXE), f"Z:{work_exe}"], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    exenew = stage / "FIRES.EXENEW"
    if not exenew.exists():
        raise RuntimeError("UNLZEXE did not create FIRES.EXENEW")
    actual = sha256(exenew)
    info = {"tool": str(UNLZEXE), "returncode": proc.returncode, "stdout_tail": normalize_temp_paths(proc.stdout[-1200:]), "path_policy": "temporary only; not copied into repo", "sha256": actual, "size": exenew.stat().st_size, "sha256_matches_expected": actual == EXPECTED_EXENEW_SHA256, "size_matches_expected": exenew.stat().st_size == EXPECTED_EXENEW_SIZE, "mz": parse_mz(exenew)}
    if not info["sha256_matches_expected"] or not info["size_matches_expected"]:
        raise RuntimeError(f"FIRES.EXENEW fixture mismatch: {info}")
    return exenew, info


def run_entry_capture(timeout_s: int = 5) -> dict[str, Any]:
    if not shutil.which("dosbox-debug") or not shutil.which("xvfb-run"):
        return {"ok": False, "blocker": "dosbox-debug or xvfb-run missing"}
    with tempfile.TemporaryDirectory(prefix="firestaff-pass235-") as td:
        stage = Path(td)
        exenew, exenew_info = make_fires_exenew(stage)
        run_exe = stage / "FIRES.EXE"
        shutil.copy2(exenew, run_exe)  # DOSBox executes .EXE names reliably; this is the unpacked image.
        conf = stage / "dosbox-debug.conf"
        conf.write_text("\n".join([
            "[sdl]", "fullscreen=false", "output=surface", "[dosbox]", "machine=svga_paradise", "memsize=4", "[cpu]", "core=normal", "cycles=3000", "[mixer]", "nosound=true", "[autoexec]", f"mount c \"{stage}\"", "c:", "DEBUG FIRES.EXE", "",
        ]))
        cmd = ["bash", "-lc", f"TERM=xterm timeout {timeout_s} xvfb-run -a dosbox-debug -conf {shlex.quote(str(conf))} -exit"]
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout_s + 8)
        clean = clean_ansi(proc.stdout)
        transcript = OUT_DIR / "entry_debugger_transcript.clean.txt"
        transcript.write_text(clean[-12000:], encoding="latin-1")
        entry_hits = sorted(set(re.findall(r"([0-9A-F]{4}):0000\s+BADA26", clean)))
        cs_regs = sorted(set(re.findall(r"CS=([0-9A-F]{4})", clean)))
        runtime_cs = entry_hits[-1] if entry_hits else (cs_regs[-1] if cs_regs else None)
        psp = f"{(int(runtime_cs, 16) - 0x10) & 0xFFFF:04X}" if runtime_cs else None
        return {"ok": bool(runtime_cs), "command": ["bash", "-lc", "TERM=xterm timeout 5 xvfb-run -a dosbox-debug -conf <TMP>/dosbox-debug.conf -exit"], "returncode": proc.returncode, "fires_exenew": exenew_info, "entry_runtime_cs_ip": f"{runtime_cs}:0000" if runtime_cs else None, "psp_segment_inferred": psp, "evidence_regex": "([0-9A-F]{4}):0000\\s+BADA26", "transcript_excerpt_path": str(transcript), "matched_entry_segments": entry_hits, "matched_cs_registers": cs_regs, "safe_interpretation": "actual DOSBox debugger runtime entry for temp FIRES.EXENEW copy; not a ReDMCSB function seam hit"}


def write_runbook(path: Path) -> None:
    path.write_text("""# Pass235 manual DOSBox debugger sequence

Automated entry capture:
1. Regenerate `FIRES.EXENEW` in a temp directory from N2-local `DungeonMasterPC34/FIRES` using Wine `unlzexe.exe`.
2. Temp-copy that unpacked image as `FIRES.EXE` only because DOSBox executes `.EXE` names reliably.
3. Start `TERM=xterm xvfb-run -a dosbox-debug -conf dosbox-debug.conf -exit` with autoexec command `DEBUG FIRES.EXE`.
4. The debugger stops at the program entry. Record the first code line `CS:0000 BADA26...` and register view `CS=....`.

Keystroke-level interactive continuation when a human display is available:
1. Run the generated config without `timeout`.
2. At debugger prompt `->`, type `CPU` Enter and confirm the entry `CS:IP`.
3. Type `LOGS 200` Enter or `LOG 200` Enter to write a bounded CPU log, then press `F5` to run.
4. Drive the game to the target command/movement/viewport scenario in the DOSBox window.
5. Press `Alt+Pause` to re-enter the debugger; type `CPU` Enter and `LOGS 200` Enter.
6. Do not promote F0380/F0365/F0366/F0267/F0128/F0097 until a FIRES.MAP/public symbol table or validated disassembly-address bridge supplies exact breakpoints/watchpoints.

Current hard blocker:
- DOSBox debugger accepts numeric `BP segment:offset` and `BPM segment:offset`; it does not understand ReDMCSB C function names.
- We have actual loader-entry runtime CS:IP, but no reproducible N2-local map from ReDMCSB source seams/globals to FIRES.EXENEW runtime offsets.
""", encoding="utf-8")


def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass235 — DM1 PC34 DOSBox debugger workflow", "", f"Status: `{manifest['status']}`", "", "## Captured runtime evidence", ""]
    cap = manifest["entry_capture"]
    fires = cap.get("fires_exenew", {})
    mz = fires.get("mz", {})
    lines.append(f"- FIRES.EXENEW SHA256: `{fires.get('sha256')}`")
    lines.append(f"- Static EXENEW entry: `{mz.get('relative_cs_ip')}`")
    lines.append(f"- Actual DOSBox debugger entry hit: `{cap.get('entry_runtime_cs_ip')}`")
    lines.append(f"- Inferred PSP segment: `{cap.get('psp_segment_inferred')}`")
    lines.append(f"- Transcript: `{cap.get('transcript_excerpt_path')}`")
    lines += ["", "## Source seam audit", ""]
    for item in manifest["source_audit"]:
        mark = "PASS" if item["ok"] else "FAIL"
        start_line, end_line = item["line_range"]
        lines.append(f"- {mark} `{item['id']}` — `{item['file']}:{start_line}-{end_line}` / `{item['function']}`")
    lines += ["", "## Command/movement/viewport CS:IP blocker", "", manifest["runtime_seam_blocker"], "", f"Runbook: `{manifest['runbook']}`", ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true", help="kept for gate compatibility; full run is always bounded")
    _ = parser.parse_args()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    audit = source_audit()
    runbook = OUT_DIR / "dosbox_debugger_runbook.md"
    write_runbook(runbook)
    entry = run_entry_capture()
    status = "PASS_ENTRY_CAPTURE_BLOCKED_SOURCE_SEAM_CSIP" if entry.get("ok") and all(i["ok"] for i in audit) else "FAIL"
    manifest = {"schema": "pass235_dm1_v1_dosbox_debugger_workflow.v1", "status": status, "repo": str(ROOT), "source_root": str(SOURCE_ROOT), "source_audit": audit, "entry_capture": entry, "runbook": str(runbook), "runtime_seam_blocker": "Actual loader entry CS:IP is captured, but COMMAND.C F0380, CLIKMENU movement, MOVESENS F0267, GAMELOOP/DUNVIEW/DRAWVIEW viewport breakpoints remain blocked by missing FIRES.EXENEW source-symbol/global-address map. Numeric DOSBox BP/BPM addresses are required before runtime hits can be claimed.", "artifact_policy": {"no_binary_payloads_in_repo": True, "no_screenshots": True, "text_only": True}}
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "entry_runtime_cs_ip": entry.get("entry_runtime_cs_ip"), "manifest": str(OUT_DIR / "manifest.json"), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
