# pass764 DM1 V1 champion-panel second-leader hand-slot priority source lock

Status: DM1_V1_CHAMPION_PANEL_SECOND_LEADER_HAND_SLOT_PRIORITY_SOURCE_LOCKED

This narrow contract-only slice extends the existing hand-slot priority source
lock to the second champion as leader.  It pins slot-box 3 as champion index 1's
action hand, then checks the companion F0292 HUD cascade for C152/C160/C164,
C196, C114, and C214.

## Primary Evidence
- PASS CHAMPION.C:677-684: F0302 routes status slot-box 3 through the status-hand path before inventory slots and resolves champion index 1 with M070 action hand.
- PASS CHAMPION.C:688-712: F0302 preserves leader-hand snapshot, storage read, mask rejection, F0077/F0298/F0300/F0297/F0301/F0292/F0078 helper order.
- PASS CHAMPION.C:243-298: F0297/F0298 own leader-hand object, pointer/name, load, and F0292 redraw side effects.
- PASS CHAMPION.C:511-515: F0300 C30+ clear uses G0425_aT_ChestSlots.
- PASS CHAMPION.C:606-614: F0301 C30+ write uses G0425_aT_ChestSlots before load update.
- PASS CHAMDRAW.C:307-342: F0287 champion index 1 HP bar uses C196 at x=115/y=2, C12 blank band, and champion color fill.
- PASS CHAMDRAW.C:771-815: F0292 fills C152, a 67x29 live status box, with C12 before child overdraw.
- PASS CHAMDRAW.C:843-895: F0292 PC34 name-color cascade uses C11 for the leader and C09 for nonleaders.
- PASS CHAMDRAW.C:1019-1051: F0292 draws champion index 1's 19x14 champion icon through C114.
- PASS CHAMDRAW.C:632-651: F0291 keeps C033/C034/C035 18x18 hand-slot box cascade for champion index 1 action hand C214.
- PASS DEFS.H:780-817,1878,2178-2199,3779-3807,5700-5881: slot, M070, graphic, zone, party, inventory, chest, and M516 anchors.

## Local Gates
- include/firestaff/dm1/v1/champion/dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.h
- src/dm1/dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.c
- tests/test_dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.c
- tools/verify_pass764_dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.py
- CMakeLists.txt

## Verification
- DeterministicHash: 0xB540AE26

## Non-Claims
- No real GRAPHICS.DAT, DUNGEON.DAT, savegame, or original DOS screenshot load.
- No original DOS pixel-parity claim.
- No broad C113..C116 champion-icon coverage beyond champion index 1/C114.
- No C037/C038/C039 shield-border runtime claim; that belongs to the other branch.
