# Theron V1 runtime screenshot promotion checklist

Status: `REVIEW_CHECKLIST_NO_ROW_PROMOTED`

Decision: **REVIEW_CHECKLIST_NO_ROW_PROMOTED**

This is a bounded, machine-checkable companion to the runtime
screenshot promotion gate. It surfaces the explicit per-row
review checklist a human reviewer must work through before any
Theron capture may be promoted into public docs, and folds an
optional operator-local reviewer sign-off file into the audit
so the path forward is auditable rather than implicit.

## Reviewer Checklist

For every readiness row, the reviewer must confirm each of the
following items is `PASS`:

- **real_runtime_capture** - real Firestaff Theron launch under dummy video (probe.sourceId=='theron', launchedEver==1)
- **no_fallback_assets** - launch output does not contain 'deterministic fallback assets'
- **tqr_level_load_marker** - launch output contains the 'TQR level load' boot marker
- **semantic_track02_evidence** - runtime probe shows semantic Track 02 loader evidence (gameTick>0, party.mapIndex!=0, or lastOutcome beyond 'THERON READY')
- **unique_source_bmp** - source BMP sha256 is unique across all rows (duplicate sha256s indicate a shared placeholder fixture)
- **presented_bmp_geometry** - presented BMP is a valid 320x200 24-bit BMP with more than 200 non-black pixels
- **reviewer_signoff** - operator-local reviewer sign-off file reports reviewed=true for this row with a non-empty reviewer and review_date

## Per-row checklist

Each row below carries the machine-checkable verdict for every
checklist item, plus the optional reviewer sign-off verdict.
A row is `REVIEWER_PROMOTED_README_ELIGIBLE` only when every
machine item is `PASS` AND the operator-local reviewer sign-off
is `PASS` for that row. The aggregate decision remains
`REVIEW_CHECKLIST_NO_ROW_PROMOTED` until both halves of the
contract are satisfied for at least one row.

| Case | Classification | Machine | Reviewer | Items |
|---|---|---|---|---|
| canonical_pcengine_root | `REVIEW_CHECKLIST_INELIGIBLE` | — | — | real_runtime_capture=PASS, no_fallback_assets=FAIL, tqr_level_load_marker=PASS, semantic_track02_evidence=FAIL, unique_source_bmp=FAIL, presented_bmp_geometry=FAIL, reviewer_signoff=PENDING_REVIEWER |
| jp_extras_track02_bin | `REVIEW_CHECKLIST_INELIGIBLE` | — | — | real_runtime_capture=PASS, no_fallback_assets=FAIL, tqr_level_load_marker=PASS, semantic_track02_evidence=FAIL, unique_source_bmp=FAIL, presented_bmp_geometry=FAIL, reviewer_signoff=PENDING_REVIEWER |
| us_extras_track02_bin | `REVIEW_CHECKLIST_INELIGIBLE` | — | — | real_runtime_capture=PASS, no_fallback_assets=FAIL, tqr_level_load_marker=PASS, semantic_track02_evidence=FAIL, unique_source_bmp=FAIL, presented_bmp_geometry=FAIL, reviewer_signoff=PENDING_REVIEWER |

## Aggregate

- Decision: **REVIEW_CHECKLIST_NO_ROW_PROMOTED**
- Machine-eligible rows (awaiting reviewer): **0**
- Reviewer-promoted rows: **0**
- Ineligible rows: **3**
- Skipped rows (no data on this host): **0**
- Unique-source-sha256 findings: **1**
- Contract-drift findings: **0**
- Reviewer sign-off file present: **False**

- Reviewer sign-off path: `/Users/bosse/.firestaff/data/theron/promotion_review_state.json`

## Public Screenshot Boundary

- This checklist is the second guardrail for whether a Theron readiness row is reviewer-promotable.
- The promotion gate (`tools/verify_theron_v1_runtime_screenshot_promotion_gate.py`) is still the source-of-truth for the machine contract; the checklist only adds the explicit reviewer workflow on top.
- README, `verification-screens/`, and `docs/compare/` must not add Theron screenshots until at least one row is `REVIEWER_PROMOTED_README_ELIGIBLE` AND the upstream promotion gate is green AND a human reviewer has signed off.
- No generated, illustrated, mocked, or synthetic Theron image may be used as a README screenshot.

## Reviewer sign-off file shape

The operator-local reviewer sign-off file is a JSON object:

```json
{
  "schema": "firestaff.theron_v1_promotion_review_state.v1",
  "rows": [
    {
      "case_id": "canonical_pcengine_root",
      "reviewed": true,
      "reviewer": "<name>",
      "review_date": "YYYY-MM-DD",
      "review_notes": "<optional free-form notes>"
    }
  ]
}
```

Default path: `~/.firestaff/data/theron/promotion_review_state.json`.
Override via `FIRESTAFF_THERON_PROMOTION_REVIEW_STATE`. The file
is operator-local and must never be committed.

## Non-claims

- No Theron screenshot is promoted into README, verification-screens/, or docs/compare/ by this checklist.
- No generated, illustrated, or mock Theron image is created or copied.
- No claim that any current Theron readiness row is reviewer-signed-off or README-eligible.
- No claim of full Theron runtime playability or semantic Track 02 dungeon-table parity.
- The checklist is a non-promotion lock, not a release gate; future promotion must satisfy every checklist item and produce a reviewer sign-off for at least one row.

Manifest: `parity-evidence/verification/theron_v1_runtime_screenshot_promotion_checklist/manifest.json`
