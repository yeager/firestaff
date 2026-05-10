# Pass502 — DM1 V1 viewport wall/occlusion ReDMCSB audit

Status: precise blocker documented; no new pixel-parity promotion.

## Primary source audit

ReDMCSB local source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

Relevant source locks, with functions and line ranges only:

- `DUNVIEW.C` `F0100_DUNGEONVIEW_DrawWallSetBitmap`, `F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency`, `F0102_DUNGEONVIEW_DrawDoorBitmap`, `F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency`: lines 3048-3180. Wall and door bitmap routes into the viewport buffer.
- `DUNVIEW.C` `F0791_DUNGEONVIEW_DrawBitmapXX`: lines 3394-3472. Zone clipping/shifted object and creature blit path.
- `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: lines 4547-5885. Cell-content order for objects, creatures, projectiles, and explosions.
- `DUNVIEW.C` `F0676_DrawD3L2`: lines 6226-6292. Far-left side wall flip/zone/return contract.
- `DUNVIEW.C` `F0677_DrawD3R2`: lines 6293-6360. Far-right side wall flip/zone/return contract.
- `DUNVIEW.C` `F0116_DUNGEONVIEW_DrawSquareD3L`: lines 6361-6499. D3L wall/alcove/return contract.
- `DUNVIEW.C` `F0117_DUNGEONVIEW_DrawSquareD3R`: lines 6500-6641. D3R wall/alcove/return contract.
- `DUNVIEW.C` `F0118_DUNGEONVIEW_DrawSquareD3C_CPSF`: lines 6642-6836. D3C center wall occlusion with alcove exception.
- `DUNVIEW.C` `F0678_DrawD2L2`, `F0679_DrawD2R2`, `F0119_DUNGEONVIEW_DrawSquareD2L`, `F0120_DUNGEONVIEW_DrawSquareD2R_CPSF`, `F0121_DUNGEONVIEW_DrawSquareD2C`: lines 6837-7390. D2 side/front wall returns and center-wall exception.
- `DUNVIEW.C` `F0122_DUNGEONVIEW_DrawSquareD1L`, `F0123_DUNGEONVIEW_DrawSquareD1R`, `F0124_DUNGEONVIEW_DrawSquareD1C`: lines 7391-7959. D1 side/front wall returns, D1C front-door two-pass content occlusion, and front-wall alcove exception.
- `DUNVIEW.C` `F0125_DUNGEONVIEW_DrawSquareD0L`, `F0126_DUNGEONVIEW_DrawSquareD0R`, `F0127_DUNGEONVIEW_DrawSquareD0C`: lines 7960-8308. Nearest side/front wall handling.
- `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF`: lines 8318-8543. Authoritative far-to-near viewport replay order.
- `DRAWVIEW.C` `F0097_DUNGEONVIEW_DrawViewport`: lines 709-858. Final viewport present boundary after the buffer is composed.
- `DUNGEON.C` door info tables: lines 560-565 and 796-801. Door/projectile visibility attributes used by occlusion gates.

Greatstone/atlas material was not used as authority in this pass; ReDMCSB and canonical DM1 anchors remain primary.

## Firestaff locks checked

- `dm1_v1_viewport_3d_pc34_compat.c`: wall draw spec matrix at lines 194-207.
- `probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c`: expected wall composition matrix at lines 46-66.
- Existing pass495/pass496/pass499/pass500 gates cover source windows, local matrix rows, runtime-evidence predicate, and blocker-cleanup predicate.

## Precise blocker

The source/probe side is locked, but this still must not be promoted to original-vs-Firestaff pixel parity. The missing proof is a same-viewport runtime capture that ties the ReDMCSB `F0128_DUNGEONVIEW_Draw_CPSF` composition path to `F0097_DUNGEONVIEW_DrawViewport` present for the exact original DM1 V1 frame being compared.

Required next evidence before parity promotion:

1. Capture one canonical DM1 V1 original viewport frame for a wall/door occlusion case.
2. Capture the matching Firestaff frame from the same map/x/y/direction and wall/door state.
3. Record the Firestaff runtime path reaching the F0128-to-F0097 present boundary for that frame.
4. Attach a pixel/crop comparison manifest without promoting static repeated screenshots or source-only evidence.

