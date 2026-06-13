#!/usr/bin/env python3
"""Verify pass652 DM1 V1 chest scroll-wheel pickup-overflow gate."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass652_dm1_v1_chest_scroll_wheel_pickup_overflow_runtime_regression"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC_ROOT = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

FILES = {
    "header": ROOT
    / "src/dm1/dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.h",
    "module": ROOT
    / "src/dm1/dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.c",
    "test": ROOT
    / "tests/test_dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.c",
    "cmake": ROOT / "CMakeLists.txt",
}

REDMCSB_NEEDLES = {
    "CHEST.C": [
        "G0425_aT_ChestSlots[L1017_i_ChestSlotIndex++] = L1018_T_Thing",
        "G0425_aT_ChestSlots[L1017_i_ChestSlotIndex++] = C0xFFFF_THING_NONE",
        "if ((L1023_T_Thing = G0425_aT_ChestSlots[L1025_i_ChestSlotIndex]) != C0xFFFF_THING_NONE)",
        "F0163_DUNGEON_LinkThingToList",
    ],
    "CHAMPION.C": [
        "void F0297_CHAMPION_PutObjectInLeaderHand",
        "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
        "void F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        "L0905_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing",
        "L0906_T_SlotThing = G0425_aT_ChestSlots[L0904_ui_SlotIndex - C30_SLOT_CHEST_1]",
        "F0301_CHAMPION_AddObjectInSlot",
    ],
    "PANEL.C": [
        "STATICFUNCTION void F0344_INVENTORY_DrawPanel_FoodOrWaterBar",
        "void F0345_INVENTORY_DrawPanel_FoodWaterPoisoned",
        "F0077_MOUSE_EnableScreenUpdate_CPSE",
        "F0078_MOUSE_DisableScreenUpdate",
    ],
    "COMMAND.C": [
        "void F0359_COMMAND_ProcessClick_CPSC",
        "case M568_PANEL_RESURRECT_REINCARNATE",
        "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        "C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8",
    ],
    "OBJECT.C": [
        "int16_t F0033_OBJECT_GetIconIndex",
        "return L0005_i_IconIndex",
    ],
    "BLITMASK.C": [
        "void F0133_VIDEO_BlitBoxFilledWithMaskedBitmap",
    ],
    "DEFS.H": [
        "#define C30_SLOT_CHEST_1          30",
        "#define C537_ZONE_SLOT_BOX_38_CHEST_1",
        "#define C544_ZONE_SLOT_BOX_45_CHEST_8",
        "extern void F0077_MOUSE_EnableScreenUpdate_CPSE",
        "extern void F0078_MOUSE_DisableScreenUpdate",
    ],
}

LOCAL_NEEDLES = [
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:113-132",
    "CHAMPION.C F0297:243-268",
    "CHAMPION.C F0298:270-298",
    "CHAMPION.C F0302:662-710",
    "PANEL.C F0344:1895-1944 + F0345:1946-1999",
    "COMMAND.C F0359:1985-1990",
    "MOUSE.C F0077:97-126 + F0078:128-168",
    "OBJECT.C F0033:147-212",
    "BLITMASK.C F0133:30-33",
    "DEFS.H:2088",
    "ROUTE_C544_REPLACEMENT",
    "not ROUTE_TO_FIRST_FREE",
    "close-rewrite skips C0xFFFF_THING_NONE",
]

TEST_NEEDLES = [
    "wheel queued",
    "panel dispatch M568",
    "command dispatch C040",
    "dispatch read C544",
    "C544 replacement route used",
    "first free route not used",
    "leader stack count after",
    "close skipped NONE gaps",
]


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="latin-1")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(cmd: list[str]) -> dict[str, object]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return {
        "cmd": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "output_tail": proc.stdout[-4000:],
    }


def check_needles(label: str, haystack: str, needles: list[str]) -> list[str]:
    return [f"{label}: missing {needle}" for needle in needles if needle not in haystack]


def collect_failures() -> tuple[list[str], dict[str, object]]:
    failures: list[str] = []
    local_text = ""
    file_hashes = {}

    for label, path in FILES.items():
        if not path.exists():
            failures.append(f"{label}: missing {path.relative_to(ROOT)}")
            continue
        text = read(path)
        local_text += f"\n--- {label} ---\n{text}"
        file_hashes[label] = digest(path)

    for source_file, needles in REDMCSB_NEEDLES.items():
        source_path = SRC_ROOT / source_file
        if not source_path.exists():
            failures.append(f"ReDMCSB source missing {source_path}")
            continue
        failures.extend(check_needles(f"ReDMCSB {source_file}", read(source_path), needles))

    failures.extend(check_needles("local module/header/test", local_text, LOCAL_NEEDLES))
    if FILES["test"].exists():
        failures.extend(check_needles("runtime test", read(FILES["test"]), TEST_NEEDLES))
    if FILES["cmake"].exists():
        failures.extend(
            check_needles(
                "CMakeLists.txt",
                read(FILES["cmake"]),
                [
                    "test_dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat",
                    "dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.c",
                    "dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat",
                ],
            )
        )

    manifest = {
        "pass": PASS,
        "status": "PASS" if not failures else "FAIL",
        "source_root": str(SRC_ROOT),
        "files": {label: str(path.relative_to(ROOT)) for label, path in FILES.items()},
        "sha256": file_hashes,
        "source_locked_route": "ROUTE_C544_REPLACEMENT",
        "route_notes": [
            "Leader hand starts occupied and C544 starts occupied.",
            "F0302 replacement route puts the old C544 item in the leader hand.",
            "The old leader item is redirected into C544, not the first free slot.",
            "F0334 close rewrite skips NONE gaps and preserves the redirected item.",
        ],
        "redmcsb_needles": REDMCSB_NEEDLES,
        "local_needles": LOCAL_NEEDLES,
        "failures": failures,
    }
    return failures, manifest


def write_evidence(manifest: dict[str, object]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# pass652 DM1 V1 Chest Scroll-Wheel Pickup Overflow Runtime Regression",
        "",
        f"Status: {manifest['status']}",
        "",
        "This pass adds a synthetic runtime regression gate for the DM1 V1 chest "
        "scroll-wheel path where both the leader hand and C544 are occupied. The "
        "source-locked route is `ROUTE_C544_REPLACEMENT`: F0302 swaps the occupied "
        "C544 item into the leader hand and redirects the old leader-hand item "
        "back into C544, then F0334 compacts only non-empty cells on close.",
        "",
        "## Checked Files",
    ]
    files = manifest["files"]
    for label in sorted(files):
        lines.append(f"- {label}: `{files[label]}`")
    lines.extend(
        [
            "",
            "## ReDMCSB Chain",
            "- CHEST.C F0333:30-67 open materialization into C537..C544.",
            "- CHEST.C F0334:113-132 close rewrite skipping `C0xFFFF_THING_NONE`.",
            "- CHAMPION.C F0297/F0298/F0302 leader-hand and occupied chest-slot swap.",
            "- PANEL.C, COMMAND.C, MOUSE.C, OBJECT.C, BLITMASK.C, and DEFS.H anchor strings are present in the runtime gate.",
            "",
            "## Result",
            f"- Failures: {len(manifest['failures'])}",
            f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
        ]
    )
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--run-test", action="store_true")
    args = parser.parse_args()

    failures, manifest = collect_failures()
    if args.run_test:
        result = run([str(Path(os.environ.get("FIRESTAFF_BUILD_DIR", str(ROOT / "build"))) / "test_dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat")])
        manifest["runtime_test"] = result
        if not result["passed"]:
            failures.append("runtime test failed")
            manifest["failures"] = failures
            manifest["status"] = "FAIL"

    if not args.check_only:
        write_evidence(manifest)

    print(json.dumps({"status": manifest["status"], "failures": failures}, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
