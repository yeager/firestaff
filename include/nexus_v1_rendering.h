#ifndef NEXUS_V1_RENDERING_H
#define NEXUS_V1_RENDERING_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"
#include <stdint.h>

/* ── Wall rendering ─────────────────────────────────────────── */

/* Render a textured wall quad.
 * world_x, world_z: dungeon square origin
 * wall_dir: 0=N, 1=E, 2=S, 3=W
 * tex_page: VDP1 texture page index (or -1 for fallback)
 * u0/v0/u1/v1: UV coordinates (0..255)
 * light_level: 0=full dark, 15=full bright */
void nexus_render_wall_quad(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int wall_dir,
    int tex_page,
    uint8_t u0, uint8_t v0, uint8_t u1, uint8_t v1,
    uint8_t light_level);

/* Render floor and ceiling tiles */
void nexus_render_floor_tile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int floor_tex_page, uint8_t fu0, uint8_t fv0,
    uint8_t light_level);

void nexus_render_ceiling_tile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int ceil_tex_page, uint8_t cu0, uint8_t cv0,
    uint8_t light_level);

/* Render walls for a dungeon square (all visible faces) */
void nexus_render_square_walls(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int square_type,
    int facing_dir,
    uint8_t light_level);

/* ── Creature rendering ──────────────────────────────────────── */

/* Render a creature using its DMDF model or billboard fallback.
 * world_x, world_y: map position
 * anim_frame: current animation frame index
 * facing: 0=N, 1=E, 2=S, 3=W
 * light_level: 0..15 */
void nexus_render_creature(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_V1_Model *model,
    float world_x, float world_y,
    int anim_frame,
    int facing,
    uint8_t light_level);

/* ── Object / item rendering ─────────────────────────────────── */

/* Item categories for fallback sprite selection */
typedef enum {
    NEXUS_ITEM_WEAPON,
    NEXUS_ITEM_ARMOR,
    NEXUS_ITEM_POTION,
    NEXUS_ITEM_SCROLL,
    NEXUS_ITEM_GOLD,
    NEXUS_ITEM_KEY,
    NEXUS_ITEM_TORCH,
    NEXUS_ITEM_OTHER
} Nexus_ItemCategory;

/* Render an item on the dungeon floor as a billboard sprite */
void nexus_render_item(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_y,
    int sprite_id,
    Nexus_ItemCategory category,
    uint8_t light_level);

/* ── Projectile rendering ────────────────────────────────────── */

/* Projectile types — canonical enum lives in nexus_v1_rasterizer.h.
 * Include that header to get the full definition.  This typedef
 * provides a forward reference so rendering-only TUs can use the
 * type without pulling in rasterizer dependencies. */
#ifndef Nexus_ProjectileType
typedef enum Nexus_ProjectileType Nexus_ProjectileType;
#endif

typedef struct {
    float x, y, z;        /* world position */
    int target_x, target_y;
    int anim_frame;
    Nexus_ProjectileType type;
    uint8_t active;
} Nexus_Projectile;

void nexus_render_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_Projectile *proj,
    uint8_t light_level);

void nexus_render_projectiles(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_Projectile *projectiles, int count,
    uint8_t light_level);

/* ── UI / HUD rendering ─────────────────────────────────────── */

/* Champion portrait indices (FACE.BIN 24 entries) */
#define NEXUS_FACE_WARRIOR   0
#define NEXUS_FACE_WIZARD    1
#define NEXUS_FACE_VALKYRIE  2
#define NEXUS_FACE_SAMURAI   3
#define NEXUS_FACE_NINJA     4
#define NEXUS_FACE_PRIEST    5
#define NEXUS_FACE_COUNT     24

/* Render HUD: portraits, health bars, minimap, compass, messages */
void nexus_render_hud(Nexus_Framebuffer *fb,
    const Nexus_V1_GameState *game,
    const Nexus_V1_CreatureManager *creatures,
    const Nexus_V1_Engine *engine);

/* Render champion portrait sprite at screen position */
void nexus_render_portrait(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int portrait_index,
    int selected,
    const Nexus_V1_Engine *engine);

/* Render minimap overlay (top-right corner) */
void nexus_render_minimap(Nexus_Framebuffer *fb,
    const Nexus_V1_Level *level,
    const Nexus_V1_GameState *game,
    const Nexus_V1_CreatureManager *creatures);

/* ── Title screen ──────────────────────────────────────────── */

typedef struct {
    uint8_t *pixels;       /* 320×200 indexed color */
    int width, height;
    int loaded;
} Nexus_TitleScreen;

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine);
void nexus_title_free(Nexus_TitleScreen *title);
void nexus_render_title(const Nexus_TitleScreen *title,
    Nexus_Framebuffer *fb, int frame);
void nexus_render_title_fallback(Nexus_Framebuffer *fb, int frame);

/* ── Main render frame ─────────────────────────────────────── */

/* Render one complete frame of the dungeon */
void nexus_v1_render_frame(Nexus_Framebuffer *fb,
    Nexus_Camera *cam,
    Nexus_V1_Engine *engine,
    const Nexus_Projectile *projectiles, int projectile_count,
    const Nexus_V1_CreatureManager *creatures,
    int frame);

#endif
