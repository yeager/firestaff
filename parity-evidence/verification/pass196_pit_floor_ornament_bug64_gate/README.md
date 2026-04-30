# pass196 pit floor-ornament BUG0_64 source gate

Scope: DM1 V1 viewport/world visuals, floor ornaments over pits.

ReDMCSB source citations:

- `DUNGEON.C:2628-2682`: pit/teleporter route reaches the sensor floor-ornament scan and then `T0172049_Footprints`.
- `DUNGEON.C:2693-2718`: stairs route through `T0172046_Stairs`; no `M558_FLOOR_ORNAMENT_ORDINAL` assignment, and stairs bit `0x08` is orientation.
- `DUNVIEW.C:6284-6286`, `6351-6353`, `7020-7031`, `7213-7224`, `7655-7704`: visible floor slots call `F0108_DUNGEONVIEW_DrawFloorOrnament(...)`; comments annotate `BUG0_64 Floor ornaments are drawn over open pits`.

Firestaff lock:

- `m11_viewport_cell_is_wall_free` keeps pits/teleporters eligible and excludes stairs.
- `m11_draw_wall_contents` and `m11_draw_side_feature` draw floor ornaments through the open-cell layer without a pit suppression guard.
- `m11_draw_floor_ornament` comments now state the BUG0_64/open-pit and stair-exclusion source boundary.
