# pass809 DM1 V1 Slot-Masks

- Status: PASS809_DM1_V1_SLOT_MASKS_LOCKED
- Gate: Graphics.dat item 562 init var G0038_ai_Graphic562_SlotMasks[38]. The per-slot allowed-slots bitmask table used to validate thing placement. 8 status-hand + 30 inventory + 8 chest slots. Body parts have single-bit masks; Ready/Action Hand + Backpack accept any item; Quiver/Pouch/Neck accept their respective item classes; Chest accepts containers only. Read sites: CHAMPION.C:697 (leader-hand placement check), REVIVE.C:307/310/338 (resurrect placement check).
- Runtime assertion floor: 152 assertions in `tests/test_dm1_v1_slot_masks_pc34_compat.c`.
- Expected test output: `152/152 assertions passed`.

## ReDMCSB Anchors

- DATA.C:44
- DATA.C:320-358
- DATA.C:1049
- CHAMPION.C:697
- REVIVE.C:307/310/338

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-808 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_slot_masks_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass809_dm1_v1_slot_masks_pc34_compat/manifest.json`
