# Pass180 DM1 V1 Elija route source gate

Status: `PASS_SOURCE_LOCKED_ROUTE_RUNTIME_INPUT_STILL_BLOCKED`

This is a source-locked route gate only. It proves the semantic Hall-of-Champions target for the first champion route and keeps the original runtime capture blocker honest.

## Verified source/data contracts

- PASS `new-game-state-decoded-from-dungeon-header` — LOADSAVE.C:1940-1944
- PASS `east-facing-forward-vector` — DUNGEON.C:35-44,1371-1391
- PASS `greatstone-start-state` — 0000.DUNGEON [Dungeon].xml:69-72
- PASS `greatstone-floor-corridor-x3-through-x9-y2` — 0000.DUNGEON [Dungeon].xml:8542-8598
- PASS `greatstone-elija-portrait-wall-10-2-west-side` — 0000.DUNGEON [Dungeon].xml:8602-8626

## Route contract

- Start: map 0, x=3, y=2, facing East.
- Source route: six forward semantic steps to map 0, x=9, y=2, facing East.
- Interaction target: champion portrait wall at map 0, x=10, y=2, west side, champion id 0 / Elija.

## Remaining blocker

- original-runtime input/address route still must prove a non-static party-control capture; no pixel parity claim is made here.
- `tools/pass113_original_party_state_probe.py` remains the semantic readiness gate for any runtime capture.
- Do not promote direct-start/no-party or static duplicate frames as original references.

## Non-claims

- No DOSBox runtime success is claimed.
- No original-vs-Firestaff pixel parity is claimed.
- No original assets are copied into the repository.

## Verification output

- Manifest: `parity-evidence/verification/pass180_dm1_v1_elija_route_source_gate/manifest.json`.
