/*
 * animated_bg_m12.c
 *
 * Background animation system for the M12 launcher startup menu.
 * See animated_bg_m12.h for the public API and design overview.
 */

#include "animated_bg_m12.h"

#include <string.h>
#include <math.h>

/* ── Internal helpers ───────────────────────────────────────────── */

static uint32_t bg_xorshift32(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static float bg_randf(uint32_t* state) {
    return (float)(bg_xorshift32(state) & 0xFFFFFF) / (float)0xFFFFFF;
}

static void bg_set_pixel(unsigned char* rgba, int width, int x, int y,
                         unsigned char r, unsigned char g, unsigned char b,
                         unsigned char a) {
    if (x < 0 || x >= width) return;
    int off = (y * width + x) * 4;
    rgba[off + 0] = r;
    rgba[off + 1] = g;
    rgba[off + 2] = b;
    rgba[off + 3] = a;
}

static unsigned char bg_clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

/* Simple additive blend onto existing pixel */
static void bg_blend_pixel(unsigned char* rgba, int width, int height,
                           int x, int y,
                           unsigned char r, unsigned char g, unsigned char b,
                           unsigned char a) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int off = (y * width + x) * 4;
    int ar = a;
    int oldR = rgba[off + 0];
    int oldG = rgba[off + 1];
    int oldB = rgba[off + 2];
    rgba[off + 0] = bg_clamp_u8(oldR + (r * ar) / 255);
    rgba[off + 1] = bg_clamp_u8(oldG + (g * ar) / 255);
    rgba[off + 2] = bg_clamp_u8(oldB + (b * ar) / 255);
    rgba[off + 3] = 255;
}

/* ── Config defaults ────────────────────────────────────────────── */

void M12_Bg_ConfigDefaults(M12_BgConfig* cfg) {
    if (!cfg) return;
    cfg->preset  = M12_BG_PRESET_STATIC;
    cfg->speed   = 3;
    cfg->density = 3;
}

/* ── Starfield initialisation ───────────────────────────────────── */

static void bg_init_stars(M12_BgState* state) {
    int count = 80 + state->config.density * 80;  /* 160..480 */
    if (count > M12_BG_MAX_STARS) count = M12_BG_MAX_STARS;
    state->starCount = count;
    for (int i = 0; i < count; i++) {
        state->stars[i].x = bg_randf(&state->rngState) * 2.0f - 1.0f;
        state->stars[i].y = bg_randf(&state->rngState) * 2.0f - 1.0f;
        state->stars[i].z = bg_randf(&state->rngState) * 0.9f + 0.1f;
    }
}

/* ── Particle initialisation ────────────────────────────────────── */

static void bg_init_particles(M12_BgState* state) {
    int count = 20 + state->config.density * 40;  /* 60..220 */
    if (count > M12_BG_MAX_PARTICLES) count = M12_BG_MAX_PARTICLES;
    state->particleCount = count;
    for (int i = 0; i < count; i++) {
        state->particles[i].x    = bg_randf(&state->rngState);
        state->particles[i].y    = bg_randf(&state->rngState);
        state->particles[i].vx   = (bg_randf(&state->rngState) - 0.5f) * 0.002f;
        state->particles[i].vy   = -(bg_randf(&state->rngState) * 0.003f + 0.001f);
        state->particles[i].life = bg_randf(&state->rngState);
        /* Orange-amber ember colour */
        state->particles[i].color = 0xFF6633FFu;
    }
}

/* ── Init ───────────────────────────────────────────────────────── */

void M12_Bg_Init(M12_BgState* state, const M12_BgConfig* cfg) {
    if (!state) return;
    memset(state, 0, sizeof(*state));

    if (cfg) {
        state->config = *cfg;
    } else {
        M12_Bg_ConfigDefaults(&state->config);
    }

    /* Clamp config values */
    if (state->config.speed < 1)   state->config.speed = 1;
    if (state->config.speed > 5)   state->config.speed = 5;
    if (state->config.density < 1) state->config.density = 1;
    if (state->config.density > 5) state->config.density = 5;

    /* Seed RNG from a fixed but non-zero value so animations are
     * deterministic across runs for a given preset. */
    state->rngState = 0xDEADBEEFu ^ (uint32_t)state->config.preset;

    state->scrollY         = 0.0f;
    state->flickerPhase    = 0.0f;
    state->flickerIntensity = 0.8f;

    if (state->config.preset == M12_BG_PRESET_STARFIELD) {
        bg_init_stars(state);
    }
    if (state->config.preset == M12_BG_PRESET_TORCH_AMBIENCE ||
        state->config.preset == M12_BG_PRESET_DUNGEON_CRAWL) {
        bg_init_particles(state);
    }

    state->initialised = 1;
}

/* ── Tick: advance animation ────────────────────────────────────── */

static void bg_tick_dungeon(M12_BgState* state) {
    float spd = (float)state->config.speed * 0.3f;
    state->scrollY += spd;
    if (state->scrollY >= (float)M12_BG_TILE_SIZE) {
        state->scrollY -= (float)M12_BG_TILE_SIZE;
    }

    /* Drift ember particles upward */
    for (int i = 0; i < state->particleCount; i++) {
        state->particles[i].x += state->particles[i].vx;
        state->particles[i].y += state->particles[i].vy * (float)state->config.speed;
        state->particles[i].life -= 0.003f * (float)state->config.speed;
        if (state->particles[i].life <= 0.0f) {
            /* Respawn */
            state->particles[i].x    = bg_randf(&state->rngState);
            state->particles[i].y    = 1.0f;
            state->particles[i].life = 0.5f + bg_randf(&state->rngState) * 0.5f;
            state->particles[i].vx   = (bg_randf(&state->rngState) - 0.5f) * 0.002f;
            state->particles[i].vy   = -(bg_randf(&state->rngState) * 0.003f + 0.001f);
        }
    }
}

static void bg_tick_torch(M12_BgState* state) {
    float spd = (float)state->config.speed * 0.05f;
    state->flickerPhase += spd;
    /* Pseudo-random flicker using sin with harmonics */
    state->flickerIntensity = 0.6f
        + 0.15f * sinf(state->flickerPhase * 3.7f)
        + 0.10f * sinf(state->flickerPhase * 7.3f)
        + 0.05f * sinf(state->flickerPhase * 13.1f);

    /* Drift ember particles */
    for (int i = 0; i < state->particleCount; i++) {
        state->particles[i].x += state->particles[i].vx;
        state->particles[i].y += state->particles[i].vy * (float)state->config.speed;
        state->particles[i].life -= 0.004f * (float)state->config.speed;
        if (state->particles[i].life <= 0.0f) {
            state->particles[i].x    = 0.4f + bg_randf(&state->rngState) * 0.2f;
            state->particles[i].y    = 1.0f;
            state->particles[i].life = 0.3f + bg_randf(&state->rngState) * 0.7f;
            state->particles[i].vx   = (bg_randf(&state->rngState) - 0.5f) * 0.003f;
            state->particles[i].vy   = -(bg_randf(&state->rngState) * 0.004f + 0.001f);
        }
    }
}

static void bg_tick_starfield(M12_BgState* state) {
    float spd = (float)state->config.speed * 0.003f;
    for (int i = 0; i < state->starCount; i++) {
        state->stars[i].z -= spd;
        if (state->stars[i].z <= 0.01f) {
            state->stars[i].x = bg_randf(&state->rngState) * 2.0f - 1.0f;
            state->stars[i].y = bg_randf(&state->rngState) * 2.0f - 1.0f;
            state->stars[i].z = 0.9f + bg_randf(&state->rngState) * 0.1f;
        }
    }
}

void M12_Bg_Tick(M12_BgState* state) {
    if (!state || !state->initialised) return;
    state->tickCount++;

    switch (state->config.preset) {
        case M12_BG_PRESET_DUNGEON_CRAWL:  bg_tick_dungeon(state);   break;
        case M12_BG_PRESET_TORCH_AMBIENCE: bg_tick_torch(state);     break;
        case M12_BG_PRESET_STARFIELD:      bg_tick_starfield(state); break;
        default: break;
    }
}

/* ── Render: Static ─────────────────────────────────────────────── */

static void bg_render_static(const M12_BgState* state,
                             unsigned char* rgba,
                             int width, int height) {
    /* Deep dungeon grey-blue */
    (void)state;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int off = (y * width + x) * 4;
            rgba[off + 0] = 18;
            rgba[off + 1] = 18;
            rgba[off + 2] = 24;
            rgba[off + 3] = 255;
        }
    }
}

/* ── Render: Dungeon Crawl ──────────────────────────────────────── */

/* Procedural stone-tile pattern (no texture assets needed). */
static unsigned char bg_stone_value(int tx, int ty) {
    /* Hash-based pseudo-random per-tile brightness variation */
    unsigned int h = (unsigned int)(tx * 374761393u + ty * 668265263u);
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    int base = 28 + (int)(h % 20);  /* 28..47 range: dark stone */
    /* Mortar lines at tile edges */
    return (unsigned char)base;
}

static void bg_render_dungeon(const M12_BgState* state,
                              unsigned char* rgba,
                              int width, int height) {
    int tileSize = M12_BG_TILE_SIZE;
    int scrollPixels = (int)state->scrollY;

    for (int py = 0; py < height; py++) {
        int sy = py + scrollPixels;
        int ty = sy / tileSize;
        int fy = sy % tileSize;  /* fractional position within tile */
        if (fy < 0) { fy += tileSize; ty--; }

        for (int px = 0; px < width; px++) {
            int tx = px / tileSize;
            int fx = px % tileSize;

            unsigned char base = bg_stone_value(tx, ty);

            /* Mortar lines: lighter 2-pixel groove at tile edges */
            int isMortar = (fx < 2 || fy < 2) ? 1 : 0;
            /* Alternating brick offset for every other row */
            if ((ty & 1) && fx < 2) {
                /* Shift mortar for odd rows */
                int shifted_px = (px + tileSize / 2) % tileSize;
                isMortar = (shifted_px < 2 || fy < 2) ? 1 : 0;
            }

            unsigned char r, g, b;
            if (isMortar) {
                r = base + 15;
                g = base + 12;
                b = base + 10;
            } else {
                /* Subtle per-pixel noise via simple hash */
                unsigned int ph = (unsigned int)(px * 2654435761u + py * 2246822519u);
                ph ^= ph >> 16;
                int noise = (int)(ph % 7) - 3;  /* -3..+3 */
                r = bg_clamp_u8(base + noise);
                g = bg_clamp_u8(base + noise - 2);
                b = bg_clamp_u8(base + noise - 4);
            }

            int off = (py * width + px) * 4;
            rgba[off + 0] = r;
            rgba[off + 1] = g;
            rgba[off + 2] = b;
            rgba[off + 3] = 255;
        }
    }

    /* Overlay ember particles */
    for (int i = 0; i < state->particleCount; i++) {
        if (state->particles[i].life <= 0.0f) continue;
        int px = (int)(state->particles[i].x * (float)width);
        int py = (int)(state->particles[i].y * (float)height);
        unsigned char alpha = bg_clamp_u8((int)(state->particles[i].life * 200.0f));
        /* 3x3 soft ember glow */
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                unsigned char a2 = (dx == 0 && dy == 0) ? alpha
                    : bg_clamp_u8(alpha / 3);
                bg_blend_pixel(rgba, width, height,
                               px + dx, py + dy,
                               255, 140, 40, a2);
            }
        }
    }
}

/* ── Render: Torch Ambience ─────────────────────────────────────── */

static void bg_render_torch(const M12_BgState* state,
                            unsigned char* rgba,
                            int width, int height) {
    float intensity = state->flickerIntensity;
    float cx = (float)width * 0.5f;
    float cy = (float)height * 0.45f;
    float maxDist = sqrtf(cx * cx + cy * cy);

    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            float dx = (float)px - cx;
            float dy = (float)py - cy;
            float dist = sqrtf(dx * dx + dy * dy) / maxDist;

            /* Vignette falloff from centre */
            float light = (1.0f - dist * dist) * intensity;
            if (light < 0.0f) light = 0.0f;

            /* Warm torch colour: orange-amber base fading to dark */
            unsigned char r = bg_clamp_u8((int)(light * 80.0f + 10.0f));
            unsigned char g = bg_clamp_u8((int)(light * 40.0f + 8.0f));
            unsigned char b = bg_clamp_u8((int)(light * 15.0f + 6.0f));

            int off = (py * width + px) * 4;
            rgba[off + 0] = r;
            rgba[off + 1] = g;
            rgba[off + 2] = b;
            rgba[off + 3] = 255;
        }
    }

    /* Floating ember particles */
    for (int i = 0; i < state->particleCount; i++) {
        if (state->particles[i].life <= 0.0f) continue;
        int px = (int)(state->particles[i].x * (float)width);
        int py = (int)(state->particles[i].y * (float)height);
        unsigned char alpha = bg_clamp_u8((int)(state->particles[i].life * 255.0f));
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                unsigned char a2 = (dx == 0 && dy == 0) ? alpha
                    : bg_clamp_u8(alpha / 2);
                bg_blend_pixel(rgba, width, height,
                               px + dx, py + dy,
                               255, 180, 60, a2);
            }
        }
    }
}

/* ── Render: Starfield ──────────────────────────────────────────── */

static void bg_render_starfield(const M12_BgState* state,
                                unsigned char* rgba,
                                int width, int height) {
    /* Black background */
    memset(rgba, 0, (size_t)(width * height * 4));
    /* Set alpha to 255 */
    for (int i = 0; i < width * height; i++) {
        rgba[i * 4 + 3] = 255;
    }

    float hw = (float)width * 0.5f;
    float hh = (float)height * 0.5f;

    for (int i = 0; i < state->starCount; i++) {
        float z = state->stars[i].z;
        if (z <= 0.01f) continue;

        float sx = (state->stars[i].x / z) * hw + hw;
        float sy = (state->stars[i].y / z) * hh + hh;
        int px = (int)sx;
        int py = (int)sy;

        if (px < 0 || px >= width || py < 0 || py >= height) continue;

        /* Brightness based on closeness (smaller z = brighter) */
        float brightness = (1.0f - z) * 1.5f;
        if (brightness > 1.0f) brightness = 1.0f;
        unsigned char lum = bg_clamp_u8((int)(brightness * 255.0f));

        /* Slight blue-white tint for distant stars */
        unsigned char r = lum;
        unsigned char g = lum;
        unsigned char b = bg_clamp_u8((int)(brightness * 255.0f + 20.0f));

        bg_set_pixel(rgba, width, px, py, r, g, b, 255);

        /* Brighter stars get a larger glow */
        if (brightness > 0.5f) {
            unsigned char glow = bg_clamp_u8((int)((brightness - 0.5f) * 120.0f));
            bg_blend_pixel(rgba, width, height, px - 1, py, r, g, b, glow);
            bg_blend_pixel(rgba, width, height, px + 1, py, r, g, b, glow);
            bg_blend_pixel(rgba, width, height, px, py - 1, r, g, b, glow);
            bg_blend_pixel(rgba, width, height, px, py + 1, r, g, b, glow);
        }
    }
}

/* ── Main render dispatch ───────────────────────────────────────── */

void M12_Bg_Render(const M12_BgState* state,
                   unsigned char* rgba,
                   int width, int height) {
    if (!state || !rgba || width < 1 || height < 1) return;
    if (!state->initialised) {
        /* Fallback: solid black */
        memset(rgba, 0, (size_t)(width * height * 4));
        return;
    }

    switch (state->config.preset) {
        case M12_BG_PRESET_DUNGEON_CRAWL:
            bg_render_dungeon(state, rgba, width, height);
            break;
        case M12_BG_PRESET_TORCH_AMBIENCE:
            bg_render_torch(state, rgba, width, height);
            break;
        case M12_BG_PRESET_STARFIELD:
            bg_render_starfield(state, rgba, width, height);
            break;
        case M12_BG_PRESET_STATIC:
        default:
            bg_render_static(state, rgba, width, height);
            break;
    }
}

/* ── Preset labels ──────────────────────────────────────────────── */

const char* M12_Bg_PresetLabel(M12_BgPreset preset) {
    switch (preset) {
        case M12_BG_PRESET_STATIC:        return "Static";
        case M12_BG_PRESET_DUNGEON_CRAWL: return "Dungeon Crawl";
        case M12_BG_PRESET_TORCH_AMBIENCE:return "Torch Ambience";
        case M12_BG_PRESET_STARFIELD:     return "Starfield";
        default:                          return "Unknown";
    }
}
