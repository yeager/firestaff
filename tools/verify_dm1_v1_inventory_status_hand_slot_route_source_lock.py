#!/usr/bin/env python3
"""Verify the DM1 V1 status-box hand slot route is source-locked.

This is a narrow Equip/body-slot evidence gate for the F0302 path that
clicks champion status-box hand slots (slot boxes 0..7). It does not claim
full backpack/chest storage expansion or object-description layout parity.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CHAMPION = RED / "CHAMPION.C"
CLIKCHAM = RED / "CLIKCHAM.C"
DEFS = RED / "DEFS.H"
SRC = ROOT / "src/dm1/dm1_v1_inventory_pc34_compat.c"
HDR = ROOT / "include/dm1_v1_inventory_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_inventory_equip_slots_pc34_compat.c"


def lines(path: Path, start: int, end: int) -> str:
    return "\n".join(path.read_text(encoding="latin-1").splitlines()[start - 1:end])


def compact(s: str) -> str:
    return " ".join(s.split())


def require(cite: str, hay: str, needles: list[str]) -> None:
    h = compact(hay)
    for needle in needles:
        if compact(needle) not in h:
            raise AssertionError(f"{cite}: missing {needle!r}")


def main() -> int:
    citations: list[str] = []

    require("CHAMPION.C:677-687", lines(CHAMPION, 677, 687), [
        "if (P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT)",
        "if (G0299_ui_CandidateChampionOrdinal)",
        "L0903_ui_ChampionIndex = P0633_ui_SlotBoxIndex >> 1",
        "L0903_ui_ChampionIndex >= G0305_ui_PartyChampionCount",
        "M000_INDEX_TO_ORDINAL(L0903_ui_ChampionIndex) == (int16_t)G0423_i_InventoryChampionOrdinal",
        "!M516_CHAMPIONS[L0903_ui_ChampionIndex].CurrentHealth",
        "L0904_ui_SlotIndex = M070_HAND_SLOT_INDEX(P0633_ui_SlotBoxIndex)",
        "L0903_ui_ChampionIndex = M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)",
        "L0904_ui_SlotIndex = P0633_ui_SlotBoxIndex - C08_SLOT_BOX_INVENTORY_FIRST_SLOT",
    ])
    citations.append("CHAMPION.C:677-687 F0302 splits status hand slots from inventory slots and applies candidate/open/dead gates")

    require("CHAMPION.C:688-710", lines(CHAMPION, 688, 710), [
        "L0905_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing",
        "if ((L0906_T_SlotThing == C0xFFFF_THING_NONE) && (L0905_T_LeaderHandObject == C0xFFFF_THING_NONE))",
        "AllowedSlots & G0038_ai_Graphic562_SlotMasks[L0904_ui_SlotIndex]",
        "F0298_CHAMPION_GetObjectRemovedFromLeaderHand()",
        "F0300_CHAMPION_GetObjectRemovedFromSlot(L0903_ui_ChampionIndex, L0904_ui_SlotIndex)",
        "F0297_CHAMPION_PutObjectInLeaderHand(L0906_T_SlotThing, C0_FALSE)",
        "F0301_CHAMPION_AddObjectInSlot(L0903_ui_ChampionIndex, L0905_T_LeaderHandObject, L0904_ui_SlotIndex)",
    ])
    citations.append("CHAMPION.C:688-710 F0302 uses the shared leader-hand/slot swap transaction after route resolution")

    require("CLIKCHAM.C:31-32", lines(CLIKCHAM, 31, 32), [
        "C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND",
        "C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND",
        "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        "L1126_ui_Command - C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND",
    ])
    citations.append("CLIKCHAM.C:31-32 dispatches status-box hand commands to F0302 with a zero-based slot-box index")

    require("DEFS.H:1874-1878", lines(DEFS, 1874, 1878), [
        "#define C08_SLOT_BOX_INVENTORY_FIRST_SLOT   8",
        "#define M070_HAND_SLOT_INDEX(slotboxindex) ((slotboxindex) & 0x0001)",
    ])
    citations.append("DEFS.H:1874-1878 defines the status/inventory split and hand-slot macro")

    source = SRC.read_text(encoding="utf-8")
    header = HDR.read_text(encoding="utf-8")
    test = TEST.read_text(encoding="utf-8")
    require("Firestaff status hand resolver", source, [
        "int m11_inventory_resolve_status_hand_slot_box",
        "slotBoxIndex < 0 || slotBoxIndex >= 8",
        "candidateChampionOrdinal != 0",
        "const int championIndex = slotBoxIndex >> 1",
        "championIndex >= partyChampionCount",
        "inventoryChampionOrdinal == championIndex + 1",
        "championCurrentHealth[championIndex] <= 0",
        "DM1_PC34_SLOT_ACTION_HAND : DM1_PC34_SLOT_READY_HAND",
        "CHAMPION.C:677-687 F0302 status hand slot routing gates",
        "CLIKCHAM.C:31-32 status box hand click dispatch",
    ])
    require("Firestaff header", header, [
        "int m11_inventory_resolve_status_hand_slot_box",
    ])
    require("Firestaff equip-slot test", test, [
        "status slotbox 0 routes champion 0 ready hand",
        "status slotbox 3 routes champion 1 action hand",
        "slotbox rejects champion outside party count",
        "slotbox rejects currently open inventory champion",
        "slotbox rejects candidate champion flow",
        "slotbox rejects dead champion",
    ])
    citations.append("Firestaff resolver/test lock the F0302 status hand route without expanding backpack/chest storage")

    print("DM1_V1_INVENTORY_STATUS_HAND_SLOT_ROUTE_SOURCE_LOCK_VERIFIED")
    for citation in citations:
        print(f"- {citation}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
