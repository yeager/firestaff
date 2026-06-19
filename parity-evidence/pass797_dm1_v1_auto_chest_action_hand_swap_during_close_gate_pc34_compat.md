# pass797_dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat

Source-locked: CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44 (C145_ICON_CONTAINER_CHEST_OPEN blit skip); CHEST.C F0333:53-67 + 70-74 (G0425_aT_ChestSlots population + C0xFFFF_THING_NONE fill); CHEST.C F0334:113-117 (G0426 clear, container Slot to C0xFFFE_THING_ENDOFLIST); CHEST.C F0334:118-132 (G0425 close-loop with L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList); CHAMPION.C F0297:243-268/F0298:270-298 (put/remove leader-hand); CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 write + load); CHAMPION.C F0302:688-710 (leader hand + destination slot click dispatcher); PANEL.C F0347:1639-1691 (C09_SLOT_BOX_INVENTORY_ACTION_HAND click to F0302)

Result: PASS
Tests: {'passes': 147, 'fails': 0}
Failures: []
