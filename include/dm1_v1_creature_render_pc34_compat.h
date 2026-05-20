#ifndef FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H

/*
 * DM1 V1 Creature Viewport Rendering — pc34 compat layer.
 *
 * Source reference: ReDMCSB DUNVIEW.C
 *   F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF (line 4547)
 *   G0219_as_Graphic558_CreatureAspects[27] (line 1625)
 *   G0224_aaaauc_Graphic558_CreatureCoordinateSets[3][11][5][2] (line 1836)
 *   G0221/G0222_auc_Graphic558_PaletteChanges_Creature_D3/D2 (line 1821-1822)
 *
 * Source reference: ReDMCSB DEFS.H
 *   M618_GRAPHIC_FIRST_CREATURE = 584 (PC 3.4, line 2392)
 *   MASK0x0003_SIZE (line 1597), C0_SIZE_QUARTER_SQUARE..C2_SIZE_FULL_SQUARE
 *   MASK0x0004_FLIP_NON_ATTACK, MASK0x0008_SIDE, MASK0x0010_BACK,
 *   MASK0x0020_ATTACK, MASK0x0080_SPECIAL_D2_FRONT,
 *   MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT, MASK0x0200_FLIP_ATTACK
 *   M050_CREATURE_VALUE (line 1369): group cell / direction extraction
 *   M071_COORDINATE_SET, M072_TRANSPARENT_COLOR (lines 2016-2017)
 *   M022_HORIZONTAL_OFFSET, M023_VERTICAL_OFFSET (lines 591-600)
 *
 * Source reference: ReDMCSB GROUP.C
 *   F0176_GROUP_GetCreatureOrdinalInCell (line 69)
 *   F0179_GROUP_GetCreatureAspectUpdateTime (lines 187-308)
 *   F0209 update-aspect dispatch (lines 2051-2075)
 *
 * Source reference: ReDMCSB CHAMPION.H
 *   C000_CREATURE_TYPE_xxx .. C026 (creature type constants)
 *
 * This M10 module provides:
 *   - Source-locked creature aspect data (G0219 verbatim)
 *   - Source-locked creature palette remapping tables (G0221, G0222)
 *   - Source-locked creature coordinate sets (G0224, subset)
 *   - Source-locked creature GraphicInfo masks and accessors
 *   - Creature sprite index computation from aspect data
 *   - Creature direction delta and pose selection
 *   - Render list collection, sort, and type-name query
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DM1 creature types — ReDMCSB CHAMPION.H C000_CREATURE_TYPE_xxx ── */
enum {
    DM1_CREATURE_GIANT_SCORPION = 0,   /* C000 */
    DM1_CREATURE_SWAMP_SLIME,          /* C001 */
    DM1_CREATURE_GIGGLER,              /* C002 */
    DM1_CREATURE_WIZARD_EYE,           /* C003 */
    DM1_CREATURE_PAIN_RAT,             /* C004 */
    DM1_CREATURE_RUSTER,               /* C005 */
    DM1_CREATURE_SCREAMER,             /* C006 */
    DM1_CREATURE_ROCKPILE,             /* C007 */
    DM1_CREATURE_GHOST,                /* C008 */
    DM1_CREATURE_STONE_GOLEM,          /* C009 */
    DM1_CREATURE_MUMMY,               /* C010 */
    DM1_CREATURE_BLACK_FLAME,          /* C011 */
    DM1_CREATURE_SKELETON,             /* C012 */
    DM1_CREATURE_COUATL,               /* C013 */
    DM1_CREATURE_VEXIRK,              /* C014 */
    DM1_CREATURE_MAGENTA_WORM,         /* C015 */
    DM1_CREATURE_TROLIN,               /* C016 */
    DM1_CREATURE_GIANT_WASP,           /* C017 */
    DM1_CREATURE_ANIMATED_ARMOUR,      /* C018 */
    DM1_CREATURE_MATERIALIZER,         /* C019 */
    DM1_CREATURE_WATER_ELEMENTAL,      /* C020 */
    DM1_CREATURE_OITU,                 /* C021 */
    DM1_CREATURE_DEMON,                /* C022 */
    DM1_CREATURE_LORD_CHAOS,           /* C023 */
    DM1_CREATURE_RED_DRAGON,           /* C024 */
    DM1_CREATURE_LORD_ORDER,           /* C025 */
    DM1_CREATURE_GREY_LORD,            /* C026 */
    DM1_CREATURE_TYPE_COUNT            /* C027 = 27 */
};

/* ── Creature size constants — ReDMCSB DEFS.H line 1612-1614 ── */
#define DM1_CREATURE_SIZE_QUARTER  0   /* C0_SIZE_QUARTER_SQUARE */
#define DM1_CREATURE_SIZE_HALF     1   /* C1_SIZE_HALF_SQUARE */
#define DM1_CREATURE_SIZE_FULL     2   /* C2_SIZE_FULL_SQUARE */

/* ── Creature GraphicInfo masks — ReDMCSB DEFS.H lines 1618-1629 ── */
#define DM1_GI_MASK_ADDITIONAL          0x0003u
#define DM1_GI_MASK_FLIP_NON_ATTACK     0x0004u
#define DM1_GI_MASK_SIDE                0x0008u
#define DM1_GI_MASK_BACK                0x0010u
#define DM1_GI_MASK_ATTACK              0x0020u
#define DM1_GI_MASK_SPECIAL_D2_FRONT    0x0080u
#define DM1_GI_MASK_D2_FRONT_IS_FLIPPED 0x0100u
#define DM1_GI_MASK_FLIP_ATTACK         0x0200u
#define DM1_GI_MASK_FLIP_DURING_ATTACK  0x0400u

/* M618_GRAPHIC_FIRST_CREATURE for PC 3.4 — ReDMCSB DEFS.H line 2392 */
#define DM1_GRAPHIC_FIRST_CREATURE 584

/* Active-group Aspect frame bits — ReDMCSB DEFS.H lines 603-606 */
#define DM1_CREATURE_ASPECT_FLIP_BITMAP  0x40u
#define DM1_CREATURE_ASPECT_IS_ATTACKING 0x80u

/* ── Creature aspect — matches ReDMCSB CREATURE_ASPECT typedef ── */
typedef struct {
    int16_t firstNativeBitmapRelativeIndex;
    int16_t firstDerivedBitmapIndex;
    /* Packed: high nibble = coordinate set, low nibble = transparent color.
     * ReDMCSB: M071_COORDINATE_SET / M072_TRANSPARENT_COLOR */
    uint8_t coordinateSet_transparentColor;
    /* Packed: low nibble = color9 repl set, high nibble = color10 repl set */
    uint8_t replacementColorSetIndices;
    /* Creature GraphicInfo from G0243_as_Graphic559_CreatureInfo[].GraphicInfo */
    uint16_t graphicInfo;
} DM1_CreatureAspect;

/* ── Creature pose enumeration ── */
enum {
    DM1_CREATURE_POSE_FRONT  = 0,
    DM1_CREATURE_POSE_SIDE   = 1,
    DM1_CREATURE_POSE_BACK   = 2,
    DM1_CREATURE_POSE_ATTACK = 3
};

/* ── Render list entry ── */
typedef struct {
    int creatureType;
    int graphicIndex;
    int mapX;
    int mapY;
    int viewDepth;
    int viewColumn;
    int flipH;
    int colorSetIdx;
    int attacking;
    int pose;
    int directionDelta;
    int animFrame;
} DM1_CreatureRenderEntry;

#define DM1_CREATURE_RENDER_MAX 16

typedef struct {
    DM1_CreatureRenderEntry entries[DM1_CREATURE_RENDER_MAX];
    int count;
} DM1_CreatureRenderList;

/* ── Source-locked API ── */

const DM1_CreatureAspect* dm1_creature_aspects(void);
unsigned int dm1_creature_native_bitmap_index(int creatureType, int pose);
int dm1_creature_direction_delta(int partyDir, int creatureDir);
int dm1_creature_pose_from_delta(int directionDelta, int attacking,
                                 uint16_t graphicInfo);
int dm1_creature_should_flip(int directionDelta, int pose,
                             int attacking, uint16_t graphicInfo,
                             uint8_t aspectBits);
uint8_t dm1_creature_cycle_aspect_frame(int creatureType,
                                        uint8_t previousAspect,
                                        int attacking, int randomBit);
int dm1_creature_next_aspect_update_delay(int animationTicks, int attacking, int randomBit);
int dm1_creature_coordinate_set(int creatureType);
int dm1_creature_transparent_color(int creatureType);
const unsigned char* dm1_creature_palette_d3(void);
const unsigned char* dm1_creature_palette_d2(void);

void dm1_creature_render_init(DM1_CreatureRenderList* list);
void dm1_creature_render_sort(DM1_CreatureRenderList* list);
const char* dm1_creature_type_name(int creatureType);

/* ── Legacy backward-compat wrappers ── */
typedef DM1_CreatureRenderEntry M11_CreatureRenderEntry;
typedef DM1_CreatureRenderList  M11_CreatureRenderList;
#define M11_CREATURE_RENDER_MAX DM1_CREATURE_RENDER_MAX

void m11_creature_render_init(DM1_CreatureRenderList* list);
void m11_creature_render_collect(DM1_CreatureRenderList* list,
                                  int partyX, int partyY, int partyDir,
                                  const void* dungeonData);
void m11_creature_render_sort(DM1_CreatureRenderList* list);
int  m11_creature_get_graphic(int creatureType, int attacking, int animFrame);
const char* m11_creature_type_name(int creatureType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H */
