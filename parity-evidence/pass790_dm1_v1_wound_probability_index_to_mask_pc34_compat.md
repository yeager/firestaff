# pass790 DM1 V1 Wound-Probability-Index-To-Mask

- Status: PASS790_DM1_V1_WOUND_PROBABILITY_INDEX_TO_MASK_LOCKED
- Gate: Graphics.dat item 562 init data. G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask[0..3] = {FEET=0x20, LEGS=0x10, TORSO=0x08, HEAD=0x04}. PROJEXPL.C:1386 reads it after a wound-test branch. The fallback branch (PROJEXPL.C:1389) uses MASK0x0001_WOUND_READY_HAND when the test-mask bits 4,5,6 are all clear.
- Runtime assertion floor: 37 assertions in `tests/test_dm1_v1_wound_probability_index_to_mask_pc34_compat.c`.
- Expected test output: `37/37 assertions passed`.

## ReDMCSB Anchors

- DATA.C:30
- DATA.C:243
- PROJEXPL.C:1378
- PROJEXPL.C:1386
- PROJEXPL.C:1389
- DEFS.H:736
- DEFS.H:741

## Non-Overlap

- Not pass784-789 mirror-candidate C040 gates.
- Not the chest cancel-reopen-pickup gate.
- Not c161/c160/c159/c061/c030 mirror-candidate gates.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_wound_probability_index_to_mask_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:243 and PROJEXPL.C:1386 in next source refresh.

Manifest: `parity-evidence/verification/pass790_dm1_v1_wound_probability_index_to_mask_pc34_compat/manifest.json`
