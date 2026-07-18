# Pass1055 DM1 V1 original closed-door collision gate

Status: `PASS1055_ORIGINAL_CLOSED_DOOR_COLLISION_GATE`

This gate makes the pass1055 closed-door stasis evidence reproducible.
It verifies original raw-frame stasis, original 224x136 viewport stasis,
the pass513 scaffold boundary, and the Firestaff-side semantic closed-door
probe. It is not a Firestaff-vs-original pixel comparison.

## Result

- Raw closed-door stasis: `True`
- Viewport closed-door stasis: `True`
- Pass513 remains scaffold-only: `True`
- Firestaff closed-door pair probe: `True`

## Non-claims

- This does not prove all wall/door/fakewall collision parity.
- This does not add a Firestaff-vs-original pixel diff.
- This does not unblock the creature-chain capture route.

Manifest: `parity-evidence/verification/pass1055_dm1_v1_original_closed_door_collision_capture/manifest.json`
