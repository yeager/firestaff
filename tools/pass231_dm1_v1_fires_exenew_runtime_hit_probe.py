#!/usr/bin/env python3
"""Pass231: reproducible FIRES.EXENEW loader-entry/runtime-hit probe.

This pass records what can be proved on N2 without promoting static offsets to
runtime source-symbol hits. It validates the ignored FIRES.EXENEW fixture,
audits the ReDMCSB command_accepted seam, records available debugger tools, and
keeps command_accepted blocked until a debugger transcript or map supplies a real
CS:IP hit.
"""
from __future__ import annotations

import hashlib
import json
import shutil
import struct
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/pass231_dm1_v1_fires_exenew_runtime_hit_probe"
REPORT = ROOT / "parity-evidence/pass231_dm1_v1_fires_exenew_runtime_hit_probe.md"
RUNTIME_IMAGE = ROOT / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
SOURCE_ROOTS = [
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source",
]


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_mz(path: Path) -> dict[str, Any]:
    data = path.read_bytes()[:64]
    if len(data) < 0x1C or data[:2] != b"MZ":
        raise ValueError(f"not an MZ executable: {path}")
    vals = struct.unpack_from("<HHHHHHHHHHHH", data, 2)
    e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc, e_ss, e_sp, _e_csum, e_ip, e_cs, e_lfarlc = vals
    file_size_from_header = (e_cp - 1) * 512 + (e_cblp or 512)
    return {
        "magic": data[:2].decode("ascii"),
        "e_cblp": e_cblp,
        "e_cp": e_cp,
        "e_crlc": e_crlc,
        "e_cparhdr": e_cparhdr,
        "header_bytes": e_cparhdr * 16,
        "e_minalloc": e_minalloc,
        "e_maxalloc": e_maxalloc,
        "relative_ss_sp": f"{e_ss:04x}:{e_sp:04x}",
        "relative_cs_ip": f"{e_cs:04x}:{e_ip:04x}",
        "e_lfarlc": f"0x{e_lfarlc:04x}",
        "declared_file_size": file_size_from_header,
        "actual_file_size": path.stat().st_size,
        "load_module_bytes": max(path.stat().st_size - e_cparhdr * 16, 0),
        "first_64": data.hex(),
    }


def command_source_audit() -> dict[str, Any]:
    src_root = next((p for p in SOURCE_ROOTS if (p / "COMMAND.C").exists()), None)
    if src_root is None:
        raise FileNotFoundError("COMMAND.C not found in known ReDMCSB source roots")
    lines = (src_root / "COMMAND.C").read_text(errors="replace").splitlines()
    needles = {
        "lock_queue": "G0435_B_CommandQueueLocked = C1_TRUE;",
        "empty_queue_i34e": "if (G2153_i_QueuedCommandsCount == 0)",
        "read_command": "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "read_x": "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "read_y": "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
        "decrement_count": "G2153_i_QueuedCommandsCount--;",
        "advance_first": "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
        "unlock_queue": "G0435_B_CommandQueueLocked = C0_FALSE;",
        "turn_dispatch": "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "move_dispatch": "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    }
    found: dict[str, int] = {}
    for key, needle in needles.items():
        start = 2117 if key in {"read_x", "read_y", "decrement_count", "advance_first", "unlock_queue"} else 2044
        for idx in range(start, min(2156, len(lines))):
            if needle in lines[idx]:
                found[key] = idx + 1
                break
    missing = [k for k in needles if k not in found]
    return {
        "source_root": str(src_root),
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "line_window": [2045, 2156],
        "needle_lines": found,
        "missing_needles": missing,
        "command_accepted_source_window": [2118, 2126],
        "dispatch_window": [2150, 2156],
    }


def run_cmd(args: list[str], timeout: int = 8) -> dict[str, Any]:
    try:
        proc = subprocess.run(args, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout)
        return {"cmd": args, "returncode": proc.returncode, "stdout_tail": proc.stdout[-4000:], "stderr_tail": proc.stderr[-4000:]}
    except Exception as exc:
        return {"cmd": args, "error": type(exc).__name__, "message": str(exc)}


def debugger_inventory() -> dict[str, Any]:
    inv = {name: shutil.which(name) for name in ["dosbox-x", "dosbox-debug", "dosbox", "ndisasm", "objdump"]}
    probes: dict[str, Any] = {}
    if inv.get("dosbox-x"):
        probes["dosbox_x_version"] = run_cmd([inv["dosbox-x"] or "dosbox-x", "-version"])
    if inv.get("dosbox-debug"):
        probes["dosbox_debug_help_probe"] = run_cmd([
            "bash",
            "-lc",
            "TERM=xterm timeout 3 dosbox-debug -help 2>&1 | perl -pe 's/\\e\\[[0-9;?]*[A-Za-z]//g' | head -80",
        ], timeout=5)
    return {"tools": inv, "probes": probes}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    runtime = load_json(RUNTIME_IMAGE)
    symbol_map = load_json(SYMBOL_MAP)
    dec = runtime["decompressed_fires"]
    exenew = ROOT / dec["path"]
    source = command_source_audit()
    if not exenew.exists():
        manifest = {
            "schema": "pass231_dm1_v1_fires_exenew_runtime_hit_probe.v1",
            "classification": "blocked/local-fires-exenew-fixture-not-committed",
            "inputs": {"runtime_image": str(RUNTIME_IMAGE), "symbol_map": str(SYMBOL_MAP), "fires_exenew": str(exenew)},
            "fires_exenew": {"present": False, "expected_size": dec.get("size"), "expected_sha256": dec.get("sha256")},
            "redmcsb_command_source_audit": source,
            "promotion_rule": "FIRES.EXENEW is intentionally local evidence only; require local regeneration plus debugger/map hit before promotion.",
            "non_claims": ["does not commit FIRES.EXENEW", "does not claim COMMAND.C F0380 has a runtime CS:IP"],
        }
        (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
        REPORT.write_text(
            "# Pass231 — FIRES.EXENEW runtime-hit probe\n\n"
            "Classification: `blocked/local-fires-exenew-fixture-not-committed`.\n\n"
            "FIRES.EXENEW is intentionally not committed; regenerate local evidence before promoting runtime CS:IP claims.\n"
        )
        print(json.dumps({"classification": manifest["classification"], "manifest": str(OUT_DIR / "manifest.json"), "report": str(REPORT)}, indent=2, sort_keys=True))
        return 0
    mz = parse_mz(exenew)
    actual_sha = sha256(exenew)
    symbol_entry = next((e for e in symbol_map.get("entries", []) if e.get("id") == "command_accepted"), {})
    command_runtime_hit = {
        "id": "command_accepted",
        "source_citation": symbol_entry.get("source_citation"),
        "runtime_cs_ip": symbol_entry.get("runtime_cs_ip"),
        "confidence": symbol_entry.get("confidence"),
        "status": "blocked/runtime-cs-ip-hit-required",
        "reason": "FIRES.EXENEW proves the decompressed MZ entry/layout, but no FIRES.MAP/public-symbol table or debugger transcript binds COMMAND.C F0380 to a loaded CS:IP.",
    }
    manifest = {
        "schema": "pass231_dm1_v1_fires_exenew_runtime_hit_probe.v1",
        "classification": "blocked/command-accepted-runtime-cs-ip-unavailable",
        "inputs": {"runtime_image": str(RUNTIME_IMAGE), "symbol_map": str(SYMBOL_MAP), "fires_exenew": str(exenew)},
        "fires_exenew": {
            "present": exenew.exists(),
            "size_matches_runtime_fixture": exenew.stat().st_size == dec.get("size"),
            "sha256_matches_runtime_fixture": actual_sha == dec.get("sha256"),
            "sha256": actual_sha,
            "mz": mz,
            "verified_static_entry": mz["relative_cs_ip"],
            "safe_interpretation": "decompressed EXE MZ entry relative to DOS program load image; not a ReDMCSB source seam CS:IP",
        },
        "runtime_load_formula": {
            "program_load_segment": "PSP + 0x10",
            "entry_runtime_cs": "program_load_segment + e_cs",
            "entry_runtime_ip": "e_ip",
            "command_accepted_runtime_cs_ip": "requires FIRES.MAP/source map or observed debugger breakpoint hit; not derivable from MZ header alone",
        },
        "redmcsb_command_source_audit": source,
        "command_accepted": command_runtime_hit,
        "debugger_inventory": debugger_inventory(),
        "promotion_rule": "Do not promote command_accepted until confidence is verified_runtime_hit and runtime_cs_ip plus hit context are recorded.",
        "non_claims": [
            "does not commit FIRES.EXENEW",
            "does not claim COMMAND.C F0380 has a runtime CS:IP",
            "does not use packed LZEXE loader entry as a decompressed source address",
        ],
    }
    if not manifest["fires_exenew"]["size_matches_runtime_fixture"] or not manifest["fires_exenew"]["sha256_matches_runtime_fixture"]:
        manifest["classification"] = "fail/fires-exenew-fixture-mismatch"
    if source["missing_needles"]:
        manifest["classification"] = "fail/redmcsb-command-source-audit-missing-needles"

    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    REPORT.write_text("\n".join([
        "# Pass231 — FIRES.EXENEW runtime-hit probe",
        "",
        f"- Classification: `{manifest['classification']}`.",
        f"- FIRES.EXENEW SHA256: `{actual_sha}`; size `{exenew.stat().st_size}` bytes.",
        f"- Decompressed EXE MZ entry: `{mz['relative_cs_ip']}`; SS:SP `{mz['relative_ss_sp']}`; header `{mz['header_bytes']}` bytes; relocations `{mz['e_crlc']}`.",
        "- Safe interpretation: this is the decompressed program entry relative to the DOS load image, not a `command_accepted` source-seam address.",
        f"- ReDMCSB command seam audited: `{source['source_root']}/COMMAND.C:{source['line_window'][0]}-{source['line_window'][1]}`.",
        f"- `command_accepted` source window: `COMMAND.C:{source['command_accepted_source_window'][0]}-{source['command_accepted_source_window'][1]}` before dispatch at `COMMAND.C:{source['dispatch_window'][0]}-{source['dispatch_window'][1]}`.",
        "- Runtime CS:IP hit: **not captured**. Blocker is exact: no FIRES.MAP/public-symbol bridge or debugger transcript binds F0380 to loaded CS:IP.",
        "",
        "Evidence file: `parity-evidence/verification/pass231_dm1_v1_fires_exenew_runtime_hit_probe/manifest.json`.",
    ]) + "\n")
    print(json.dumps({"classification": manifest["classification"], "manifest": str(OUT_DIR / "manifest.json"), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0 if manifest["classification"].startswith("blocked/") else 1


if __name__ == "__main__":
    raise SystemExit(main())
