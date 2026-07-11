/* DM1 V1 Creature Viewport Rendering — source-locked from ReDMCSB
 * GROUP.C: creature group management, G0217-G0228 creature data
 * DUNVIEW.C: creature drawing in viewport at scaled depths
 * OBJECT.C: creature object table, thing types
 * PROJEXPL.C: creature hit flash rendering */
#ifndef FIRESTAFF_DM1_V1_CREATURE_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_VIEWPORT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CREATURE_VIEWPORT_MAX_CREATURES_PC34     64  /* Max creatures per level */
#define DM1_V1_CREATURE_VIEWPORT_MAX_VISIBLE_PC34        8  /* Max visible in viewport at once */
#define DM1_V1_CREATURE_VIEWPORT_ANIM_FRAMES_PC34        4  /* Animation frames per creature */
#define DM1_V1_CREATURE_VIEWPORT_FLASH_DURATION_PC34     4  /* Damage flash frames */

/* Creature types — from ReDMCSB creature tables (GROUP.C G0217) */
typedef enum {
    DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34 = 0,
    DM1_V1_CREATURE_VIEWPORT_SCREAMER_PC34,
    DM1_V1_CREATURE_VIEWPORT_ROCK_PILE_PC34,
    DM1_V1_CREATURE_VIEWPORT_GIANT_SCORPION_PC34,
    DM1_V1_CREATURE_VIEWPORT_TROLIN_PC34,
    DM1_V1_CREATURE_VIEWPORT_MAGENTA_WORM_PC34,
    DM1_V1_CREATURE_VIEWPORT_PAIN_RAT_PC34,
    DM1_V1_CREATURE_VIEWPORT_SKELETON_PC34,
    DM1_V1_CREATURE_VIEWPORT_GIANT_WASP_PC34,
    DM1_V1_CREATURE_VIEWPORT_STONE_GOLEM_PC34,
    DM1_V1_CREATURE_VIEWPORT_GHOST_PC34,
    DM1_V1_CREATURE_VIEWPORT_COUATL_PC34,
    DM1_V1_CREATURE_VIEWPORT_WATER_ELEMENTAL_PC34,
    DM1_V1_CREATURE_VIEWPORT_OITU_PC34,
    DM1_V1_CREATURE_VIEWPORT_DEMON_PC34,
    DM1_V1_CREATURE_VIEWPORT_LORD_CHAOS_PC34,
    DM1_V1_CREATURE_VIEWPORT_RED_DRAGON_PC34,
    DM1_V1_CREATURE_VIEWPORT_KNIGHT_PC34,
    DM1_V1_CREATURE_VIEWPORT_SWAMP_SLIME_PC34,
    DM1_V1_CREATURE_VIEWPORT_ANIMATED_ARMOR_PC34,
    DM1_V1_CREATURE_VIEWPORT_BLACK_FLAME_PC34,
    DM1_V1_CREATURE_VIEWPORT_TYPE_COUNT_PC34
} DM1_V1_CreatureViewportTypePc34;

/* Sprite info for one creature type */
typedef struct {
    uint16_t gfx_index;         /* Base bitmap index in GRAPHICS.DAT */
    uint16_t frame_count;       /* Number of animation frames */
    uint16_t base_width;        /* Sprite width at depth 0 */
    uint16_t base_height;       /* Sprite height at depth 0 */
    bool     mirror_walk;       /* Mirror sprite for left/right walk */
} DM1_V1_CreatureViewportSpriteInfoPc34;

/* Per-creature instance state */
typedef struct {
    DM1_V1_CreatureViewportTypePc34 type;
    int16_t  map_x, map_y;      /* Tile position */
    uint8_t  facing;            /* 0-3 direction */
    uint8_t  cell;              /* Sub-cell position (0-3) within tile */
    int16_t  hit_points;
    int16_t  max_hit_points;
    uint8_t  anim_frame;        /* Current animation frame */
    uint8_t  anim_timer;        /* Frames until next anim step */
    uint8_t  flash_timer;       /* Damage flash countdown */
    bool     alive;
    bool     visible;           /* Currently in viewport */
} DM1_V1_CreatureViewportCreaturePc34;

/* Viewport rendering info for a visible creature */
typedef struct {
    const DM1_V1_CreatureViewportCreaturePc34* creature;
    int16_t screen_x, screen_y; /* Viewport screen position */
    int16_t draw_w, draw_h;     /* Scaled size for depth */
    uint8_t depth;              /* 0=closest, 3=farthest */
    uint8_t side;               /* 0=left, 1=center, 2=right */
    bool    flipped;            /* Horizontally mirrored */
} DM1_V1_CreatureViewportEntryPc34;

typedef struct {
    DM1_V1_CreatureViewportCreaturePc34 creatures[DM1_V1_CREATURE_VIEWPORT_MAX_CREATURES_PC34];
    uint16_t creature_count;
    DM1_V1_CreatureViewportSpriteInfoPc34 sprite_info[DM1_V1_CREATURE_VIEWPORT_TYPE_COUNT_PC34];
    DM1_V1_CreatureViewportEntryPc34 visible[DM1_V1_CREATURE_VIEWPORT_MAX_VISIBLE_PC34];
    uint8_t visible_count;
    int16_t party_x, party_y;   /* Current party position for visibility */
    uint8_t party_facing;       /* Party direction */
} DM1_V1_CreatureViewportStatePc34;

void DM1_V1_CreatureViewport_InitPc34Compat(DM1_V1_CreatureViewportStatePc34* state);
void DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(DM1_V1_CreatureViewportStatePc34* state);
uint16_t DM1_V1_CreatureViewport_AddCreaturePc34Compat(DM1_V1_CreatureViewportStatePc34* state, DM1_V1_CreatureViewportTypePc34 type,
                              int16_t x, int16_t y, uint8_t facing,
                              int16_t hp);
void DM1_V1_CreatureViewport_SetPartyPosPc34Compat(DM1_V1_CreatureViewportStatePc34* state, int16_t x, int16_t y,
                           uint8_t facing);
void DM1_V1_CreatureViewport_UpdateVisibilityPc34Compat(DM1_V1_CreatureViewportStatePc34* state);
void DM1_V1_CreatureViewport_AnimateFramePc34Compat(DM1_V1_CreatureViewportStatePc34* state);
void DM1_V1_CreatureViewport_DamagePc34Compat(DM1_V1_CreatureViewportStatePc34* state, uint16_t index, int16_t damage);
bool DM1_V1_CreatureViewport_IsAlivePc34Compat(const DM1_V1_CreatureViewportStatePc34* state, uint16_t index);
uint8_t DM1_V1_CreatureViewport_GetVisibleCountPc34Compat(const DM1_V1_CreatureViewportStatePc34* state);

#define DM1_CR_MAX_CREATURES DM1_V1_CREATURE_VIEWPORT_MAX_CREATURES_PC34
#define DM1_CR_MAX_VISIBLE DM1_V1_CREATURE_VIEWPORT_MAX_VISIBLE_PC34
#define DM1_CR_ANIM_FRAMES DM1_V1_CREATURE_VIEWPORT_ANIM_FRAMES_PC34
#define DM1_CR_FLASH_DURATION DM1_V1_CREATURE_VIEWPORT_FLASH_DURATION_PC34

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_VIEWPORT_PC34_COMPAT_H */
