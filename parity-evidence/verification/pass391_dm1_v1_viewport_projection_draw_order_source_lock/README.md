# Pass391 DM1 V1 viewport projection/draw-order source lock

Source root audited first: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

## Locked facts

- `F0128_DUNGEONVIEW_Draw_CPSF` is the source draw pipeline for the DM1 V1 dungeon viewport.
- Map projection is not screen-space inference: `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` mutates map X/Y from `(direction, steps forward, steps right)`, and `F0152/F0153` use that to fetch relative squares/types.
- `F0128` paints base floor/ceiling first, then replays visible squares in far-to-near order:
  - D4 objects: `D4L`, `D4R`, `D4C`
  - D3 squares: `D3L`, `D3R`, `D3C`
  - D2 squares: `D2L`, `D2R`, `D2C`
  - D1 squares: `D1L`, `D1R`, `D1C`
  - D0 squares: `D0L`, `D0R`, `D0C`
  - final viewport present via `F0097_DUNGEONVIEW_DrawViewport`.
- Viewport bounds/zones are source-defined: viewport bitmap is 224x136, presented through `C007_ZONE_VIEWPORT`; floor/ceiling/wall/object routes use `C700+`, `C2500+`, and `C2548+` zone families.
- Wall/object handoff is explicit: wall squares draw wall art and normally return; alcoves and open/door squares call `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` with encoded cell-order nibbles.
- Door-front squares are two-pass: rear contents, then door/frame, then front contents. A flat depth sort would not preserve this source order.

## Source anchors verified

- `DUNGEON.C:1371-1510` — relative coordinate projection and relative square/type lookup.
- `DEFS.H:2568-4234` — view-square constants and viewport/wall/object zone IDs.
- `COORD.C:1693-1731` + `DRAWVIEW.C:709-858` — 224x136 viewport bitmap and final present through `C007_ZONE_VIEWPORT`.
- `DUNVIEW.C:8318-8618` — `F0128` floor/ceiling, D4→D0 replay, and presentation.
- `DUNVIEW.C:6400-6488` — wall return / alcove handoff and door-front two-pass object handoff.
- `DUNVIEW.C:4547-5215` — `F0115` encoded cell-order processing and D4 detail cutoff.
- `DUNVIEW.C:8200-8312` — D0C near-square handoff and optional field overlay.

## Gate

Run:

```sh
python3 scripts/verify_pass391_dm1_v1_viewport_projection_draw_order_source_lock.py
```

This verifier checks ordered anchors and emits SHA256 digests for each source slice, without embedding long raw source excerpts here.
