# DM1 V1 D1C F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0107:3502-3938` resolves non-zero wall-ornament ordinals, applies the D1C front wall side effects, blits through `C10_COLOR_FLESH`, and returns the alcove boolean.
- `DUNVIEW.C F0104:3113-3156` anchors the native C10 transparent blit shape used by D1C wall-adjacent overlays.
- `DUNVIEW.C F0108:3940-4011` is a contrast anchor: the D1C wall route keeps floor ornaments out.
- `DUNVIEW.C F0115:4547-4581` anchors the conditional alcove thing-pass cell-order route.
- `DUNVIEW.C F0124:7727-7924` anchors the D1C wall dispatch: wall body, `F0107(M552, M587)`, then `F0115(M550, M606, C0x0000)` only when F0107 returns alcove.
- `DUNVIEW.C F0128:8318-8542` anchors the global draw order reaching D1C after D1L/D1R and before D0L/D0R/D0C.
- `DEFS.H:2088` anchors `C10_COLOR_FLESH`; `DEFS.H:2538-2554` anchors `M550/M551/M552/M553`; `DEFS.H:2596-2611` anchors `M606_VIEW_SQUARE_D1C`; `DEFS.H:2696-2711` anchors `M587_VIEW_WALL_D1C_FRONT`; `DEFS.H:4045-4046` anchors adjacent wall zones; `DEFS.H:4139-4153` anchors the cell-order zone band.

## Disjointness

This is a D1C F0107 front wall-ornament contract only. It does not touch or duplicate the existing D0C/D0L/D0R/D2/D3 viewport gates, any F0108 test, `src/dm1/dm1_v1_viewport_d1c_wall_pc34_compat.*`, D1C F0111/F0115/stairs/pit/center-field gates, or CSB/Nexus/Theron/DM2 files. The model is asset-free and does not read `GRAPHICS.DAT`.
