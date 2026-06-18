# pass785 DM1 V1 Mirror Candidate C040 Inventory-Toggle-While-Panel-Live

- Status: PASS785_DM1_V1_MIRROR_CANDIDATE_C040_INVENTORY_TOGGLE_WHILE_PANEL_LIVE_LOCKED
- Gate: inventory-toggle commands C007..C011 dispatched while G0299 is set are dropped by COMMAND.C F0380:2181-2183; after F0282(C162) clears G0299 the toggle becomes live again.
- Runtime assertion floor: 44 assertions in `tests/test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat.c`.
- Expected test output: `44/44 assertions passed`.

## ReDMCSB Anchors

- COMMAND.C F0380:2181-2183
- PANEL.C F0355:2299-2318
- REVIVE.C F0280:124-132
- REVIVE.C F0282:744-806
- DEFS.H C007..C011, C040/M568, G0299, G0305, G0411

## Non-Overlap

- Not pass784 cancel-then-reopen same-tick.
- Not the C160/C161 accept path.
- Not the chest cancel-reopen-pickup gate.

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin COMMAND.C F0380:2181 line number in next source refresh.

Manifest: `parity-evidence/verification/pass785_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat/manifest.json`
