# Pass389 — DM1 V1 keyboard producer path

Status: `BLOCKED_PASS389_MOVEMENT_KEYS_NOT_REACHING_F0361`

## Decision

armed movement route ran, but no movement key reached F0361; keyboard buffer/input delivery is before the command producer

## Source-locked producer path

- `GAMELOOP.C` drains `M527_IsCharacterInKeyboardBuffer()` into `F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())`.
- `STARTUP2.C` enables `G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement` during gameplay.
- In PC34, `G0459` maps movement commands to key codes `0x004B`, `0x004C`, `0x004D`, `0x004F`, `0x0050`, and `0x0051`; `F0361` writes `G0432_as_CommandQueue[...]` and increments `G2153_i_QueuedCommandsCount` only after exact key-code equality.
- `F0380` can dispatch only after `G2153_i_QueuedCommandsCount != 0`; otherwise it follows the empty-queue bypass already proven in pass387.

## Runtime predicates

- `sourceAuditOk`: `True`
- `routeRanAfterArm`: `True`
- `movementKeyReachedF0361`: `False`
- `movementF0361KeyCodes`: `[]`
- `movementF0361KeyCodeMatchesPc34MovementTable`: `False`
- `queueCountChanged`: `False`
- `f0380Reached`: `True`

## Evidence

- Manifest: `parity-evidence/verification/pass389_dm1_v1_keyboard_producer_path/manifest.json`
- Transcript: `parity-evidence/verification/pass389_dm1_v1_keyboard_producer_path/pass389_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass389_dm1_v1_keyboard_producer_path/pass389_route_keylog.json`
