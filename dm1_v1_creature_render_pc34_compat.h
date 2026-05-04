#ifndef FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H

/*
 * DM1 V1 Creature Viewport Rendering — pc34 compat layer.
 * Based on ReDMCSB DUNVIEW.C G0219_as_Graphic558_CreatureAspects,
 * M618_GRAPHIC_FIRST_CREATURE, F0128_DUNGEONVIEW_Draw_CPSF creature section.
 * Creature types from ReDMCSB CHAMPION.H C000-C026.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM1 creature types — ReDMCSB CHAMPION.H C000_CREATURE_TYPE_xxx */
enum {
    DM1_CREATURE_GIANT_SCORPION = 0,   /* C000 */
    DM1_CREATURE_SWAMP_SLIME,          /* C001 */
    DM1_CREATURE_GIGGLER,              /* C002 */
    DM1_CREATURE_WIZARD_EYE,           /* C003 */
    DM1_CREATURE_PAIN_RAT,             /* C004 */
    DM1_CREATURE_SCREAMER,             /* C005 */
    DM1_CREATURE_ROCKPILE,             /* C006 */
    DM1_CREATURE_GHOST,                /* C007 */
    DM1_CREATURE_STONE_GOLEM,          /* C008 */
    DM1_CREATURE_MUMMY,               /* C009 */
    DM1_CREATURE_BLACK_FLAME,          /* C010 */
    DM1_CREATURE_SKELETON,             /* C011 */
    DM1_CREATURE_COUATL,               /* C012 */
    DM1_CREATURE_VEXIRK,              /* C013 */
    DM1_CREATURE_MAGENTA_WORM,         /* C014 */
    DM1_CREATURE_TROLIN,               /* C015 */
    DM1_CREATURE_GIANT_WASP,           /* C016 */
    DM1_CREATURE_ANIMATED_ARMOUR,      /* C017 */
    DM1_CREATURE_MATERIALIZER,         /* C018 */
    DM1_CREATURE_WATER_ELEMENTAL,      /* C019 */
    DM1_CREATURE_OITU,                 /* C020 */
    DM1_CREATURE_DEMON,                /* C021 */
    DM1_CREATURE_LORD_CHAOS,           /* C022 */
    DM1_CREATURE_RED_DRAGON,           /* C023 */
    DM1_CREATURE_LORD_ORDER,           /* C024 */
    DM1_CREATURE_GREY_LORD,            /* C025 */
    DM1_CREATURE_ZYTAZ,                /* C026 */
    DM1_CREATURE_TYPE_COUNT            /* C027 = 27 */
};

typedef struct {
    int creatureType;
    int graphicIndex;
    int mapX;
    int mapY;
    int viewDepth;    /* 0=closest (D0), 3=farthest (D3) */
    int viewColumn;   /* -1=left, 0=center, 1=right */
    int flipH;        /* horizontal flip for parity */
    int colorSetIdx;  /* replacement color set ordinal */
    int attacking;    /* 1 if creature is mid-attack */
    int animFrame;    /* animation frame index */
} M11_CreatureRenderEntry;

#define M11_CREATURE_RENDER_MAX 16

typedef struct {
    M11_CreatureRenderEntry entries[M11_CREATURE_RENDER_MAX];
    int count;
} M11_CreatureRenderList;

void m11_creature_render_init(M11_CreatureRenderList* list);
void m11_creature_render_collect(M11_CreatureRenderList* list,
                                  int partyX, int partyY, int partyDir,
                                  const void* dungeonData);
void m11_creature_render_sort(M11_CreatureRenderList* list);
int  m11_creature_get_graphic(int creatureType, int attacking, int animFrame);
const char* m11_creature_type_name(int creatureType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_RENDER_PC34_COMPAT_H */
