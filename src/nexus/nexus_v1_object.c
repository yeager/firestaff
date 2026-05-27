#include "nexus_v1_rendering.h"
#include <string.h>
#include <stdio.h>

/* ── Item sprite ID → category ─────────────────────────────── */
/* Currently unused but will be used when ITEM.IBS sprite sheet
 * loading is implemented. Kept for future reference. */
static Nexus_ItemCategory nexus_item_get_category(int sprite_id) {
    if (sprite_id < 0) return NEXUS_ITEM_OTHER;
    if (sprite_id < 32)  return NEXUS_ITEM_WEAPON;
    if (sprite_id < 64)  return NEXUS_ITEM_ARMOR;
    if (sprite_id < 96)  return NEXUS_ITEM_POTION;
    if (sprite_id < 128) return NEXUS_ITEM_SCROLL;
    if (sprite_id < 160) return NEXUS_ITEM_GOLD;
    if (sprite_id < 176) return NEXUS_ITEM_KEY;
    if (sprite_id < 192) return NEXUS_ITEM_TORCH;
    return NEXUS_ITEM_OTHER;
}

/* ── Fallback item sprite: colored rectangle on the floor ───── */

static void nexus_item_fallback_rect(Nexus_Framebuffer *fb,
    Vec2i screen_pos, int size, uint8_t color_index)
{
    int dx, dy;
    int min_x = screen_pos.x - size / 2;
    int max_x = screen_pos.x + size / 2;
    int min_y = screen_pos.y - size / 2;
    int max_y = screen_pos.y + size / 2;

    if (min_x < 0) min_x = 0;
    if (max_x >= NEXUS_FB_W) max_x = NEXUS_FB_W - 1;
    if (min_y < 0) min_y = 0;
    if (max_y >= NEXUS_FB_H) max_y = NEXUS_FB_H - 1;

    for (dy = min_y; dy <= max_y; dy++) {
        for (dx = min_x; dx <= max_x; dx++) {
            int idx = dy * NEXUS_FB_W + dx;
            if (idx >= 0 && idx < NEXUS_FB_W * NEXUS_FB_H)
                fb->color_buffer[idx] = color_index;
        }
    }
}

/* ── Item world position → projected screen position ─────────── */

static Vec2i nexus_item_screen_pos(float world_x, float world_y,
    const Nexus_Camera *cam)
{
    Vec3 world = {world_x + 0.5f, 0.05f, world_y + 0.5f};
    return v3_project(world, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
}

/* ── Item rendering ─────────────────────────────────────────── */

void nexus_render_item(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_y,
    int sprite_id,
    Nexus_ItemCategory category,
    uint8_t light_level)
{
    Vec2i screen_pos;
    int size;
    uint8_t color;

    if (!fb || !cam) return;

    /* TODO: look up sprite in ITEM.IBS sprite sheet */
    (void)sprite_id;
    (void)nexus_item_get_category; /* future: use when ITEM.IBS loads */

    screen_pos = nexus_item_screen_pos(world_x, world_y, cam);

    /* Clip: behind camera */
    {
        Vec4 clip_test = m4_transform(cam->view,
            (Vec4){world_x + 0.5f, 0.05f, world_y + 0.5f, 1});
        if (clip_test.z <= 0) return;
    }

    /* Size and color by category (deterministic fallback) */
    switch (category) {
    case NEXUS_ITEM_WEAPON:
        size = 8; color = 7; break;    /* silver sword */
    case NEXUS_ITEM_ARMOR:
        size = 10; color = 10; break;  /* brown/leather */
    case NEXUS_ITEM_POTION:
        size = 6; color = 12; break;   /* red health */
    case NEXUS_ITEM_SCROLL:
        size = 8; color = 14; break;   /* golden parchment */
    case NEXUS_ITEM_GOLD:
        size = 6 + (sprite_id & 3); color = 14; break; /* gold */
    case NEXUS_ITEM_KEY:
        size = 6; color = 14; break;  /* gold key */
    case NEXUS_ITEM_TORCH:
        size = 6; color = 12; break;   /* orange flame */
    default:
        size = 6; color = 7; break;    /* gray unknown */
    }

    /* Modulate color by light level */
    color = (uint8_t)(color + (light_level >> 2));
    if (color > 15) color = 15;

    nexus_item_fallback_rect(fb, screen_pos, size, color);
}
