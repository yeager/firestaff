# Pass171 — ReDMCSB source-lock closure

## Result

- status: **closed**
- pass168 total pass groups: **268**
- pass168 directly source-locked: **211**
- pass169 resolved `needs-redmcsb-anchor`: **11**
- pass170 resolved `source-mentioned-unresolved`: **46**
- closed: **268/268**
- remaining: **0**

## Inputs

- `parity-evidence/verification/pass168_redmcsb_pass_source_map/manifest.json`
- `parity-evidence/verification/pass169_redmcsb_anchor_gap_resolution/manifest.json`
- `parity-evidence/verification/pass170_source_mentioned_unresolved_batch/manifest.json`

## Interpretation

The full historical pass corpus covered by pass168 now has a ReDMCSB source-lock path: direct anchors for 211 groups, explicit addendum anchors for the 11 weak/missing groups, and lane-level source anchors for the 46 source-mentioned unresolved groups.
