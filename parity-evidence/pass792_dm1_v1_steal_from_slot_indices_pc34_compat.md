# pass792 DM1 V1 Steal-From-Slot-Indices

- Status: PASS792_DM1_V1_STEAL_FROM_SLOT_INDICES_LOCKED
- Gate: Graphics.dat item 562 init data. G0025_auc_Graphic562_StealFromSlotIndices[0..7] = {NECK=10, POUCH_1=11, BACKPACK_LINE1_1=13, QUIVER_LINE1_1=12, NECK=10, BACKPACK_LINE1_1=13, POUCH_2=6, BACKPACK_LINE1_1=13}. GROUP.C F0193 dispatch: counter = RANDOM(8), lookup G0025[counter]; if the result is the backpack-base slot, +RANDOM(17) selects a random backpack line-1 slot. Counter loops with += 1 &= 0x0007 (mod 8).
- Runtime assertion floor: 96 assertions in `tests/test_dm1_v1_steal_from_slot_indices_pc34_compat.c`.
- Expected test output: `96/96 assertions passed`.

## ReDMCSB Anchors

- DATA.C:31
- DATA.C:244-251
- GROUP.C:1032
- GROUP.C:1041
- GROUP.C:1045
- GROUP.C:1075

## Non-Overlap

- Not pass784-790.
- Not the chest cancel-reopen-pickup gate.
- Not c161/c160/c159/c061/c030 mirror-candidate gates.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_steal_from_slot_indices_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass792_dm1_v1_steal_from_slot_indices_pc34_compat/manifest.json`
