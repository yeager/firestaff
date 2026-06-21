#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_hud_overlay_source_lock.json"

SOURCE_CHECKS = [
    {
        "file": "TIMELINE.C",
        "start": 1817,
        "end": 1830,
        "needles": [
            "F0260_TIMELINE_RefreshAllChampionStatusBoxes",
            "MASK0x1000_STATUS_BOX",
            "F0293_CHAMPION_DrawAllChampionStates",
        ],
        "meaning": "champion status-box refresh remains V1 tick/source owned",
    },
    {
        "file": "PANEL.C",
        "start": 2195,
        "end": 2239,
        "needles": [
            "F0354_INVENTORY_DrawStatusBoxPortrait",
            "C69_CHAMPION_STATUS_BOX_SPACING",
            "G2078_C32_PortraitWidth",
            "G2079_C29_PortraitHeight",
        ],
        "meaning": "champion status-box geometry and portrait blits stay source-locked",
    },
    {
        "file": "CHAMDRAW.C",
        "start": 880,
        "end": 942,
        "needles": [
            "F0287_CHAMPION_DrawBarGraphs",
            "F0290_CHAMPION_DrawHealthStaminaManaValues",
            "F0291_CHAMPION_DrawSlot",
        ],
        "meaning": "champion bar/value redraw and hand slots are V1 state, only mirrored by V2 pixels",
    },
    {
        "file": "CHAMDRAW.C",
        "start": 1080,
        "end": 1090,
        "needles": [
            "F0291_CHAMPION_DrawSlot",
            "F0386_MENUS_DrawActionIcon",
        ],
        "meaning": "action hand/icon rendering remains owned by V1 champion draw code",
    },
    {
        "file": "COMMAND.C",
        "start": 461,
        "end": 482,
        "needles": [
            "G0452_as_Graphic561_MouseInput_ActionAreaNames",
            "G0453_as_Graphic561_MouseInput_ActionAreaIcons",
            "G0454_as_Graphic561_MouseInput_SpellArea",
            "C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1",
            "C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL",
            "C109_COMMAND_CLICK_IN_SPELL_AREA_SET_MAGIC_CASTER",
        ],
        "meaning": "action/rune/cast/caster command tables are mirrored, not reimplemented",
    },
    {
        "file": "COMMAND.C",
        "start": 2302,
        "end": 2311,
        "needles": [
            "C100_COMMAND_CLICK_IN_SPELL_AREA",
            "F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE",
            "C111_COMMAND_CLICK_IN_ACTION_AREA",
            "F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE",
        ],
        "meaning": "spell/action transactions stay behind source V1 command handlers",
    },
    {
        "file": "STATS.C",
        "start": 1,
        "end": 8,
        "needles": [
            "COMPILE.H",
            "MEDIA701_I34E",
        ],
        "meaning": "local STATS.C source shard is present; visible PC34 status bar redraw anchors are checked in CHAMDRAW.C",
    },
]

FIRESTAFF_CHECKS = {
    "include/dm1_v2_hud_overlay_pc34.h": [
        "M11_V2_HudChampionOverlayPc34",
        "M11_V2_HudActionOverlayPc34",
        "M11_V2_HudRuneOverlayPc34",
        "v2_hud_set_champion_overlay_state",
        "v2_hud_set_action_overlay_state",
        "v2_hud_set_rune_overlay_state",
        "v2_hud_tick_presentation_state",
    ],
    "src/dm1v2/dm1_v2_hud_overlay_pc34.c": [
        "presentation-only",
        "champion/action/rune presentation state",
        "finished V2 art",
        "COMMAND.C:375-395",
        "COMMAND.C:461-482",
        "PANEL.C:F0354",
        "CHAMDRAW.C",
        "v2_hud_render_presentation_state",
    ],
    "tests/test_dm1_v2_hud_overlay_pc34.c": [
        "active leader border",
        "spell-ready cue",
        "active rune",
        "action flash",
        "flash decays",
        "clear hides action strip",
    ],
}

FORBIDDEN_OVERLAY_CALLS = [
    "F0291_CHAMPION_DrawSlot(",
    "F0293_CHAMPION_DrawAllChampionStates(",
    "F0354_INVENTORY_DrawStatusBoxPortrait(",
    "F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE(",
    "F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE(",
    "F0386_MENUS_DrawActionIcon(",
]


def main() -> int:
    errors: list[str] = []
    anchors: list[dict[str, object]] = []

    for check in SOURCE_CHECKS:
        source_path = SOURCE / str(check["file"])
        if not source_path.exists():
            errors.append(f"missing ReDMCSB source {source_path}")
            continue
        lines = source_path.read_text(encoding="utf-8", errors="replace").splitlines()
        start = int(check["start"])
        end = int(check["end"])
        if start < 1 or end > len(lines) or start > end:
            errors.append(f"invalid line range {check['file']}:{start}-{end}")
            continue
        excerpt = "\n".join(lines[start - 1:end])
        for needle in check["needles"]:
            if str(needle) not in excerpt:
                errors.append(f"{check['file']}:{start}-{end}: missing {needle!r}")
        anchors.append({
            "file": check["file"],
            "lineRange": f"{start}-{end}",
            "meaning": check["meaning"],
            "needles": check["needles"],
        })

    for rel, needles in FIRESTAFF_CHECKS.items():
        path = ROOT / rel
        if not path.exists():
            errors.append(f"missing Firestaff file {rel}")
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for needle in needles:
            if needle not in text:
                errors.append(f"{rel}: missing {needle!r}")

    overlay_text = (ROOT / "src/dm1v2/dm1_v2_hud_overlay_pc34.c").read_text(
        encoding="utf-8", errors="replace")
    for forbidden in FORBIDDEN_OVERLAY_CALLS:
        if forbidden in overlay_text:
            errors.append(f"V2 HUD overlay must not call source transaction/draw owner directly: {forbidden}")

    result = {
        "status": "failed" if errors else "passed",
        "scope": "dm1_v2_hud_overlay_pc34 presentation-state pixel/source-lock",
        "redmcsbSourceRoot": str(SOURCE),
        "honesty": (
            "Adds data-free champion/action/rune presentation-state pixels only; "
            "does not claim finished V2 art, original screenshot parity, real-asset "
            "parity, or complete V2 UI parity."
        ),
        "anchors": anchors,
        "firestaffFiles": sorted(FIRESTAFF_CHECKS),
        "forbiddenOverlayCalls": FORBIDDEN_OVERLAY_CALLS,
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        print("dm1_v2_hud_overlay_source_lock=FAIL")
        for err in errors:
            print(err)
        return 1

    print(f"dm1_v2_hud_overlay_source_lock=OK evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
