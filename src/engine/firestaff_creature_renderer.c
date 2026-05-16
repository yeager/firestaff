
#include "firestaff_creature_renderer.h"
#include <string.h>
#include <stdio.h>

/* Creature sprite base indices in GRAPHICS.DAT (approximate) */
static const int g_creature_sprite_base[] = {
    420, /* 0: Mummy */
    430, /* 1: Screamer */
    440, /* 2: Rock pile */
    450, /* 3: Ghost */
    460, /* 4: Pain rat */
    470, /* 5: Skeleton */
    480, /* 6: Scorpion */
    490, /* 7: Worm */
    500, /* 8: Dragon */
    510, /* 9: Vexirk */
    520, /* 10: Giggler */
    530, /* 11: Stone golem */
    540, /* 12: Black flame */
    550, /* 13: Couatl */
    560, /* 14: Demon */
    570, /* 15: Lord Chaos */
};
#define NUM_CREATURE_TYPES 16

int fs_creatures_find_visible(int party_x, int party_y, int party_dir,
    FS_VisibleCreature *out, int max_out)
{
    /* This would query the creature list in DUNGEON.DAT.
     * For now: return 0 (no creatures visible) — filled by game loop. */
    (void)party_x; (void)party_y; (void)party_dir;
    (void)out; (void)max_out;
    return 0;
}

void fs_creatures_render(FS_ViewportRenderer *vp,
    const FS_VisibleCreature *creatures, int count)
{
    int i;
    if (!vp || !creatures) return;

    for (i = 0; i < count; i++) {
        const FS_VisibleCreature *cr = &creatures[i];
        /* Scale by distance: D0=64px, D1=48px, D2=32px, D3=16px */
        int sprite_size = 64 / (cr->distance + 1);
        int center_x = VP_X + VP_W / 2 + cr->column * (VP_W / 3);
        int center_y = VP_Y + VP_H / 2;
        int dx = center_x - sprite_size / 2;
        int dy = center_y - sprite_size / 2;

        /* Try to draw actual sprite from atlas */
        if (cr->sprite_index > 0 && vp->atlas &&
            cr->sprite_index < vp->atlas->bitmap_count) {
            fs_vp_draw_bitmap_scaled(vp, cr->sprite_index, dx, dy,
                sprite_size, sprite_size);
        } else {
            /* Fallback: colored rectangle for creature */
            int px, py2;
            uint8_t color = 11 + (cr->type_id % 5);
            for (py2 = dy; py2 < dy + sprite_size; py2++)
                for (px = dx; px < dx + sprite_size; px++)
                    if (px >= VP_X && px < VP_X+VP_W && py2 >= VP_Y && py2 < VP_Y+VP_H)
                        vp->framebuffer[py2 * FB_W + px] = color;
        }

        /* Health bar below creature */
        if (cr->health_pct < 100)
            fs_creature_draw_health(vp, dx, dy + sprite_size + 1,
                sprite_size, cr->health_pct);
    }
}

void fs_creature_draw_health(FS_ViewportRenderer *vp,
    int sx, int sy, int width, int health_pct)
{
    int px, fill;
    if (!vp || sy < VP_Y || sy >= VP_Y + VP_H) return;
    fill = width * health_pct / 100;
    for (px = sx; px < sx + width && px < VP_X + VP_W; px++) {
        if (px >= VP_X && sy >= 0 && sy < FB_H) {
            /* Green for healthy, red for low */
            vp->framebuffer[sy * FB_W + px] = (px - sx < fill) ?
                ((health_pct > 30) ? 2 : 4) : 8;
        }
    }
}

