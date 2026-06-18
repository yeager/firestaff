# pass788 DM1 V1 Mirror Candidate C040 Status-Box-Click-While-Panel-Live

- Status: PASS788_DM1_V1_MIRROR_CANDIDATE_C040_STATUS_BOX_CLICK_WHILE_PANEL_LIVE_LOCKED
- Gate: COMMAND.C F0380:2159-2161 drops C012..C015 status-box clicks while G0299 is set; after F0282(C162) clears G0299 the click fires F0367.
- Runtime assertion floor: 46 assertions in `tests/test_dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_pc34_compat.c`.
- Expected test output: `46/46 assertions passed`.

## ReDMCSB Anchors

- COMMAND.C F0380:2159-2161
- COMMAND.C F0367
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- DEFS.H C012..C015, C040/M568, G0299, G0305, G0411

## Non-Overlap

- Not pass787 (C111 action-area).
- Not pass786 (C100 spell-area).
- Not pass785 (C007..C011 inventory-toggle).
- Not pass784 (cancel-then-reopen same-tick).
- Not the C140 save gate.
- Not the chest cancel-reopen-pickup gate.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin COMMAND.C F0380:2159 and F0367 line numbers in next source refresh.

Manifest: `parity-evidence/verification/pass788_dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_pc34_compat/manifest.json`
