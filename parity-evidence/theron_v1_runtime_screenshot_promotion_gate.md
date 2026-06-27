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
| canonical_pcengine_root | `README_INELIGIBLE` | `9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5` | sourceId=theron launchedEver=1 gameTick=0 mapIndex=0 lastOutcome='THERON READY' | row used 'deterministic fallback assets'; placeholder art cannot be promoted as a real Theron Track 02 README screenshot; no semantic Track 02 loader evidence in probe (gameTick=0, party.mapIndex=0, lastOutcome='THERON READY'); boot probe parked at THERON READY without decoding Track 02 content; presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels); sha256 9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5 shared with rows ['canonical_pcengine_root', 'jp_extras_track02_bin', 'us_extras_track02_bin']; ineligible |
| jp_extras_track02_bin | `README_INELIGIBLE` | `9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5` | sourceId=theron launchedEver=1 gameTick=0 mapIndex=0 lastOutcome='THERON READY' | row used 'deterministic fallback assets'; placeholder art cannot be promoted as a real Theron Track 02 README screenshot; no semantic Track 02 loader evidence in probe (gameTick=0, party.mapIndex=0, lastOutcome='THERON READY'); boot probe parked at THERON READY without decoding Track 02 content; presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels); sha256 9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5 shared with rows ['canonical_pcengine_root', 'jp_extras_track02_bin', 'us_extras_track02_bin']; ineligible |
| us_extras_track02_bin | `README_INELIGIBLE` | `9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5` | sourceId=theron launchedEver=1 gameTick=0 mapIndex=0 lastOutcome='THERON READY' | row used 'deterministic fallback assets'; placeholder art cannot be promoted as a real Theron Track 02 README screenshot; no semantic Track 02 loader evidence in probe (gameTick=0, party.mapIndex=0, lastOutcome='THERON READY'); boot probe parked at THERON READY without decoding Track 02 content; presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels); sha256 9a1f804df318fb8d8643eb023995f77f808289feb059614246d8f787af609cf5 shared with rows ['canonical_pcengine_root', 'jp_extras_track02_bin', 'us_extras_track02_bin']; ineligible |

## Aggregate

- Eligible rows: **0**
- Ineligible rows: **3**
- Skipped rows (no data on this host): **0**
- Unique-source-sha256 findings: **1**
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
