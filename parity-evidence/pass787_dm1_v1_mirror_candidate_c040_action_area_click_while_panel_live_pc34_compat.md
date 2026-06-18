# pass787 DM1 V1 Mirror Candidate C040 Action-Area-Click-While-Panel-Live

- Status: PASS787_DM1_V1_MIRROR_CANDIDATE_C040_ACTION_AREA_CLICK_WHILE_PANEL_LIVE_LOCKED
- Gate: COMMAND.C F0380:2309-2311 drops C111 action-area clicks while G0299 is set; after F0282(C162) clears G0299 the click fires F0371.
- Runtime assertion floor: 42 assertions in `tests/test_dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_pc34_compat.c`.
- Expected test output: `42/42 assertions passed`.

## ReDMCSB Anchors

- COMMAND.C F0380:2309-2311
- COMMAND.C F0371
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- DEFS.H C111, C040/M568, G0299, G0305, G0411

## Non-Overlap

- Not pass786 (C100 spell-area with G0514 magic-caster gate).
- Not pass785 (C007..C011 inventory-toggle).
- Not pass784 (cancel-then-reopen same-tick).
- Not the C012..C016 status-box gate.
- Not the C140 save gate.
- Not the chest cancel-reopen-pickup gate.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin COMMAND.C F0380:2309 and F0371 line numbers in next source refresh.

Manifest: `parity-evidence/verification/pass787_dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_pc34_compat/manifest.json`
