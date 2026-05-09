#!/usr/bin/env python3
"""Source-lock the DM1/V1 inventory panel open/draw gate to ReDMCSB.

This is a narrow V1 gate, distinct from the V2 inventory-art acceptance gate.
It verifies the original F0347 panel-content decision order and F0291 slot-box
namespace, then checks Firestaff's normal V1 inventory path uses the source
slot-box draw pass rather than the older debug/freehand inventory layout.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import sys
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_REDMCSB_ROOT))
FIRESTAFF_SRC = ROOT / "m11_game_view.c"
EVIDENCE_JSON = ROOT / "parity-evidence/verification/v1_inventory_panel_open_redmcsb_gate.json"

SOURCE_RANGES = [
    {
        "file": "PANEL.C",
        "start": 1639,
        "end": 1692,
        "function": "F0347_INVENTORY_DrawPanel",
        "assertion": "Panel redraw closes any open chest, reads the inventory champion action hand, classifies container as chest panel, scroll as scroll panel, otherwise falls back to food/water, then draws either food/water or object panel.",
    },
    {
        "file": "COMMAND.C",
        "start": 2180,
        "end": 2183,
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "assertion": "Inventory toggle/close commands C007..C011 dispatch to F0355_INVENTORY_Toggle_CPSE when no candidate champion blocks input.",
    },
    {
        "file": "PANEL.C",
        "start": 2244,
        "end": 2447,
        "function": "F0355_INVENTORY_Toggle_CPSE",
        "assertion": "Opening inventory sets G0423_i_InventoryChampionOrdinal, expands C017 inventory backdrop, draws all champion slots with F0291, marks redraw attributes, then switches secondary input to ChampionInventory.",
    },
    {
        "file": "CHAMDRAW.C",
        "start": 536,
        "end": 673,
        "function": "F0291_CHAMPION_DrawSlot",
        "assertion": "Inventory champion slots map to C08_SLOT_BOX_INVENTORY_FIRST_SLOT + slot, render to G0296_puc_Bitmap_Viewport, choose empty/object icon, and finish through F0038_OBJECT_DrawIconInSlotBox.",
    },
    {
        "file": "m11_game_view.c",
        "start": 19359,
        "end": 19459,
        "function": "m11_draw_inv_slot",
        "assertion": "Firestaff occupied slots draw 16x16 DM object icons inside original 18x18 slot boxes when assets are available.",
    },
    {
        "file": "m11_game_view.c",
        "start": 19460,
        "end": 19602,
        "function": "m11_draw_inventory_panel",
        "assertion": "Firestaff normal V1 inventory path draws source slot boxes 8..37, overlays champion objects by source slot-box zone, and returns before the debug/freehand layout.",
    },
]


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing source file: {path}")
    return path.read_text(encoding="latin-1" if path.suffix.upper() == ".C" else "utf-8", errors="replace")


def find_function(text: str, name: str, *, allow_kr_semicolons: bool = False) -> tuple[int, int, str]:
    pattern = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0:
            continue
        semi = text.find(";", match.end(), brace)
        if semi >= 0 and not allow_kr_semicolons:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            ch = text[pos]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return match.start(), pos + 1, text[match.start() : pos + 1]
    raise AssertionError(f"missing function body for {name}")


def find_c_function(text: str, name: str, *, allow_kr_semicolons: bool = False) -> tuple[int, int, str]:
    pattern = re.compile(r"(?m)^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|THING|unsigned\s+int16_t|static\s+void)\s+" + re.escape(name) + r"\s*\(")
    match = pattern.search(text)
    if not match:
        raise AssertionError(f"missing function declaration for {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing function body for {name}")
    semi = text.find(";", match.end(), brace)
    if semi >= 0 and not allow_kr_semicolons:
        raise AssertionError(f"matched prototype, not body, for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return match.start(), pos + 1, text[match.start() : pos + 1]
    raise AssertionError(f"unterminated function body for {name}")


def find_source_window(text: str, name: str, next_name: str | None = None) -> tuple[int, int, str]:
    pattern = re.compile(r"(?m)^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|THING|unsigned\s+int16_t)\s+" + re.escape(name) + r"\s*\(")
    match = pattern.search(text)
    if not match:
        raise AssertionError(f"missing function declaration for {name}")
    if next_name:
        next_pattern = re.compile(r"(?m)^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|THING|unsigned\s+int16_t)\s+" + re.escape(next_name) + r"\s*\(")
        next_match = next_pattern.search(text, match.end())
        if not next_match:
            raise AssertionError(f"missing following function declaration for {next_name}")
        return match.start(), next_match.start(), text[match.start() : next_match.start()]
    return match.start(), min(len(text), match.start() + 12000), text[match.start() : min(len(text), match.start() + 12000)]


def require_in_order(body: str, markers: Iterable[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    found: list[tuple[str, int]] = []
    for name, needle in markers:
        pos = body.find(needle)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        found.append((name, pos))
    for (prev_name, prev_pos), (next_name, next_pos) in zip(found, found[1:]):
        if prev_pos >= next_pos:
            raise AssertionError(f"{label}: {prev_name} appears after {next_name}")
    return found


def require_excerpt(rel: str, start: int, end: int, needles: list[str]) -> None:
    path = (REDMCSB_ROOT / rel) if rel.endswith(".C") else (ROOT / rel)
    lines = read_text(path).splitlines()
    if len(lines) < end:
        raise AssertionError(f"{path} has only {len(lines)} lines, need {end}")
    excerpt = "\n".join(lines[start - 1 : end])
    missing = [needle for needle in needles if needle not in excerpt]
    if missing:
        raise AssertionError(f"{rel}:{start}-{end} missing {missing}")
    print(f"sourceRange={path}:{start}-{end} status=ok")


def verify_redmcsb() -> list[str]:
    panel_path = REDMCSB_ROOT / "PANEL.C"
    panel_text = read_text(panel_path)
    panel_start, _panel_end, panel_body = find_source_window(panel_text, "F0347_INVENTORY_DrawPanel", "F0349_INVENTORY_ProcessCommand70_ClickOnMouth")
    panel_markers = require_in_order(
        panel_body,
        [
            ("close chest", "F0334_INVENTORY_CloseChest"),
            ("candidate resurrect panel", "G0299_ui_CandidateChampionOrdinal"),
            ("action-hand source", "Slots[C01_SLOT_ACTION_HAND]"),
            ("type switch", "switch (M012_TYPE(L1075_T_Thing))"),
            ("container chest panel", "G0424_i_PanelContent = M569_PANEL_CHEST"),
            ("scroll panel", "G0424_i_PanelContent = M643_PANEL_SCROLL"),
            ("default food/water", "L1075_T_Thing = C0xFFFF_THING_NONE"),
            ("draw food/water", "F0345_INVENTORY_DrawPanel_FoodWaterPoisoned"),
            ("draw object panel", "F0342_INVENTORY_DrawPanel_Object"),
        ],
        "ReDMCSB F0347 panel decision order",
    )

    command_path = REDMCSB_ROOT / "COMMAND.C"
    command_text = read_text(command_path)
    command_start = command_text.find("if ((L1160_i_Command >= C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0)")
    if command_start < 0:
        raise AssertionError("missing inventory toggle dispatch block in COMMAND.C")
    command_body = command_text[command_start : command_text.find("goto T0380042;", command_start)]
    command_markers = require_in_order(
        command_body,
        [
            ("toggle command range", "L1160_i_Command >= C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0"),
            ("close inventory command", "L1160_i_Command <= C011_COMMAND_CLOSE_INVENTORY"),
            ("candidate champion guard", "!G0299_ui_CandidateChampionOrdinal"),
            ("toggle dispatch", "F0355_INVENTORY_Toggle_CPSE(AL1159_i_ChampionIndex)"),
        ],
        "ReDMCSB F0380 inventory command dispatch",
    )

    toggle_start, _toggle_end, toggle_body = find_source_window(panel_text, "F0355_INVENTORY_Toggle_CPSE")
    toggle_markers = require_in_order(
        toggle_body,
        [
            ("existing inventory ordinal", "AL1102_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal"),
            ("set inventory ordinal", "G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(P0719_i_ChampionIndex)"),
            ("inventory backdrop", "F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY"),
            ("draw source slots", "F0291_CHAMPION_DrawSlot(P0719_i_ChampionIndex, AL1102_ui_SlotIndex)"),
            ("mark redraw attributes", "MASK0x4000_VIEWPORT | MASK0x1000_STATUS_BOX | MASK0x0800_PANEL"),
            ("champion inventory input", "G0442_ps_SecondaryMouseInput = G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory"),
        ],
        "ReDMCSB F0355 inventory open/draw sequence",
    )

    cham_path = REDMCSB_ROOT / "CHAMDRAW.C"
    cham_text = read_text(cham_path)
    cham_start, _cham_end, cham_body = find_source_window(cham_text, "F0291_CHAMPION_DrawSlot", "F0292_CHAMPION_DrawState")
    slot_markers = require_in_order(
        cham_body,
        [
            ("inventory champion predicate", "L0852_B_IsInventoryChampion"),
            ("inventory slotbox namespace", "C08_SLOT_BOX_INVENTORY_FIRST_SLOT + P0614_ui_SlotIndex"),
            ("chest slot source", "G0425_aT_ChestSlots[P0614_ui_SlotIndex - C30_SLOT_CHEST_1]"),
            ("viewport destination", "L0860_puc_Bitmap = G0296_puc_Bitmap_Viewport"),
            ("empty slot branch", "if (L0851_T_Thing == C0xFFFF_THING_NONE)"),
            ("object icon index", "F0033_OBJECT_GetIconIndex(L0851_T_Thing)"),
            ("draw icon in slot box", "F0038_OBJECT_DrawIconInSlotBox(L0856_ui_SlotBoxIndex, L0853_i_IconIndex)"),
        ],
        "ReDMCSB F0291 inventory slot-box draw",
    )

    require_excerpt("PANEL.C", 1639, 1692, ["F0347_INVENTORY_DrawPanel", "M569_PANEL_CHEST", "M643_PANEL_SCROLL"])
    require_excerpt("COMMAND.C", 2180, 2183, ["C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0", "C011_COMMAND_CLOSE_INVENTORY", "F0355_INVENTORY_Toggle_CPSE"])
    require_excerpt("PANEL.C", 2244, 2447, ["F0355_INVENTORY_Toggle_CPSE", "C017_GRAPHIC_INVENTORY", "F0291_CHAMPION_DrawSlot", "G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory"])
    require_excerpt("CHAMDRAW.C", 536, 673, ["C08_SLOT_BOX_INVENTORY_FIRST_SLOT", "G0296_puc_Bitmap_Viewport", "F0038_OBJECT_DrawIconInSlotBox"])

    return [
        f"ReDMCSB PANEL.C F0347 starts at {panel_path}:{line_no(panel_text, panel_start)}",
        *(f"ReDMCSB F0347 {name}: line {line_no(panel_text, panel_start + pos)}" for name, pos in panel_markers),
        f"ReDMCSB COMMAND.C F0380 inventory dispatch block starts at {command_path}:{line_no(command_text, command_start)}",
        *(f"ReDMCSB F0380 {name}: line {line_no(command_text, command_start + pos)}" for name, pos in command_markers),
        f"ReDMCSB PANEL.C F0355 starts at {panel_path}:{line_no(panel_text, toggle_start)}",
        *(f"ReDMCSB F0355 {name}: line {line_no(panel_text, toggle_start + pos)}" for name, pos in toggle_markers),
        f"ReDMCSB CHAMDRAW.C F0291 starts at {cham_path}:{line_no(cham_text, cham_start)}",
        *(f"ReDMCSB F0291 {name}: line {line_no(cham_text, cham_start + pos)}" for name, pos in slot_markers),
    ]


def verify_firestaff() -> list[str]:
    text = read_text(FIRESTAFF_SRC)
    slot_start, _slot_end, slot_body = find_function(text, "m11_draw_inv_slot")
    slot_markers = require_in_order(
        slot_body,
        [
            ("original 18x18 slot asset", "static const int SZ_ORIG = 18"),
            ("normal slot graphic", "M11_GameView_GetV1SlotBoxNormalGraphicId"),
            ("occupied branch", "if (thingId != THING_NONE && thingId != THING_ENDOFLIST)"),
            ("dm icon index", "m11_object_icon_index_for_thing"),
            ("dm object icon draw", "m11_draw_dm_object_icon_index"),
        ],
        "Firestaff slot draw helper",
    )

    panel_start, _panel_end, panel_body = find_function(text, "m11_draw_inventory_panel")
    normal_idx = panel_body.find("} else {\n        /* Normal V1 dynamic overlay phase")
    if normal_idx < 0:
        raise AssertionError("Firestaff inventory panel missing normal V1 dynamic overlay branch")
    debug_identity_idx = panel_body.find("Champion identity")
    normal_return_idx = panel_body.find("return;", normal_idx)
    if normal_return_idx < 0 or debug_identity_idx < 0 or normal_return_idx > debug_identity_idx:
        raise AssertionError("Firestaff normal V1 source-slot branch must return before debug/freehand layout")
    normal_body = panel_body[normal_idx:normal_return_idx]
    normal_markers = require_in_order(
        normal_body,
        [
            ("source slotbox loop", "for (sourceSlotBox = 8; sourceSlotBox <= 37; ++sourceSlotBox)"),
            ("source slotbox graphic", "M11_GameView_GetV1InventorySourceSlotBoxGraphicId"),
            ("source slotbox zone", "M11_GameView_GetV1InventorySourceSlotBoxZone"),
            ("champion slot loop", "for (slotIdx = 0; slotIdx < CHAMPION_SLOT_COUNT; ++slotIdx)"),
            ("champion slot to source slotbox", "M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot"),
            ("dm icon index", "m11_object_icon_index_for_thing"),
            ("draw into source slot zone", "m11_draw_dm_object_icon_index"),
        ],
        "Firestaff normal V1 inventory source-slot branch",
    )
    require_excerpt("m11_game_view.c", 19359, 19459, ["m11_draw_inv_slot", "m11_draw_dm_object_icon_index"])
    require_excerpt("m11_game_view.c", 19460, 19602, ["for (sourceSlotBox = 8; sourceSlotBox <= 37; ++sourceSlotBox)", "return;"])

    return [
        f"Firestaff m11_draw_inv_slot starts at {FIRESTAFF_SRC}:{line_no(text, slot_start)}",
        *(f"Firestaff slot {name}: line {line_no(text, slot_start + pos)}" for name, pos in slot_markers),
        f"Firestaff m11_draw_inventory_panel starts at {FIRESTAFF_SRC}:{line_no(text, panel_start)}",
        *(f"Firestaff normal branch {name}: line {line_no(text, panel_start + normal_idx + pos)}" for name, pos in normal_markers),
        f"Firestaff normal branch return before debug/freehand layout: line {line_no(text, panel_start + normal_return_idx)} < line {line_no(text, panel_start + debug_identity_idx)}",
    ]


def verify_evidence_json() -> None:
    data = json.loads(EVIDENCE_JSON.read_text(encoding="utf-8"))
    if data.get("gate") != "v1_inventory_panel_open_redmcsb":
        raise AssertionError("evidence JSON gate id mismatch")
    ranges = {(entry.get("file"), entry.get("start"), entry.get("end")) for entry in data.get("sourceRanges", [])}
    expected = {(entry["file"], entry["start"], entry["end"]) for entry in SOURCE_RANGES}
    missing = expected - ranges
    if missing:
        raise AssertionError(f"evidence JSON missing source ranges: {sorted(missing)}")
    print(f"evidence={EVIDENCE_JSON.relative_to(ROOT)} status=ok")


def main() -> int:
    print("probe=v1_inventory_panel_open_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    lines = verify_redmcsb() + verify_firestaff()
    verify_evidence_json()
    print("v1InventoryPanelOpenRedmcsbGateOk=1")
    for line in lines:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
