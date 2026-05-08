#!/usr/bin/env python3
"""Verify the DM1 V1 viewport/world visual lane against local ReDMCSB source.

This is intentionally source/evidence-only: it does not touch renderer code or CMake,
and it uses only local source/original-game anchors.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

DEFAULT_REDMCSB_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
DEFAULT_DM1_CANONICAL = Path(
    "~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"
).expanduser()

CHECKS: list[dict[str, Any]] = [
    {
        "id": "redmcsb-viewport-present-blit-palette-gate",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-820",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "F0565_VIEWPORT_SetPalette(G1010_pui_DungeonViewCurrentPalette, G0347_aui_Palette_TopAndBottomScreen);",
            "F0706_GetMouseState(&L2410_i_, &L2411_i_, &L2412_i_);",
        ],
        "why": "Viewport presentation is a distinct source step after world drawing, with palette switching and screen-update gating.",
    },
    {
        "id": "redmcsb-floor-ceiling-base-copy-gate",
        "file": "DUNVIEW.C",
        "function": "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "range": "2962-3003",
        "needles": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0008_MAIN_ClearBytes(G0086_puc_Bitmap_ViewportBlackArea",
            "F0007_MAIN_CopyBytes(G0085_puc_Bitmap_Ceiling, G0296_puc_Bitmap_Viewport",
            "F0007_MAIN_CopyBytes(G0084_puc_Bitmap_Floor, G0087_puc_Bitmap_ViewportFloorArea",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ],
        "why": "The viewport/world base begins by clearing/copying ceiling and floor before square content is layered.",
    },
    {
        "id": "redmcsb-wall-door-blit-zones-gate",
        "file": "DUNVIEW.C",
        "function": "F0100_DUNGEONVIEW_DrawWallSetBitmap / F0102_DUNGEONVIEW_DrawDoorBitmap",
        "range": "3048-3110",
        "needles": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport",
            "void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally(",
            "F0130_VIDEO_FlipHorizontal(P0110_puc_Bitmap",
        ],
        "why": "Walls and doors route through framed blits into G0296_puc_Bitmap_Viewport, including horizontal door-frame flipping.",
    },
    {
        "id": "redmcsb-field-teleporter-mask-cache-gate",
        "file": "DUNVIEW.C",
        "function": "F0113_DUNGEONVIEW_DrawField",
        "range": "4382-4474",
        "needles": [
            "void F0113_DUNGEONVIEW_DrawField(",
            "L2470_i_Width = M732_BYTE_WIDTH(P0135_puc_FieldAspect);",
            "F0491_CACHE_IsDerivedBitmapInCache(C000_DERIVED_BITMAP_VIEWPORT);",
            "L0119_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(C076_GRAPHIC_FIRST_FIELD + M728_NATIVE_BITMAP_RELATIVE_INDEX(P0135_puc_FieldAspect));",
            "F0133_VIDEO_BlitBoxFilledWithMaskedBitmap",
            "F0493_CACHE_AddDerivedBitmap(C000_DERIVED_BITMAP_VIEWPORT);",
        ],
        "why": "Teleporter/field visuals are source-locked to field aspects, masks, and derived viewport bitmap cache behavior.",
    },
    {
        "id": "redmcsb-world-draw-order-near-to-far-gate",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8618",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "if (G0297_B_DrawFloorAndCeilingRequested) {",
            "G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001",
            "F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M597_VIEW_SQUARE_D4C",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        ],
        "why": "The main world visual lane controls wall flipping, far-to-near square/object draw order, final viewport presentation, and next-frame floor/ceiling anticipation.",
    },
    {
        "id": "redmcsb-side-d3-wall-field-gate",
        "file": "DUNVIEW.C",
        "function": "F0676_DrawD3L2 / F0677_DrawD3R2",
        "range": "6226-6325",
        "needles": [
            "void F0676_DrawD3L2(",
            "F0172_DUNGEON_SetSquareAspect(L2484_ai_SquareAspect",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects",
            "void F0677_DrawD3R2(",
        ],
        "why": "Side D3 world visuals use square aspects, wall zones, object/creature/projectile dispatch, and teleporter field drawing.",
    },
    {
        "id": "redmcsb-f0115-thing-pass-order-gate",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "range": "4547-4582",
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "if there is a projectile, explosion or creature, take note of it, but do not draw them yet",
            "draw each object found",
            "Draw one creature at the cell being processed",
            "Draw only projectiles at specified cell.",
            "Draw only explosions at specified cell, except for Fluxcages",
        ],
        "why": "F0115 is the authoritative layering routine for objects, creatures, projectiles, explosions, and fluxcages inside each visible square/cell.",
    },
    {
        "id": "redmcsb-f0115-view-square-row-map-gate",
        "file": "DUNVIEW.C",
        "function": "G2028_ac_ViewSquareIndexTo",
        "range": "373-373",
        "needles": [
            "char G2028_ac_ViewSquareIndexTo[23] = { 11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1 };",
        ],
        "why": "The source table maps MEDIA720 view-square ids onto layout-696 C2500/C2900 row families, including D3/D2/D1 center and side rows.",
    },
    {
        "id": "redmcsb-f0115-object-perspective-c2500-zone-gate",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF object pass",
        "range": "4820-5180",
        "needles": [
            "AL0126_i_ViewCell = M001_ORDINAL_TO_INDEX((int16_t)L0130_ul_RemainingViewCellOrdinalsToProcess & 0x000F);",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
            "if ((AL0127_i_ThingType >= C05_THING_TYPE_WEAPON) && (AL0127_i_ThingType <= C10_THING_TYPE_JUNK) && (L2476_i_ >= 0) && (M011_CELL(P0141_T_Thing) == L0139_i_Cell) && ((L2475_i_ViewDepth != 3) || (AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT)) && ((L2475_i_ViewDepth != 0) || (AL0126_i_ViewCell < C02_VIEW_CELL_BACK_RIGHT)))",
            "M007_GET(L0129_ps_ObjectAspect->GraphicInfo, MASK0x0001_FLIP_ON_RIGHT)",
            "AL0150_ui_ShiftSetIndex = (L2475_i_ViewDepth * 2) - 1;",
            "AL0150_ui_ShiftSetIndex -= AL0126_i_ViewCell >> 1;",
            "L0176_i_Scale = G2030_auc_ObjectScales[AL0150_ui_ShiftSetIndex];",
            "L2474_i_ZoneIndex = (C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES) + (L2476_i_ * 4) + AL0126_i_ViewCell;",
            "G2154_i_ZoneShiftX = G0223_aac_Graphic558_ShiftSets[AL0150_ui_ShiftSetIndex]",
            "G0292_aT_PileTopObject[AL0126_i_ViewCell] = P0141_T_Thing;",
        ],
        "why": "Object/item visibility and perspective are source-gated by ordered view-cell selection, absolute cell matching, D3/D0 near/far clipping, right-side flipping, depth/cell scale selection, C2500 zone layout, pile shifts, and top-object click evidence.",
    },
    {
        "id": "redmcsb-f0115-creature-perspective-cell-aspect-gate",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF creature pass",
        "range": "5200-5472",
        "needles": [
            "/* Draw creatures */",
            "L0168_B_DrawingLastBackRowCell = ((AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT) || (L0148_i_CellCounter == 1))",
            "if ((L0151_T_GroupThing == C0xFFFF_THING_NONE) || L0174_B_DrawCreaturesCompleted",
            "if ((L2477_i_ = G2033_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)",
            "F0176_GROUP_GetCreatureOrdinalInCell(L0152_ps_Group, L0139_i_Cell)",
            "L0157_i_CreatureDirectionDelta = M021_NORMALIZE(P0142_i_Direction - M050_CREATURE_VALUE(L0153_ps_ActiveGroup->Directions, AP0141_ui_CreatureIndex));",
            "AL0126_i_ViewCell = C02_VIEW_CELL_BACK_ROW; /* Side view of a half square creature on the back row. Drawn during pass 1 for a door square */",
            "AL0126_i_ViewCell = C04_VIEW_CELL_FRONT_ROW; /* Side view of a half square creature on the front row. Drawn during pass 2 for a door square */",
            "L0170_B_UseCreatureBackBitmap = M007_GET(AP0141_ui_CreatureGraphicInfo, MASK0x0010_BACK) && (L0157_i_CreatureDirectionDelta == 0);",
            "L0172_B_UseCreatureAttackBitmap = !L0170_B_UseCreatureBackBitmap && M007_GET(L0159_i_CreatureAspect, MASK0x0080_IS_ATTACKING)",
            "if ((L0169_B_UseCreatureSideBitmap && (L0157_i_CreatureDirectionDelta == 1)) ||",
        ],
        "why": "Creature perspective is source-gated by group presence, view-square support, cell ordinal lookup, relative facing delta, half-square front/back row side-view handling, back/attack/front aspect selection, and horizontal flip rules.",
    },
    {
        "id": "redmcsb-f0115-projectile-visible-cells-and-c2900-gate",
        "file": "DUNVIEW.C",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF projectile pass",
        "range": "5645-5885",
        "needles": [
            "T0115129_DrawProjectiles:",
            "if (!L0186_B_SquareHasProjectile)",
            "if ((L2479_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)",
            "if ((L2475_i_ViewDepth == 3) && (AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT))",
            "if ((L2475_i_ViewDepth == 0) && (AL0126_i_ViewCell >= C02_VIEW_CELL_BACK_RIGHT))",
            "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
            "AL0127_i_ProjectileAspect = F0142_DUNGEON_GetProjectileAspect(L0178_ps_Projectile->Slot)",
            "AL0150_ui_ProjectileScaleIndex = (L2475_i_ViewDepth << 1) - (AL0126_i_ViewCell >> 1);",
            "L0176_i_Scale = G0215_auc_Graphic558_ProjectileScales[AL0150_ui_ProjectileScaleIndex];",
            "F0791_DUNGEONVIEW_DrawBitmapXX",
        ],
        "why": "Projectile visibility is source-gated by F0115 depth/cell clipping, C2900 row selection through G2028, projectile aspect resolution, scale selection, and final clipped blit.",
    },
    {
        "id": "redmcsb-f0791-zone-clip-blit-gate",
        "file": "DUNVIEW.C",
        "function": "F0791_DUNGEONVIEW_DrawBitmapXX",
        "range": "3394-3472",
        "needles": [
            "STATICFUNCTION void F0791_DUNGEONVIEW_DrawBitmapXX(",
            "if (P2081_i_ZoneIndex == CM1_UNKNOWN)",
            "MASK0x8000_SHIFT_OBJECTS_AND_CREATURES",
            "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex",
            "F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination",
        ],
        "why": "The PC/Amiga-era scaled object/projectile/explosion draw path resolves layout zones, applies source shift masks, clips through F0635_, then blits into the viewport.",
    },

    {
        "id": "redmcsb-square-aspect-door-teleporter-state-gate",
        "file": "DUNGEON.C",
        "function": "F0172_DUNGEON_SetSquareAspect",
        "range": "2683-2721",
        "needles": [
            "case C05_ELEMENT_TELEPORTER:",
            "if ((!M007_GET(AL0307_uc_Square, MASK0x0008_TELEPORTER_OPEN) || !M007_GET(AL0307_uc_Square, MASK0x0004_TELEPORTER_VISIBLE)))",
            "P0317_pui_SquareAspect[C0_ELEMENT] = C01_ELEMENT_CORRIDOR;",
            "case C04_ELEMENT_DOOR:",
            "P0317_pui_SquareAspect[C0_ELEMENT] = C16_ELEMENT_DOOR_SIDE;",
            "P0317_pui_SquareAspect[C0_ELEMENT] = C17_ELEMENT_DOOR_FRONT;",
            "P0317_pui_SquareAspect[M556_DOOR_STATE] = M036_DOOR_STATE(AL0307_uc_Square);",
            "P0317_pui_SquareAspect[M557_DOOR_THING_INDEX] = M013_INDEX(F0161_DUNGEON_GetSquareFirstThing(P0319_i_MapX, P0320_i_MapY));",
            "P0317_pui_SquareAspect[M550_FIRST_THING] = L0314_T_Thing;",
        ],
        "why": "Cell aspect setup is the source of truth for visible teleporter fields, door side-vs-front classification, door state/index, and the first non-sensor thing passed into viewport drawing.",
    },
    {
        "id": "redmcsb-d1c-front-wall-alcove-occlusion-gate",
        "file": "DUNVIEW.C",
        "function": "F0124_DUNGEONVIEW_DrawSquareD1C wall branch",
        "range": "7727-7872",
        "needles": [
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0172_DUNGEON_SetSquareAspect(L0218_ai_SquareAspect",
            "case C00_ELEMENT_WALL:",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);",
            "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
            "return;",
        ],
        "why": "The D1C front-wall branch draws the wall panel, permits open-cell content only for the front alcove special case, then returns; normal front walls occlude door/field/content work behind them.",
    },
    {
        "id": "redmcsb-d1lr-side-wall-return-gate",
        "file": "DUNVIEW.C",
        "function": "F0122_DUNGEONVIEW_DrawSquareD1L / F0123_DUNGEONVIEW_DrawSquareD1R wall branches",
        "range": "7391-7678",
        "needles": [
            "STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L(",
            "case C00_ELEMENT_WALL:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);",
            "return;",
            "STATICFUNCTION void F0123_DUNGEONVIEW_DrawSquareD1R(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);",
            "L0215_i_Order = C0x0041_CELL_ORDER_BACKLEFT_FRONTLEFT;",
        ],
        "why": "Near D1L/D1R side walls draw their opaque side-wall bitmap, draw only the facing wall ornament/alcove test, and return before open-cell/door-side content paths; they are side-wall blockers, not center front-wall occluders.",
    },
    {
        "id": "redmcsb-pc34-d2l2-d2r2-side-wall-return-gate",
        "file": "DUNVIEW.C",
        "function": "F0678_DrawD2L2 / F0679_DrawD2R2 wall branches",
        "range": "6837-6897",
        "needles": [
            "STATICFUNCTION void F0678_DrawD2L2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
            "C707_ZONE_WALL_D2L2);",
            "return;",
            "STATICFUNCTION void F0679_DrawD2R2(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]",
            "C708_ZONE_WALL_D2R2);",
            "case C05_ELEMENT_TELEPORTER:",
        ],
        "why": "PC34/I34E outer D2 side lanes are narrow wall/teleporter-only source branches: solid side walls draw their zone and return before field/open behavior.",
    },
    {
        "id": "redmcsb-d0lr-nearest-side-wall-return-gate",
        "file": "DUNVIEW.C",
        "function": "F0125_DUNGEONVIEW_DrawSquareD0L / F0126_DUNGEONVIEW_DrawSquareD0R wall branches",
        "range": "7960-8164",
        "needles": [
            "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
            "return;",
            "STATICFUNCTION void F0126_DUNGEONVIEW_DrawSquareD0R(",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0221_ai_SquareAspect[M550_FIRST_THING], P0177_i_Direction, P0178_i_MapX, P0179_i_MapY, M611_VIEW_SQUARE_D0R, C0x0001_CELL_ORDER_BACKLEFT);",
        ],
        "why": "Nearest D0 side walls are also wall-case returns; open/door-side/teleporter paths reach F0115, but wall branches stop before those content paths.",
    },
    {
        "id": "redmcsb-d1c-front-door-pass-order-gate",
        "file": "DUNVIEW.C",
        "function": "F0124_DUNGEONVIEW_DrawSquareD1C door branch",
        "range": "7873-7937",
        "needles": [
            "case C17_ELEMENT_DOOR_FRONT:",
            "F0108_DUNGEONVIEW_DrawFloorOrnament(L0218_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M595_VIEW_FLOOR_D1C);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2112_DoorFrameTopD1LCR, C733_ZONE_DOOR_FRAME_TOP_D1C);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2117_DoorFrameLeftD1C, C726_ZONE_DOOR_FRAME_LEFT_D1C);",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2117_DoorFrameLeftD1C, C727_ZONE_DOOR_FRAME_RIGHT_D1C);",
            "if (((DOOR*)G0284_apuc_ThingData[C00_THING_TYPE_DOOR])[L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]].Button)",
            "F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX], L0218_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M631_ZONE_DOOR_D1C);",
            "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, L0217_i_Order);",
        ],
        "why": "The D1C front-door source order is floor ornament, behind-door thing pass, door frame/button/panel draw, then the in-front thing pass. This is the narrow contract for future center-door parity work.",
    },
    {
        "id": "redmcsb-d1c-teleporter-field-after-content-gate",
        "file": "DUNVIEW.C",
        "function": "F0124_DUNGEONVIEW_DrawSquareD1C teleporter tail",
        "range": "7922-7957",
        "needles": [
            "case C05_ELEMENT_TELEPORTER:",
            "F0108_DUNGEONVIEW_DrawFloorOrnament(L0218_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M595_VIEW_FLOOR_D1C);",
            "F0112_DUNGEONVIEW_DrawCeilingPit(C067_GRAPHIC_CEILING_PIT_D1C, C868_ZONE_CEILING_PIT_D1C, P0172_i_MapX, P0173_i_MapY, C0_FALSE);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, L0217_i_Order);",
            "if (L0218_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M606_VIEW_SQUARE_D1C]], C712_ZONE_WALL_D1C);",
        ],
        "why": "For the center front teleporter, ReDMCSB draws floor/ceiling/content first and overlays the field last via G2035 aspect selection and the D1C wall zone.",
    },
    {
        "id": "redmcsb-door-projectile-occlusion-attrs-gate",
        "file": "DUNGEON.C",
        "function": "G0254_as_Graphic559_DoorInfo",
        "range": "559-565",
        "needles": [
            "DOOR_INFO G0254_as_Graphic559_DoorInfo[4] = {",
            "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH | MASK0x0001_CREATURES_CAN_SEE_THROUGH",
            "/* Door type 0 Portcullis */",
            "/* Door type 1 Wooden door */",
            "/* Door type 2 Iron door */",
            "MASK0x0004_ANIMATED | MASK0x0001_CREATURES_CAN_SEE_THROUGH",
        ],
        "why": "Door/projectile occlusion is data-backed: only portcullis carries PROJECTILES_CAN_PASS_THROUGH; solid/iron doors do not.",
    },
]

DM1_ANCHORS = ["GRAPHICS.DAT", "DUNGEON.DAT", "TITLE", "README.md"]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def require_local(path: Path) -> None:
    raw = str(path)
    resolved = str(path.resolve()) if path.exists() else raw
    allowed = tuple(str(Path(p).expanduser()) for p in (
        "~/.openclaw/data/firestaff-redmcsb-source/",
        "~/.openclaw/data/firestaff-original-games/DM/",
    ))
    if "deprecated-remote-source" in raw.lower() or "deprecated-remote-source" in resolved.lower() or "<deprecated-remote-host>" in raw:
        raise SystemExit(f"refusing non-local path: {path}")
    if not (raw.startswith(allowed) or resolved.startswith(allowed)):
        raise SystemExit(f"refusing path outside local evidence roots: {path}")


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(x) for x in line_range.split("-")]
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def verify(source: Path, dm1: Path) -> tuple[dict[str, Any], list[str]]:
    require_local(source)
    require_local(dm1)
    failures: list[str] = []
    checks_out: list[dict[str, Any]] = []
    source_hashes: dict[str, str] = {}

    for check in CHECKS:
        path = source / check["file"]
        require_local(path)
        if not path.exists():
            failures.append(f"missing source file: {path}")
            continue
        text = path.read_text(errors="replace")
        excerpt = line_slice(text, check["range"])
        missing = [needle for needle in check["needles"] if needle not in excerpt]
        status = "passed" if not missing else "failed"
        if missing:
            failures.append(
                f"{check['id']} {check['file']}:{check['range']} missing "
                + "; ".join(repr(x) for x in missing)
            )
        source_hashes.setdefault(check["file"], sha256(path))
        checks_out.append(
            {
                "id": check["id"],
                "status": status,
                "source": {
                    "file": check["file"],
                    "function": check["function"],
                    "lines": check["range"],
                    "sha256": source_hashes[check["file"]],
                },
                "why": check["why"],
            }
        )

    anchors: list[dict[str, Any]] = []
    for name in DM1_ANCHORS:
        path = dm1 / name
        require_local(path)
        if not path.exists():
            failures.append(f"missing DM1 canonical anchor: {path}")
            anchors.append({"name": name, "status": "missing"})
            continue
        resolved = path.resolve()
        anchors.append(
            {
                "name": name,
                "status": "found",
                "path": str(path),
                "resolved": str(resolved),
                "bytes": resolved.stat().st_size,
                "sha256": sha256(resolved),
            }
        )

    result = {
        "gate": "dm1-v1-viewport-world-redmcsb-source-lock",
        "status": "passed" if not failures else "failed",
        "redmcsbSourceRoot": str(source),
        "dm1CanonicalRoot": str(dm1),
        "checks": checks_out,
        "dm1Anchors": anchors,
    }
    return result, failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--dm1", type=Path, default=DEFAULT_DM1_CANONICAL)
    parser.add_argument("--json", action="store_true", help="emit full JSON evidence")
    args = parser.parse_args()

    result, failures = verify(args.source, args.dm1)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"{result['status'].upper()} {result['gate']}")
        for check in result["checks"]:
            src = check["source"]
            print(
                f"{check['status'].upper()} {check['id']}: "
                f"{src['file']} {src['function']} lines {src['lines']}"
            )
        for anchor in result["dm1Anchors"]:
            print(f"{anchor['status'].upper()} dm1-anchor {anchor['name']}")
    if failures:
        for failure in failures:
            print(f"FAIL {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
