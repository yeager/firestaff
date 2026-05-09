# Pass373 — DM1 V1 launcher viewport redraw wall/occlusion path

Status: `PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH_PROVED`

## Verdict

The full Firestaff launcher route-token path reaches a live DM1 V1 turn, marks the viewport dirty, and source-locks the consequent redraw path into the normal V1 wall/door/occlusion renderer stack.

## Runtime proof

- Script: enter,down,down,down,down,down,down,enter,right,up
- Probe JSON: parity-evidence/verification/pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path/launcher_route_viewport_redraw_probe.json
- Party: {"championCount": 0, "direction": 3, "mapIndex": 0, "mapX": 1, "mapY": 3}
- Pipeline: {"anyMovementOccurred": 0, "anyTurnOccurred": 1, "command": 2, "dequeued": 1, "movementBlocked": 0, "stepApplied": 0, "turnApplied": 1, "viewportDirty": 1}

## ReDMCSB source audit anchors

- DUNVIEW.C:2962-3003 — F0098_DUNGEONVIEW_DrawFloorAndCeiling: viewport floor/ceiling base is drawn into G0296/G0087 before wall passes
- DUNVIEW.C:3048-3082 — F0100/F0101/F0102 blitters: wall, opaque wall, and door bitmaps target the viewport bitmap
- DUNVIEW.C:4547-4910 — F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF: ordered cell contents use source cell-order nibbles and normalized view cells
- DUNVIEW.C:6400-6835 — F0116/F0117/F0118 D3 side/center squares: D3 wall/doorpass branches draw wall panels and return to enforce occlusion
- DUNVIEW.C:7244-7937 — F0121/F0124 D2C/D1C center squares: center wall/door branches draw opaque/door layers before ordered contents
- DUNVIEW.C:8318-8618 — F0128_DUNGEONVIEW_Draw_CPSF: main viewport pass samples view squares, draws far-to-near D3/D2/D1/D0, then requests viewport blit
- DRAWVIEW.C:709-722 — F0097_DUNGEONVIEW_DrawViewport: source draw pass flips the viewport redraw request and waits for vblank

## Firestaff path anchors

- main_loop_m11.c — script token route feeds the active game view and redraw result calls M11_GameView_Draw
- main_loop_m11.c — runtime probe records the same live movement pipeline result used by the redraw route
- m11_game_view.c — input dispatch enters DM1 V1 compat command pipeline and returns redraw for dirty viewport
- m11_game_view.c — normal V1 viewport renderer draws source-backed wall/occlusion layers before debug-only procedural fallback

## Gates

- firestaff_viewport_order_lock ok=True
- prior_wall_occlusion_gate ok=True
- prior_wall_occlusion_gate ok=True
- prior_wall_occlusion_gate ok=True
- prior_wall_occlusion_gate ok=True
- cmake_configure ok=True
- cmake_build_firestaff ok=True
- launcher_route_runtime_probe ok=True
- live_runtime_redraw_state ok=True

## Scope guard

- This is not original DOS keyboard-buffer/NumLock parity.
- This is not a DOSBox/FIRES debugger-hit proof.
- This is not pixel-perfect viewport parity.
