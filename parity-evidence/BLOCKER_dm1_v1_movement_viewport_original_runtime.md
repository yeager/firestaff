# BLOCKER — DM1 V1 movement/viewport original-runtime decision

Status: `DECISION_REQUIRED_ORIGINAL_CAPTURE_PROOF`

Pass500 closes the source-lock gap for viewport/wall occlusion, but it does not prove original DOSBox pixel/state parity.

## Option A — accept source/probe/runtime boundary evidence now
- Use Pass398, Pass404, Pass496, Pass499, and Pass500 as sufficient for movement/viewport/walls merge readiness.
- Benefit: unblocks the DM1 V1 lane without chasing brittle DOSBox-debug stops.
- Cost: no fresh original state-delta/pixel-crop promotion for the repeated static gameplay frame family.

## Option B — require a new N2 DOSBox-debug capture before promotion
- Run an owned-PTY N2 capture that proves post-command state deltas after controlled movement/click input and binds them to F0365/F0366 -> F0128 -> F0097.
- Benefit: stronger original-runtime proof.
- Cost: this is the current blocker family (`pass357`, `pass360`, `pass487`, `pass497`, `pass498`) and may require live FIRES CS:IP/source-bound locator work.

## Recommendation

Pick Option A for source-code merge-readiness, and keep Option B as a follow-up pixel/original-runtime evidence task. The remaining blocker is evidence policy, not a ReDMCSB-backed product-code fix.
