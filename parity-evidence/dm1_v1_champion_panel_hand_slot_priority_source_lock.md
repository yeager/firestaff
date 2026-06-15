# DM1 V1 champion-panel hand-slot priority source lock

Status: DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LOCKED

DM1 V1 champion-panel/HUD hand-slot priority is source-locked: status hand -> leader hand -> backpack -> belt, with F0302 status hand routing and C30/G0425 chest storage evidence.

## Primary Evidence
- PASS CHAMPION.C:677-684: F0302 routes slot-box indices 0..7 through candidate/open/dead guards and M070 hand-slot mapping before inventory slots.
- PASS CHAMPION.C:243-298: F0297/F0298 own leader-hand object, pointer/name, load byte, and F0292 redraw side effects.
- PASS CHAMPION.C:511-515: F0300 clears C30+ slots through G0425_aT_ChestSlots.
- PASS CHAMPION.C:606-614: F0301 writes C30+ slots through G0425_aT_ChestSlots and then updates the champion load.
- PASS CHAMPION.C:688-712: F0302 snapshots leader hand, reads storage, rejects empty/mask cases, then executes F0077/F0298/F0300/F0297/F0301/F0292/F0078 order.
- PASS DEFS.H:780-817: DEFS.H binds hand/body/backpack/chest slot constants including C30.
- PASS DEFS.H:873-876: DEFS.H maps M516_CHAMPIONS to the party champion array.
- PASS DEFS.H:1874-1878: DEFS.H binds the status/inventory slot-box boundary and M070 hand-slot macro.
- PASS DEFS.H:5324-5332: DEFS.H binds slot-box metadata and the slot masks used by F0302 AllowedSlots.
- PASS DEFS.H:5700-5881: DEFS.H exposes party count, inventory champion ordinal, chest slots, and open chest.
- PASS CHAMDRAW.C:498-559: F0291 redraws a requested champion slot and reads C30+ inventory from G0425_aT_ChestSlots.
- PASS CHAMDRAW.C:703-711: F0292 is the champion state redraw entry called by F0297/F0302.
- PASS CHAMDRAW.C:1060-1088: F0292 redraws panel content and action hand when the relevant attributes are set.
- PASS PANEL.C:1493-1616: PANEL.C F0344/F0345 are the food/water panel redraw dependency reached by the action-hand panel path.
- PASS DUNVIEW.C:8337-8349: DUNVIEW viewport redraw/clickable setup exists as a non-claim fence for this HUD gate.

## Local Gates
- PASS src/dm1/dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h
- PASS src/dm1/dm1_v1_champion_panel_hand_slot_priority_pc34_compat.c
- PASS tests/test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat.c
- PASS CMakeLists.txt

## Verification
- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat: rc=0
~~~
Assertions: 111
Failures: 0
DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_PC34_COMPAT_OK
~~~

## Non-Claims
- No real GRAPHICS.DAT, DUNGEON.DAT, or savegame load.
- No bitmap or original DOS pixel parity claim.
- No change to DUNVIEW viewport presentation order.
- No expansion of chest contents, backpack storage, or object type semantics beyond this contract.
