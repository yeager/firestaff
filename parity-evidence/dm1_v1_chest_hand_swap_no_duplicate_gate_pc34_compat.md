# dm1_v1_chest_hand_swap_no_duplicate_gate_pc34_compat

Source-locked: CHEST.C F0333:30-67 + 70-74 (F0333_INVENTORY_OpenAndDrawChest) - G0426_T_OpenChest + G0425_aT_ChestSlots / C537..C544; CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest) - G0426 clear + container Slot to C0xFFFE_THING_ENDOFLIST + L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList; CHAMPION.C F0297:243-268 (F0297_CHAMPION_PutObjectInLeaderHand) - put into leader hand + refresh load; CHAMPION.C F0298:270-298 (F0298_CHAMPION_GetObjectRemovedFromLeaderHand) - remove from leader hand + refresh load; CHAMPION.C F0300:511-584 (F0300_CHAMPION_GetObjectRemovedFromSlot) - G0425_aT_ChestSlots remove path for C30+; CHAMPION.C F0301:606-660 (F0301_CHAMPION_AddObjectInSlot) - G0425 write path for C30+; CHAMPION.C F0302:662-713 (F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox) - C30+ slot click dispatcher; CHAMPION.C F0302:694-700 rejects empty/empty and incompatible AllowedSlots; PANEL.C F0347:1639-1691 (C09_SLOT_BOX_INVENTORY_ACTION_HAND click) - routes the leader action hand click into F0302; DEFS.H C30..C37, C537..C544, C0xFFFF_THING_NONE, C0xFFFE_THING_ENDOFLIST, M070_HAND_SLOT_INDEX, M569_PANEL_CHEST

Result: PASS
Tests: {'passes': 121, 'fails': 0, 'assertionCount': 121, 'failures': 0}
Failures: []
