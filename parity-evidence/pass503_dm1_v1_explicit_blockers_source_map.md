# Pass503 — DM1 V1 explicit blockers source map

Status: `PASS503_DM1_V1_EXPLICIT_BLOCKERS_SOURCE_MAP_VERIFIED`

Covered Daniel blockers: door with buttons, title animation, end animation.

Audited ReDMCSB anchors:
- `TITLE.C:309-410` — title-animation-pc-f20-cadence — `passed`
- `ENTRANCE.C:739-944` — entrance-door-buttons-source-flow — `passed`
- `ENTRANCE.C:142-304` — entrance-door-open-31-step-vblank-loop — `passed`
- `MOVESENS.C:1309-1550` — wall-click-sensor-button-semantics — `passed`
- `ENDGAME.C:742-923` — end-animation-fuse-sequence-cadence — `passed`
- `VBLANK.C:35-64,626-646` — vblank-wait-contract-for-frontend-cadence — `passed`

Firestaff local seams checked: title frontend cadence, entrance door-step model, pass486 door-button occlusion evidence, endgame fuse-sequence gate, and VBlank timing seam.

Guardrail: this is source/evidence only; it does not claim original pixel/runtime parity.
