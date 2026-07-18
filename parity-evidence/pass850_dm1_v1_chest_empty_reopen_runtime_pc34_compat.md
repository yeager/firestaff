# pass850_dm1_v1_chest_empty_reopen_runtime_pc34_compat

Source-locked: CHEST.C F0333:30-32 (F0333_INVENTORY_OpenAndDrawChest: open path entry, same-open return); CHEST.C F0333:36-46 (chain walk via F0159, eight-item cap, G0425_aT_ChestSlots writes); CHEST.C F0333:67-76 (fills the unused visible G0425_aT_ChestSlots [0..7] with C0xFFFF_THING_NONE so the panel never shows stale icons from a previous chest); CHEST.C F0334:113-117 (close-rewire: no-open return, G0426 clear, Container->Slot=C0xFFFE_THING_ENDOFLIST clobber); CHEST.C F0334:117-132 (close-loop with L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList); CHAMPION.C F0297:243-298 (leader-hand put/remove, weight/charges/AllowedSlots/load, not called by CHEST.C F0333 or F0334); DEFS.H:434, 778-817

Result: PASS
Tests: {'passes': 65, 'fails': 0}
Failures: []
