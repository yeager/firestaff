# Pass179 DM1 V1 original evidence/capture integration gate

Status: `PASS_SOURCE_LOCKED_ORIGINAL_RUNTIME_BLOCKED_ON_ADDRESS_MAP`

This is the merge lane for the current big original-faithful blocker cluster. It is deliberately source-first: it verifies ReDMCSB movement/viewport/draw/click contracts and joins the existing capture/debugger artifacts, but it does **not** claim original pixel parity.

## What is now consolidated

- Capture-overlay recovery: `tooling-recovered-source-locked-not-overlay-ready` via `tools/verify_original_overlay_capture_source_lock.py` and classifier/route tools.
- Pass175 status: runtime gate `blocked/address-map-required`; queue probe `retired/firestaff-source-locked-c080-gate-passed-original-debugger-symbol-binding-blocked`.
- Debugger/address-map boundary: map original LZEXE/real-mode addresses for F0359 enqueue, F0380 dequeue, F0377 C080 handler, and F0280 candidate transition.
- Movement/viewport/walls: `PASS` with entry state `{'direction': 2, 'mapIndex': 0, 'mapX': 1, 'mapY': 3}` and 8 golden assertions.

## ReDMCSB source citations verified

- PASS `new-game-party-state` — LOADSAVE.C:1940-1945: Original new-game viewport state is decoded from DUNGEON_HEADER.InitialPartyLocation and starts on map 0.
- PASS `movement-vectors-relative-step` — DUNGEON.C:35-44, DUNGEON.C:1371-1391: Every relative party/viewport probe must rotate the facing direction first, then apply the original east/north vectors.
- PASS `movement-legality-and-cooldown` — CLIKMENU.C:156-173, CLIKMENU.C:237-347: Party turn/step commands mutate state only through source movement legality and then stop the wait loop for redraw.
- PASS `main-loop-draw-command-cadence` — GAMELOOP.C:90-90, GAMELOOP.C:150-155, GAMELOOP.C:215-219: Capture timing must respect draw-from-current-party-state, command queue processing, and cooldown decrement cadence.
- PASS `viewport-state-draw-request` — DUNVIEW.C:8318-8616: Dungeon-view cells are derived from the updated party direction/map coordinates before presentation.
- PASS `viewport-floor-ceiling-base` — DUNVIEW.C:2962-3003: Viewport draw order starts from black/ceiling/floor source bitmaps, not from a captured previous frame.
- PASS `wall-door-blits-into-viewport` — DUNVIEW.C:3048-3110: Wall/door assets share the same viewport bitmap target and must be compared as composited viewport content.
- PASS `object-creature-projectile-draw-stack` — DUNVIEW.C:4547-5113, DUNVIEW.C:5311-5316, DUNVIEW.C:5693-5700: Open-cell visual evidence must preserve the original object, creature, projectile/effect deferral order.
- PASS `viewport-present-seam` — DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: The comparison seam is the presented 224x136 viewport zone after the viewport draw request/vblank path.
- PASS `click-queue-c080-boundary` — COMMAND.C:1-16, COMMAND.C:1452-1662, COMMAND.C:2045-2127, COMMAND.C:2322-2324: Pass175 original-runtime work must bind addresses at enqueue, dequeue, C080 dispatch, and candidate transition; visual no-delta alone is not evidence.

## Remaining original-runtime blockers

1. Build or obtain an original DM.EXE address/symbol map for the pass175 gates: F0359 enqueue, F0380 dequeue, F0377 C080 handler, and F0280 candidate transition.
2. Produce a semantic-ready 320x200 original route capture after party/champion control is proven; source/tool recovery alone is not an overlay reference.
3. Only then run original-vs-Firestaff viewport/HUD/inventory pixel overlays from the source-locked movement state.

## Non-claims

- No DANNESBURK/192.168.2.126 use.
- No push.
- No original pixel parity claim.
- No claim that DOSBox has reached F0359/F0380/F0377/F0280 until the address-map gate is solved.
