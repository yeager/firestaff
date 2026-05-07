# Pass273 — DM1 V1 DUNVIEW D1C door-pass source lock

Status: source-locked narrow gate; **not** complete DM1 V1 viewport parity.

## Source audit first

Primary source audited on N2 only:
`<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

ReDMCSB anchors:

- `DUNVIEW.C:371-377` defines the view-square lane/depth/field mapping tables used by D0-D4 viewport gates.
- `DUNVIEW.C:4547-4582` documents `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: objects are drawn first, creatures are deferred, projectiles are drawn after creatures, explosions are final.
- `DUNVIEW.C:7800-7937` (`F0124_DUNGEONVIEW_DrawSquareD1C`) locks the high-risk D1 center square order:
  - `C17_ELEMENT_DOOR_FRONT` draws `F0108_DUNGEONVIEW_DrawFloorOrnament`, then `F0115(... C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT)`, then door frame pieces/button/door slab, then sets `C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT` and jumps to shared `T0124018` for the second `F0115` pass.
  - pit/teleporter/corridor sets `C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT`, draws floor ornament and ceiling pit, then uses the same `T0124018` `F0115` call.
- `DUNVIEW.C:8164-8294` (`F0127_DUNGEONVIEW_DrawSquareD0C`) locks D0C as non-doorpass: door-side frame/stairs/pit first, ceiling pit, then one `F0115(... C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT)`.
- `DUNVIEW.C:8466-8542` (`F0128_DUNGEONVIEW_Draw_CPSF`) locks whole viewport traversal as D4 objects, optional D3/D2 outer side helpers, then D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C.
- `DRAWVIEW.C:709-722` and `DRAWVIEW.C:821-858` remain the present/blit seam after the composed viewport buffer.

## Invariant covered

The new `v1_viewport_d1c_doorpass_source_lock_gate` protects one exact ordering invariant:

`D1C door-front = floor ornament -> F0115 doorpass1 back cells -> door frame/button/door slab -> F0115 doorpass2 front cells`.

This catches the highest-risk batching mistake for near center doors: drawing all center-door slabs/buttons as one primitive class before/after all contents is not equivalent to DUNVIEW.C when contents straddle the door plane.

## Firestaff state guarded

Firestaff `m11_draw_viewport` is still class-batched with a near-side replay guard, not a full per-square DUNVIEW stack replay. The gate deliberately verifies that this gap remains documented in-code so we do not silently promote the current renderer to complete parity.

## Remaining blockers

- Implement/replay the complete DUNVIEW.C per-square stack for D3/D2/D1/D0, especially front-door pass1/pass2 ordering and wall/ornament/door frame interleaving.
- Lock exact wall-ornament and door-ornament coordinate replay for every D0-D3 view index.
- Lock creature/object half-square ordering for all `F0115` cell-order constants, not only the broad object -> creature -> projectile stack.
- Runtime global-state capture and original PC34 `DUNVIEW.OBJ`/`FIRES.MAP` route remain separate blockers.
