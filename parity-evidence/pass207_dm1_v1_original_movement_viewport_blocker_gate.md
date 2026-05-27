# Pass207 — DM1 V1 original movement/viewport blocker gate

Status: `SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING`

Scope: N2-only focused follow-up to pass206. This gate does **not** rerun DOSBox or salvage broad captures; it records the exact ReDMCSB movement→viewport seam and explains whether the current original-runner attempt can be promoted.

## ReDMCSB source seam audit

- PASS `title-launch-draws-original-front-door` — `F0437_STARTEND_DrawTitle` at TITLE.C:12-40: Original route capture starts at the ReDMCSB title/front-door path, before any dungeon movement evidence is promotable.
- PASS `entrance-wait-processes-input-queue-before-load` — `F0441_STARTEND_ProcessEntrance` at ENTRANCE.C:850-883, ENTRANCE.C:906-943: A launch/capture route must cross the source entrance input-wait loop and post-enter delay before dungeon-entry frames are evidence.
- PASS `entrance-door-animation-vblank-seam` — `F0579_ENTRANCE_InitializeBitPlanes / F0580_ENTRANCE_DrawDoorAnimationStep / F0581_ENTRANCE_BlitDoors` at ENTRANCE.C:1095-1147: Entrance animation capture boundaries are tied to the original bitplane/vblank door-blit seam, not arbitrary screenshots.
- PASS `vblank-wait-is-explicit-capture-timing-seam` — `F0693_WaitVerticalBlank` at VBLANK.C:626-646: Original-faithful capture timing must respect the source vblank wait seam used by entrance, highlighting, and viewport presentation.
- PASS `command-discard-keypress-queue-primes-route` — `F0357_COMMAND_DiscardAllInput / F0361_COMMAND_ProcessKeyPress` at COMMAND.C:1305-1325, COMMAND.C:1709-1812: Route keypress/click evidence must begin from the source discard/input queue path before movement dispatch is evaluated.
- PASS `click-highlight-vblank-feedback-does-not-equal-movement` — `F0665_F0362_sub / F0363_COMMAND_HighlightBoxDisable` at CLIKMENU.C:8-35, CLIKMENU.C:83-95: Highlight/vblank feedback can appear in captures; it is not by itself proof that the movement handler or viewport redraw seam was reached.
- PASS `command-dispatch-reaches-turn-step-handlers` — `F0380_COMMAND_ProcessQueue_CPSC` at COMMAND.C:2045-2156: Movement/turn shots are only original-faithful after the command queue reaches the source turn/step handlers.
- PASS `turn-handler-mutates-direction-and-stops-wait` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` at CLIKMENU.C:142-179: Turn evidence must be captured after source direction mutation and input-wait release.
- PASS `step-handler-resolves-destination-and-cooldown` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` at CLIKMENU.C:180-347: Forward/side movement evidence must pass source legality, move-result, cooldown, and wait-release logic.
- PASS `game-loop-redraws-before-next-input-wait` — `F0002_MAIN_GameLoop_CPSDF` at GAMELOOP.C:35-97, GAMELOOP.C:215-219: The next viewport reference must come from the game-loop redraw using the mutated party state.
- PASS `viewport-draw-uses-direction-and-map-coordinates` — `F0128_DUNGEONVIEW_Draw_CPSF` at DUNVIEW.C:8318-8616: The comparable viewport is the source dungeon-view draw from current direction/X/Y, not a transient full-screen frame.
- PASS `viewport-present-requests-vblank-blit` — `F0097_DUNGEONVIEW_DrawViewport` at DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: A promotable capture must observe the presented 224x136 viewport after the source draw-request/vblank seam.
- PASS `viewport-floor-ceiling-clears-base-before-walls` — `F0098_DUNGEONVIEW_DrawFloorAndCeiling` at DUNVIEW.C:2962-3003: Wall evidence must be compared after the original viewport base is cleared and floor/ceiling are copied into the viewport bitmap.
- PASS `wall-door-bitmaps-composite-into-same-viewport-target` — `F0100_DUNGEONVIEW_DrawWallSetBitmap / F0102_DUNGEONVIEW_DrawDoorBitmap / F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally` at DUNVIEW.C:3048-3110: Wall and door captures are viewport-composited wallset evidence, not separate asset screenshots or emulator guesses.

## Capture asset manifest boundary

- status: `MANIFESTS_SOURCE_LOCK_CAPTURE_ASSETS_WITHOUT_EMULATOR_GUESSING`
- stable raw frames: `6`; labels: `6`; missing materialized ignored PNGs: `6`
- stable duplicate raw SHA counts >1: `{'ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c': 4}`; blank labels: `['02_ingame_turn_right_original_viewport_224x136.ppm', '03_ingame_move_forward_original_viewport_224x136.ppm', '05_ingame_after_cast_original_viewport_224x136.ppm']`
- route-probe 224x136 viewport crops: `6`; missing materialized ignored PPMs: `6`
- route-probe duplicate viewport SHA counts >1: `{'e0b1843b4342d09408dd92e52484a578da3875c8392d690a694f23ff2080c844': 3}`; duplicate raw SHA counts >1: `{'307323fbc1f7772cb259160f2f988a482981e33a339e4414b0bfc8c85f8d4bd0': 3}`
- route-probe blank labels: `['02_ingame_turn_right_original_viewport_224x136.ppm', '03_ingame_move_forward_original_viewport_224x136.ppm', '05_ingame_after_cast_original_viewport_224x136.ppm']`
- boundary: Tracked TSV manifests preserve filenames, dimensions, labels, and sha256 for ignored PNG/PPM capture assets. They unblock reproducible asset requests, but do not promote the route until semantic classifier/pass206 is clean and command-specific shots have distinct post-vblank viewport hashes.

## Current N2 original-runner attempt

- pass206 manifest: `/Users/bosse/.openclaw/workspace-main/parity-evidence/verification/pass206_dm1_v1_original_runner_minimal_gate/manifest.json`
- pass206 status: `SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING`
- attempt status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`
- attempt dir: `/home/trv2/work/firestaff/verification-screens/pass112-n2-stable-hud-route`
- capture count / dimensions: `6` / `{'320x200': 6}`
- viewport crop PPM count: `0`
- class counts: `{'dungeon_gameplay': 2, 'wall_closeup': 4}`
- duplicate SHA counts >1: `{'ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c': 4}`
- missing tools: `[]`
- canonical files ok: `{'DATA/DUNGEON.DAT': True, 'DATA/GRAPHICS.DAT': True, 'DM.EXE': True}`

## Blocker decision

The old movement/viewport route blocker is retired as an active blocker by pass304/pass308 batch capture coverage. The remaining active blocker is state-oracle proof for binding original runtime party tuple/F0128 to those route labels; no pixel parity is claimed.

Non-claims: no <private-host> use, no push, no new capture route, no original-vs-Firestaff pixel parity claim.
