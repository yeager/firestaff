#!/usr/bin/env python3
"""Verify pass803 — DM1 V1 chest auto-close on leader death."""
import os, subprocess, sys, json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass803_dm1_v1_chest_auto_close_on_leader_death_pc34_compat"
HDR = ROOT / "include/firestaff/dm1/v1/chest/auto_close_on_leader_death_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    "CHAMPION.C F0319:1552-1607 (F0319_CHAMPION_Kill: CurrentHealth=0, "
    "clear G0333 + G0331, dispatch F0355 with C04_CHAMPION_CLOSE_INVENTORY, "
    "then F0318_DropAllObjects); "
    "CHAMPION.C F0318:1527-1551 (F0318_CHAMPION_DropAllObjects: every "
    "C00..C29 slot to leader's current Cell); "
    "PANEL.C F0355:2244-2310 (F0355_INVENTORY_Toggle_CPSE closes inventory "
    "panel with C04_CHAMPION_CLOSE_INVENTORY); "
    "PANEL.C F0355:2268-2275 (death short-circuit returns early when "
    "champion is dead and not closing inventory); "
    "PANEL.C F0355:2318-2322 (F0334 call mutates G0426 to C0xFFFF_THING_NONE); "
    "CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest clears G0426 and "
    "rewires G0425_aT_ChestSlots into the container Slot list); "
    "CHEST.C F0333:30-67 (negative no-open anchor: no reopen during death); "
    "CHAMPION.C F0297/F0298:243-298 (no-leader-hand-mutate anchor during death); "
    "CHAMPION.C F0300/F0301:511-614 (C00..C29 get/put object primitives for F0318); "
    "COMMAND.C F0380:2045-2184 (negative no-queue-drain anchor); "
    "DEFS.H C00..C29, C30..C37, C038, C040, C04_CHAMPION_CLOSE_INVENTORY, "
    "C10_COLOR_FLESH, G0299, G0331, G0333, G0423, G0424, G0425, G0426, "
    "M516_CHAMPIONS[].CurrentHealth/Load/Slots"
)
NEEDLES_T = [
    "F0319", "F0318", "F0355", "F0334", "F0333", "F0297", "F0298",
    "F0300", "F0301", "F0380",
    "G0425", "G0426", "G0331", "G0333", "G0423", "G0424", "G0299",
    "CurrentHealth", "M516_CHAMPIONS",
    "C04_CHAMPION_CLOSE_INVENTORY", "DEFS.H",
    "auto_close", "leader_death", "death"
]
NEEDLES_H = [
    "F0319", "F0318", "F0355", "F0334", "F0333", "F0297", "F0298",
    "F0300", "F0301",
    "G0425", "G0426", "G0331", "G0333", "G0423", "G0424",
    "CurrentHealth", "M516_CHAMPIONS",
    "C04_CHAMPION_CLOSE_INVENTORY"
]
NEEDLES_C = [
    "test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat",
    "src/dm1/dm1_v1_chest_auto_close_on_leader_death_pc34_compat.c",
    "NAME dm1_v1_chest_auto_close_on_leader_death_pc34_compat",
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

    binary_name = "test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat"
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
