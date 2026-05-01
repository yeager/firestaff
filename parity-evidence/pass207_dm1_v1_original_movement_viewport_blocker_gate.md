# Pass207 — DM1 V1 original movement/viewport blocker gate

Status: `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE`

Scope: N2-only focused follow-up to pass206. This gate does **not** rerun DOSBox or salvage broad captures; it records the exact ReDMCSB movement→viewport seam and explains whether the current original-runner attempt can be promoted.

## ReDMCSB source seam audit

- PASS `command-dispatch-reaches-turn-step-handlers` — `F0380_COMMAND_ProcessQueue_CPSC` at COMMAND.C:2045-2156: Movement/turn shots are only original-faithful after the command queue reaches the source turn/step handlers.
- PASS `turn-handler-mutates-direction-and-stops-wait` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` at CLIKMENU.C:142-179: Turn evidence must be captured after source direction mutation and input-wait release.
- PASS `step-handler-resolves-destination-and-cooldown` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` at CLIKMENU.C:180-347: Forward/side movement evidence must pass source legality, move-result, cooldown, and wait-release logic.
- PASS `game-loop-redraws-before-next-input-wait` — `F0002_MAIN_GameLoop_CPSDF` at GAMELOOP.C:35-97, GAMELOOP.C:215-219: The next viewport reference must come from the game-loop redraw using the mutated party state.
- PASS `viewport-draw-uses-direction-and-map-coordinates` — `F0128_DUNGEONVIEW_Draw_CPSF` at DUNVIEW.C:8318-8616: The comparable viewport is the source dungeon-view draw from current direction/X/Y, not a transient full-screen frame.
- PASS `viewport-present-requests-vblank-blit` — `F0097_DUNGEONVIEW_DrawViewport` at DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: A promotable capture must observe the presented 224x136 viewport after the source draw-request/vblank seam.

## Current N2 original-runner attempt

- pass206 manifest: `/home/trv2/work/firestaff/parity-evidence/verification/pass206_dm1_v1_original_runner_minimal_gate/manifest.json`
- pass206 status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`
- attempt status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`
- attempt dir: `/home/trv2/work/firestaff/verification-screens/pass112-n2-stable-hud-route`
- capture count / dimensions: `6` / `{'320x200': 6}`
- viewport crop PPM count: `6`
- class counts: `{'dungeon_gameplay': 2, 'wall_closeup': 4}`
- duplicate SHA counts >1: `{'ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c': 4}`
- missing tools: `[]`
- canonical files ok: `{'DATA/DUNGEON.DAT': True, 'DATA/GRAPHICS.DAT': True, 'DM.EXE': True}`

## Blocker decision

The current attempt is **not promotable** as original-faithful movement/viewport evidence. It has the right local runner prerequisites, but the captured sequence does not remain semantically aligned after movement: classifier mismatches and repeated wall-closeup frames mean the shots cannot be tied to the ReDMCSB post-command redraw seam above.

Mismatches:
- shot 3: `wall_closeup` expected `dungeon_gameplay` (`verification-screens/pass112-n2-stable-hud-route/image0003-raw.png`)
- shot 4: `wall_closeup` expected `spell_panel` (`verification-screens/pass112-n2-stable-hud-route/image0004-raw.png`)
- shot 5: `wall_closeup` expected `dungeon_gameplay` (`verification-screens/pass112-n2-stable-hud-route/image0005-raw.png`)
- shot 6: `wall_closeup` expected `inventory` (`verification-screens/pass112-n2-stable-hud-route/image0006-raw.png`)

Non-claims: no DANNESBURK use, no push, no new capture route, no original-vs-Firestaff pixel parity claim.
