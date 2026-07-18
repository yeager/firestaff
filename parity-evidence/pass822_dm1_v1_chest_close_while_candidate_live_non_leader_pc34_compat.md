# pass822_dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat

Source-locked: CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest: materializes G0425 from the open G0426 chest); CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest: clears G0426, rewires non-empty G0425 into container Slot list, terminates with C0xFFFE_THING_ENDOFLIST); CHAMPION.C F0284:93-131 (keeps champion ownership independent from party direction updates); CHAMPION.C F0297:243-298 (leader-hand put); F0298:270-298 (leader-hand remove); F0300:511-515 (clears C30+ and champion slots); F0301:606-614 (writes C30+/champion slots); F0302:662-714 (snapshots leader hand and selected slot before remove/put exchange); CLIKCHAM.C F0367:24-35 (status-box nested G0455 dispatch); CLIKCHAM.C F0368:51-72 (set-leader state transition with old-leader dirty marking + leader-hand weight remove/add); PANEL.C F0344/F0345/F0352 (inactive food/water, eye, and resurrect-panel side routes); DEFS.H: C00..C29, C037..C040, C537..C545, G0423, G0425, G0426, G0411, G0455

Result: PASS
Tests: {'passes': 1, 'fails': 0}
Failures: []
