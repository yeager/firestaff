# pass809_dm1_v1_chest_close_stack_merge_pc34_compat

Source-locked: CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest: same-open return, chain walk via F0159, eight-item cap, G0425_aT_ChestSlots writes, C0xFFFE_THING_ENDOFLIST stop, C0xFFFF_THING_NONE tail fill); CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest: no-open return, G0426 clear, Container->Slot=C0xFFFE_THING_ENDOFLIST clobber, scan eight G0425 entries, skip C0xFFFF_THING_NONE, clear slots, relink via F0163 with CM1_MAPX_NOT_ON_A_SQUARE list-append mode); DUNGEON.C F0163:1769-1838 (DUNGEON_LinkThingToList list-append: P0287_T_ThingToLink->Next forced to C0xFFFE_THING_ENDOFLIST, then F0159 walks P0288_T_ThingInList until C0xFFFE_THING_ENDOFLIST, last walked thing's Next overwritten with P0287_T_ThingToLink); DUNGEON.C F0159:1664-1681 (DUNGEON_GetNextThing: returns the Next field verbatim); CHAMPION.C F0297:243-268 (leader-hand put: store Thing, derive icon with F0033, add F0140 weight, mark load); CHAMPION.C F0298:270-298 (leader-hand remove); CHAMPION.C F0300:511-515 (G0425 remove); F0301:606-614 (G0425 write + load); F0302:688-710 (leader hand + destination slot click dispatcher)

Result: FAIL
Tests: {'passes': 0, 'fails': 0}
Failures: ["cmake_registration missing: ['verify_pass809_dm1_v1_chest_close_stack_merge_pc34_compat']"]
