# DM1 V1 D2L2/D2R2 F0107 Wall-Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0678/F0679:6837-6896` are the local C09/C10 D2L2/D2R2 guard-wall routes. They draw wall carriers and fields, but do not call `F0107`.
- `DUNVIEW.C F0119:6900-7049` and `F0120:7051-7224` are the D2L/D2R carrier bodies used by this D2L2/D2R2 side-wall contract.
- `DUNVIEW.C:6968` calls `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` with `M551_RIGHT_WALL_ORNAMENT_ORDINAL` and `M580_VIEW_WALL_D2L_RIGHT`.
- `DUNVIEW.C:7119` calls `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` with `M553_LEFT_WALL_ORNAMENT_ORDINAL` and `M581_VIEW_WALL_D2R_LEFT`.
- `DUNVIEW.C F0107:3502-3938` owns the wall-ornament ordinal gate, `C1004 + CoordinateSet * 15 + ViewWall` zone math, `C10_COLOR_FLESH` transparent blit, and alcove boolean return.
- `DUNVIEW.C F0108:3940-4011` is a separate floor/ceiling baseline.
- `DUNVIEW.C F0128:8503-8521` dispatches D2L2, then D2R2, then D2L, D2R, and D2C.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor the thing-list and square-aspect feeds.
- `DEFS.H` anchors `C10_COLOR_FLESH`, `M550/M551/M552/M553`, `M604/M605/C09/C10`, `M580/M581`, and `C1004_ZONE_WALL_ORNAMENT`.

## Local Label Deviation

The task cue asked to verify `C711_ZONE_WALL_D2L2 / C712_ZONE_WALL_D2R2`. In the local ReDMCSB WIP20210206 `DEFS.H`, the exact labels are `C707_ZONE_WALL_D2L2` and `C708_ZONE_WALL_D2R2` at lines 4047-4048. `C711` is `C711_ZONE_WALL_D2R`; `C712` is not D2R2 in the local zone definitions.

## Contract

This is a contract-only, asset-free marker. It pins the D2L2/D2R2 guard positions, the D2L/D2R carrier wall-ornament calls at `6968` and `7119`, `C10` transparent preservation, C0..C5 sensor-ordinal acceptance, the F0128 back-before-front dispatch order, F0108 separation, and the wall-ornament zone formula. It does not read original assets and does not claim original-DOS pixel parity.
