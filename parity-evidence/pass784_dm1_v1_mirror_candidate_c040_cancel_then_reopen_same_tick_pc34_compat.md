# pass784 DM1 V1 Mirror Candidate C040 Cancel-Then-Reopen Same Tick

- Status: PASS784_DM1_V1_MIRROR_CANDIDATE_C040_CANCEL_THEN_REOPEN_SAME_TICK_LOCKED
- Gate: C162 cancel followed by a new-sensor F0280 reopen inside the same tick.
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- PANEL.C F0355:2299-2318
- COMMAND.C F0378:1956-1990
- MOVESENS.C F0275:1502
- DEFS.H C040/M568, C127, C162, G0299, G0305, G0415, G0424

## Non-Overlap

- Not pass760 mirror-candidate chrome after non-candidate transition.
- Not pass762 mirror-candidate rotate-in-progress open.
- Not the M569 chest cancel-reopen-pickup gate.
- Not C045 food/water mirror-candidate gates.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat`: rc=0

## TODO

- Anchor drift note: DEFS.H and SENSOR.C rows are line-0 best-effort; re-pin when the local ReDMCSB tree is updated.

Manifest: `parity-evidence/verification/pass784_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat/manifest.json`
