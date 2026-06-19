# pass833 DM1 V1 Box-Entrance-Dungeon-View

- Status: PASS833_DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_LOCKED
- Gate: Graphics.dat item 562 init var G0006_ai_Graphic562_Box_Entrance_DungeonView[4] = {0, 223, 3, 138}. The {X, Y, W, H} byte-coordinate sub-rectangle for the entrance dungeon-view backdrop blit during the door-opening animation. Read sites: ENTRANCE.C:178/181/184/187 F0132_VIDEO_Blit.
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_entrance_dungeon_view_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:12
- DATA.C:131
- DATA.C:551
- ENTRANCE.C:178/181/184/187

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-832 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_entrance_dungeon_view_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:131/551, ENTRANCE.C:178/181/184/187 in next source refresh.

Manifest: `parity-evidence/verification/pass833_dm1_v1_box_entrance_dungeon_view_pc34_compat/manifest.json`
