#!/usr/bin/env python3
"""Verify pass794 — DM1 V1 champion-panel all-states redraw dispatcher."""
import os, subprocess, sys, json
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
PASS = "pass794_dm1_v1_champion_panel_all_states_pc34_compat"
HDR = ROOT / "include/dm1_v1_champion_panel_all_states_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_panel_all_states_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = ("CHAMDRAW.C F0293:1117-1143 (all-champion redraw dispatcher with "
       "dirty-mask OR, F0292 call order, G2149_ clear after loop on PC34); "
       "DEFS.H:724-732 (dirty flags 0x0080..0x8000); "
       "DEFS.H:8090-8100 (C00..C03 champion indices, G0305, G2149)")
NEEDLES_T = ["F0293", "CHAMDRAW", "dirty_mask", "G2149", "F0292", "all_states", "redraw"]
NEEDLES_H = ["F0293", "G2149", "dirty", "all_states"]
NEEDLES_C = [
    "test_dm1_v1_champion_panel_all_states_pc34_compat",
    "src/dm1/dm1_v1_champion_panel_all_states_pc34_compat.c",
    "NAME dm1_v1_champion_panel_all_states_pc34_compat",
    f"verify_{PASS}",
]
def check_needles(label, path, needles):
    if not path.exists():
        return [f"missing {label}: {path}"]
    text = path.read_text()
    missing = [n for n in needles if n not in text]
    return [f"{label} missing: {missing}"] if missing else []
def resolve_build_dir(binary_name=""):
    candidates = [ROOT / "build", ROOT / "builds" / "nv1-build", ROOT / "builds" / "n2-build"]
    if binary_name:
        for c in candidates:
            if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
                return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]
def main():
    failures = []
    for label, path, needles in [("header", HDR, NEEDLES_H), ("test_source", TEST, NEEDLES_T), ("cmake_registration", CMAKE, NEEDLES_C)]:
        failures.extend(check_needles(label, path, needles))
    binary_name = "test_dm1_v1_champion_panel_all_states_pc34_compat"
    build_dir = resolve_build_dir(binary_name)
    binary = build_dir / binary_name
    if not binary.exists():
        failures.append(f"binary not found: {binary}")
    else:
        proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
        passes = proc.stdout.count("PASS ")
        fails = proc.stdout.count("FAIL ")
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} passes={passes} fails={fails}")
        else:
            print(f"{PASS}: PASS passes={passes}")
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {"pass": PASS, "status": "PASS" if not failures else "FAIL",
                "failures": failures, "redmcsb": RED, "tests": {"passes": passes if not failures else 0, "fails": fails if not failures else 0}}
    MANIFEST.write_text(json.dumps(manifest, indent=2))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(f"# {PASS}\n\nSource-locked: {RED}\n\nResult: {manifest['status']}\nTests: {manifest['tests']}\nFailures: {failures}\n")
    return 0 if not failures else 1
if __name__ == "__main__":
    sys.exit(main())
