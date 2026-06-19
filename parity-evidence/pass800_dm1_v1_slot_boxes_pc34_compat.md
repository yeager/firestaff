# pass800 DM1 V1 Slot-Boxes

- Status: PASS800_DM1_V1_SLOT_BOXES_LOCKED
- Gate: Graphics.dat item 562 init var G0030_as_Graphic562_SlotBoxes[46]. 8 status-box hands (Y=10, +20/within-pair, +69/between-champions), 30 inventory slots (X in [6, 202], Y in [16, 90]), 8 chest slots (curved bottom row, Y monotonically increasing 59..105, X traces 117->106->111->128->145->162->179->196). All ZoneIndex=0 in PC 3.4 init.
- Runtime assertion floor: 565 assertions in `tests/test_dm1_v1_slot_boxes_pc34_compat.c`.
- Expected test output: `565/565 assertions passed`.

## ReDMCSB Anchors

- DATA.C:36
- DATA.C:264-309
- OBJECT.C:435
- OBJECT.C:521
- CHAMDRAW.C:557
- CHAMDRAW.C:562

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-796 (champion-panel/leader/mirror).
- Not pass797 (icon-graphic-first-icon-index).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_slot_boxes_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass800_dm1_v1_slot_boxes_pc34_compat/manifest.json`
