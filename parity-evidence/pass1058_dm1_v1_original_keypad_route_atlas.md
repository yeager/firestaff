# Pass1058 DM1 V1 original keypad route atlas

Status: `PASS1058_ORIGINAL_KEYPAD_ROUTE_ATLAS_LOCKED`

This pass preserves the original DM1 PC 3.4 DOSBox keypad mapping that the
next creature-route capture must use. It also records the first corrected
level-1 creature route attempt and why that attempt is not promotable as a
creature evidence closure.

## Source run

- Canonical game: Dungeon Master PC DOS English v3.4
- Capture harness: DOSBox original-frame route runner plus pass80 classifier
- Evidence directory:
  `parity-evidence/verification/pass1058_dm1_v1_original_keypad_route_atlas/`

## Keypad atlas

The original runtime was checked from the same start pose:

| Action | Result |
|---|---|
| `kp5` | same frame as forward |
| `kp8` | same frame as forward |
| viewport forward click | same frame as forward |
| `kp2` | returns to the start frame |
| `kp4` | turns right from the start pose |
| `kp6` | turns left / returns from the right-turn pose |

This means route scripts that use the original DOSBox keypad path must map:

| Route token | Key |
|---|---|
| `F` | `kp8` |
| `B` | `kp2` |
| `TR` | `kp4` |
| `TL` | `kp6` |

## Corrected route attempt

After applying the corrected keypad mapping, the long route reaches distinct
original states:

| Label | Raw SHA256 |
|---|---|
| `start` | `7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f` |
| `stair_entry` | `b235ea66987020ac21113543d5fcf0291b53060d812d61536d2bc8a1bdfea570` |
| `creature_door_closed` | `87271b0649d8a00500a2284cc69b6a6b275f1ce8e3d27607c346393e997dd4ed` |
| `creature_after_door_click` | `87271b0649d8a00500a2284cc69b6a6b275f1ce8e3d27607c346393e997dd4ed` |

The route is therefore real and no longer stuck at the selector/entrance, but
the first chosen target remains blocked by a closed/inert door. A follow-up
door probe tried enter, space, two door clicks, and a forward key after reaching
the same frame. All post-door raw frames remained byte-identical.

## Promoted evidence

- Original keypad direction semantics are now locked for future DOSBox route
  work.
- The corrected route reaches map-transition/new-state evidence instead of
  duplicate start frames.
- The first selected creature target is documented as a blocker because the
  original view remains unchanged after all tested door/open/forward attempts.

## Non-claims

- This is not a paired original creature screenshot.
- This does not close the creature-chain row in
  `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`.
- This does not prove Firestaff-vs-original pixel parity for map 1.
- The next capture attempt should choose a level-1 group with open line of
  sight or use a controlled save/debug route, rather than this inert-door path.
