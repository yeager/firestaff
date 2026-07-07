/*
 * DM1 V1 Creature Viewport Rendering — pc34 compat layer.
 *
 * Source-locked to ReDMCSB DUNVIEW.C F0115 creature rendering pipeline.
 * All data tables are verbatim from ReDMCSB source (PC 3.4 / I34E variant).
 *
 * ReDMCSB DUNVIEW.C:
 *   G0219_as_Graphic558_CreatureAspects[27]           — line 1656 (I34E block)
 *   G0221_auc_Graphic558_PaletteChanges_Creature_D3   — line 1821
 *   G0222_auc_Graphic558_PaletteChanges_Creature_D2   — line 1822
 *   F0115 native bitmap selection (front/side/back/attack) — lines 5354-5379
 *
 * ReDMCSB GROUP.C:
 *   F0179_GROUP_GetCreatureAspectUpdateTime            — lines 187-308
 *     (aspect frame cycling, horizontal/vertical offset via M052/M053 +
 *      M024/M025, FLIP_BITMAP/IS_ATTACKING via M008/M009/M010)
 *   F0185 single/multi-creature group placement        — lines 524-560
 *     (C0xFF_SINGLE_CENTERED_CREATURE single, packed 2-bit cells)
 *
 * Pass 1090 additions:
 *   - dm1_creature_max_horizontal_offset (M052)
 *   - dm1_creature_max_vertical_offset   (M053)
 *   - dm1_creature_aspect_horizontal_offset / dm1_creature_aspect_vertical_offset
 *     (M022 / M023)
 *   - dm1_creature_place_group_cells    (F0185 packed-cells builder)
 *   - dm1_creature_cycle_aspect_frame extended with the F0179 horizontal /
 *     vertical offset randomization that the Mummy (GraphicInfo 0x1480)
 *     and the Ghost (0x5864) actually need
 *   - dm1_creature_native_bitmap_index fixed so BACK/ATTACK poses fall
 *     back to the FRONT bitmap when the corresponding MASK_* flag is
 *     unset (previously offset=1 was applied unconditionally for BACK /
 *     ATTACK which referenced non-existent bitmaps for creatures
 *     without SIDE / BACK / ATTACK bitmaps).
 *   F0115 creature draw section                       — lines 5201-5520
 *
 * ReDMCSB DEFS.H:
 *   M618_GRAPHIC_FIRST_CREATURE = 584                 — line 2392
 *   GraphicInfo masks                                 — lines 1618-1629
 *   M071_COORDINATE_SET, M072_TRANSPARENT_COLOR       — lines 2016-2017
 */

#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#define DM1_NEXT_NON_ATTACK_ASPECT_UPDATE_TICKS(a) (((a) >> 4) & 0x000F)
#define DM1_NEXT_ATTACK_ASPECT_UPDATE_TICKS(a)     (((a) >> 8) & 0x000F)

#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════
 * Source-locked creature aspect table — ReDMCSB DUNVIEW.C G0219
 * (I34E / PC 3.4 variant, line 1656).
 *
 * Fields per entry:
 *   firstNativeBitmapRelativeIndex — offset from M618 (584)
 *   firstDerivedBitmapIndex        — index into derived bitmap cache
 *   coordinateSet_transparentColor — packed (coordSet<<4 | transparentColor)
 *   replacementColorSetIndices     — packed (color10Repl<<4 | color9Repl)
 *   graphicInfo                    — from G0243 CreatureInfo[].GraphicInfo
 *
 * The graphicInfo values are cross-referenced from
 * G0243_as_Graphic559_CreatureInfo (extracted separately in
 * firestaff_extracted_frontends_probe.c). The CreatureAspect table
 * itself (G0219) provides byteDimensions (widthFront/heightFront etc)
 * that are not needed for sprite index selection but would be needed
 * for the derived bitmap cache sizing; we omit them here.
 * ═══════════════════════════════════════════════════════════════════════ */
static const DM1_CreatureAspect s_aspects[27] = {
    /*  0: Giant Scorpion  */  {  0, 495, 0x1D, 0x01, 0x0482 },
    /*  1: Swamp Slime     */  {  4, 507, 0x0B, 0x20, 0x0480 },
    /*  2: Giggler          */  {  6, 519, 0x0B, 0x00, 0x4510 },
    /*  3: Wizard Eye       */  { 10, 531, 0x24, 0x31, 0x04B4 },
    /*  4: Pain Rat         */  { 12, 543, 0x14, 0x34, 0x0701 },
    /*  5: Ruster           */  { 16, 555, 0x18, 0x34, 0x0581 },
    /*  6: Screamer         */  { 19, 567, 0x0D, 0x00, 0x070C },
    /*  7: Rockpile         */  { 21, 579, 0x04, 0x00, 0x0300 },
    /*  8: Ghost            */  { 23, 591, 0x04, 0x00, 0x5864 },
    /*  9: Stone Golem      */  { 25, 603, 0x14, 0x00, 0x0282 },
    /* 10: Mummy            */  { 29, 615, 0x04, 0x00, 0x1480 },
    /* 11: Black Flame      */  { 33, 627, 0x14, 0x00, 0x18C6 },
    /* 12: Skeleton         */  { 35, 639, 0x04, 0x00, 0x1280 },
    /* 13: Couatl           */  { 39, 651, 0x1D, 0x20, 0x14A2 },
    /* 14: Vexirk           */  { 43, 663, 0x04, 0x30, 0x05B8 },
    /* 15: Magenta Worm     */  { 47, 675, 0x14, 0x78, 0x0381 },
    /* 16: Trolin           */  { 51, 687, 0x04, 0x65, 0x0680 },
    /* 17: Giant Wasp       */  { 55, 699, 0x24, 0x00, 0x04A0 },
    /* 18: Animated Armour  */  { 59, 711, 0x04, 0x00, 0x0280 },
    /* 19: Materializer     */  { 63, 723, 0x0D, 0xA9, 0x4060 },
    /* 20: Water Elemental  */  { 67, 735, 0x14, 0x65, 0x10DE },
    /* 21: Oitu             */  { 69, 747, 0x14, 0xA9, 0x0082 },
    /* 22: Demon            */  { 73, 759, 0x04, 0xCB, 0x1480 },
    /* 23: Lord Chaos       */  { 77, 771, 0x14, 0x00, 0x78AA },
    /* 24: Red Dragon       */  { 81, 783, 0x14, 0xCB, 0x068A },
    /* 25: Lord Order       */  { 85, 795, 0x14, 0xCB, 0x78AA },
    /* 26: Grey Lord        */  { 86, 807, 0x14, 0xCB, 0x78AA }
};

/* ReDMCSB DUNVIEW.C line 1821 (I34E):
 * G0221_auc_Graphic558_PaletteChanges_Creature_D3 */
static const unsigned char s_paletteD3[16] = {
    0, 12, 1, 3, 4, 3, 0, 6, 3, 0, 0, 11, 0, 2, 0, 13
};

/* ReDMCSB DUNVIEW.C line 1822 (I34E):
 * G0222_auc_Graphic558_PaletteChanges_Creature_D2 */
static const unsigned char s_paletteD2[16] = {
    0, 1, 2, 3, 4, 3, 6, 7, 5, 0, 0, 11, 12, 13, 14, 15
};

/* ═══════════════════════════════════════════════════════════════════════ */

const DM1_CreatureAspect* dm1_creature_aspects(void) {
    return s_aspects;
}

const unsigned char* dm1_creature_palette_d3(void) {
    return s_paletteD3;
}

const unsigned char* dm1_creature_palette_d2(void) {
    return s_paletteD2;
}

int dm1_creature_coordinate_set(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    return (s_aspects[creatureType].coordinateSet_transparentColor >> 4) & 0x0F;
}

int dm1_creature_transparent_color(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    return s_aspects[creatureType].coordinateSet_transparentColor & 0x0F;
}

static int dm1_creature_repl_color9_index(const DM1_CreatureAspect *aspect) {
    return aspect ? (int)(aspect->replacementColorSetIndices & 0x0F) : 0;
}

static int dm1_creature_repl_color10_index(const DM1_CreatureAspect *aspect) {
    return aspect ? (int)((aspect->replacementColorSetIndices >> 4) & 0x0F) : 0;
}

static int dm1_creature_native_bitmap_count_from_gi(unsigned int gi) {
    int count = 1;
    int additional = (int)(gi & DM1_GI_MASK_ADDITIONAL);
    int hasSpecialD2 = ((gi & DM1_GI_MASK_SPECIAL_D2_FRONT) != 0) &&
                       ((gi & DM1_GI_MASK_D2_FRONT_IS_FLIPPED) == 0);
    if (gi & DM1_GI_MASK_SIDE) count += 1;
    if (gi & DM1_GI_MASK_BACK) count += 1;
    if (hasSpecialD2) count += 1;
    if (gi & DM1_GI_MASK_ATTACK) count += 1;
    if (additional && !(gi & DM1_GI_MASK_FLIP_NON_ATTACK)) {
        count += additional;
    }
    return count;
}

static int dm1_creature_derived_bitmap_count_from_gi(unsigned int gi) {
    int count = 2;
    int additional = (int)(gi & DM1_GI_MASK_ADDITIONAL);
    if (gi & DM1_GI_MASK_SIDE) count += 2;
    if (gi & DM1_GI_MASK_BACK) count += 2;
    if (gi & DM1_GI_MASK_ATTACK) count += 2;
    count += additional * 3;
    return count;
}

unsigned int dm1_creature_sprite_for_depth(int creatureType, int depthIndex) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    if (depthIndex <= 0) {
        return dm1_creature_native_bitmap_index(creatureType,
                                                DM1_CREATURE_POSE_FRONT);
    }
    return (unsigned int)s_aspects[creatureType].firstDerivedBitmapIndex +
           (depthIndex >= 2 ? 0u : 1u);
}

static int dm1_creature_m11_relative_facing(int creatureDir, int partyDir) {
    if (creatureDir < 0 || partyDir < 0) return 2;
    return (creatureDir - partyDir) & 3;
}

static int dm1_creature_m11_pose_for_view(int relFacing, int attacking) {
    if (attacking && relFacing == 2) return DM1_CREATURE_POSE_ATTACK;
    switch (relFacing & 3) {
        case 0: return DM1_CREATURE_POSE_BACK;
        case 1:
        case 3: return DM1_CREATURE_POSE_SIDE;
        default: return DM1_CREATURE_POSE_FRONT;
    }
}

static int dm1_creature_m11_pose_mirror_with_info(int creatureType,
                                                  int relFacing,
                                                  int pose,
                                                  int attacking) {
    unsigned int gi;
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) {
        return pose == DM1_CREATURE_POSE_SIDE && ((relFacing & 3) == 1);
    }
    gi = (unsigned int)s_aspects[creatureType].graphicInfo;
    if (pose == DM1_CREATURE_POSE_SIDE) {
        if (gi & DM1_GI_MASK_SIDE) return (relFacing & 3) == 1;
        if (gi & DM1_GI_MASK_FLIP_NON_ATTACK) return (relFacing & 3) == 1;
        return 0;
    }
    if (pose == DM1_CREATURE_POSE_BACK) return 0;
    if (pose == DM1_CREATURE_POSE_ATTACK) {
        if (gi & DM1_GI_MASK_ATTACK) {
            if ((gi & DM1_GI_MASK_FLIP_ATTACK) &&
                !(gi & DM1_GI_MASK_FLIP_DURING_ATTACK)) {
                return (relFacing & 3) == 1;
            }
            return 0;
        }
        if (attacking && (gi & DM1_GI_MASK_FLIP_ATTACK)) {
            return (relFacing & 3) == 1;
        }
    }
    return 0;
}

static unsigned int dm1_creature_sprite_for_pose(int creatureType,
                                                 int depthIndex,
                                                 int pose) {
    static const unsigned char k_native_pose_offset[4] = {0, 1, 2, 3};
    static const unsigned char k_derived_pose_offset[4][2] = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}
    };
    const DM1_CreatureAspect *aspect;
    unsigned int gi;
    int dIdx;

    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    if (pose < DM1_CREATURE_POSE_FRONT || pose > DM1_CREATURE_POSE_ATTACK) {
        pose = DM1_CREATURE_POSE_FRONT;
    }
    aspect = &s_aspects[creatureType];
    gi = (unsigned int)aspect->graphicInfo;
    if (pose == DM1_CREATURE_POSE_SIDE && !(gi & DM1_GI_MASK_SIDE)) {
        pose = DM1_CREATURE_POSE_FRONT;
    } else if (pose == DM1_CREATURE_POSE_BACK && !(gi & DM1_GI_MASK_BACK)) {
        pose = DM1_CREATURE_POSE_FRONT;
    } else if (pose == DM1_CREATURE_POSE_ATTACK && !(gi & DM1_GI_MASK_ATTACK)) {
        pose = DM1_CREATURE_POSE_FRONT;
    }
    if (depthIndex <= 0) {
        return (unsigned int)(DM1_GRAPHIC_FIRST_CREATURE +
                              aspect->firstNativeBitmapRelativeIndex +
                              k_native_pose_offset[pose]);
    }
    dIdx = depthIndex >= 2 ? 0 : 1;
    return (unsigned int)aspect->firstDerivedBitmapIndex +
           (unsigned int)k_derived_pose_offset[pose][dIdx];
}

unsigned int dm1_creature_sprite_for_view(int creatureType,
                                          int depthIndex,
                                          int creatureDir,
                                          int partyDir,
                                          int attacking,
                                          int *outMirror) {
    int relFacing = dm1_creature_m11_relative_facing(creatureDir, partyDir);
    int pose = dm1_creature_m11_pose_for_view(relFacing, attacking);
    if (outMirror) {
        *outMirror = dm1_creature_m11_pose_mirror_with_info(creatureType,
                                                            relFacing,
                                                            pose,
                                                            attacking);
    }
    return dm1_creature_sprite_for_pose(creatureType, depthIndex, pose);
}

unsigned int dm1_creature_graphic_info(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0u;
    return (unsigned int)s_aspects[creatureType].graphicInfo;
}

int dm1_creature_additional(int creatureType) {
    return (int)(dm1_creature_graphic_info(creatureType) & DM1_GI_MASK_ADDITIONAL);
}

int dm1_creature_has_special_d2_front(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) &
            DM1_GI_MASK_SPECIAL_D2_FRONT) ? 1 : 0;
}

int dm1_creature_has_d2_front_is_flipped_front(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) &
            DM1_GI_MASK_D2_FRONT_IS_FLIPPED) ? 1 : 0;
}

int dm1_creature_has_flip_during_attack(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) &
            DM1_GI_MASK_FLIP_DURING_ATTACK) ? 1 : 0;
}

int dm1_creature_native_bitmap_count(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    return dm1_creature_native_bitmap_count_from_gi(
        (unsigned int)s_aspects[creatureType].graphicInfo);
}

int dm1_creature_derived_bitmap_count(int creatureType) {
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    return dm1_creature_derived_bitmap_count_from_gi(
        (unsigned int)s_aspects[creatureType].graphicInfo);
}

int dm1_creature_max_horizontal_offset_for_type(int creatureType) {
    return dm1_creature_max_horizontal_offset(
        (uint16_t)dm1_creature_graphic_info(creatureType));
}

int dm1_creature_max_vertical_offset_for_type(int creatureType) {
    return dm1_creature_max_vertical_offset(
        (uint16_t)dm1_creature_graphic_info(creatureType));
}

int dm1_creature_has_side_bitmap(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) & DM1_GI_MASK_SIDE) ? 1 : 0;
}

int dm1_creature_has_back_bitmap(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) & DM1_GI_MASK_BACK) ? 1 : 0;
}

int dm1_creature_has_attack_bitmap(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) & DM1_GI_MASK_ATTACK) ? 1 : 0;
}

int dm1_creature_has_flip_non_attack(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) &
            DM1_GI_MASK_FLIP_NON_ATTACK) ? 1 : 0;
}

int dm1_creature_has_flip_attack(int creatureType) {
    return (dm1_creature_graphic_info(creatureType) &
            DM1_GI_MASK_FLIP_ATTACK) ? 1 : 0;
}

int dm1_creature_replacement_colors(int creatureType,
                                    int *outReplDst9,
                                    int *outReplDst10) {
    static const unsigned char k_repl_color9[13] = {
        0, 4, 2, 1, 4, 5, 3, 6, 5, 6, 4, 1, 12
    };
    static const unsigned char k_repl_color10[13] = {
        0, 14, 14, 12, 14, 15, 9, 14, 13, 14, 12, 13, 14
    };
    int setIdx9;
    int setIdx10;
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    setIdx9 = dm1_creature_repl_color9_index(&s_aspects[creatureType]);
    setIdx10 = dm1_creature_repl_color10_index(&s_aspects[creatureType]);
    if (setIdx9 == 0 && setIdx10 == 0) return 0;
    if (outReplDst9) {
        *outReplDst9 = (setIdx9 > 0 && setIdx9 < 13)
                     ? (int)k_repl_color9[setIdx9] : 9;
    }
    if (outReplDst10) {
        *outReplDst10 = (setIdx10 > 0 && setIdx10 < 13)
                      ? (int)k_repl_color10[setIdx10] : 10;
    }
    return 1;
}

static int dm1_creature_visible_duplicates(int creatureCount, int maxVisible) {
    if (creatureCount > maxVisible) return maxVisible;
    if (creatureCount < 1) return 1;
    return creatureCount;
}

int dm1_creature_center_draw_placement(int creatureType,
                                       int depthIndex,
                                       int faceX,
                                       int faceY,
                                       int faceW,
                                       int faceH,
                                       int groupCount,
                                       int groupIndex,
                                       int creatureCount,
                                       int duplicateIndex,
                                       DM1_CreatureDrawPlacement *outPlacement) {
    int visible;
    int slotW;
    int slotH;
    int stepX;
    int x;
    int y;
    int w;
    int h;
    int coordSet;
    int zoneX = 0;
    int zoneY = 0;

    if (!outPlacement || creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT ||
        faceW <= 0 || faceH <= 0 || groupCount < 1 || groupIndex < 0 ||
        groupIndex >= groupCount) {
        return 0;
    }
    visible = dm1_creature_visible_duplicates(creatureCount, 4);
    if (duplicateIndex < 0 || duplicateIndex >= visible) return 0;
    slotW = (groupCount > 1) ? (faceW - 8) * 2 / (groupCount + 1) : faceW - 8;
    slotH = faceH - 10;
    if (slotW < 4 || slotH < 4) return 0;
    stepX = (groupCount > 1) ? ((faceW - 8) - slotW) / (groupCount - 1) : 0;
    x = faceX + 4 + groupIndex * stepX;
    y = faceY + 5;
    w = slotW;
    h = slotH;
    if (visible > 1) {
        w = slotW * 3 / 4;
        h = slotH * 3 / 4;
    }
    coordSet = dm1_creature_coordinate_set(creatureType);
    if (dm1_viewport_3d_c3200_creature_zone_point(coordSet,
                                                  depthIndex < 3 ? depthIndex : 2,
                                                  visible,
                                                  duplicateIndex,
                                                  &zoneX,
                                                  &zoneY)) {
        int localCenterX = (zoneX * faceW) / 224;
        int localBottomY = (zoneY * faceH) / 136;
        x = faceX + localCenterX - w / 2;
        y = faceY + localBottomY - h;
    }
    if (x < faceX) x = faceX;
    if (y < faceY) y = faceY;
    if (x + w > faceX + faceW) x = faceX + faceW - w;
    if (y + h > faceY + faceH) y = faceY + faceH - h;
    outPlacement->x = x;
    outPlacement->y = y;
    outPlacement->w = w;
    outPlacement->h = h;
    outPlacement->side_hint = 0;
    return 1;
}

int dm1_creature_side_draw_placement(int creatureType,
                                     int depthIndex,
                                     int side,
                                     int paneX,
                                     int paneY,
                                     int paneW,
                                     int paneH,
                                     int groupCount,
                                     int groupIndex,
                                     int creatureCount,
                                     int duplicateIndex,
                                     DM1_CreatureDrawPlacement *outPlacement) {
    int visible;
    int slotH;
    int stepY;
    int x;
    int y;
    int w;
    int h;
    int coordSet;
    int zoneX = 0;
    int zoneY = 0;

    if (!outPlacement || creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT ||
        paneW <= 2 || paneH <= 2 || groupCount < 1 || groupIndex < 0 ||
        groupIndex >= groupCount || (side != -1 && side != 1)) {
        return 0;
    }
    visible = dm1_creature_visible_duplicates(creatureCount, 3);
    if (duplicateIndex < 0 || duplicateIndex >= visible) return 0;
    slotH = (groupCount > 1) ? (paneH - 2) * 2 / (groupCount + 1) : paneH - 2;
    if (slotH < 4) return 0;
    stepY = (groupCount > 1) ? ((paneH - 2) - slotH) / (groupCount - 1) : 0;
    w = paneW - 2;
    h = (visible == 1) ? slotH : slotH * 2 / 3;
    x = paneX + 1;
    y = paneY + 1 + groupIndex * stepY;
    if (visible > 1) {
        int ofsY = (slotH - h) / (visible > 1 ? visible - 1 : 1);
        if (ofsY < 1) ofsY = 1;
        y += duplicateIndex * ofsY;
    }
    coordSet = dm1_creature_coordinate_set(creatureType);
    if (dm1_viewport_3d_c3200_creature_side_zone_point(coordSet,
                                                       depthIndex < 3 ? depthIndex : 2,
                                                       side,
                                                       visible,
                                                       duplicateIndex,
                                                       &zoneX,
                                                       &zoneY)) {
        x = DM1_VIEWPORT_SCREEN_X + zoneX - w / 2;
        y = DM1_VIEWPORT_SCREEN_Y + zoneY - h;
        outPlacement->side_hint = 0;
    } else {
        outPlacement->side_hint = side;
    }
    outPlacement->x = x;
    outPlacement->y = y;
    outPlacement->w = w;
    outPlacement->h = h;
    return 1;
}

int dm1_creature_center_draw_plan(const int *creatureTypes,
                                  const int *creatureCounts,
                                  const int *creatureDirections,
                                  int groupCount,
                                  int depthIndex,
                                  int faceX,
                                  int faceY,
                                  int faceW,
                                  int faceH,
                                  DM1_CreatureDrawPlan *outPlan) {
    int gi;
    int outCount = 0;
    if (!outPlan) return 0;
    outPlan->count = 0;
    if (!creatureTypes || !creatureCounts || !creatureDirections ||
        groupCount < 1 || faceW <= 0 || faceH <= 0) {
        return 0;
    }
    /* ReDMCSB DUNVIEW.C F0115 lines 4567-4581 and 5201-5520: group
     * occupants are expanded in cell order before the creature draw branch
     * consumes the C3200 coordinate zones. */
    for (gi = 0; gi < groupCount && outCount < DM1_CREATURE_DRAW_PLAN_MAX; ++gi) {
        int di;
        int visible;
        if (creatureTypes[gi] < 0 || creatureTypes[gi] >= DM1_CREATURE_TYPE_COUNT) {
            continue;
        }
        visible = dm1_creature_visible_duplicates(creatureCounts[gi], 4);
        for (di = 0; di < visible && outCount < DM1_CREATURE_DRAW_PLAN_MAX; ++di) {
            DM1_CreatureDrawPlanEntry *entry = &outPlan->entries[outCount];
            if (!dm1_creature_center_draw_placement(creatureTypes[gi],
                                                    depthIndex,
                                                    faceX,
                                                    faceY,
                                                    faceW,
                                                    faceH,
                                                    groupCount,
                                                    gi,
                                                    creatureCounts[gi],
                                                    di,
                                                    &entry->placement)) {
                continue;
            }
            entry->creature_type = creatureTypes[gi];
            entry->creature_direction = creatureDirections[gi];
            entry->creature_count = creatureCounts[gi];
            entry->group_index = gi;
            entry->duplicate_index = di;
            entry->first_in_group = (di == 0);
            ++outCount;
        }
    }
    outPlan->count = outCount;
    return outCount;
}

int dm1_creature_side_draw_plan(const int *creatureTypes,
                                const int *creatureCounts,
                                const int *creatureDirections,
                                int groupCount,
                                int depthIndex,
                                int side,
                                int paneX,
                                int paneY,
                                int paneW,
                                int paneH,
                                DM1_CreatureDrawPlan *outPlan) {
    int gi;
    int outCount = 0;
    if (!outPlan) return 0;
    outPlan->count = 0;
    if (!creatureTypes || !creatureCounts || !creatureDirections ||
        groupCount < 1 || paneW <= 2 || paneH <= 2 || (side != -1 && side != 1)) {
        return 0;
    }
    /* ReDMCSB DUNVIEW.C F0115 lines 5613-5616 uses the side C3200 zone
     * variant after the same group/duplicate expansion as center squares. */
    for (gi = 0; gi < groupCount && outCount < DM1_CREATURE_DRAW_PLAN_MAX; ++gi) {
        int di;
        int visible;
        if (creatureTypes[gi] < 0 || creatureTypes[gi] >= DM1_CREATURE_TYPE_COUNT) {
            continue;
        }
        visible = dm1_creature_visible_duplicates(creatureCounts[gi], 3);
        for (di = 0; di < visible && outCount < DM1_CREATURE_DRAW_PLAN_MAX; ++di) {
            DM1_CreatureDrawPlanEntry *entry = &outPlan->entries[outCount];
            if (!dm1_creature_side_draw_placement(creatureTypes[gi],
                                                  depthIndex,
                                                  side,
                                                  paneX,
                                                  paneY,
                                                  paneW,
                                                  paneH,
                                                  groupCount,
                                                  gi,
                                                  creatureCounts[gi],
                                                  di,
                                                  &entry->placement)) {
                continue;
            }
            entry->creature_type = creatureTypes[gi];
            entry->creature_direction = creatureDirections[gi];
            entry->creature_count = creatureCounts[gi];
            entry->group_index = gi;
            entry->duplicate_index = di;
            entry->first_in_group = (di == 0);
            ++outCount;
        }
    }
    outPlan->count = outCount;
    return outCount;
}

/*
 * Aspect frame cycling — ReDMCSB GROUP.C F0179 lines 222-305.
 *
 * The original does not increment a linear sprite-frame number. Instead it
 * rewrites the active-group Aspect bits. The mask at the top of F0179
 * keeps only the FLIP_BITMAP and IS_ATTACKING bits from the previous
 * Aspect (lines 224-225), then sets the per-update horizontal/vertical
 * offset bits via M052/M053 + M024/M025 before deciding the flip state.
 * The caller supplies randomBit as the source-locked M005_RANDOM(2) result.
 *
 * For maximum offset = 0 (e.g. Giant Scorpion 0x0482) the if-conditions in
 * F0179 are skipped, so the offset bits remain cleared by the opening
 * mask. For maximum offset >= 1 the offset is set to a sign-magnitude
 * 3-bit value computed as (-M002_RANDOM(max)) & 0x0007 when randomBit is
 * set, else +M002_RANDOM(max). Note that M002_RANDOM(max) returns
 * [0, max), so for max = 1 the magnitude is always 0 and only the sign
 * bit (0x04) can flip.
 *
 * randomValue must be an unsigned value of at least 16 bits. The function
 * only consumes its low two bits (M002_RANDOM uses the low bits; the high
 * bit picks the sign via M005_RANDOM(2)). For backward compatibility with
 * the existing M11 caller, randomValue == (randomBit | (randomOffset << 1)).
 */
uint8_t dm1_creature_cycle_aspect_frame(int creatureType,
                                        uint8_t previousAspect,
                                        int attacking, int randomBit) {
    uint16_t gi;
    uint8_t aspect;
    int randomSet;
    int maxH, maxV;
    int hOffset, vOffset;

    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;

    gi = s_aspects[creatureType].graphicInfo;
    /* ReDMCSB GROUP.C line 224:
     *   AL0326_ui_Aspect = ...->Aspect[i] & (MASK0x0080_IS_ATTACKING |
     *                                       MASK0x0040_FLIP_BITMAP);
     * The horizontal/vertical offset bits (0x07, 0x38) are implicitly
     * cleared by this mask and re-set below via M024/M025. */
    aspect = (uint8_t)(previousAspect &
                       (DM1_CREATURE_ASPECT_IS_ATTACKING |
                        DM1_CREATURE_ASPECT_FLIP_BITMAP));
    randomSet = randomBit & 1;

    /* ReDMCSB GROUP.C F0179 lines 226-243 — horizontal/vertical offset
     * randomization. M052 / M053 macros read bits 12..15 of GraphicInfo.
     * When the max is 0 the body of the if-statement is skipped and the
     * aspect's offset bits stay cleared. The "(-r) & 0x0007" sign flip
     * produces the 3-bit sign-magnitude value that DUNVIEW.C F0115
     * line 5407 consumes as a signed [-maxH..+maxH] pixel offset. */
    maxH = (int)((gi >> 12) & 0x0003);
    maxV = (int)((gi >> 14) & 0x0003);
    hOffset = 0;
    vOffset = 0;
    if (maxH > 0) {
        int r = (randomBit >> 1) & ((1 << 12) - 1);
        hOffset = r % maxH;
        if (randomSet) hOffset = (-hOffset) & DM1_CREATURE_ASPECT_HMASK;
    }
    if (maxV > 0) {
        int r = (randomBit >> 5) & ((1 << 12) - 1);
        vOffset = r % maxV;
        if (randomSet) vOffset = (-vOffset) & 0x07;
    }
    aspect |= (uint8_t)(hOffset & DM1_CREATURE_ASPECT_HMASK);
    aspect |= (uint8_t)((vOffset << DM1_CREATURE_ASPECT_VSHIFT) &
                        DM1_CREATURE_ASPECT_VMASK);

    if (attacking) {
        if (gi & DM1_GI_MASK_FLIP_ATTACK) {
            if ((aspect & DM1_CREATURE_ASPECT_IS_ATTACKING) &&
                creatureType == DM1_CREATURE_ANIMATED_ARMOUR) {
                if (randomSet) {
                    aspect ^= DM1_CREATURE_ASPECT_FLIP_BITMAP;
                }
            } else if (!(aspect & DM1_CREATURE_ASPECT_IS_ATTACKING) ||
                       !(gi & DM1_GI_MASK_FLIP_DURING_ATTACK)) {
                if (randomSet) {
                    aspect |= DM1_CREATURE_ASPECT_FLIP_BITMAP;
                } else {
                    aspect &= (uint8_t)~DM1_CREATURE_ASPECT_FLIP_BITMAP;
                }
            }
        } else {
            aspect &= (uint8_t)~DM1_CREATURE_ASPECT_FLIP_BITMAP;
        }
        aspect |= DM1_CREATURE_ASPECT_IS_ATTACKING;
    } else {
        if (gi & DM1_GI_MASK_FLIP_NON_ATTACK) {
            if (creatureType == DM1_CREATURE_COUATL) {
                if (randomSet) {
                    aspect ^= DM1_CREATURE_ASPECT_FLIP_BITMAP;
                }
            } else if (randomSet) {
                aspect |= DM1_CREATURE_ASPECT_FLIP_BITMAP;
            } else {
                aspect &= (uint8_t)~DM1_CREATURE_ASPECT_FLIP_BITMAP;
            }
        } else {
            aspect &= (uint8_t)~DM1_CREATURE_ASPECT_FLIP_BITMAP;
        }
        aspect &= (uint8_t)~DM1_CREATURE_ASPECT_IS_ATTACKING;
    }

    return aspect;
}

int dm1_creature_next_aspect_update_delay(int animationTicks, int attacking, int randomBit) {
    int base = attacking ? DM1_NEXT_ATTACK_ASPECT_UPDATE_TICKS(animationTicks)
                         : DM1_NEXT_NON_ATTACK_ASPECT_UPDATE_TICKS(animationTicks);
    return base + (randomBit & 1);
}

/* ── Aspect offset accessors — ReDMCSB DEFS.H M022/M023 ── */
int dm1_creature_max_horizontal_offset(uint16_t graphicInfo) {
    return (int)((graphicInfo >> 12) & 0x0003);
}

int dm1_creature_max_vertical_offset(uint16_t graphicInfo) {
    return (int)((graphicInfo >> 14) & 0x0003);
}

int dm1_creature_aspect_horizontal_offset(uint8_t aspectBits) {
    return (int)(aspectBits & DM1_CREATURE_ASPECT_HMASK);
}

int dm1_creature_aspect_vertical_offset(uint8_t aspectBits) {
    return (int)((aspectBits & DM1_CREATURE_ASPECT_VMASK) >>
                 DM1_CREATURE_ASPECT_VSHIFT);
}

/* ── Group placement helper — ReDMCSB GROUP.C F0185 lines 521-560 ──
 * Single creature  → DM1_GROUP_CELL_SINGLE_CENTERED (0xFF, equivalent to
 *                     C0xFF_SINGLE_CENTERED_CREATURE) so the active-group
 *                     code resolves the cell to the center of the tile.
 * Multiple creatures → pack 2 bits per slot; the source increments
 *                     L0351_ui_Cell once per slot via F0178's
 *                     post-increment, then a second time only for
 *                     half-square creatures (C1_SIZE_HALF_SQUARE):
 *
 *                        do { cells |= cell++ << (slot*2);
 *                              if (size == HALF_SQUARE) cell++;
 *                              cell &= 3; } while (slot--);
 *
 *                     Quarter- and full-square creatures therefore
 *                     advance by one cell per slot, half-square by two.
 *
 * rng must return an int in [0, range) and must accept range > 0.
 * Passing NULL is treated as "no randomness available" and returns the
 * deterministic slot ordering cells = startCell, startCell+1, ... (used
 * by tests for regression reproducibility). */
int dm1_creature_place_group_cells(int creatureCount, int creatureSize,
                                   int (*rng)(void* user, int range),
                                   void* rngUser) {
    unsigned cells;
    int slot;
    int cell;

    if (creatureCount <= 0) return DM1_GROUP_CELL_SINGLE_CENTERED;

    cells = 0;
    cell = (rng != NULL) ? (rng(rngUser, 4) & 3) : 0;

    for (slot = 0; slot < creatureCount; ++slot) {
        unsigned packed = ((unsigned)(cell & 3)) << (slot * 2);
        cells |= packed;
        /* F0185: post-increment the cell, then +1 for half-square only. */
        cell = (cell + 1) & 3;
        if (creatureSize == DM1_CREATURE_SIZE_HALF) {
            cell = (cell + 1) & 3;
        }
        /* Re-randomise the next slot's start cell the same way F0185
         * does for non-quarter-square creatures.  With rng == NULL we
         * fall back to the deterministic successor so tests can pin
         * exact cells values. */
        if (rng != NULL && creatureSize != DM1_CREATURE_SIZE_QUARTER) {
            cell = rng(rngUser, 4) & 3;
        }
    }

    return (int)(cells & 0xFF);
}

/*
 * Direction delta — ReDMCSB DUNVIEW.C line 5242:
 *   L0157_i_CreatureDirectionDelta = M021_NORMALIZE(
 *       P0142_i_Direction - M050_CREATURE_VALUE(
 *           L0153_ps_ActiveGroup->Directions, AP0141_ui_CreatureIndex));
 * M021_NORMALIZE(x) = (x) & 3
 * Result: 0=back, 1=side-right, 2=front, 3=side-left
 */
int dm1_creature_direction_delta(int partyDir, int creatureDir) {
    return (partyDir - creatureDir) & 3;
}

/*
 * Pose selection — ReDMCSB DUNVIEW.C F0115 lines 5318-5390.
 *
 * Logic:
 *   if MASK_SIDE set && delta & 1 → SIDE
 *   else:
 *     back = MASK_BACK set && delta == 0
 *     attack = !back && isAttacking && MASK_ATTACK set
 *     if !back && !attack → FRONT (possibly flipped)
 */
int dm1_creature_pose_from_delta(int directionDelta, int attacking,
                                 uint16_t graphicInfo) {
    /* Side view: creature facing perpendicular to party */
    if ((graphicInfo & DM1_GI_MASK_SIDE) && (directionDelta & 1)) {
        return DM1_CREATURE_POSE_SIDE;
    }
    /* Back view: creature facing same direction as party */
    if ((graphicInfo & DM1_GI_MASK_BACK) && directionDelta == 0) {
        return DM1_CREATURE_POSE_BACK;
    }
    /* Attack view: creature facing party and attacking */
    if (attacking && (graphicInfo & DM1_GI_MASK_ATTACK) && directionDelta != 0) {
        return DM1_CREATURE_POSE_ATTACK;
    }
    /* Default: front view */
    return DM1_CREATURE_POSE_FRONT;
}

/*
 * Native bitmap index computation — ReDMCSB DUNVIEW.C F0115 lines 5312-5390.
 *
 * The bitmap sequence for each creature in GRAPHICS.DAT is:
 *   [FRONT] [SIDE if MASK_SIDE] [BACK if MASK_BACK] [ATTACK if MASK_ATTACK]
 *   [ADDITIONAL bitmaps if GI_ADDITIONAL > 0 and !FLIP_NON_ATTACK]
 *   [SPECIAL_D2 if MASK_SPECIAL_D2_FRONT and !MASK_D2_FRONT_IS_FLIPPED]
 *
 * The front bitmap is always first (offset 0 from firstNative).
 * Side, back, attack follow in that order when present.
 */
unsigned int dm1_creature_native_bitmap_index(int creatureType, int pose) {
    const DM1_CreatureAspect* a;
    uint16_t gi;
    int offset;

    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    a = &s_aspects[creatureType];
    gi = a->graphicInfo;
    offset = 0; /* Start at front bitmap (DUNVIEW.C line 5354) */

    switch (pose) {
    case DM1_CREATURE_POSE_FRONT:
        break;
    case DM1_CREATURE_POSE_SIDE:
        if (gi & DM1_GI_MASK_SIDE) {
            offset = 1; /* Side follows front (line 5361) */
        }
        break;
    case DM1_CREATURE_POSE_BACK:
        /* DUNVIEW.C lines 5371-5379: only advance past the front bitmap
         * if MASK_BACK is actually set.  When BACK is unset (e.g. the
         * Mummy's GraphicInfo 0x1480) the front bitmap is reused. */
        if (gi & DM1_GI_MASK_BACK) {
            offset = 1; /* Skip front */
            if (gi & DM1_GI_MASK_SIDE) offset++; /* Skip side if present */
        }
        break;
    case DM1_CREATURE_POSE_ATTACK:
        /* DUNVIEW.C lines 5339-5360: only advance if MASK_ATTACK is
         * actually set.  When ATTACK is unset the front bitmap is
         * reused for the attack pose too. */
        if (gi & DM1_GI_MASK_ATTACK) {
            offset = 1; /* Skip front */
            if (gi & DM1_GI_MASK_SIDE) offset++;
            if (gi & DM1_GI_MASK_BACK) offset++;
        }
        break;
    default:
        break;
    }

    return (unsigned int)(DM1_GRAPHIC_FIRST_CREATURE +
                          a->firstNativeBitmapRelativeIndex + offset);
}

/*
 * Flip determination — ReDMCSB DUNVIEW.C F0115 flip logic.
 *
 * For SIDE pose: flip when directionDelta == 1 (creature facing
 *   from party's right). DUNVIEW.C line 5432: "If creature is
 *   viewed from the right, the side view must be flipped".
 *
 * For ATTACK pose: flip when aspectBits & MASK0x0040_FLIP_BITMAP.
 *   DUNVIEW.C line 5433.
 *
 * For FRONT pose with FLIP_NON_ATTACK: flip when aspectBits & 0x40.
 *   DUNVIEW.C lines 5370-5390.
 */
int dm1_creature_should_flip(int directionDelta, int pose,
                             int attacking, uint16_t graphicInfo,
                             uint8_t aspectBits) {
    (void)attacking;

    if (pose == DM1_CREATURE_POSE_SIDE) {
        if (graphicInfo & DM1_GI_MASK_SIDE) {
            /* Dedicated side bitmap: flip when viewed from right */
            return directionDelta == 1;
        }
        /* Fell back to front: use FLIP_NON_ATTACK */
        if (graphicInfo & DM1_GI_MASK_FLIP_NON_ATTACK) {
            return (aspectBits & 0x40) != 0; /* MASK0x0040_FLIP_BITMAP */
        }
        return 0;
    }

    if (pose == DM1_CREATURE_POSE_ATTACK) {
        /* Attack bitmap: flip when aspect says FLIP_BITMAP */
        return (aspectBits & 0x40) != 0;
    }

    if (pose == DM1_CREATURE_POSE_FRONT) {
        /* Front bitmap: flip when FLIP_NON_ATTACK is set and
         * aspect says FLIP_BITMAP. */
        if (graphicInfo & DM1_GI_MASK_FLIP_NON_ATTACK) {
            return (aspectBits & 0x40) != 0;
        }
        return 0;
    }

    /* Back pose: never flipped in original */
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════
 * Render list management
 * ═══════════════════════════════════════════════════════════════════════ */

void dm1_creature_render_init(DM1_CreatureRenderList* list) {
    if (!list) return;
    list->count = 0;
    memset(list->entries, 0, sizeof(list->entries));
}

/*
 * Sort by viewDepth descending (far first = back-to-front painter order),
 * then by viewColumn for deterministic draw order.
 * ReDMCSB F0128 draws D3 squares first, then D2, D1, D0.
 */
void dm1_creature_render_sort(DM1_CreatureRenderList* list) {
    int i, j, swap;
    DM1_CreatureRenderEntry temp;
    if (!list || list->count < 2) return;
    for (i = 0; i < list->count - 1; i++) {
        for (j = 0; j < list->count - i - 1; j++) {
            swap = 0;
            if (list->entries[j].viewDepth < list->entries[j + 1].viewDepth) {
                swap = 1;
            } else if (list->entries[j].viewDepth == list->entries[j + 1].viewDepth) {
                if (list->entries[j].viewColumn > list->entries[j + 1].viewColumn) {
                    swap = 1;
                }
            }
            if (swap) {
                temp = list->entries[j];
                list->entries[j] = list->entries[j + 1];
                list->entries[j + 1] = temp;
            }
        }
    }
}

const char* dm1_creature_type_name(int creatureType) {
    static const char* const names[27] = {
        "Giant Scorpion",   /* 0 */
        "Swamp Slime",      /* 1 */
        "Giggler",          /* 2 */
        "Wizard Eye",       /* 3 */
        "Pain Rat",         /* 4 */
        "Ruster",           /* 5 */
        "Screamer",         /* 6 */
        "Rockpile",         /* 7 */
        "Ghost",            /* 8 */
        "Stone Golem",      /* 9 */
        "Mummy",            /* 10 */
        "Black Flame",      /* 11 */
        "Skeleton",         /* 12 */
        "Couatl",           /* 13 */
        "Vexirk",           /* 14 */
        "Magenta Worm",     /* 15 */
        "Trolin",           /* 16 */
        "Giant Wasp",       /* 17 */
        "Animated Armour",  /* 18 */
        "Materializer",     /* 19 */
        "Water Elemental",  /* 20 */
        "Oitu",             /* 21 */
        "Demon",            /* 22 */
        "Lord Chaos",       /* 23 */
        "Red Dragon",       /* 24 */
        "Lord Order",       /* 25 */
        "Grey Lord"         /* 26 */
    };
    if (creatureType >= 0 && creatureType < 27) return names[creatureType];
    return "Unknown";
}

/* ═══════════════════════════════════════════════════════════════════════
 * Legacy backward-compat wrappers
 * ═══════════════════════════════════════════════════════════════════════ */

void m11_creature_render_init(DM1_CreatureRenderList* list) {
    dm1_creature_render_init(list);
}

void m11_creature_render_collect(DM1_CreatureRenderList* list,
                                  int partyX, int partyY, int partyDir,
                                  const void* dungeonData) {
    /* Stub: actual collection requires dungeon square iteration.
     * The M11 layer (m11_game_view.c) collects creature data via
     * m11_sample_viewport_cell → ViewportCell.creatureTypes[] before
     * calling draw functions. This M10-level stub preserves the API. */
    if (!list) return;
    list->count = 0;
    (void)partyX; (void)partyY; (void)partyDir; (void)dungeonData;
}

void m11_creature_render_sort(DM1_CreatureRenderList* list) {
    dm1_creature_render_sort(list);
}

int m11_creature_get_graphic(int creatureType, int attacking, int animFrame) {
    /* Source-locked: resolve pose through GraphicInfo flags, matching
     * ReDMCSB F0115 which checks MASK_ATTACK before selecting attack pose.
     * Direction delta 2 (facing party) is assumed for this legacy API. */
    int pose;
    (void)animFrame;
    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;
    pose = dm1_creature_pose_from_delta(2, attacking,
               s_aspects[creatureType].graphicInfo);
    return (int)dm1_creature_native_bitmap_index(creatureType, pose);
}

const char* m11_creature_type_name(int creatureType) {
    return dm1_creature_type_name(creatureType);
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — ANIM.C remaining function citations
 *
 *   ANIM.C:1307 F0908_I
 *   ANIM.C:909 F1024_S
 *   ANIM.C:1302 F1088_O
 *   ANIM.C:1317 F1089_C
 *   ANIM.C:586 F1179_P
 *   ANIM.C:593 F1180_P
 *   ANIM.C:601 F1181_P
 *   ANIM.C:620 F1182_P
 *   ANIM.C:628 F1183_P
 *   ANIM.C:636 F1184_P
 *   ANIM.C:644 F1185_P
 *   ANIM.C:652 F1186_P
 *   ANIM.C:664 F1187_P
 *   ANIM.C:672 F1188_P
 *   ANIM.C:705 F1189_P
 *   ANIM.C:738 F1190_P
 *   ANIM.C:747 F1191_P
 *   ANIM.C:759 F1192_P
 *   ANIM.C:766 F1193_P
 *   ANIM.C:775 F1194_P
 *   ANIM.C:783 F1195_P
 *   ANIM.C:480 F1206_U
 *   ANIM.C:494 F1221_A
 *   ANIM.C:495 F1222_F
 *   ANIM.C:1315 F1230_C
 *   ANIM.C:1379 F1253_F
 *   ANIM.C:98 F1525_O
 *   ANIM.C:110 F1526_C
 *   ANIM.C:24 F1792_L
 *   ANIM.C:118 F1795_P
 *   ANIM.C:159 F1796_C
 *   ANIM.C:171 F1797_R
 *   ANIM.C:67 F1799_L
 *   ANIM.C:106 F1824_A
 *   ANIM.C:8 F1825_F
 *   ANIM.C:2332 F2248_P
 *   ANIM.C:2237 F2257_I
 *   ANIM.C:5 F7227_R
 *   ANIM.C:9 F7228_F
 *   ANIM.C:1684 F8271_P
 *   ANIM.C:1689 F8272_S
 *   ANIM.C:1695 F8273_TR
 *   ANIM.C:1701 F8275_I
 *   ANIM.C:1567 F8290_C
 *   ANIM.C:1592 F8294_S
 *   ANIM.C:1603 F8296_PALETTE_S
 *   ANIM.C:1677 F8303_W
 *   ANIM.C:468 F9000_O
 *   ANIM.C:931 F9001_C
 *   ANIM.C:470 F9002_O
 *   ANIM.C:929 F9003_C
 *   ANIM.C:467 F9004_O
 *   ANIM.C:932 F9005_C
 *   ANIM.C:469 F9006_O
 *   ANIM.C:930 F9007_C
 *   ANIM.C:471 F9016_O
 *   ANIM.C:927 F9017_C
 *   ANIM.C:1304 F9073_D
 * ══════════════════════════════════════════════════════════════════════ */
