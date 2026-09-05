# DM2 V1 original-overlay pair readiness

Status: `FAIL_ORIGINAL_CAPTURE_INCOMPLETE`

This gate checks whether operator-local H2313 original viewport crops
and Firestaff-side 224x136 viewport crops are ready to be compared as
same-state DM2 original-overlay evidence. It is a readiness gate only;
it does not claim pixel parity.

## Inputs

- Original attempt: `verification-screens/passH2313-dm2-original-overlays`
- Firestaff pair manifest: `parity-evidence/dm2_v1_original_overlay_pair_manifest.json`

## Result

- Original manifests present: `True`
- Original crop rows OK: `False`
- Firestaff pair manifest present: `False`
- Firestaff pair rows OK: `False`
- Dungeon gameplay pairs: `0`

## Boundary

- Missing operator-local captures keep the gate OPEN-BOUNDED and passing.
- Present but malformed captures or pair manifests fail the gate.
- Promotion still requires reviewed original bytes plus same-route Firestaff bytes.
