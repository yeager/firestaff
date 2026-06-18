#!/usr/bin/env python3
"""
verify_pass793_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.py
"""
import os
import subprocess
import sys
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass793_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat"
HDR = ROOT / "include/dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = (
    "CHAMPION.C F0302:662-714 (slot-box click dispatcher with "
    "slot/chest selection, AllowedSlots rejection, helper order, "
    "BUG0_39 order preservation); "
    "CHAMPION.C F0297:243-268; F0298; F0300; F0301; F0292; "
    "COMMAND.C F0359 (C020..C065 click commands into F0302)"
)

C_NEEDLES = [
    "F0302",
    "F0297",
    "F0298",
    "F0300",
    "F0301",
    "F0292",
    "F0359",
    "slot_priority",
    "AllowedSlots",
    "BUG0_39",
    "chest_slot",
    "C020..C065",
]
LOCAL_NEEDLES = [
    "action-hand slot-priority",
    "F0302:662-714",
    "slot/chest selection",
    "AllowedSlots rejection",
    "BUG0_39 order preservation",
    "pass793",
]
CMAKE_NEEDLES = [
    "test_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat",
    "src/dm1/dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat.c",
    "NAME dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat",
    f"verify_{PASS}",
]

def check_needles(label, path, needles):
    if not path.exists():
        return [f"missing {label}: {path}"]
    text = path.read_text()
    return [f"{label} missing: {[n for n in needles if n not in text]}"] if any(n not in text for n in needles) else []

def resolve_build_dir():
    candidates = [
        Path.cwd(),
        ROOT / "builds" / "nv1-build",
        ROOT / "build",
    ]
    for c in candidates:
        if (c / "test_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat").exists():
            return c
    return candidates[1]

def run_test(build_dir):
    binary = build_dir / "test_dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat"
    if not binary.exists():
        return None, f"binary not found: {binary}"
    proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
    return proc, None

def parse_output(text):
    passes = text.count("PASS ")
    fails = text.count("FAIL ")
    final = ""
    for line in text.splitlines():
        if "assertions" in line:
            final = line.strip()
    return passes, fails, final

def main():
    failures = []
    for label, path, needles in [
        ("header", HDR, C_NEEDLES[:6] + ["slot_priority", "AllowedSlots"]),
        ("test_source", TEST, C_NEEDLES),
        ("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]:
        for m in check_needles(label, path, needles):
            failures.append(m)
    build_dir = resolve_build_dir()
    proc, err = run_test(build_dir)
    if proc is None:
        failures.append(err)
    else:
        text = proc.stdout + proc.stderr
        passes, fails, final = parse_output(text)
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} passes={passes} fails={fails} final='{final}'")
        else:
            print(f"{PASS}: PASS passes={passes} final='{final}'")
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "pass": PASS, "status": "PASS" if not failures else "FAIL",
        "failures": failures, "redmcsb": RED,
        "tests": {"passes": proc and passes or 0, "fails": proc and fails or 0},
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(
        f"# {PASS}\n\n"
        f"Source-locked: {RED}\n\n"
        f"Result: {manifest['status']}\n"
        f"Tests: {manifest['tests']}\n"
        f"Failures: {failures}\n"
    )
    return 0 if not failures else 1

if __name__ == "__main__":
    sys.exit(main())
