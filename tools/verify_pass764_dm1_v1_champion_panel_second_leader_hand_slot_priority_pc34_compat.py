#!/usr/bin/env python3
"""Verify pass764 DM1 V1 second-leader champion-panel source lock."""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
TEST = resolve_build_dir(ROOT, ROOT / "build") / "test_dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat"


def read(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def check_span(path: Path, first: int, last: int, needles: list[str]) -> list[str]:
    lines = read(path).splitlines()
    body = "\n".join(lines[first - 1:last])
    missing: list[str] = []
    cursor = 0
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(f"{path.name}:{first}-{last} missing {needle!r}")
        else:
            cursor = pos + len(needle)
    return missing


def check_file(rel: str, needles: list[str]) -> list[str]:
    body = read(ROOT / rel)
    return [f"{rel} missing {needle!r}" for needle in needles if needle not in body]


def run_test() -> tuple[bool, str]:
    if not TEST.exists():
        return False, f"missing test binary: {TEST}"
    proc = subprocess.run(
        [str(TEST)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=180,
    )
    return proc.returncode == 0, "\n".join(proc.stdout.strip().splitlines()[-12:])


def main() -> int:
    failures: list[str] = []
    failures += check_span(
        RED / "CHAMPION.C",
        677,
        684,
        [
            "if (P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT)",
            "L0903_ui_ChampionIndex = P0633_ui_SlotBoxIndex >> 1;",
            "M070_HAND_SLOT_INDEX(P0633_ui_SlotBoxIndex)",
        ],
    )
    failures += check_span(
        RED / "CHAMPION.C",
        688,
        712,
        [
            "L0905_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing;",
            "F0298_CHAMPION_GetObjectRemovedFromLeaderHand();",
            "F0300_CHAMPION_GetObjectRemovedFromSlot",
            "F0297_CHAMPION_PutObjectInLeaderHand",
            "F0301_CHAMPION_AddObjectInSlot",
            "F0292_CHAMPION_DrawState(L0903_ui_ChampionIndex);",
        ],
    )
    failures += check_span(
        RED / "CHAMDRAW.C",
        307,
        342,
        [
            "L2252_i_BoxIndex = P0605_i_ChampionIndex + C195_ZONE_FIRST_BAR_GRAPH;",
            "F0732_FillScreenArea(L2004_ai_XYZBlankBar, C12_COLOR_DARKEST_GRAY);",
            "F0732_FillScreenArea(L2005_ai_XYZColoredBar, G0046_auc_Graphic562_ChampionColor[P0605_i_ChampionIndex]);",
        ],
    )
    failures += check_span(
        RED / "CHAMDRAW.C",
        771,
        815,
        [
            "P0615_ui_ChampionIndex + C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS",
            "F0732_FillScreenArea(L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY);",
            "MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x2000_WOUNDS | MASK0x8000_ACTION_HAND",
        ],
    )
    failures += check_span(
        RED / "CHAMDRAW.C",
        843,
        895,
        [
            "C11_COLOR_YELLOW",
            "C09_COLOR_GOLD",
            "P0615_ui_ChampionIndex + C159_ZONE_CHAMPION_0_STATUS_BOX_NAME",
            "P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME",
        ],
    )
    failures += check_span(
        RED / "CHAMDRAW.C",
        1019,
        1051,
        [
            "M026_CHAMPION_ICON_INDEX",
            "G2080_C19_ChampionIconWidth",
            "G2081_C14_ChampionIconHeight",
            "C113_ZONE_CHAMPION_ICON_TOP_LEFT",
        ],
    )
    failures += check_span(
        RED / "DEFS.H",
        3779,
        3807,
        [
            "C114_ZONE_CHAMPION_ICON_TOP_RIGHT",
            "C152_ZONE_CHAMPION_1_STATUS_BOX_NAME_HANDS",
            "C160_ZONE_CHAMPION_1_STATUS_BOX_NAME",
            "C214_ZONE_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND",
        ],
    )
    failures += check_file(
        "tests/test_dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.c",
        [
            "CHAMPION.C F0302:677-684",
            "CHAMDRAW.C F0292:850-851 PC34 leader C11",
            "DeterministicHash:",
        ],
    )
    failures += check_file(
        "parity-evidence/pass764_dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.md",
        [
            "Status: DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_SOURCE_LOCKED",
            "No original DOS pixel-parity claim.",
        ],
    )

    ok, output = run_test()
    if not ok:
        failures.append(output)

    if failures:
        print("FAILED_DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_SOURCE_LOCK")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_SOURCE_LOCKED")
    print(output)
    return 0


if __name__ == "__main__":
    sys.exit(main())
