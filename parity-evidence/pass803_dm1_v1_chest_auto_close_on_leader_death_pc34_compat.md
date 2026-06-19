# pass803_dm1_v1_chest_auto_close_on_leader_death_pc34_compat

Source-locked: CHAMPION.C F0319:1552-1607 (F0319_CHAMPION_Kill: CurrentHealth=0, clear G0333 + G0331, dispatch F0355 with C04_CHAMPION_CLOSE_INVENTORY, then F0318_DropAllObjects); CHAMPION.C F0318:1527-1551 (F0318_CHAMPION_DropAllObjects: every C00..C29 slot to leader's current Cell); PANEL.C F0355:2244-2310 (F0355_INVENTORY_Toggle_CPSE closes inventory panel with C04_CHAMPION_CLOSE_INVENTORY); PANEL.C F0355:2268-2275 (death short-circuit returns early when champion is dead and not closing inventory); PANEL.C F0355:2318-2322 (F0334 call mutates G0426 to C0xFFFF_THING_NONE); CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest clears G0426 and rewires G0425_aT_ChestSlots into the container Slot list); CHEST.C F0333:30-67 (negative no-open anchor: no reopen during death); CHAMPION.C F0297/F0298:243-298 (no-leader-hand-mutate anchor during death); CHAMPION.C F0300/F0301:511-614 (C00..C29 get/put object primitives for F0318); COMMAND.C F0380:2045-2184 (negative no-queue-drain anchor); DEFS.H C00..C29, C30..C37, C038, C040, C04_CHAMPION_CLOSE_INVENTORY, C10_COLOR_FLESH, G0299, G0331, G0333, G0423, G0424, G0425, G0426, M516_CHAMPIONS[].CurrentHealth/Load/Slots

Result: PASS
Tests: {'passes': 1, 'fails': 0}
Failures: []
