# Pass503 - DM1 V1 viewport wall draw-order evidence

Status: PASS_PASS503_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_EVIDENCE

## ReDMCSB source locks
- DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8543 ok=True: Viewport composition replays source square functions from far to near before presentation.
- DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF lines 4547-4582 ok=True: Per-cell content order is objects, creatures, projectiles, then explosions/fluxcage.
- DUNVIEW.C F0100/F0101/F0102/F0765 wall and door blits lines 3048-3180 ok=True: Wall and door panels write into G0296 with explicit transparent or opaque routes.
- DUNVIEW.C F0676/F0677/F0116/F0117/F0118 D3 wall branches lines 6226-6836 ok=True: D3 walls occlude by drawing wall zones and returning, except front alcoves that intentionally hand contents to F0115.
- DUNVIEW.C F0678/F0679/F0119-F0127 near wall branches lines 6837-8308 ok=True: D2, D1, and D0 wall paths preserve the same wall-return/alcove exception pattern.
- DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C lines 7873-7938 ok=True: Front doors draw rear contents, door/frame, then front contents.
- DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport lines 709-858 ok=True: DUNVIEW composition buffer G0296 is presented by DRAWVIEW after vblank/present gating.

## Firestaff hooks

- dm1_v1_viewport_3d_pc34_compat.c line 82 ok=True: firestaff-draw-order-table
- dm1_v1_viewport_3d_pc34_compat.c line 194 ok=True: firestaff-wall-spec-table
- dm1_v1_viewport_3d_pc34_compat.c line 112 ok=True: firestaff-thing-layer-table
- dm1_v1_viewport_3d_pc34_compat.c line 135 ok=True: firestaff-door-front-occlusion-table
- tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py line 15 ok=True: pass496-matrix-gate-present
- parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md line 38 ok=True: pass502-blocker-doc-present

## DM1 anchors

- GRAPHICS.DAT exists=True sha256=2c3aa836925c
- DUNGEON.DAT exists=True sha256=d90b6b1c38fd
- TITLE exists=True sha256=adc7f1916eee
- README.md exists=True sha256=e8e82274f72f

## N2-local secondary references

- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/pages.json lines 1-120 ok=True: Greatstone local atlas is present as data-extraction context for DM/CSB graphics and dungeon assets.
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp lines 935-1938 ok=True: CSBWin carries a table-driven viewport cell/draw-order model with door-facing two-phase object/door/object rows.
- /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp lines 6694-6819 ok=True: CSBWin DrawViewport summarizes each relative cell and interprets the selected draw script in cell order.
- /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp lines 935-1938 ok=True: CSB lineage source mirrors the same viewport cell/draw-order and door-facing script structure.
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.md lines 1-220 ok=True: DM originals include a local PC34-vs-Greatstone manifest for asset provenance cross-checking.

## Non-claims

- This is source/probe evidence only.
- Pixel parity still needs the same-viewport original/Firestaff runtime capture described by pass502.
