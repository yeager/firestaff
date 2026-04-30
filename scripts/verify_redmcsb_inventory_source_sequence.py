#!/usr/bin/env python3
"""Verify the ReDMCSB source-first inventory UI/click sequence citations.

This is intentionally source-only: it guards the source ranges used by the
Firestaff DM1/V1 inventory parity notes without touching runtime rendering.
"""
from __future__ import annotations

import os
from pathlib import Path

DEFAULT_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_ROOT))

CHECKS = [
    ("DEFS.H", 778, 817, ["#define C00_SLOT_READY_HAND", "#define C29_SLOT_BACKPACK_LINE1_9", "#define C30_SLOT_CHEST_1", "#define C37_SLOT_CHEST_8"], "slot ids 0..37, including chest slots"),
    ("DEFS.H", 1873, 1878, ["C08_SLOT_BOX_INVENTORY_FIRST_SLOT", "C38_SLOT_BOX_CHEST_FIRST_SLOT", "M070_HAND_SLOT_INDEX"], "slot-box/chest constants and hand-slot index macro"),
    ("DATA.C", 927, 1023, ["SLOT_BOX G0030_as_Graphic562_SlotBoxes[46]", "C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND", "C536_ZONE_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9", "C544_ZONE_SLOT_BOX_45_CHEST_8"], "slot-box locations/zones for status hands, inventory, and chest"),
    ("DATA.C", 1049, 1087, ["G0038_ai_Graphic562_SlotMasks[38]", "MASK0x0002_HEAD", "MASK0x0040_QUIVER_LINE1", "MASK0x0400_CONTAINER"], "allowed-slot masks for inventory and chest"),
    ("COMMAND.C", 407, 451, ["G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory", "C011_COMMAND_CLOSE_INVENTORY", "C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND", "C070_COMMAND_CLICK_ON_MOUTH", "C071_COMMAND_CLICK_ON_EYE", "C057_COMMAND_CLICK_ON_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9", "C081_COMMAND_CLICK_IN_PANEL"], "inventory secondary mouse controls, slots, mouth/eye, panel route"),
    ("COMMAND.C", 1379, 1412, ["F0358_COMMAND_GetCommandFromMouseInput_CPSC", "P0721_ps_MouseInput->Command", "F0638_GetZone", "F0798_COMMAND_IsPointInZone"], "mouse-input command scanner and zone hit path"),
    ("COMMAND.C", 1980, 1983, ["G0456_as_Graphic561_MouseInput_PanelChest", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox", "C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND"], "open-chest panel click routing into slot boxes"),
    ("COMMAND.C", 2158, 2161, ["C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX", "F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox"], "status-box command dispatch"),
    ("COMMAND.C", 2174, 2177, ["C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND", "C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox"], "inventory/chest command dispatch"),
    ("COMMAND.C", 2314, 2320, ["C070_COMMAND_CLICK_ON_MOUTH", "F0349_INVENTORY_ProcessCommand70_ClickOnMouth", "C071_COMMAND_CLICK_ON_EYE", "F0352_INVENTORY_ProcessCommand71_ClickOnEye"], "mouth/eye command dispatch"),
    ("CLIKCHAM.C", 24, 32, ["G0423_i_InventoryChampionOrdinal", "F0368_COMMAND_SetLeader", "G0455_as_Graphic561_MouseInput_ChampionNamesHands", "C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox"], "champion status-box leader/hand-slot click routing"),
    ("CHAMPION.C", 677, 692, ["P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT", "M070_HAND_SLOT_INDEX", "G0423_i_InventoryChampionOrdinal", "C30_SLOT_CHEST_1", "G0425_aT_ChestSlots"], "slot-box click resolves status hands vs inventory/chest storage"),
    ("OBJECT.C", 435, 502, ["G0030_as_Graphic562_SlotBoxes[P0047_ui_SlotBoxIndex]", "L0017_ps_SlotBox->IconIndex", "G0026_ai_Graphic562_IconGraphicFirstIconIndex", "C08_SLOT_BOX_INVENTORY_FIRST_SLOT", "G0296_puc_Bitmap_Viewport", "F0132_VIDEO_Blit"], "object icon selection/destination/blit by slot-box"),
    ("CHAMDRAW.C", 595, 673, ["C212_ICON_READY_HAND", "C208_ICON_NECK", "C204_ICON_EMPTY_BOX", "F0033_OBJECT_GetIconIndex", "F0038_OBJECT_DrawIconInSlotBox"], "inventory slot placeholder/object icon draw"),
    ("CHAMDRAW.C", 1153, 1182, ["F0295_CHAMPION_HasObjectIconInSlotBoxChanged", "C032_ICON_WEAPON_DAGGER", "C148_ICON_POTION_MA_POTION_MON_POTION", "C163_ICON_POTION_WATER_FLASK", "C195_ICON_POTION_EMPTY_FLASK", "F0038_OBJECT_DrawIconInSlotBox"], "mutable carried-object icon refresh gate"),
    ("CHAMDRAW.C", 1226, 1252, ["AL0882_ui_SlotBoxIndex = 0", "C08_SLOT_BOX_INVENTORY_FIRST_SLOT", "C30_SLOT_CHEST_1", "G0425_aT_ChestSlots", "C38_SLOT_BOX_CHEST_FIRST_SLOT"], "changed-icon pass over status, inventory, and chest slot boxes"),
    ("COORD.C", 1915, 1934, ["F0798_COMMAND_IsPointInZone", "M704_ZONE_LEFT", "M707_ZONE_BOTTOM", "F0629_IsPointInZoneIndex", "F0638_GetZone"], "zone hit-test primitive used by click routing"),
]


def read_lines(rel: str) -> list[str]:
    path = ROOT / rel
    if not path.exists():
        raise AssertionError(f"missing source file: {path}")
    return path.read_text(errors="replace").splitlines()


def main() -> int:
    print("probe=redmcsb_inventory_source_sequence")
    print(f"sourceRoot={ROOT}")
    ok = True
    for rel, start, end, needles, label in CHECKS:
        lines = read_lines(rel)
        excerpt = "\n".join(lines[start - 1 : end])
        missing = [needle for needle in needles if needle not in excerpt]
        status = "ok" if not missing else "missing:" + ",".join(missing)
        print(f"sourceRange={rel}:{start}-{end} label={label} status={status}")
        if missing:
            ok = False
    print(f"inventorySourceSequenceInvariantOk={1 if ok else 0}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
