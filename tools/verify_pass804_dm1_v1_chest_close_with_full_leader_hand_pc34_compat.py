#!/usr/bin/env python3
"""Verify pass804 — DM1 V1 chest close with full leader hand."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass804_dm1_v1_chest_close_with_full_leader_hand_pc34_compat"
HDR = ROOT / "include/dm1_v1_chest_close_with_full_leader_hand_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_chest_close_with_full_leader_hand_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHEST.C F0333:53-67 (F0333_INVENTORY_OpenAndDrawChest: copy first "
    "eight linked things into G0425_aT_ChestSlots in list order); "
    "CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest: clear G0426, "
    "rewire G0425 chain into the container Slot list, terminate with "
    "C0xFFFE_THING_ENDOFLIST); "
    "CHAMPION.C F0297:243-268/F0298:270-298 (put/remove leader-hand "
    "identity and update leader load); "
    "CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 write "
    "+ load); F0302:688-710 (leader hand + destination slot click "
    "dispatcher with empty/empty + incompatible destination rejection); "
    "DUNGEON.C F0140:1114-1120 (container base weight preservation "
    "through close rewire); "
    "DEFS.H C00..C29, C038, C040, C04_CHAMPION_CLOSE_INVENTORY, "
    "C0xFFFE_THING_ENDOFLIST, C0xFFFF_THING_NONE"
)
NEEDLES_T = [
    "F0333", "F0334", "F0297", "F0300", "F0301", "F0302",
    "F0140", "F0140", "G0425",
    "close_with_full_leader_hand", "leader hand", "close"
]
NEEDLES_H = [
    "close_with_full_leader_hand",
    "DM1_PC34_SLOT_CHEST", "CHEST_SLOT_COUNT"
]
NEEDLES_C = [
    "test_dm1_v1_chest_close_with_full_leader_hand_pc34_compat",
    "src/dm1/dm1_v1_chest_close_with_full_leader_hand_pc34_compat.c",
    "NAME dm1_v1_chest_close_with_full_leader_hand_pc34_compat",
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

    binary_name = "test_dm1_v1_chest_close_with_full_leader_hand_pc34_compat"
    build_dir = resolve_build_dir(binary_name)
    binary = build_dir / binary_name
    if not binary.exists():
        failures.append(f"binary not found: {binary}")
        passes = 0
        fails = 0
    else:
        proc = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
        passes = proc.stdout.count("ok ")
        fails = proc.stdout.count("FAIL ")
        if proc.returncode != 0 or fails > 0:
            failures.append(f"runtime: exit={proc.returncode} ok={passes} FAIL={fails}")
        else:
            print(f"{PASS}: PASS ok={passes}")

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
