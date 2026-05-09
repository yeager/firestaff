# Pass435 — DM1 V1 semantic original-route readiness gate

Status: `BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY`

## Verdict

Original viewport crop tooling is ready, but semantic original-route promotion remains blocked.

## ReDMCSB WIP20210206 source audit

- `CLIKMENU.C:142-174` `F0365_COMMAND_ProcessTypes1To2_TurnParty` — ok=`True`; turn commands must be observed reaching F0365, where stop-wait is set and party direction mutates
- `CLIKMENU.C:180-347` `F0366_COMMAND_ProcessTypes3To6_MoveParty` — ok=`True`; move commands must be observed reaching F0366, where target square, move result, and movement cooldown are committed
- `DUNVIEW.C:8318-8611` `F0128_DUNGEONVIEW_Draw_CPSF` — ok=`True`; viewport crops are promotable only when F0128 composes G0296 for a known direction/X/Y tuple
- `DRAWVIEW.C:709-858` `F0097_DUNGEONVIEW_DrawViewport` — ok=`True`; the capture seam must be the PC34 viewport present path, not setup/menu echo

## Runtime semantic proof carried forward

- pass385 status: `BLOCKED_PASS385_F0365_F0366_COMMAND_DISPATCH_NOT_PROVEN`
- F0380 command queue hit: `True`
- F0365/F0366 command dispatch hit: `False`
- G0321 stop-wait write observed: `True`
- later F0128 after stop-wait observed: `True`

## Artifact semantics

- pass434 readiness: `PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS`
- raw classifier sequence ok: `False`; classes: `['dungeon_gameplay', 'wall_closeup', 'dungeon_gameplay', 'dungeon_gameplay', 'wall_closeup', 'dungeon_gameplay']`
- raw duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 4, 'fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde': 2}`
- crop manifest rows_all_224x136: `True`
- crop duplicate hashes: `{'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c': 4, '1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81': 2}`

## Blockers

- runtime still does not prove F0365/F0366 command dispatch
- pass376 raw route classifier is not green
- pass376 raw route classes do not match the semantic promotion sequence
- pass376 raw route repeats screenshot hashes
- pass376 viewport crops repeat hashes, so labels are not semantically distinct

## Promotion rule

Promote only when pass434 readiness is green, a bounded post-load runtime proves F0380 plus F0365/F0366 command dispatch plus G0321 stop-wait write plus a later F0128 viewport draw, and six raw/cropped route states are non-duplicate and match dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory.

This gate does not launch DOSBox and does not claim pixel parity.

Manifest: `parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json`
