#include "nexus_v1_rendering.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── HUD rendering ───────────────────────────────────────────── */

/* Draw a filled rectangle on the framebuffer */
static void draw_rect(Nexus_Framebuffer *fb,
    int x, int y, int w, int h, uint8_t color_index)
{
    int dx, dy;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > NEXUS_FB_W) w = NEXUS_FB_W - x;
    if (y + h > NEXUS_FB_H) h = NEXUS_FB_H - y;
    if (w <= 0 || h <= 0) return;
    for (dy = y; dy < y + h; dy++)
        for (dx = x; dx < x + w; dx++)
            fb->color_buffer[dy * NEXUS_FB_W + dx] = color_index;
}

/* Draw a character glyph on the framebuffer (8×8 pixel font) */
static void draw_glyph(Nexus_Framebuffer *fb,
    int x, int y, const uint8_t *glyph,
    int glyph_w, int glyph_h,
    uint8_t fg, uint8_t bg)
{
    int gy, gx;
    if (x < 0 || y < 0 || x + glyph_w > NEXUS_FB_W || y + glyph_h > NEXUS_FB_H)
        return;
    for (gy = 0; gy < glyph_h; gy++) {
        for (gx = 0; gx < glyph_w; gx++) {
            int byte_idx = gy * ((glyph_w + 7) >> 3);
            int bit_idx = 7 - (gx & 7);
            int byte = glyph[byte_idx + (gx >> 3)];
            int bit = (byte >> bit_idx) & 1;
            int px = x + gx, py = y + gy;
            if (px >= 0 && px < NEXUS_FB_W && py >= 0 && py < NEXUS_FB_H) {
                fb->color_buffer[py * NEXUS_FB_W + px] = bit ? fg : bg;
            }
        }
    }
}

/* Draw a string using the Saturn font */
static void draw_string(Nexus_Framebuffer *fb,
    int x, int y,
    const char *str,
    uint8_t fg, uint8_t bg,
    const Nexus_V1_Font *font)
{
    int cx = x;
    const uint8_t *glyph;
    int g;

    if (!str) return;
    while (*str) {
        g = (unsigned char)(*str);
        if (font && font->bitmap_data) {
            glyph = nexus_v1_font_get_glyph(font, g);
            if (glyph) {
                int glyph_w = font->char_width ? font->char_width : 8;
                int glyph_h = font->char_height ? font->char_height : 8;
                draw_glyph(fb, cx, y, glyph, glyph_w, glyph_h, fg, bg);
                cx += glyph_w + 1;
            } else {
                cx += 8 + 1;
            }
        } else {
            /* No font: draw a simple ASCII rectangle placeholder */
            draw_rect(fb, cx, y, 5, 7, fg);
            cx += 6;
        }
        str++;
    }
}

/* ── Champion portrait ───────────────────────────────────────── */

/* Draw champion portrait area with procedural class icon fallback */
static void nexus_render_portrait_impl(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int champion_index,
    int selected,
    uint8_t portrait_color)
{
    int border = selected ? 15 : 7;
    /* Portrait background */
    draw_rect(fb, screen_x, screen_y, 48, 40, portrait_color);
    /* Border */
    draw_rect(fb, screen_x, screen_y, 48, 1, border);
    draw_rect(fb, screen_x, screen_y + 39, 48, 1, border);
    draw_rect(fb, screen_x, screen_y, 1, 40, border);
    draw_rect(fb, screen_x + 47, screen_y, 1, 40, border);
    /* Class icon inside */
    {
        int icon_x = screen_x + 8;
        int icon_y = screen_y + 8;
        /* Draw class icon as simple geometric shapes */
        draw_rect(fb, icon_x, icon_y + 10, 32, 12, 7); /* body rect */
        draw_rect(fb, icon_x + 12, icon_y, 8, 12, portrait_color); /* head */
    }
}

void nexus_render_portrait(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int portrait_index,
    int selected,
    const Nexus_V1_Engine *engine)
{
    uint8_t portrait_colors[NEXUS_FACE_COUNT] = {
        /* Champion class-based colors */
        12, 13, 10, 14, 4, 6,  /* Warrior=red, Wizard=blue, Valkyrie=olive, Samurai=gold, Ninja=purple, Priest=gray */
        12, 13, 10, 14, 4, 6,
        12, 13, 10, 14, 4, 6,
        12, 13, 10, 14, 4, 6,
    };
    uint8_t color;

    if (!fb) return;
    if (portrait_index < 0 || portrait_index >= NEXUS_FACE_COUNT)
        portrait_index = 0;
    color = portrait_colors[portrait_index];

    nexus_render_portrait_impl(fb, screen_x, screen_y,
        portrait_index, selected, color);
}

/* ── Health bar ─────────────────────────────────────────────── */

static void nexus_render_health_bar(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int health, int max_health,
    uint8_t bar_color)
{
    int bar_width = 40;
    int bar_height = 4;
    int fill_width = max_health > 0 ? (health * bar_width / max_health) : 0;

    draw_rect(fb, screen_x, screen_y, bar_width, bar_height, 0); /* black bg */
    if (fill_width > 0)
        draw_rect(fb, screen_x, screen_y, fill_width, bar_height, bar_color);
    draw_rect(fb, screen_x, screen_y, bar_width, 1, 15); /* border */
    draw_rect(fb, screen_x, screen_y + bar_height - 1, bar_width, 1, 15);
}

/* ── Minimap ───────────────────────────────────────────────── */

void nexus_render_minimap(Nexus_Framebuffer *fb,
    const Nexus_V1_Level *level,
    const Nexus_V1_GameState *game,
    const Nexus_V1_CreatureManager *creatures)
{
    int mm_x = NEXUS_FB_W - 68;  /* top-right corner */
    int mm_y = 4;
    int mm_size = 64;  /* 64×64 minimap */
    int cell_size = 2;  /* each square = 2×2 pixels */
    int gx, gy;

    if (!fb || !level || !game) return;

    /* Minimap background */
    draw_rect(fb, mm_x, mm_y, mm_size, mm_size, 0);
    draw_rect(fb, mm_x, mm_y, mm_size, 1, 15);
    draw_rect(fb, mm_x, mm_y + mm_size - 1, mm_size, 1, 15);
    draw_rect(fb, mm_x, mm_y, 1, mm_size, 15);
    draw_rect(fb, mm_x + mm_size - 1, mm_y, 1, mm_size, 15);

    /* Dungeon squares */
    for (gy = 0; gy < level->height && gy < 32; gy++) {
        for (gx = 0; gx < level->width && gx < 32; gx++) {
            int sq = nexus_v1_level_get_square(level, gx, gy);
            int px = mm_x + 2 + gx * cell_size;
            int py = mm_y + 2 + gy * cell_size;
            uint8_t color = 8; /* floor = gray */
            if (sq == 0) color = 5; /* wall = dark gray */
            else if (sq == 2) color = 10; /* door = brown */
            else if (sq == 3) color = 9; /* pit = very dark */
            draw_rect(fb, px, py, cell_size, cell_size, color);
        }
    }

    /* Party position */
    {
        int px = mm_x + 2 + game->party_x * cell_size;
        int py = mm_y + 2 + game->party_y * cell_size;
        draw_rect(fb, px - 1, py - 1, cell_size + 2, cell_size + 2, 15);
    }

    /* Creatures on minimap */
    if (creatures) {
        int ci;
        for (ci = 0; ci < creatures->active_count; ci++) {
            const Nexus_Creature *c = &creatures->active[ci];
            if (!c->alive) continue;
            if (c->x < 0 || c->x >= 32 || c->y < 0 || c->y >= 32) continue;
            {
                int px = mm_x + 2 + c->x * cell_size;
                int py = mm_y + 2 + c->y * cell_size;
                draw_rect(fb, px, py, cell_size, cell_size, 12); /* red blip */
            }
        }
    }
}

/* ── Compass ───────────────────────────────────────────────── */

static void nexus_render_compass(Nexus_Framebuffer *fb,
    int screen_x, int screen_y,
    int facing)
{
    /* Simple compass: N/E/S/W arrow */
    const char *dirs[4] = {"N", "E", "S", "W"};
    /* Draw compass text */
    draw_string(fb, screen_x + 8, screen_y, dirs[facing & 3], 15, 0, NULL);
}

/* ── HUD rendering ─────────────────────────────────────────── */

void nexus_render_hud(Nexus_Framebuffer *fb,
    const Nexus_V1_GameState *game,
    const Nexus_V1_CreatureManager *creatures,
    const Nexus_V1_Engine *engine)
{
    int i;
    int portrait_x = 4;
    int portrait_y = NEXUS_FB_H - 48;
    int spacing = 52;

    if (!fb || !game) return;

    /* Champion portraits (up to 4 champions)
     * Note: Nexus_V1_GameState doesn't yet store champion HP data.
     * Portraits render with class-based fallback colors.
     * Health bars are shown only when engine->creatures data is available. */
    for (i = 0; i < 4; i++) {
        int selected = (i == 0);  /* default: champion 0 is active */
        nexus_render_portrait(fb,
            portrait_x + i * spacing, portrait_y,
            i, selected, engine);
        /* Health bars: shown only if champion count > 0 (engine provides creature data) */
        if (i < game->champion_count && creatures && i < creatures->active_count) {
            const Nexus_Creature *c = &creatures->active[i];
            if (c->alive && c->health > 0) {
                int max_h = creatures->types[c->type_index].health;
                nexus_render_health_bar(fb,
                    portrait_x + i * spacing,
                    portrait_y - 8,
                    c->health,
                    max_h > 0 ? max_h : 1,
                    12); /* red health bar */
            }
        }
    }

    /* Minimap */
    if (engine && engine->level_loaded)
        nexus_render_minimap(fb, &engine->current_level, game, creatures);

    /* Compass */
    nexus_render_compass(fb, NEXUS_FB_W / 2 - 8, 4, game->party_dir);

    /* Floor indicator */
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "FL:%d", game->current_level);
        draw_string(fb, NEXUS_FB_W / 2 - 12, NEXUS_FB_H - 16, buf, 15, 0,
            engine && engine->font_loaded ? &engine->font : NULL);
    }
}

/* ── Title screen ──────────────────────────────────────────── */

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine) {
    int size;
    uint8_t *data;
    if (!title || !engine) return -1;
    memset(title, 0, sizeof(*title));

    data = nexus_v1_read_file(engine, "TITLE.BIN", &size);
    if (data && size >= 320 * 200) {
        /* TITLE.BIN is a raw 320×200 4-bit indexed surface.
         * Until the exact format is confirmed, load as raw pixel dump. */
        title->pixels = (uint8_t *)malloc(320 * 200);
        if (title->pixels) {
            memcpy(title->pixels, data, 320 * 200);
            title->width = 320;
            title->height = 200;
            title->loaded = 1;
        }
        free(data);
        printf("Nexus title: loaded TITLE.BIN (%dx%d)\n",
            title->width, title->height);
        return 0;
    }
    free(data);
    title->loaded = 0;
    return -1;
}

void nexus_title_free(Nexus_TitleScreen *title) {
    if (!title) return;
    free(title->pixels);
    memset(title, 0, sizeof(*title));
}

/* Copy title surface to framebuffer with fade (frame 0-15) */
void nexus_render_title(const Nexus_TitleScreen *title,
    Nexus_Framebuffer *fb, int frame)
{
    int fade = frame & 15;  /* 0..15 fade level */
    int i;

    if (!fb) return;

    if (!title || !title->loaded || !title->pixels) {
        nexus_render_title_fallback(fb, frame);
        return;
    }

    /* Copy pixels with fade */
    {
        int src_i = 0;
        for (i = 0; i < 320 * 200 && i < NEXUS_FB_W * NEXUS_FB_H; i++) {
            uint8_t pixel = title->pixels[src_i++];
            /* Fade: multiply palette index by fade level */
            if (fade < 15) {
                int light = fade + 1;  /* 1..16 */
                pixel = (uint8_t)((pixel * light) >> 4);
            }
            fb->color_buffer[i] = pixel;
        }
    }
}

void nexus_render_title_fallback(Nexus_Framebuffer *fb, int frame) {
    int i;
    int fade = frame & 15;

    if (!fb) return;

    /* Black background */
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; i++)
        fb->color_buffer[i] = 0;

    /* Title text: "DUNGEON MASTER NEXUS" */
    /* Since we may not have font loaded, draw a simple placeholder */
    {
        int x, y;
        int title_x = NEXUS_FB_W / 2 - 80;
        int title_y = NEXUS_FB_H / 2 - 20;

        /* Draw black rectangle behind title text */
        for (y = title_y - 8; y < title_y + 16; y++)
            for (x = title_x - 8; x < title_x + 168; x++)
                if (x >= 0 && x < NEXUS_FB_W && y >= 0 && y < NEXUS_FB_H)
                    fb->color_buffer[y * NEXUS_FB_W + x] = 0;

        /* Draw simple white border around title area */
        for (x = title_x - 8; x < title_x + 168; x++) {
            if (title_y - 8 >= 0 && title_y - 8 < NEXUS_FB_H && x >= 0 && x < NEXUS_FB_W)
                fb->color_buffer[(title_y - 8) * NEXUS_FB_W + x] = (uint8_t)(fade < 15 ? fade : 15);
            if (title_y + 16 >= 0 && title_y + 16 < NEXUS_FB_H && x >= 0 && x < NEXUS_FB_W)
                fb->color_buffer[(title_y + 16) * NEXUS_FB_W + x] = (uint8_t)(fade < 15 ? fade : 15);
        }
        for (y = title_y - 8; y < title_y + 16; y++) {
            if (y >= 0 && y < NEXUS_FB_H) {
                if (title_x - 8 >= 0 && title_x - 8 < NEXUS_FB_W)
                    fb->color_buffer[y * NEXUS_FB_W + title_x - 8] = (uint8_t)(fade < 15 ? fade : 15);
                if (title_x + 167 >= 0 && title_x + 167 < NEXUS_FB_W)
                    fb->color_buffer[y * NEXUS_FB_W + title_x + 167] = (uint8_t)(fade < 15 ? fade : 15);
            }
        }
    }
}
