# pass832 DM1 V1 Box-Movement-Arrows

- Status: PASS832_DM1_V1_BOX_MOVEMENT_ARROWS_LOCKED
- Gate: Graphics.dat item 562 init var G0002_ai_Graphic562_Box_MovementArrows[4] = {224, 319, 124, 168}. The {X, Y, W, H} byte-coordinate sub-rectangle for the movement-arrows background blit on the champion panel. Read sites: MENUDRAW.C:13/31 + STARTUP2.C:369 (movement-arrows blit + clear + hatch).
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_movement_arrows_pc34_compat.c`.
- Expected test output: `54/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:8
- DATA.C:123
- DATA.C:543
- MENUDRAW.C:13
- PANEL.C:2369
- STARTUP2.C:369

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_movement_arrows_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:123/539, MENUDRAW.C:13/31, STARTUP2.C:369 in next source refresh.

Manifest: `parity-evidence/verification/pass832_dm1_v1_box_movement_arrows_pc34_compat/manifest.json`
