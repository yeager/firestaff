# Pass376 — DM1 V1 deferred-explosion completion credit

Status: `PASS376_DM1_V1_DEFERRED_EXPLOSION_COMPLETION_CREDIT_PROVED`

## ReDMCSB source audit first

- `DUNVIEW.C:4547-4582` defines `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` and its per-cell object/creature/projectile plus later explosion-pass contract.
- `DUNVIEW.C:5915-5933` exits the packed-cell loop, starts `/* Draw explosions */`, restarts the thing list, and filters `C15_THING_TYPE_EXPLOSION` after the prior cell passes.
- pass375 verifies Firestaff now keeps projectiles in the per-cell effect path while moving explosions into `m11_draw_dm1_deferred_explosion_pass()` after visible side and center contents.

## Landable update

This pass credits pass375 in the conservative completion matrix: DM1 V1 moves from `56/100` to `57/100`; `viewport_ui_render` moves from `11/20` to `12/20`.

The credit is narrow: it covers the source-locked layer boundary for viewport explosions in the movement/viewport/wall renderer. It does not claim pixel parity, representative original overlay regression, or complete gameplay-system parity.

## Gates

- `pass375_deferred_explosion_prerequisite` ok=True
- `pass375_evidence_marker` ok=True
- `pass375_evidence_marker` ok=True
- `pass375_evidence_marker` ok=True
- `pass375_evidence_marker` ok=True
- `completion_matrix_credit` ok=True
- `completion_doc_credit` ok=True
- `firestaff_completion_matrix_verifier` ok=True
- `firestaff_completion_status_cli` ok=True

Manifest: `parity-evidence/verification/pass376_dm1_v1_completion_deferred_explosion_credit/manifest.json`
