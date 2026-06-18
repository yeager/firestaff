# pass786 DM1 V1 Mirror Candidate C040 Spell-Area-Click-While-Panel-Live

- Status: FAILED_PASS786_DM1_V1_MIRROR_CANDIDATE_C040_SPELL_AREA_CLICK_WHILE_PANEL_LIVE_LOCKED
- Gate: COMMAND.C F0380:2303-2306 drops C100 spell-area clicks while G0299 is set; after F0282(C162) clears G0299 with a valid G0514 the click fires F0370; if G0514 is then cleared the click is dropped again.
- Runtime assertion floor: 48 assertions in `tests/test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat.c`.
- Expected test output: `48/48 assertions passed`.

## ReDMCSB Anchors

- COMMAND.C F0380:2303-2306
- COMMAND.C F0370:2482-2520
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- DEFS.H C100, C040/M568, G0299, G0305, G0411, G0514

## Non-Overlap

- Not pass785 inventory-toggle-while-c040-live.
- Not pass784 cancel-then-reopen same-tick.
- Not the C111 action-area gate (separate F0380 line).
- Not the chest cancel-reopen-pickup gate.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin COMMAND.C F0380:2303 and F0370:2482 line numbers in next source refresh.

Manifest: `parity-evidence/verification/pass786_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat/manifest.json`
