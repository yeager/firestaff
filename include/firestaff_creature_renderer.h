
#ifndef FIRESTAFF_CREATURE_RENDERER_H
#define FIRESTAFF_CREATURE_RENDERER_H

#include "firestaff_viewport_renderer.h"
#include <stdint.h>

/* Creature rendering — draws creatures in the viewport.
 * DM1 creatures are 2D sprites from GRAPHICS.DAT, drawn at distance-based scale.
 *
 * Creature sprite indices in GRAPHICS.DAT:
 *   Each creature type has 4 facing sprites × animation frames.
 *   Creatures at D0 are largest, D3 smallest. */

#define MAX_VISIBLE_CREATURES 16

typedef struct {
    int type_id;
    int x, y;
    int facing;        /* relative to party */
    int distance;      /* 0-3 (D0-D3) */
    int column;        /* -1=left, 0=center, 1=right */
    int health_pct;    /* 0-100 */
    int anim_frame;
    int sprite_index;  /* GRAPHICS.DAT index */
} FS_VisibleCreature;

/* Find creatures visible in current view cone */
int fs_creatures_find_visible(int party_x, int party_y, int party_dir,
    FS_VisibleCreature *out, int max_out);

/* Render visible creatures into viewport */
void fs_creatures_render(FS_ViewportRenderer *vp,
    const FS_VisibleCreature *creatures, int count);

/* Draw creature health bar */
void fs_creature_draw_health(FS_ViewportRenderer *vp,
    int screen_x, int screen_y, int width, int health_pct);

#endif

