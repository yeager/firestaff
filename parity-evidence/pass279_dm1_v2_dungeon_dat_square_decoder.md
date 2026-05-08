# Pass279 — DM1 V2 DUNGEON.DAT square decoder feeding D0-D3 draw-list

## Scope

This pass advances the pass277 blocker by replacing the sparse entry-state-only path with a minimal canonical `DUNGEON.DAT` decoder for viewport composition input. It does **not** claim pixel parity.

## Source lock

Decoder/source anchors:

- `DEFS.H:922-941` — `M034_SQUARE_TYPE(square)` and square element definitions.
- `DEFS.H:972-1016` — `MAP.RawMapDataByteOffset` and PC/I34E `MAP.A` bitfields (`Level:6`, `Width:5`, `Height:5`).
- `LOADSAVE.C:906-923` — raw map data is loaded as `G0276_puc_DungeonRawMapData`; map columns are built from `RawMapDataByteOffset`, advancing by `Height + 1` per column.
- `DUNGEON.C:2238-2246` — raw stairs/door square types become front/side aspect based on orientation and direction.
- `DUNVIEW.C:8490-8542` — D3/D2/D1/D0 visible-square traversal order.

## Canonical data proved

`tools/verify_dm1_v2_dungeon_dat_square_decoder_source_lock.py` verifies the canonical PC34 file:

- path: `<firestaff-original-games>/_canonical/dm1/DUNGEON.DAT`
- SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- byte count: `33357`
- raw map data byte count: `12283`
- map count: `14`
- initial state: map `0`, x `1`, y `3`, direction `2` from raw `0x0861`
- map 0 dimensions: level `0`, width `18`, height `19`
- raw map data file offset: `21072`

## Landed implementation

- `DM1_V2_DungeonDatState` / `DM1_V2_DungeonDatMap` capture decoded header and map metadata.
- `dm1_v2_vp_dungeon_dat_init()` parses the canonical file layout and anchors raw map data at EOF minus checksum and `RawMapDataByteCount`.
- `dm1_v2_vp_dungeon_dat_get_square_raw()` decodes column-major raw squares using `x * height + y`.
- `dm1_v2_vp_square_element_from_raw()` maps raw square type/aspect into the existing V2 draw-list element vocabulary.
- `dm1_v2_vp_build_composition_from_dungeon()` feeds decoded D0-D3 squares into `dm1_v2_vp_emit_d0_d3_draw_list()`.

## Test evidence

`test_viewport_dungeon_dat_decoder_entry_draw_list` loads the canonical `DUNGEON.DAT` and proves:

- decoded entry state `0x0861` => map 0, x=1, y=3, direction=2;
- D0C/current square raw `0xB0` feeds a teleporter/object draw-list path;
- D1C/near front square raw `0x30` feeds a corridor/object path;
- decoded D3 side wall squares feed wall draw commands through the same D0-D3 compositor.

No original/ReDMCSB pixel capture or final viewport pixel parity is claimed here.
