# pass840_dm1_v1_box_entrance_doors_pc34_compat

- Status: PASS840_DM1_V1_ENTRANCE_DOORS_LOCKED

- Gate: Graphics.dat item 562 init var G0009.

- Runtime assertion floor: 53 assertions.

- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:15
- DATA.C:137
- DATA.C:557
- ENTRANCE.C:529/538/541/544/547

## Non-Overlap

- Not pass784-790.

- Not pass791+ (champion-panel + champion + mirror + chest).

- Not pass798+ (Graphics.dat init-table gates).


## Verification

- \`/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_entrance_doors_pc34_compat\`: rc=0