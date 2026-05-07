# Pass323 — DM1 V1 wall graphics offset binding

Status: `WALL_GRAPHICS_OFFSET_BINDING_SOURCE_DATA_LOCKED_RUNTIME_BLOCKED`

## Verdict

Wall graphics offsets are source/data locked: ReDMCSB binds PC wall ordinals to `G2107_WallSet`, pass305 binds those slots to GRAPHICS.DAT indices `93..107`, and DUNVIEW frame tables bind viewport source/destination offsets. Runtime promotion remains blocked by the pass320 strict-stop issue.

## Checks

- PASS `source_audit_ok`
- PASS `tiledefs_absence_recorded`
- PASS `pass305_wall_records_ok`
- PASS `g2107_wall_order_ok`
- PASS `frame_offsets_extracted`
- PASS `greatstone_pc34_graphics_listed`
- PASS `pass318_blocker_preserved`
- PASS `pass320_blocker_preserved`
- PASS `vidrv_candidate_bound`

## Source audit

- PASS `DRAWVIEW.C:709-858` — drawview_pc_viewport_present
- PASS `VIDEODRV.C:941-957` — videodrv_slot9_viewport_blit
- PASS `VIDEODRV.C:3566-3582` — videodrv_vga_color_offset_scope
- PASS `DEFS.H:3423-3437` — tiledefs_substitute_view_square_wall_ordinals
- PASS `DEFS.H:4042-4057` — tiledefs_substitute_zone_wall_ordinals
- PASS `DEFS.H:2359-2373` — wall_graphics_constants
- PASS `DUNVIEW.C:183-200` — wallset_negative_bitmap_slots
- PASS `DUNVIEW.C:577-594` — wall_frames_and_offsets
- PASS `DUNVIEW.C:2214-2217` — wallset_load_loop
- PASS `DUNVIEW.C:3048-3074` — wall_bitmap_draw_helpers
- PASS `DUNVIEW.C:6699-6714` — wall_center_draw_sites
- PASS `DUNVIEW.C:7291-7306` — wall_d2_draw_sites

## TILEDEFS.C

- `NOT_PRESENT_NONBLOCKING_DEFS_H_DUNVIEW_ARE_TILEDEF_AUTHORITY` — This ReDMCSB tree has no TILEDEFS.C; PC wall/tile ordinals and zones are in DEFS.H, while wall-set bitmap tables/load/draw offsets are in DUNVIEW.C.

## Wall slot bindings

- 00 `D0R`: G2107 `-17` -> GRAPHICS.DAT `93` (33x136)
- 01 `D0L`: G2107 `-16` -> GRAPHICS.DAT `94` (33x136)
- 02 `D1R`: G2107 `-15` -> GRAPHICS.DAT `95` (60x111)
- 03 `D1L`: G2107 `-14` -> GRAPHICS.DAT `96` (60x111)
- 04 `D1C`: G2107 `-13` -> GRAPHICS.DAT `97` (160x111)
- 05 `D2R2`: G2107 `-9` -> GRAPHICS.DAT `98` (8x52)
- 06 `D2L2`: G2107 `-8` -> GRAPHICS.DAT `99` (8x52)
- 07 `D2R`: G2107 `-12` -> GRAPHICS.DAT `100` (78x74)
- 08 `D2L`: G2107 `-11` -> GRAPHICS.DAT `101` (78x74)
- 09 `D2C`: G2107 `-10` -> GRAPHICS.DAT `102` (106x74)
- 10 `D3R2`: G2107 `-4` -> GRAPHICS.DAT `103` (44x49)
- 11 `D3L2`: G2107 `-3` -> GRAPHICS.DAT `104` (44x49)
- 12 `D3R`: G2107 `-7` -> GRAPHICS.DAT `105` (83x49)
- 13 `D3L`: G2107 `-6` -> GRAPHICS.DAT `106` (83x49)
- 14 `D3C`: G2107 `-5` -> GRAPHICS.DAT `107` (70x49)

## Frame offsets

- `D3C`: dst x `74..149`, y `25..75`, byteWidth `64`, height `51`, src `18,0`
- `D3L`: dst x `0..83`, y `25..75`, byteWidth `64`, height `51`, src `32,0`
- `D3R`: dst x `139..223`, y `25..75`, byteWidth `64`, height `51`, src `0,0`
- `D2C`: dst x `60..163`, y `20..90`, byteWidth `72`, height `71`, src `16,0`
- `D2L`: dst x `0..74`, y `20..90`, byteWidth `72`, height `71`, src `61,0`
- `D2R`: dst x `149..223`, y `20..90`, byteWidth `72`, height `71`, src `0,0`
- `D1C`: dst x `32..191`, y `9..119`, byteWidth `128`, height `111`, src `48,0`
- `D1L`: dst x `0..63`, y `9..119`, byteWidth `128`, height `111`, src `192,0`
- `D1R`: dst x `160..223`, y `9..119`, byteWidth `128`, height `111`, src `0,0`
- `D0C`: dst x `0..223`, y `0..135`, byteWidth `0`, height `0`, src `0,0`
- `D0L`: dst x `0..31`, y `0..135`, byteWidth `16`, height `136`, src `0,0`
- `D0R`: dst x `192..223`, y `0..135`, byteWidth `16`, height `136`, src `0,0`

## Greatstone

- Greatstone lists PC 3.4 graphics.dat/dungeon graphics pages, but the local crawl does not contain the per-entry graphics.dat HTML payload; pass305 canonical GRAPHICS.DAT decode remains the byte-level wall tile authority.

## Runtime offset binding

- F0097 entry: `2809:1E31`
- VIDRV slot-9 call candidate: `2809:1EFF`
- Promotion rule: Promote only after strict true-stop recapture of F0128 followed by F0097/VIDRV call-window stop; do not use BPLIST/setup echoes.

## Next blocker

Debugger stop/control sequencing: pass320 did not recapture the strict F0128 gate, so wall tile offset bindings are source/data locked but not live-runtime promoted.
