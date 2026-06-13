# pass760 DM1 V1 Mirror Candidate Chrome After Non-Candidate Transition

- Status: PASS760_DM1_V1_MIRROR_CANDIDATE_CHROME_AFTER_NON_CANDIDATE_TRANSITION_LOCKED
- Gate: candidate close -> non-candidate inventory/chest/slot transition -> reopened C040 chrome.
- Runtime assertion floor: >=80 assertions in `tests/test_dm1_v1_mirror_candidate_pc34_compat.c`.
- Expected test output: `PASS pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression`.

## ReDMCSB Anchors

- CHAMDRAW.C F0291/F0296:551-552,1249-1252
- CHAMPION.C F0284:93-131
- CHAMPION.C F0297:243-268
- CHAMPION.C F0298:270-298
- CHAMPION.C F0300:511-515
- CHAMPION.C F0301:606-614
- CHAMPION.C F0302:662-714
- COMMAND.C F0359:1985-1990
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- CHEST.C F0333:30-67
- CHEST.C F0334:113-132
- PANEL.C F0344:1895-1944
- PANEL.C F0345:1946-1999
- OBJECT.C F0033:147-212
- BLITMASK.C F0133:30-33
- DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040

## Non-Overlap

- Not pass674 scroll-pickup-leader-rotation-inventory-click.
- Not pass686 keyboard-browse-occupied-slot.
- Not pass710/pass711 live-panel C045/C038 drop/pickup.
- Not pass736 close-while-resurrect-pending with inventory pickup.

## Verification

- `/Users/bosse/.openclaw/workspace-main-20260612142622-mirror-candidate-chrome-non-candidate-transition/build/test_dm1_v1_mirror_candidate_pc34_compat`: rc=0

## TODO

- Anchor drift note: keep requested `DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040`; local line 2088 is `C10_COLOR_FLESH`, while those symbols are elsewhere in DEFS.H.

Manifest: `parity-evidence/verification/pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression/manifest.json`
