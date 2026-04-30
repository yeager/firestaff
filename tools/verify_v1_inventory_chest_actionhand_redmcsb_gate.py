#!/usr/bin/env python3
"""Source-lock DM1/V1 open-chest action-hand icon refresh semantics to ReDMCSB."""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import sys
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB_ROOT = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_REDMCSB_ROOT))
FIRESTAFF_SRC = ROOT / "m11_game_view.c"
EVIDENCE_JSON = ROOT / "parity-evidence/verification/v1_inventory_chest_actionhand_redmcsb_gate.json"

SOURCE_RANGES = [
    {"file": "CHAMDRAW.C", "start": 498, "end": 631},
    {"file": "CHEST.C", "start": 2, "end": 46},
    {"file": "CHAMPION.C", "start": 587, "end": 640},
    {"file": "m11_game_view.c", "start": 18458, "end": 18469},
    {"file": "m11_game_view.c", "start": 18627, "end": 18645},
]


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing source file: {path}")
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def source_path(rel: str) -> Path:
    name = Path(rel).name
    if name == name.upper() and name.upper().endswith((".C", ".H")):
        return REDMCSB_ROOT / rel
    return ROOT / rel


def find_source_window(text: str, name: str, next_name: str | None = None) -> tuple[int, int, str]:
    prefix = r"(?m)^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|THING|unsigned\s+int16_t|static\s+void)\s+"
    pattern = re.compile(prefix + re.escape(name) + r"\s*\(")
    match = pattern.search(text)
    if not match:
        raise AssertionError(f"missing function declaration for {name}")
    if next_name:
        next_pattern = re.compile(prefix + re.escape(next_name) + r"\s*\(")
        next_match = next_pattern.search(text, match.end())
        if not next_match:
            raise AssertionError(f"missing following function declaration for {next_name}")
        return match.start(), next_match.start(), text[match.start() : next_match.start()]
    return match.start(), min(len(text), match.start() + 12000), text[match.start() : min(len(text), match.start() + 12000)]


def require_in_order(body: str, markers: Iterable[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    found: list[tuple[str, int]] = []
    cursor = -1
    for name, needle in markers:
        pos = body.find(needle, cursor + 1)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        found.append((name, pos))
        cursor = pos
    return found


def require_excerpt(rel: str, start: int, end: int, needles: list[str]) -> None:
    path = source_path(rel)
    lines = read_text(path).splitlines()
    if len(lines) < end:
        raise AssertionError(f"{path} has only {len(lines)} lines, need {end}")
    excerpt = "\n".join(lines[start - 1 : end])
    missing = [needle for needle in needles if needle not in excerpt]
    if missing:
        raise AssertionError(f"{rel}:{start}-{end} missing {missing}")
    print(f"sourceRange={path}:{start}-{end} status=ok")


def verify_redmcsb() -> list[str]:
    cham_path = REDMCSB_ROOT / "CHAMDRAW.C"
    cham_text = read_text(cham_path)
    cham_start, _cham_end, cham_body = find_source_window(cham_text, "F0291_CHAMPION_DrawSlot", "F0292_CHAMPION_DrawState")
    cham_markers = require_in_order(
        cham_body,
        [
            ("inventory champion predicate", "L0852_B_IsInventoryChampion"),
            ("thing source", "L0851_T_Thing = L0854_ps_Champion->Slots[P0614_ui_SlotIndex]"),
            ("object icon lookup", "L0853_i_IconIndex = F0033_OBJECT_GetIconIndex(L0851_T_Thing)"),
            ("BUG0_35 source note", "BUG0_35 The closed chest icon is drawn when the chest is opened"),
            ("action-hand inventory gate", "L0852_B_IsInventoryChampion && (P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND)"),
            ("closed chest icon predicate", "L0853_i_IconIndex == C144_ICON_CONTAINER_CHEST_CLOSED"),
            ("open icon increment", "L0853_i_IconIndex++"),
        ],
        "ReDMCSB F0291 BUG0_35 action-hand chest icon refresh",
    )

    chest_path = REDMCSB_ROOT / "CHEST.C"
    chest_text = read_text(chest_path)
    chest_start, _chest_end, chest_body = find_source_window(chest_text, "F0333_INVENTORY_OpenAndDrawChest", "F0334_INVENTORY_CloseChest")
    chest_markers = require_in_order(
        chest_body,
        [
            ("open chest state", "G0426_T_OpenChest = P0692_T_Thing"),
            ("not eye press gate", "if (!P0694_B_PressingEye)"),
            ("action hand open icon draw", "F0038_OBJECT_DrawIconInSlotBox(C09_SLOT_BOX_INVENTORY_ACTION_HAND, C145_ICON_CONTAINER_CHEST_OPEN)"),
        ],
        "ReDMCSB F0333 open chest action-hand icon draw",
    )

    champ_path = REDMCSB_ROOT / "CHAMPION.C"
    champ_text = read_text(champ_path)
    add_start, _add_end, add_body = find_source_window(champ_text, "F0301_CHAMPION_AddObjectInSlot", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox")
    add_markers = require_in_order(
        add_body,
        [
            ("slot write", "L0900_ps_Champion->Slots[P0632_ui_SlotIndex] = P0631_T_Thing"),
            ("icon lookup", "L0899_i_IconIndex = F0033_OBJECT_GetIconIndex(P0631_T_Thing)"),
            ("inventory champion predicate", "L0902_B_IsInventoryChampion"),
            ("action hand predicate", "P0632_ui_SlotIndex == C01_SLOT_ACTION_HAND"),
            ("closed chest predicate", "L0899_i_IconIndex == C144_ICON_CONTAINER_CHEST_CLOSED"),
            ("panel refresh flag", "M008_SET(L0900_ps_Champion->Attributes, MASK0x0800_PANEL)"),
        ],
        "ReDMCSB F0301 action-hand chest panel refresh flag",
    )

    require_excerpt("CHAMDRAW.C", 498, 631, ["F0291_CHAMPION_DrawSlot", "BUG0_35", "C144_ICON_CONTAINER_CHEST_CLOSED", "L0853_i_IconIndex++"])
    require_excerpt("CHEST.C", 2, 46, ["F0333_INVENTORY_OpenAndDrawChest", "G0426_T_OpenChest", "C145_ICON_CONTAINER_CHEST_OPEN"])
    require_excerpt("CHAMPION.C", 587, 640, ["F0301_CHAMPION_AddObjectInSlot", "C144_ICON_CONTAINER_CHEST_CLOSED", "MASK0x0800_PANEL"])

    return [
        f"ReDMCSB CHAMDRAW.C F0291 starts at {cham_path}:{line_no(cham_text, cham_start)}",
        *(f"ReDMCSB F0291 {name}: line {line_no(cham_text, cham_start + pos)}" for name, pos in cham_markers),
        f"ReDMCSB CHEST.C F0333 starts at {chest_path}:{line_no(chest_text, chest_start)}",
        *(f"ReDMCSB F0333 {name}: line {line_no(chest_text, chest_start + pos)}" for name, pos in chest_markers),
        f"ReDMCSB CHAMPION.C F0301 starts at {champ_path}:{line_no(champ_text, add_start)}",
        *(f"ReDMCSB F0301 {name}: line {line_no(champ_text, add_start + pos)}" for name, pos in add_markers),
    ]


def verify_firestaff_seam() -> list[str]:
    text = read_text(FIRESTAFF_SRC)
    slot_start, _slot_end, slot_body = find_source_window(text, "m11_draw_inv_slot")
    slot_markers = require_in_order(
        slot_body,
        [
            ("occupied branch", "if (thingId != THING_NONE && thingId != THING_ENDOFLIST)"),
            ("dm icon index", "m11_object_icon_index_for_thing"),
            ("dm object icon draw", "m11_draw_dm_object_icon_index"),
        ],
        "Firestaff occupied inventory slot seam",
    )
    panel_start, _panel_end, panel_body = find_source_window(text, "m11_draw_inventory_panel")
    panel_markers = require_in_order(
        panel_body,
        [
            ("champion slot loop", "for (slotIdx = 0; slotIdx < CHAMPION_SLOT_COUNT; ++slotIdx)"),
            ("source slotbox map", "M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot"),
            ("thing nonempty gate", "thingId != THING_NONE && thingId != THING_ENDOFLIST"),
            ("dm icon index", "m11_object_icon_index_for_thing"),
            ("source slotbox zone", "M11_GameView_GetV1InventorySourceSlotBoxZone"),
            ("draw object icon", "m11_draw_dm_object_icon_index"),
        ],
        "Firestaff normal V1 inventory source-slot seam",
    )
    require_excerpt("m11_game_view.c", 18458, 18469, ["m11_object_icon_index_for_thing", "m11_draw_dm_object_icon_index"])
    require_excerpt("m11_game_view.c", 18627, 18645, ["M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot", "m11_draw_dm_object_icon_index", "return;"])
    return [
        f"Firestaff m11_draw_inv_slot starts at {FIRESTAFF_SRC}:{line_no(text, slot_start)}",
        *(f"Firestaff slot seam {name}: line {line_no(text, slot_start + pos)}" for name, pos in slot_markers),
        f"Firestaff m11_draw_inventory_panel starts at {FIRESTAFF_SRC}:{line_no(text, panel_start)}",
        *(f"Firestaff panel seam {name}: line {line_no(text, panel_start + pos)}" for name, pos in panel_markers),
    ]


def verify_evidence_json() -> None:
    data = json.loads(EVIDENCE_JSON.read_text(encoding="utf-8"))
    if data.get("gate") != "v1_inventory_chest_actionhand_redmcsb":
        raise AssertionError("evidence JSON gate id mismatch")
    ranges = {(entry.get("file"), entry.get("start"), entry.get("end")) for entry in data.get("sourceRanges", [])}
    expected = {(entry["file"], entry["start"], entry["end"]) for entry in SOURCE_RANGES}
    missing = expected - ranges
    if missing:
        raise AssertionError(f"evidence JSON missing source ranges: {sorted(missing)}")
    non_claims = data.get("nonClaims", [])
    if not any("runtime" in claim.lower() for claim in non_claims):
        raise AssertionError("evidence JSON must preserve runtime non-claim")
    print(f"evidence={EVIDENCE_JSON.relative_to(ROOT)} status=ok")


def main() -> int:
    print("probe=v1_inventory_chest_actionhand_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    lines = verify_redmcsb() + verify_firestaff_seam()
    verify_evidence_json()
    print("v1InventoryChestActionHandRedmcsbGateOk=1")
    for line in lines:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
