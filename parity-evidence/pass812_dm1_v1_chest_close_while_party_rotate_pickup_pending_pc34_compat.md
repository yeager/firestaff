# pass812_dm1_v1_chest_close_while_party_rotate_pickup_pending_pc34_compat

Source-locked: CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest: materializes G0425 from the open G0426 chest); CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest: clears G0426, rewires non-empty G0425 into container Slot list, terminates with C0xFFFE_THING_ENDOFLIST); CHAMPION.C F0284:93-131 (keeps champion ownership independent from party direction updates); CHAMPION.C F0297:243-298 (leader-hand put); F0298:270-298 (leader-hand remove); F0300:511-515 (clears C30+ and champion slots); F0301:606-614 (writes C30+/champion slots); F0302:662-714 (snapshots leader hand and selected slot before remove/put exchange); PANEL.C F0347:1639-1691 (C09_SLOT_BOX_INVENTORY_ACTION_HAND click to F0302 for the action-hand pickup); COMMAND.C F0359:1985-1990 (inactive panel side routes); DEFS.H: C30..C37, C537..C545, G0425, G0426

Result: PASS
Tests: {'passes': 87, 'fails': 0}
Failures: []
