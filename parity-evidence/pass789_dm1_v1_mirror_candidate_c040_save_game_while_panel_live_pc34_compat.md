# pass789 DM1 V1 Mirror Candidate C040 Save-Game-While-Panel-Live

- Status: PASS789_DM1_V1_MIRROR_CANDIDATE_C040_SAVE_GAME_WHILE_PANEL_LIVE_LOCKED
- Gate: COMMAND.C F0380:2367-2369 drops C140 save-game commands while G0299 is set; after F0282(C162) clears G0299 the save fires F0433.
- Runtime assertion floor: 43 assertions in `tests/test_dm1_v1_mirror_candidate_c040_save_game_while_panel_live_pc34_compat.c`.
- Expected test output: `43/43 assertions passed`.

## ReDMCSB Anchors

- COMMAND.C F0380:2367-2369
- STARTEND.C F0433
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- DEFS.H C140, C040/M568, G0299, G0305, G0411

## Non-Overlap

- Not pass788 (C012..C015 status-box).
- Not pass787 (C111 action-area).
- Not pass786 (C100 spell-area).
- Not pass785 (C007..C011 inventory-toggle).
- Not pass784 (cancel-then-reopen same-tick).
- Not the chest cancel-reopen-pickup gate.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_save_game_while_panel_live_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin COMMAND.C F0380:2367 line number in next source refresh.

Manifest: `parity-evidence/verification/pass789_dm1_v1_mirror_candidate_c040_save_game_while_panel_live_pc34_compat/manifest.json`
