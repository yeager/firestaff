#!/usr/bin/env python3
"""Verify pass850 — DM1 V1 chest empty reopen runtime."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass850_dm1_v1_chest_empty_reopen_runtime_pc34_compat"
HDR = ROOT / "include/dm1_v1_chest_empty_reopen_runtime_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_chest_empty_reopen_runtime_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHEST.C F0333:30-32 (F0333_INVENTORY_OpenAndDrawChest: open path "
    "entry, same-open return); "
    "CHEST.C F0333:36-46 (chain walk via F0159, eight-item cap, "
    "G0425_aT_ChestSlots writes); "
    "CHEST.C F0333:67-76 (fills the unused visible G0425_aT_ChestSlots "
    "[0..7] with C0xFFFF_THING_NONE so the panel never shows stale "
    "icons from a previous chest); "
    "CHEST.C F0334:113-117 (close-rewire: no-open return, G0426 clear, "
    "Container->Slot=C0xFFFE_THING_ENDOFLIST clobber); "
    "CHEST.C F0334:117-132 (close-loop with L1026_B_ProcessFirstChestSlot "
    "+ F0163_DUNGEON_LinkThingToList); "
    "CHAMPION.C F0297:243-298 (leader-hand put/remove, weight/charges/"
    "AllowedSlots/load, not called by CHEST.C F0333 or F0334); "
    "DEFS.H:434, 778-817"
)
NEEDLES_T = [
    "F0333", "F0334",
    "F0297", "F0298",
    "G0425", "G0426",
    "C0xFFFF",
    "empty", "reopen", "runtime"
]
NEEDLES_H = [
    "F0333", "F0334",
    "F0298", "F0302", "F0358", "F0378",
    "G0425", "G0426",
    "empty", "reopen"
]
NEEDLES_C = [
    "test_dm1_v1_chest_empty_reopen_runtime_pc34_compat",
    "src/dm1/dm1_v1_chest_empty_reopen_runtime_pc34_compat.c",
    "NAME dm1_v1_chest_empty_reopen_runtime_pc34_compat",
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
    for label, path, needles in [
        ("header", HDR, NEEDLES_H),
        ("test_source", TEST, NEEDLES_T),
        ("cmake_registration", CMAKE, NEEDLES_C),
    ]:
        failures.extend(check_needles(label, path, needles))

    binary_name = "test_dm1_v1_chest_empty_reopen_runtime_pc34_compat"
    build_dir = resolve_build_dir(binary_name)
    binary = build_dir / binary_name
    if not binary.exists():
        failures.append(f"binary not found: {binary}")
        passes = 0
        fails = 0
    else:
        proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
        passes = proc.stdout.count("PASS ")
        fails = proc.stdout.count("FAIL ")
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} passes={passes} fails={fails}")
        else:
            print(f"{PASS}: PASS passes={passes}")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "pass": PASS,
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "redmcsb": RED,
        "tests": {"passes": passes if not failures else 0, "fails": fails if not failures else 0},
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(
        f"# {PASS}\n\nSource-locked: {RED}\n\nResult: {manifest['status']}\n"
        f"Tests: {manifest['tests']}\nFailures: {failures}\n"
    )
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
