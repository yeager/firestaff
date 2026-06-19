# pass804_dm1_v1_chest_close_with_full_leader_hand_pc34_compat

Source-locked: CHEST.C F0333:53-67 (F0333_INVENTORY_OpenAndDrawChest: copy first eight linked things into G0425_aT_ChestSlots in list order); CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest: clear G0426, rewire G0425 chain into the container Slot list, terminate with C0xFFFE_THING_ENDOFLIST); CHAMPION.C F0297:243-268/F0298:270-298 (put/remove leader-hand identity and update leader load); CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 write + load); F0302:688-710 (leader hand + destination slot click dispatcher with empty/empty + incompatible destination rejection); DUNGEON.C F0140:1114-1120 (container base weight preservation through close rewire); DEFS.H C00..C29, C038, C040, C04_CHAMPION_CLOSE_INVENTORY, C0xFFFE_THING_ENDOFLIST, C0xFFFF_THING_NONE

Result: PASS
Tests: {'passes': 121, 'fails': 0}
Failures: []
