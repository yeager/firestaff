# DM1 V1 D0L/D0R F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0107:3502-3938` owns the wall-ornament draw and alcove boolean. The boolean comes from the map alcove classifier after the non-zero ordinal guard, and the final blit uses `C10_COLOR_FLESH`.
- `DUNVIEW.C F0125:7960-8062` anchors `F0125_DUNGEONVIEW_DrawSquareD0L`: D0L open side lanes call `F0115` with `M610_VIEW_SQUARE_D0L` and `C0x0002_CELL_ORDER_BACKRIGHT`, while the wall case draws `C716_ZONE_WALL_D0L` and returns without a direct F0107 call.
- `DUNVIEW.C F0126:8064-8162` anchors `F0126_DUNGEONVIEW_DrawSquareD0R`: D0R mirrors the side lane with `M611_VIEW_SQUARE_D0R` and `C0x0001_CELL_ORDER_BACKLEFT`; its wall case draws `C717_ZONE_WALL_D0R` and returns without a direct F0107 call.
- `DUNVIEW.C F0128:8536-8541` anchors the near-side dispatch order: D0L is updated and drawn before D0R.
- `DUNVIEW.C F0108:3940-4011` is the floor-ornament contrast anchor for D0 floor/ceiling composition. This slice keeps F0108 separate from the D0 wall-ornament keepout.
- `DUNVIEW.C F0115:4547-4581` anchors the D0 side-lane thing-pass cell order.
- `DUNVIEW.C F0111:4218-4337` anchors the door-front relationship: partly-open horizontal doors preserve transparency through the C10 half-blit at `4322-4324` and apply the shifted second zone at `4325`.
- `DEFS.H:2088` anchors `C10_COLOR_FLESH`; `DEFS.H:2538-2554` anchors `M550/M551/M552/M553`; `DEFS.H:2596-2611` anchors `M610/M611`; `DEFS.H:2696-2711` anchors the F0107 view-wall ordinals; `DEFS.H:4040-4057` anchors D0 wall zones; `DEFS.H:4139-4153` anchors cell-order values.

## Contract

This is contract-only evidence on a synthetic 320x200-style framebuffer. It does not claim original DOS pixel parity, does not read game data, and does not duplicate the D1C F0107 gate. The D0L/D0R-specific point is that F0128 reaches the near side lanes in order, F0125/F0126 pin their F0115 side-lane ordering, and their wall cases return before any direct F0107 wall-ornament call.
