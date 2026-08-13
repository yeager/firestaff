# Theron V1 runtime screenshot promotion gate

Status: `NO_README_PROMOTION_PERMITTED`

Decision: **NO_README_PROMOTION_PERMITTED**

This is a bounded, machine-checkable provenance gate for promoting
Theron runtime screenshots into public docs. It does not promote any
image, copy bytes, or rewrite the README. It only audits the existing
readiness manifest against an explicit eligibility contract.

## Eligibility contract

A row is `README_ELIGIBLE` only if **all** of the following hold:

1. Readiness row `status == "PASS"` (real Firestaff boot/tick).
2. Probe `sourceId == "theron"`, `launchedEver == 1`, and `schema == "firestaff_m11_autotest_runtime_probe.v1"`.
3. Launch output does **not** contain `deterministic fallback assets`.
4. Launch output contains the `TQR level load` boot marker.
5. Probe shows semantic Track 02 loader evidence (`gameTick > 0`, non-zero `party.mapIndex`, or a `lastOutcome` beyond `THERON READY`).
6. Source BMP sha256 is unique across all rows (duplicate sha256s indicate a shared placeholder fixture).
7. Presented BMP is a valid 320x200 24-bit BMP with more than 200 non-black pixels.

Rows that fail any criterion are recorded as `README_INELIGIBLE`. SKIP is
returned for rows whose data directory is missing on this host.

## Row classification

| Case | Classification | Source sha256 | Probe summary | Notes |
|---|---|---|---|---|
| canonical_pcengine_root | `README_INELIGIBLE` | `a70034eeabbfab4da3e6e2846a111fcf41c08c21ab17bf0228cd23255be7074f` | sourceId=theron launchedEver=1 gameTick=0 mapIndex=0 lastOutcome='THERON RUNTIME (TRACK 02 DUNGEON)' | readiness row status is 'FAIL', not PASS; presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels) |
| jp_extras_track02_bin | `SKIPPED_NO_DATA` | `—` | sourceId=None launchedEver=None gameTick=None mapIndex=None lastOutcome=None | data directory missing; row skipped, not promoted |
| us_extras_track02_bin | `SKIPPED_NO_DATA` | `—` | sourceId=None launchedEver=None gameTick=None mapIndex=None lastOutcome=None | data directory missing; row skipped, not promoted |

## Aggregate

- Eligible rows: **0**
- Ineligible rows: **1**
- Skipped rows (no data on this host): **2**
- Unique-source-sha256 findings: **0**
- Contract-drift findings: **0**

## Public Screenshot Boundary

- This gate is the source of truth for whether a Theron readiness row is `README_ELIGIBLE`.
- README, `verification-screens/`, and `docs/compare/` must not add Theron screenshots until at least one readiness row is `README_ELIGIBLE` AND a human reviewer promotes it from tracked evidence.
- No generated, illustrated, mocked, or synthetic Theron image may be used as a README screenshot.

## Non-claims

- No Theron screenshot is promoted into README, verification-screens/, or docs/compare/ by this gate.
- No generated, illustrated, or mock Theron image is created or copied.
- No claim that any current Theron readiness row is README-eligible.
- No claim of full Theron runtime playability or semantic Track 02 dungeon-table parity.
- The contract is a non-promotion lock, not a release gate; future promotion must satisfy every eligibility criterion in the contract.

Manifest: `parity-evidence/verification/theron_v1_runtime_screenshot_promotion_gate/manifest.json`
