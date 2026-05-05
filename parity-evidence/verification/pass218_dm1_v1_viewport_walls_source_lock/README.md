# pass218 DM1 V1 viewport walls source lock

Scope: DM1 V1 viewport wall zones, draw order, occlusion, and near-side replay.

Gate:

```sh
scripts/verify_dm1_v1_viewport_wall_draw_order_source_lock.py --json
```

Recorded result: `source_gate.exit` = 0.

Source citations locked by the gate:

- `DRAWVIEW.C:709-900` (`F0097_DUNGEONVIEW_DrawViewport`): final presentation blits `G0296_puc_Bitmap_Viewport` after DUNVIEW has composed the buffer.
- `DUNVIEW.C:2962-3003` (`F0098_DUNGEONVIEW_DrawFloorAndCeiling`): viewport base is cleared/copied before square content is replayed.
- `DUNVIEW.C:8318-8618` (`F0128_DUNGEONVIEW_Draw_CPSF`): draw order is explicit far-to-near square replay, from D4 through D0, then `F0097_DUNGEONVIEW_DrawViewport(...)`.
- `DUNVIEW.C:6400-6835` (`F0116/F0117/F0118`): D3 wall squares draw concrete wall zones (`C705`, `C706`, `C704`) and normally `return`; alcove wall ornaments are the intentional exception that re-enters `F0115` with `C0x0000_CELL_ORDER_ALCOVE`.
- `DUNVIEW.C:4547-4910` (`F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`): in-square layering consumes encoded cell-order nibbles and rescans the thing list per cell, so a flat primitive-depth batch would not be source-equivalent.
- `DUNVIEW.C:6428-6816` door-front branches: rear-side contents are drawn first, the door/frame is drawn, then front-side contents replay using `DOORPASS2` order constants. This is the near-side replay source lock.

Implication for Firestaff:

The viewport wall renderer should stay modeled as ordered source replay into zones. Primitive batching is only safe if it preserves these source events exactly: base floor/ceiling, far-to-near square calls, wall early-return occlusion, alcove exception, door two-pass replay, and final viewport presentation.
