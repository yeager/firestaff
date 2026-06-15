#!/usr/bin/env python3
"""Verify pass764 DM1 V1 chest partial-drop-to-floor runtime gate."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass764_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC_ROOT = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

FILES = {
    "header": ROOT
    / "include/firestaff/dm1/v1/chest/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.h",
    "module": ROOT
    / "src/dm1/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.c",
    "test": ROOT
    / "tests/test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.c",
    "cmake": ROOT / "CMakeLists.txt",
    "evidence": REPORT,
}

REDMCSB_NEEDLES = {
    "CHEST.C": [
        "void F0333_INVENTORY_OpenAndDrawChest",
        "if (G0426_T_OpenChest != C0xFFFF_THING_NONE)",
        "G0425_aT_ChestSlots[L1017_i_ChestSlotIndex++] = L1018_T_Thing",
        "void F0334_INVENTORY_CloseChest",
        "F0163_DUNGEON_LinkThingToList",
    ],
    "CHAMPION.C": [
        "void F0297_CHAMPION_PutObjectInLeaderHand",
        "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
        "G0425_aT_ChestSlots[P0629_ui_SlotIndex - C30_SLOT_CHEST_1]",
        "G0425_aT_ChestSlots[P0632_ui_SlotIndex - C30_SLOT_CHEST_1]",
        "void F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
    ],
    "COMMAND.C": [
        "case M569_PANEL_CHEST",
        "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        "case M568_PANEL_RESURRECT_REINCARNATE",
    ],
    "OBJECT.C": [
        "int16_t F0032_OBJECT_GetType",
        "int16_t F0033_OBJECT_GetIconIndex",
    ],
    "BLITMASK.C": [
        "void F0133_VIDEO_BlitBoxFilledWithMaskedBitmap",
    ],
    "DUNGEON.C": [
        "void F0163_DUNGEON_LinkThingToList",
        "if (P0289_i_MapX >= 0)",
    ],
    "DEFS.H": [
        "#define C10_COLOR_FLESH",
        "#define C537_ZONE_SLOT_BOX_38_CHEST_1",
        "#define C544_ZONE_SLOT_BOX_45_CHEST_8",
    ],
}

LOCAL_NEEDLES = [
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:113-132",
    "CHAMPION.C F0297:243-268",
    "CHAMPION.C F0298:270-298",
    "CHAMPION.C F0300:511-515",
    "CHAMPION.C F0301:606-614",
    "CHAMPION.C F0302:662-710",
    "COMMAND.C F0359:1973-1983",
    "COMMAND.C F0359:1985-1990",
    "OBJECT.C F0032:121-145",
    "OBJECT.C F0033:147-212",
    "BLITMASK.C F0133:30-33",
    "DUNGEON.C F0163:1796-1837",
    "DEFS.H:2088",
    "G0425/G0426",
    "M070",
    "M516",
    "partialDropCount",
    "floorReceivedPartial",
    "closedChainPreservesRemaining",
]

TEST_NEEDLES = [
    "deterministic hash",
    "leader hand split count",
    "floor received partial",
    "closed chain preserves remaining",
    "assertions=",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat",
    "dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.c",
    "dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat",
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


def missing(label: str, haystack: str, needles: list[str]) -> list[str]:
    return [f"{label}: missing {needle}" for needle in needles if needle not in haystack]


def collect_failures() -> tuple[list[str], dict[str, object]]:
    failures: list[str] = []
    local_text = ""
    file_hashes: dict[str, str] = {}

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
        failures.extend(missing(f"ReDMCSB {source_file}", read(source_path), needles))

    failures.extend(missing("local files", local_text, LOCAL_NEEDLES))
    if FILES["test"].exists():
        failures.extend(missing("runtime test", read(FILES["test"]), TEST_NEEDLES))
    if FILES["cmake"].exists():
        failures.extend(missing("CMakeLists.txt", read(FILES["cmake"]), CMAKE_NEEDLES))

    build = run(
        [
            "cmake",
            "--build",
            "build",
            "--target",
            "test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat",
            "--parallel",
        ]
    )
    binary = run(["./build/test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat"])
    ctest = run(
        [
            "ctest",
            "--test-dir",
            "build",
            "-R",
            "dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat",
            "--output-on-failure",
        ]
    )
    if not build["passed"]:
        failures.append("build failed")
    if not binary["passed"]:
        failures.append("test binary failed")
    if not ctest["passed"]:
        failures.append("ctest failed")

    manifest = {
        "pass": PASS,
        "status": "PASS" if not failures else "FAIL",
        "source_root": str(SRC_ROOT),
        "files": {label: str(path.relative_to(ROOT)) for label, path in FILES.items()},
        "sha256": file_hashes,
        "expected_hash": "0xe50d0bc4",
        "build": build,
        "binary": binary,
        "ctest": ctest,
        "failures": failures,
    }
    return failures, manifest


def write_manifest(manifest: dict[str, object]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    failures, manifest = collect_failures()
    write_manifest(manifest)
    if failures:
        for failure in failures:
            print(f"FAIL {failure}")
        print(f"manifest={MANIFEST.relative_to(ROOT)}")
        return 1
    print(f"PASS {PASS}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
