
#include "firestaff_creature_renderer.h"

int fs_creatures_find_visible(int party_x, int party_y, int party_dir,
    FS_VisibleCreature *out, int max_out)
{
    /* This legacy API has no authenticated DUNGEON.DAT/GRAPHICS.DAT owner.
     * The active DM1 M11 route uses its source-bound group and creature
     * consumers instead.  Do not manufacture visibility from coordinates. */
    (void)party_x; (void)party_y; (void)party_dir;
    (void)out; (void)max_out;
    return 0;
}

void fs_creatures_render(FS_ViewportRenderer *vp,
    const FS_VisibleCreature *creatures, int count)
{
    /* The old implementation guessed distance rectangles and drew caller
     * supplied sprite indices without a source placement receipt.  That is
     * not a valid DM1 renderer; leave the framebuffer untouched until the
     * source-bound M11 consumer is provided. */
    (void)vp;
    (void)creatures;
    (void)count;
}

void fs_creature_draw_health(FS_ViewportRenderer *vp,
    int sx, int sy, int width, int health_pct)
{
    /* Health bars need the same source-owned panel geometry as the active
     * M11 HUD.  This legacy helper has no such receipt, so it must not draw
     * a guessed overlay. */
    (void)vp;
    (void)sx;
    (void)sy;
    (void)width;
    (void)health_pct;
}
