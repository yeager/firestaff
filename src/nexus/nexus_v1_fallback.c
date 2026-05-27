#include "nexus_v1_rendering.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Fallback registry ──────────────────────────────────────── */

/* Fallback kinds */
typedef enum {
    NEXUS_FALLBACK_NONE = 0,
    NEXUS_FALLBACK_WALL,
    NEXUS_FALLBACK_CREATURE,
    NEXUS_FALLBACK_OBJECT,
    NEXUS_FALLBACK_PROJECTILE,
    NEXUS_FALLBACK_UI_PORTRAIT,
    NEXUS_FALLBACK_TITLE,
    NEXUS_FALLBACK_MINIMAP
} Nexus_FallbackKind;

/* A fallback sprite entry */
typedef struct {
    Nexus_FallbackKind kind;
    int id;              /* type identifier */
    uint8_t base_color;  /* base palette color */
    const char *description;
} Nexus_FallbackEntry;

#define MAX_FALLBACK_ENTRIES 256
static Nexus_FallbackEntry g_fallback_entries[MAX_FALLBACK_ENTRIES];
static int g_fallback_count = 0;

/* Register a fallback entry (deterministic — same input always gets same output) */
static void nexus_fallback_register(Nexus_FallbackKind kind,
    int id, uint8_t base_color, const char *desc)
{
    if (g_fallback_count >= MAX_FALLBACK_ENTRIES) return;
    g_fallback_entries[g_fallback_count++] = (Nexus_FallbackEntry){
        kind, id, base_color, desc
    };
}

/* ── Wall type → deterministic fallback texture ─────────────── */

/* Hash a wall position + type to get a deterministic base color.
 * This ensures the same wall always renders identically. */
static uint8_t nexus_fallback_wall_color(int world_x, int world_y, int wall_dir) {
    /* Mix position and direction into a deterministic hash.
     * Using a simple hash: (x * 7 + y * 13 + dir * 17) & 0x0F */
    int hash = (world_x * 7 + world_y * 13 + wall_dir * 17) & 0x0F;
    /* Map hash to wall-like colors: grays and browns (palette 5-7, 10) */
    static const uint8_t wall_colors[16] = {
        5, 5, 5, 6, 6, 6, 7, 7, 5, 6, 10, 5, 6, 7, 5, 6
    };
    return wall_colors[hash];
}

/* ── Creature type → deterministic fallback billboard color ─── */

static uint8_t nexus_fallback_creature_color(int creature_type_idx) {
    static const uint8_t colors[] = {
        10, /* Scorpion — brown */
        10, /* Mummy — brown */
        12, /* Dragon — firebrick */
        7,  /* Skeleton — light gray */
        4,  /* Ghost — purple */
        10, /* Worm — brown */
        6,  /* Golem — medium gray */
        10, /* Spider — brown */
    };
    return colors[creature_type_idx & 7];
}

/* ── Item category → deterministic fallback color ─────────────── */

static uint8_t nexus_fallback_item_color(int sprite_id) {
    int category = sprite_id >> 5;  /* divide by 32 */
    switch (category) {
    case 0: return 7;   /* weapon — silver */
    case 1: return 10;  /* armor — brown */
    case 2: return 12;  /* potion — red */
    case 3: return 14;  /* scroll — golden */
    case 4: return 14;  /* gold — gold */
    case 5: return 14;  /* key — gold */
    case 6: return 12;  /* torch — orange */
    default: return 7;   /* other — gray */
    }
}

/* ── Fallback: procedural wall texture page ─────────────────── */

/* Generate a procedural brick pattern for a wall texture page.
 * This is used when the real DGN geometry blob cannot be parsed. */
void nexus_fallback_generate_wall_texture(uint8_t *page,
    int wall_type, int wall_x, int wall_y)
{
    int brick_w, brick_h, mortar;
    uint8_t brick_c, mortar_c;

    if (!page) return;

    /* Vary texture by wall type and position (deterministic) */
    switch (wall_type & 3) {
    case 0: /* solid stone */
        brick_w = 32; brick_h = 16; mortar = 2;
        brick_c = 5; mortar_c = 8; break;
    case 1: /* rough */
        brick_w = 24; brick_h = 12; mortar = 3;
        brick_c = 6; mortar_c = 8; break;
    case 2: /* brick */
        brick_w = 16; brick_h = 8; mortar = 2;
        brick_c = 10; mortar_c = 8; break;
    default: /* carved */
        brick_w = 32; brick_h = 32; mortar = 1;
        brick_c = 7; mortar_c = 9; break;
    }

    /* Seed variation by wall position */
    {
        int seed = (wall_x * 17 + wall_y * 31 + wall_type * 7) & 0xFF;
        brick_c = (uint8_t)((brick_c + seed) & 0x0F);
        mortar_c = (uint8_t)((mortar_c + (seed >> 2)) & 0x0F);
    }

    /* Generate brick pattern */
    /* Fill entire 256x256 page with mortar color (bottom-to-top byte order) */
    {
        int x, y;
        for (y = 0; y < 256; y++) {
            for (x = 0; x < 256; x++) {
                int byte_idx = (y * 256 + x) >> 1;
                int shift = (x & 1) ? 0 : 4;
                page[byte_idx] = (uint8_t)((mortar_c << shift) | (page[byte_idx] & ~(0x0F << shift)));
            }
        }
    }

    /* Draw horizontal mortar lines */
    {
        int y;
        for (y = 0; y < 256; y++) {
            if ((y % brick_h) < mortar) {
                int x;
                for (x = 0; x < 256; x++) {
                    int byte_idx = (y * 256 + x) >> 1;
                    int shift = (x & 1) ? 0 : 4;
                    page[byte_idx] = (uint8_t)((page[byte_idx] & ~(0x0F << shift)) | (mortar_c << shift));
                }
            }
        }
    }
}

/* ── Fallback: procedural floor texture page ───────────────── */

/* Generate a procedural stone floor texture page */
void nexus_fallback_generate_floor_texture(uint8_t *page, int level_idx) {
    int x, y;
    if (!page) return;

    /* Fill with dark floor color */
    for (y = 0; y < 256; y++) {
        for (x = 0; x < 256; x++) {
            int byte_idx = (y * 256 + x) >> 1;
            int shift = (x & 1) ? 0 : 4;
            /* Stone floor: alternating dark/medium gray tiles */
            uint8_t color = ((x ^ y) & 16) ? 8 : 9;
            page[byte_idx] = (uint8_t)((page[byte_idx] & ~(0x0F << shift)) | (color << shift));
        }
    }
}

/* ── Fallback: creature billboard sprite ───────────────────── */

/* Build a fallback billboard quad for a creature.
 * The billboard always faces the camera. */
void nexus_fallback_build_creature_billboard(Nexus_RasterVertex *verts,
    float world_x, float world_y,
    float height, float width,
    uint8_t creature_color,
    const Nexus_Camera *cam)
{
    int i;
    Vec3 center = {world_x + 0.5f, height * 0.5f, world_y + 0.5f};
    Vec3 to_cam = v3_normalize(v3_sub(cam->pos, center));
    Vec3 up = {0, 1, 0};
    Vec3 right = v3_normalize(v3_cross(to_cam, up));
    float hw = width * 0.5f;

    verts[0].position = v3_sub(v3_sub(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    verts[1].position = v3_add(v3_sub(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    verts[2].position = v3_add(v3_add(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    verts[3].position = v3_sub(v3_add(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));

    for (i = 0; i < 4; i++) {
        verts[i].color = creature_color;
        verts[i].uv = (Vec2){0, 0};
    }

    verts[0].uv = (Vec2){0, 1};
    verts[1].uv = (Vec2){0, 0};
    verts[2].uv = (Vec2){1, 0};
    verts[3].uv = (Vec2){1, 1};
}

/* ── Fallback: projectile circle+tail ──────────────────────── */

void nexus_fallback_render_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_y, float world_z,
    int target_x, int target_y,
    Nexus_ProjectileType type,
    int anim_frame)
{
    Vec2i screen_pos, target_pos;
    int radius;
    uint8_t color;
    Vec3 world = {world_x, world_y, world_z};
    Vec4 clip = m4_transform(cam->view, (Vec4){world_x, world_y, world_z, 1});
    if (clip.z <= 0) return;

    screen_pos = v3_project(world, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
    radius = 3 + (anim_frame & 3);

    switch (type) {
    case NEXUS_PROJ_FIREBALL:   color = 12; break; /* firebrick */
    case NEXUS_PROJ_ICEBOLT:    color = 13; break; /* royal blue */
    case NEXUS_PROJ_LIGHTNING:  color = 15; break; /* white */
    case NEXUS_PROJ_ARROW:      color = 6;  break; /* gray */
    case NEXUS_PROJ_POISON:     color = 10; break; /* brown/green */
    case NEXUS_PROJ_DEATHRAY:   color = 4;  break; /* purple */
    case NEXUS_PROJ_ACID:       color = 10; break; /* brown-green */
    default:                     color = 15; break; /* white */
    }

    /* Draw circle (approximate with a filled rect for simplicity) */
    {
        int dx, dy;
        for (dy = screen_pos.y - radius; dy <= screen_pos.y + radius; dy++)
            for (dx = screen_pos.x - radius; dx <= screen_pos.x + radius; dx++)
                if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                    if ((dx - screen_pos.x) * (dx - screen_pos.x) + (dy - screen_pos.y) * (dy - screen_pos.y) <= radius * radius)
                        fb->color_buffer[dy * NEXUS_FB_W + dx] = color;
    }

    /* Draw directional tail */
    {
        Vec3 t_world = {(float)target_x + 0.5f, 0.5f, (float)target_y + 0.5f};
        target_pos = v3_project(t_world, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
        int dx = target_pos.x - screen_pos.x;
        int dy = target_pos.y - screen_pos.y;
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (steps > 0) {
            float fx = screen_pos.x, fy = screen_pos.y;
            int i;
            for (i = 0; i < steps; i++) {
                int px = (int)(fx + 0.5f), py = (int)(fy + 0.5f);
                if (px >= 0 && px < NEXUS_FB_W && py >= 0 && py < NEXUS_FB_H)
                    fb->color_buffer[py * NEXUS_FB_W + px] = (uint8_t)(color - 4);
                fx += (float)dx / steps;
                fy += (float)dy / steps;
            }
        }
    }
}

/* ── Fallback: UI portrait for missing FACE.BIN ─────────────── */

void nexus_fallback_render_portrait(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int champion_index,
    int selected)
{
    /* Use champion class to select color */
    uint8_t colors[6] = {12, 13, 10, 14, 4, 6};
    uint8_t color = colors[champion_index % 6];

    /* Portrait background */
    int border = selected ? 15 : 7;
    int dx, dy;

    /* Fill background */
    for (dy = screen_y; dy < screen_y + 40; dy++)
        for (dx = screen_x; dx < screen_x + 48; dx++)
            if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                fb->color_buffer[dy * NEXUS_FB_W + dx] = color;

    /* Border */
    for (dx = screen_x; dx < screen_x + 48; dx++) {
        if (screen_y >= 0 && screen_y < NEXUS_FB_H && dx >= 0 && dx < NEXUS_FB_W)
            fb->color_buffer[screen_y * NEXUS_FB_W + dx] = border;
        if (screen_y + 39 >= 0 && screen_y + 39 < NEXUS_FB_H && dx >= 0 && dx < NEXUS_FB_W)
            fb->color_buffer[(screen_y + 39) * NEXUS_FB_W + dx] = border;
    }
    for (dy = screen_y; dy < screen_y + 40; dy++) {
        if (dy >= 0 && dy < NEXUS_FB_H && screen_x >= 0 && screen_x < NEXUS_FB_W)
            fb->color_buffer[dy * NEXUS_FB_W + screen_x] = border;
        if (dy >= 0 && dy < NEXUS_FB_H && screen_x + 47 >= 0 && screen_x + 47 < NEXUS_FB_W)
            fb->color_buffer[dy * NEXUS_FB_W + screen_x + 47] = border;
    }

    /* Class icon */
    for (dy = screen_y + 8; dy < screen_y + 20; dy++)
        for (dx = screen_x + 8; dx < screen_x + 40; dx++)
            if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                fb->color_buffer[dy * NEXUS_FB_W + dx] = 7;
    for (dy = screen_y + 4; dy < screen_y + 16; dy++)
        for (dx = screen_x + 18; dx < screen_x + 30; dx++)
            if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                fb->color_buffer[dy * NEXUS_FB_W + dx] = color;
}

/* ── Fallback: minimap for missing SMAP*.BIN ───────────────── */

void nexus_fallback_render_minimap(Nexus_Framebuffer *fb,
    const Nexus_V1_GameState *game)
{
    int mm_x = NEXUS_FB_W - 68;
    int mm_y = 4;
    int mm_size = 64;
    int cell_size = 2;
    int dx, dy;

    /* Dark gray background (all unexplored) */
    for (dy = mm_y; dy < mm_y + mm_size; dy++)
        for (dx = mm_x; dx < mm_x + mm_size; dx++)
            if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                fb->color_buffer[dy * NEXUS_FB_W + dx] = 8; /* dark gray */

    /* Border */
    for (dx = mm_x; dx < mm_x + mm_size; dx++) {
        if (mm_y >= 0 && mm_y < NEXUS_FB_H && dx >= 0 && dx < NEXUS_FB_W)
            fb->color_buffer[mm_y * NEXUS_FB_W + dx] = 15;
        if (mm_y + mm_size - 1 >= 0 && mm_y + mm_size - 1 < NEXUS_FB_H && dx >= 0 && dx < NEXUS_FB_W)
            fb->color_buffer[(mm_y + mm_size - 1) * NEXUS_FB_W + dx] = 15;
    }
    for (dy = mm_y; dy < mm_y + mm_size; dy++) {
        if (dy >= 0 && dy < NEXUS_FB_H && mm_x >= 0 && mm_x < NEXUS_FB_W)
            fb->color_buffer[dy * NEXUS_FB_W + mm_x] = 15;
        if (dy >= 0 && dy < NEXUS_FB_H && mm_x + mm_size - 1 >= 0 && mm_x + mm_size - 1 < NEXUS_FB_W)
            fb->color_buffer[dy * NEXUS_FB_W + mm_x + mm_size - 1] = 15;
    }

    /* Party position (always visible as white blip) */
    if (game) {
        int px = mm_x + 2 + game->party_x * cell_size;
        int py = mm_y + 2 + game->party_y * cell_size;
        for (dy = py - 1; dy <= py + 1; dy++)
            for (dx = px - 1; dx <= px + 1; dx++)
                if (dx >= 0 && dx < NEXUS_FB_W && dy >= 0 && dy < NEXUS_FB_H)
                    fb->color_buffer[dy * NEXUS_FB_W + dx] = 15;
    }
}

/* ── Fallback: title screen text-on-black ──────────────────── */

void nexus_fallback_render_title(Nexus_Framebuffer *fb, int frame) {
    int fade = frame & 15;
    int i;

    if (!fb) return;

    /* Black background */
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; i++)
        fb->color_buffer[i] = 0;

    /* Simple title text: "DUNGEON MASTER NEXUS" in white rectangles */
    {
        int x, y;
        int title_x = NEXUS_FB_W / 2 - 80;
        int title_y = NEXUS_FB_H / 2 - 20;
        uint8_t fg = (uint8_t)(fade < 15 ? fade : 15);

        /* Title background rect */
        for (y = title_y - 8; y < title_y + 16; y++)
            for (x = title_x - 8; x < title_x + 168; x++)
                if (x >= 0 && x < NEXUS_FB_W && y >= 0 && y < NEXUS_FB_H)
                    fb->color_buffer[y * NEXUS_FB_W + x] = 0;

        /* Border */
        for (x = title_x - 8; x < title_x + 168; x++) {
            if (title_y - 8 >= 0 && title_y - 8 < NEXUS_FB_H && x >= 0 && x < NEXUS_FB_W)
                fb->color_buffer[(title_y - 8) * NEXUS_FB_W + x] = fg;
            if (title_y + 16 >= 0 && title_y + 16 < NEXUS_FB_H && x >= 0 && x < NEXUS_FB_W)
                fb->color_buffer[(title_y + 16) * NEXUS_FB_W + x] = fg;
        }
        for (y = title_y - 8; y < title_y + 16; y++) {
            if (y >= 0 && y < NEXUS_FB_H) {
                if (title_x - 8 >= 0 && title_x - 8 < NEXUS_FB_W)
                    fb->color_buffer[y * NEXUS_FB_W + title_x - 8] = fg;
                if (title_x + 167 >= 0 && title_x + 167 < NEXUS_FB_W)
                    fb->color_buffer[y * NEXUS_FB_W + title_x + 167] = fg;
            }
        }
    }
}

/* ── Fallback initializer ──────────────────────────────────── */

void nexus_fallback_init(void) {
    g_fallback_count = 0;

    /* Register wall fallbacks by wall type */
    nexus_fallback_register(NEXUS_FALLBACK_WALL, 0, 5, "solid stone");
    nexus_fallback_register(NEXUS_FALLBACK_WALL, 1, 6, "rough wall");
    nexus_fallback_register(NEXUS_FALLBACK_WALL, 2, 10, "brick wall");
    nexus_fallback_register(NEXUS_FALLBACK_WALL, 3, 7, "carved stone");

    /* Register creature fallbacks */
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 0, 10, "Scorpion");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 1, 10, "Mummy");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 2, 12, "Dragon");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 3, 7, "Skeleton");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 4, 4, "Ghost");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 5, 10, "Worm");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 6, 6, "Golem");
    nexus_fallback_register(NEXUS_FALLBACK_CREATURE, 7, 10, "Spider");

    printf("Nexus fallback: registered %d fallback entries\n", g_fallback_count);
}
