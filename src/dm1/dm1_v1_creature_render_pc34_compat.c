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
 *   F0115 creature draw section                       — lines 5201-5520
 *
 * ReDMCSB DEFS.H:
 *   M618_GRAPHIC_FIRST_CREATURE = 584                 — line 2392
 *   GraphicInfo masks                                 — lines 1618-1629
 *   M071_COORDINATE_SET, M072_TRANSPARENT_COLOR       — lines 2016-2017
 */

#include "dm1_v1_creature_render_pc34_compat.h"

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

/*
 * Aspect frame cycling — ReDMCSB GROUP.C F0179 lines 222-305.
 *
 * The original does not increment a linear sprite-frame number. Instead it
 * rewrites the active-group Aspect bits. Bits 0-5 hold per-update offsets;
 * this bounded helper implements only the frame-selection bits used by
 * DUNVIEW.C F0115: MASK0x0040_FLIP_BITMAP and MASK0x0080_IS_ATTACKING.
 * The caller supplies randomBit as the source-locked M005_RANDOM(2) result.
 */
uint8_t dm1_creature_cycle_aspect_frame(int creatureType,
                                        uint8_t previousAspect,
                                        int attacking, int randomBit) {
    uint16_t gi;
    uint8_t aspect;
    int randomSet;

    if (creatureType < 0 || creatureType >= DM1_CREATURE_TYPE_COUNT) return 0;

    gi = s_aspects[creatureType].graphicInfo;
    aspect = (uint8_t)(previousAspect &
                       (DM1_CREATURE_ASPECT_IS_ATTACKING |
                        DM1_CREATURE_ASPECT_FLIP_BITMAP));
    randomSet = randomBit & 1;

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
    offset = 0; /* Start at front bitmap */

    switch (pose) {
    case DM1_CREATURE_POSE_FRONT:
        break;
    case DM1_CREATURE_POSE_SIDE:
        if (gi & DM1_GI_MASK_SIDE) {
            offset = 1; /* Side follows front */
        }
        break;
    case DM1_CREATURE_POSE_BACK:
        offset = 1; /* Skip front */
        if (gi & DM1_GI_MASK_SIDE) offset++; /* Skip side if present */
        break;
    case DM1_CREATURE_POSE_ATTACK:
        offset = 1; /* Skip front */
        if (gi & DM1_GI_MASK_SIDE) offset++;
        if (gi & DM1_GI_MASK_BACK) offset++;
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

