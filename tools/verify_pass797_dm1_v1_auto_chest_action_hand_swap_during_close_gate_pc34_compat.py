#!/usr/bin/env python3
"""Verify pass797 — DM1 V1 auto chest action-hand swap during close gate."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass797_dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat"
HDR = ROOT / "include/dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44 (C145_ICON_CONTAINER_CHEST_OPEN blit skip); "
    "CHEST.C F0333:53-67 + 70-74 (G0425_aT_ChestSlots population + C0xFFFF_THING_NONE fill); "
    "CHEST.C F0334:113-117 (G0426 clear, container Slot to C0xFFFE_THING_ENDOFLIST); "
    "CHEST.C F0334:118-132 (G0425 close-loop with L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList); "
    "CHAMPION.C F0297:243-268/F0298:270-298 (put/remove leader-hand); "
    "CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 write + load); "
    "CHAMPION.C F0302:688-710 (leader hand + destination slot click dispatcher); "
    "PANEL.C F0347:1639-1691 (C09_SLOT_BOX_INVENTORY_ACTION_HAND click to F0302)"
)
NEEDLES_T = [
    "F0333", "F0334", "F0297", "F0298", "F0300", "F0301", "F0302",
    "PANEL.C F0347", "C09_SLOT_BOX_INVENTORY_ACTION_HAND",
    "P0694_B_PressingEye", "C145_ICON_CONTAINER_CHEST_OPEN",
    "C0xFFFE_THING_ENDOFLIST",
    "DM1_PC34_SLOT_ACTION_HAND", "press-eye",
    "auto-close", "action hand"
]
NEEDLES_H = [
    "F0333", "F0334", "press-eye", "action hand",
    "DM1_PC34_SLOT_ACTION_HAND", "C09_SLOT_BOX_INVENTORY_ACTION_HAND"
]
NEEDLES_C = [
    "test_dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat",
    "src/dm1/dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat.c",
    "NAME dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat",
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

    binary_name = "test_dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat"
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
