# pass760 DM1 V1 champion-panel status-hand rotation source lock

Status: PASS760_DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_SOURCE_LOCKED

DM1 V1 champion-panel status-hand C033/C034/C035 box rotation is source-locked across acting-champion changes and preserves the status-hand -> leader-hand -> backpack -> belt priority chain.

## Primary Evidence
- PASS CHAMPION.C:677-684: F0302 routes C00..C07 status-hand slot boxes before inventory, mapping champion and hand slot from the slot-box index.
- PASS CHAMPION.C:243-298: F0297/F0298 own leader-hand put/remove state, load, pointer, and redraw side effects.
- PASS CHAMPION.C:270-298: F0298 removes the leader-hand object and redraws/load-adjusts the leader before the slot write path continues.
- PASS CHAMPION.C:511-515: F0300 clears C30+ slots through G0425_aT_ChestSlots.
- PASS CHAMPION.C:606-614: F0301 writes C30+ slots through G0425_aT_ChestSlots and updates load.
- PASS CHAMPION.C:688-712: F0302 snapshots leader hand, reads storage, applies mask/empty guards, and performs the occupied-slot swap helper order.
- PASS CHAMDRAW.C:621-630: F0291 resolves the status-panel action-hand icon before the status-hand border rotation.
- PASS CHAMDRAW.C:634-642: F0291 selects C035 for the acting champion's action hand, otherwise C034 wounded or C033 normal.
- PASS CHAMDRAW.C:898-935: F0292 redraw tuple is recorded as context only; this pass does not duplicate the mouth/eye or food/water gates.
- PASS DEFS.H:780-817: DEFS.H:780-817 anchors hand, belt/quick, backpack, and C30..C37 slots.
- PASS DEFS.H:873-876: M516_CHAMPIONS lives outside the requested 780-817 slice in this ReDMCSB snapshot.
- PASS DEFS.H:1874-1878: M070_HAND_SLOT_INDEX and the C08 status/inventory boundary live outside the requested 780-817 slice.
- PASS DEFS.H:2088-2088: DEFS.H:2088 anchors C10_COLOR_FLESH transparency/color lineage.
- PASS DEFS.H:5700-5881: G0305/G0423/G0425/G0426 live outside the requested 780-817 slice in this ReDMCSB snapshot.

## Local Gates
- PASS include/dm1_v1_champion_panel_pc34_compat.h
- PASS src/dm1/dm1_v1_champion_panel_pc34_compat.c
- PASS tests/test_dm1_v1_champion_panel_pc34_compat.c
- PASS CMakeLists.txt

## Verification
- build/test_dm1_v1_champion_panel_pc34_compat: rc=0; assertions=851; failures=0
~~~
Assertions: 851
Failures: 0
PASS pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock assertions=851
~~~

## Anchor Notes
- Requested DEFS.H:780-817 is preserved as the C00/C01/backpack/belt/C30 slot anchor.
- G0425/G0426/G0423/G0305/M070/M516 are not located in DEFS.H:780-817 in this ReDMCSB snapshot; this verifier records their exact actual lines as supplemental anchors.

## Non-Claims
- No real GRAPHICS.DAT, DUNGEON.DAT, savegame, or bitmap load.
- No original-vs-Firestaff pixel parity claim.
- No duplicate pass673 mouth/eye status-box gate.
- No duplicate pass683 food/water status-box gate.
