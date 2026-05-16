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

#define DM1_CR_MAX_CREATURES     64  /* Max creatures per level */
#define DM1_CR_MAX_VISIBLE        8  /* Max visible in viewport at once */
#define DM1_CR_ANIM_FRAMES        4  /* Animation frames per creature */
#define DM1_CR_FLASH_DURATION     4  /* Damage flash frames */

/* Creature types — from ReDMCSB creature tables (GROUP.C G0217) */
typedef enum {
    M11_CR_MUMMY = 0,
    M11_CR_SCREAMER,
    M11_CR_ROCK_PILE,
    M11_CR_GIANT_SCORPION,
    M11_CR_TROLIN,
    M11_CR_MAGENTA_WORM,
    M11_CR_PAIN_RAT,
    M11_CR_SKELETON,
    M11_CR_GIANT_WASP,
    M11_CR_STONE_GOLEM,
    M11_CR_GHOST,
    M11_CR_COUATL,
    M11_CR_WATER_ELEMENTAL,
    M11_CR_OITU,
    M11_CR_DEMON,
    M11_CR_LORD_CHAOS,
    M11_CR_RED_DRAGON,
    M11_CR_KNIGHT,
    M11_CR_SWAMP_SLIME,
    M11_CR_ANIMATED_ARMOR,
    M11_CR_BLACK_FLAME,
    M11_CR_TYPE_COUNT
} M11_CR_CreatureType;

/* Sprite info for one creature type */
typedef struct {
    uint16_t gfx_index;         /* Base bitmap index in GRAPHICS.DAT */
    uint16_t frame_count;       /* Number of animation frames */
    uint16_t base_width;        /* Sprite width at depth 0 */
    uint16_t base_height;       /* Sprite height at depth 0 */
    bool     mirror_walk;       /* Mirror sprite for left/right walk */
} M11_CR_SpriteInfo;

/* Per-creature instance state */
typedef struct {
    M11_CR_CreatureType type;
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
} M11_CR_Creature;

/* Viewport rendering info for a visible creature */
typedef struct {
    const M11_CR_Creature* creature;
    int16_t screen_x, screen_y; /* Viewport screen position */
    int16_t draw_w, draw_h;     /* Scaled size for depth */
    uint8_t depth;              /* 0=closest, 3=farthest */
    uint8_t side;               /* 0=left, 1=center, 2=right */
    bool    flipped;            /* Horizontally mirrored */
} M11_CR_ViewportEntry;

typedef struct {
    M11_CR_Creature creatures[DM1_CR_MAX_CREATURES];
    uint16_t creature_count;
    M11_CR_SpriteInfo sprite_info[M11_CR_TYPE_COUNT];
    M11_CR_ViewportEntry visible[DM1_CR_MAX_VISIBLE];
    uint8_t visible_count;
    int16_t party_x, party_y;   /* Current party position for visibility */
    uint8_t party_facing;       /* Party direction */
} M11_CR_State;

void m11_cr_init(M11_CR_State* state);
void m11_cr_setup_sprite_table(M11_CR_State* state);
uint16_t m11_cr_add_creature(M11_CR_State* state, M11_CR_CreatureType type,
                              int16_t x, int16_t y, uint8_t facing,
                              int16_t hp);
void m11_cr_set_party_pos(M11_CR_State* state, int16_t x, int16_t y,
                           uint8_t facing);
void m11_cr_update_visibility(M11_CR_State* state);
void m11_cr_animate_frame(M11_CR_State* state);
void m11_cr_damage(M11_CR_State* state, uint16_t index, int16_t damage);
bool m11_cr_is_alive(const M11_CR_State* state, uint16_t index);
uint8_t m11_cr_get_visible_count(const M11_CR_State* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_VIEWPORT_PC34_COMPAT_H */
