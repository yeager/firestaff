# pass834 DM1 V1 Indirect-Stop-Expiring-Event

- Status: PASS834_DM1_V1_INDIRECT_STOP_EXPIRING_EVENT_LOCKED
- Gate: Graphics.dat item 562 init var G0022_i_Graphic562_IndirectStopExpiringEvent_CPSE. A single int16_t copy-protection state flag initialized to C00555_FALSE. Read sites: MOVESENS.C:744/746 (with BUG0_00 useless compare) + TIMELINE.C:1922 (write site, set to C00136_TRUE).
- Runtime assertion floor: 13 assertions in `tests/test_dm1_v1_indirect_stop_expiring_event_pc34_compat.c`.
- Expected test output: `13/13 assertions passed`.

## ReDMCSB Anchors

- DATA.C:28
- DATA.C:232
- DATA.C:874
- MOVESENS.C:744/746
- TIMELINE.C:1922

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-833 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11+12).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_indirect_stop_expiring_event_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass834_dm1_v1_indirect_stop_expiring_event_pc34_compat/manifest.json`
