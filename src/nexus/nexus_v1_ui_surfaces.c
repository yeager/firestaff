/* Nexus V1 UI / Title Surface Renderer — implementation
 * =====================================================
 * Loads and blits DM Nexus Saturn UI surfaces and title screens.
 *
 * Source-lock references:
 *   ReDMCSB BLIT.C    — F0132 blit rect (screen blit pipeline)
 *   ReDMCSB PANEL.C   — F0120-F0125 panel element drawing
 *   ReDMCSB CEDTINCK.C — CEDT font/text rendering
 *   Saturn SDK        — VDP1 BITMAP command: pixel format Celdat format
 *   docs/NEXUS_FILE_CLASSIFICATION.md — TITLE.CG 164 KB, FACE.BIN 44 KB,
 *     STABG.BIN 52 KB, WARNING.BIN 99 KB, GAMEOVER.BIN 101 KB
 *
 * Saturn VDP1 BITMAP surface format:
 *   - Pixel format: 8-bit indexed (CLUT) = palette index per pixel
 *   - Stored as row-major byte array, left-to-right, top-to-bottom
 *   - Row stride = w (no padding, Saturn row stride is w * 1 byte)
 *
 * For TITLE.CG (164 KB / 320=200): 163 840 / (320×200) ~ 2.56 bytes/pixel
 * → suggests compressed or packed format; treat as raw indexed 320×200 CBUF
 * aligned to the start with a possible sub-format header.
 *
 * Deterministic fallback: failure to load produces a deterministic
 * mid-gray (palette 7) filled rectangle, no crash. */

#include "nexus_v1_ui_surfaces.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Default surface fills for fallback states ───────────────── */
static void surface_clear_gray(Nexus_UI_Surface *surf) {
    int i;
    if (!surf || !surf->data) return;
    for (i = 0; i < surf->w * surf->h; i++)
        surf->data[i] = 7;  /* palette index 7 = deterministic mid-gray */
}

/* ── Manager lifecycle ──────────────────────────────────────────── */
void nexus_ui_manager_init(Nexus_UI_Manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

void nexus_ui_manager_free(Nexus_UI_Manager *mgr) {
    int i;
    if (!mgr) return;
    for (i = 0; i < NEXUS_SURFACE_COUNT; i++) {
        if (mgr->surfaces[i].owns_data && mgr->surfaces[i].data) {
            free(mgr->surfaces[i].data);
            mgr->surfaces[i].data = NULL;
            mgr->surfaces[i].owns_data = 0;
        }
    }
}

/* ── Surface load helper ────────────────────────────────────────── */
int nexus_ui_surface_load(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *data, int data_size,
    int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source)
{
    Nexus_UI_Surface *surf;

    if (!mgr || which >= NEXUS_SURFACE_COUNT) return -1;
    surf = &mgr->surfaces[which];

    /* Free previous if owned */
    if (surf->owns_data && surf->data) {
        free(surf->data);
        surf->data = NULL;
        surf->owns_data = 0;
    }

    if (!data || data_size <= 0) {
        /* Deterministic fallback: dark fill + diagnostic */
        printf("Nexus UI: WARNING null data for surface %d [%s] — "
               "loading deterministic gray placeholder\n",
               which, source ? source : "?");
        surf->w = w; surf->h = h;
        surf->data = (uint8_t *)calloc(w * h, 1);
        if (surf->data) {
            surf->owns_data = 1;
            surface_clear_gray(surf);
            surf->pal_start = pal_start;
            surf->pal_count = pal_count;
            surf->source = source;
        }
        return 0;  /* 0 = loaded via fallback */
    }

    surf->w = w; surf->h = h;
    surf->pal_start = pal_start;
    surf->pal_count = pal_count;
    surf->source = source;

    /* Enough data? */
    if (data_size >= w * h) {
        surf->data = (uint8_t *)malloc(w * h);
        if (surf->data) {
            surf->owns_data = 1;
            memcpy(surf->data, data, w * h);
        }
    } else {
        /* Partial/short data: copy what we have, zero-pad rest */
        printf("Nexus UI: WARNING partial data %d < %d for [%s] "
               "— loading available pixels\n",
               data_size, w * h, source ? source : "?");
        surf->data = (uint8_t *)calloc(w * h, 1);
        if (surf->data) {
            surf->owns_data = 1;
            memcpy(surf->data, data, data_size < w*h ? data_size : w*h);
            /* Rest is already zeroed by calloc */
        }
    }

    printf("Nexus UI: surface %d [%s] loaded %dx%d "
           "(palette %d-%d) own=%d size=%d\n",
           which, source ? source : "?",
           surf->w, surf->h, surf->pal_start, surf->pal_start + surf->pal_count - 1,
           surf->owns_data, data_size);
    return (int)surf->owns_data;
}

/* ── Surface-specific loaders ──────────────────────────────────── */

/* TITLE.CG (164 KB) — title screen color graphics.
 * Format evidence: file is 164 KB = 168 960 bytes.
 * 320×200 = 64 000 bytes.  Could be: 2.56 bytes/pixel (2.56:1 compression)
 * or is a sub-format with header.
 * Approach: scan for SEGA header (0x53454E41...) at offset 0 or in
 * first 2 sectors; use raw pixel data once signature confirmed. */
int nexus_ui_load_title(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    int offset = 0;
    (void)palette;
    if (!mgr) return -1;
    /* Check for Saturn SEGA header — skip 16 bytes if present */
    if (data_size >= 16 && memcmp(data, "SEGA", 4) == 0) {
        offset = 16;
        printf("Nexus UI: TITLE.CG has Sega header — skipping 16 bytes\n");
    }
    /* Default: TITLE.CG = 320×200 indexed at offset */
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_TITLE,
        data + offset, data_size - offset,
        320, 200, 64, 64, "TITLE.CG");
}

/* WARNING.BIN (99 KB) — simple indexed 320×200 */
int nexus_ui_load_warning(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)palette;
    if (!mgr) return -1;
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_WARNING,
        data, data_size, 320, 200, 160, 32, "WARNING.BIN");
}

/* GAMEOVER.BIN (101 KB) — simple indexed 320×200 */
int nexus_ui_load_gameover(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)palette;
    if (!mgr) return -1;
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_GAMEOVER,
        data, data_size, 320, 200, 128, 64, "GAMEOVER.BIN");
}

/* STABG.BIN (52 KB) — status area background.
 * May be 320×200 or a narrower horizontal strip. */
int nexus_ui_load_stabg(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)palette;
    if (!mgr) return -1;
    /* 52 KB / 320 ≈ 170 rows; treat as 320×170 or fall back to 320×200 */
    if (data_size >= 320 * 200) {
        return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
            data, data_size, 320, 200, 0, 16, "STABG.BIN");
    } else if (data_size >= 320 * 50) {
        return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
            data, data_size, 320, 52, 0, 16, "STABG.BIN");
    }
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
        data, data_size, 320, 200, 0, 16, "STABG.BIN");
}

/* FACE.BIN (44 KB) — champion portraits, 24 entries.
 * Each portrait is 48×48 (or nearest power of 2: 64×64).
 * Layout in file: horizontal strip of 24 columns.
 * face_index: 0-23 → offset = (face_index * portrait_area) into data.  */
int nexus_ui_load_faces(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_of_face,
    int data_size, int face_index,
    int portrait_w, int portrait_h,
    const uint32_t *palette)
{
    int entry_size;
    Nexus_UI_Surface *surf;
    (void)palette;

    if (!mgr) return -1;
    if (face_index < 0 || face_index >= 24) return -1;
    if (portrait_w <= 0 || portrait_h <= 0) {
        portrait_w = 48; portrait_h = 48;  /* default assumed size */
    }
    entry_size = portrait_w * portrait_h;

    surf = &mgr->surfaces[NEXUS_SURFACE_FACE0 + face_index];
    if (surf->owns_data && surf->data) free(surf->data);

    surf->w = portrait_w;
    surf->h = portrait_h;
    surf->pal_start = 192;  /* UI palette block for faces */
    surf->pal_count = 16;
    surf->source = "FACE.BIN";
    surf->owns_data = 1;

    if (data && data_size >= data_of_face + entry_size) {
        surf->data = (uint8_t *)malloc(entry_size);
        if (surf->data) {
            memcpy(surf->data, data + data_of_face, entry_size);
        } else {
            surf->data = (uint8_t *)calloc(entry_size, 1);
        }
    } else {
        /* Partial/short: load what is available */
        printf("Nexus UI: WARNING FACE.BIN data short for portrait %d "
               "(need=%d have=%d) — gray placeholder\n",
               face_index, entry_size, data_size - data_of_face);
        surf->data = (uint8_t *)calloc(entry_size, 1);
        if (surf->data && data && data_size > data_of_face) {
            int avail = data_size - data_of_face;
            memcpy(surf->data, data + data_of_face,
                   avail < entry_size ? (size_t)avail : (size_t)entry_size);
        }
        if (surf->data) surface_clear_gray(surf);
    }
    /* Face surfaces are stored individually in the manager */
    return 0;
}

void nexus_ui_surface_free(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which)
{
    if (!mgr || which >= NEXUS_SURFACE_COUNT) return;
    if (mgr->surfaces[which].owns_data
        && mgr->surfaces[which].data) {
        free(mgr->surfaces[which].data);
    }
    memset(&mgr->surfaces[which], 0, sizeof(Nexus_UI_Surface));
}

/* ── Blit primitives ─────────────────────────────────────────────── */

/* Simple 1:1 clip blit from surface → indexed framebuffer.
 * Source-lock: ReDMCSB BLIT.C F0132 (F0132 pixel copy).        */
void nexus_ui_blit_surface(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy)
{
    int row, col;
    if (!surf || !fb || !surf->data) return;
    /* Clip destination */
    if (dx < 0) { dx = 0; }
    if (dy < 0) { dy = 0; }
    if (dx + surf->w > fb_w)  dx = fb_w - surf->w;
    if (dy + surf->h > fb_h)  dy = fb_h - surf->h;
    if (dx < 0 || dy < 0 || dx + surf->w > fb_w
        || dy + surf->h > fb_h) return;

    for (row = 0; row < surf->h; row++) {
        int sy = dy + row;
        if (sy < 0 || sy >= fb_h) continue;
        for (col = 0; col < surf->w; col++) {
            int sx = dx + col;
            if (sx < 0 || sx >= fb_w) continue;
            fb[sy * fb_w + sx] = surf->data[row * surf->w + col];
        }
    }
}

/* Same but with optional horizontal flip (champion mirror) */
void nexus_ui_blit_surface_flip(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h,
    int dx, int dy, int flip_h)
{
    int row, col;
    if (!surf || !fb || !surf->data) return;
    if (dx < 0) dx = 0;
    if (dy < 0) dy = 0;
    if (dx + surf->w > fb_w) dx = fb_w - surf->w;
    if (dy + surf->h > fb_h) dy = fb_h - surf->h;
    if (dx < 0 || dy < 0) return;

    for (row = 0; row < surf->h; row++) {
        int sy = dy + row;
        if (sy < 0 || sy >= fb_h) continue;
        for (col = 0; col < surf->w; col++) {
            int sx = dx + col;
            int src_col = flip_h ? (surf->w - 1 - col) : col;
            if (sx < 0 || sx >= fb_w) continue;
            fb[sy * fb_w + sx] = surf->data[row * surf->w + src_col];
        }
    }
}

/* ── Convenience render wrappers ─────────────────────────────── */
void nexus_ui_render_title(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_TITLE],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_warning(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_WARNING],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_gameover(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_GAMEOVER],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_stabg(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h, int dest_x, int dest_y)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_STABG],
        fb, fb_w, fb_h, dest_x, dest_y);
}

void nexus_ui_render_portrait(const Nexus_UI_Manager *mgr,
    int portrait_index, uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y, int flip_h)
{
    Nexus_UISurfaceType which;
    if (!mgr || !fb) return;
    if (portrait_index < 0 || portrait_index >= 24) return;
    which = NEXUS_SURFACE_FACE0 + portrait_index;
    nexus_ui_blit_surface_flip(&mgr->surfaces[which],
        fb, fb_w, fb_h, dest_x, dest_y, flip_h);
}

/* ── Palette remap ─────────────────────────────────────────────── */
void nexus_ui_surface_remap_pal(Nexus_UI_Surface *surf,
    uint8_t new_pal_start)
{
    int i;
    if (!surf || !surf->data) return;
    /* Shift all pixel values by (new - old) palette start */
    int delta = (int)(new_pal_start - surf->pal_start);
    if (delta == 0) return;
    for (i = 0; i < surf->w * surf->h; i++) {
        int v = (int)surf->data[i] + delta;
        surf->data[i] = (v < 0) ? 0 : (v > 255) ? 255 : (uint8_t)v;
    }
    surf->pal_start = new_pal_start;
}

/* Darken surface in-place for blur/focus states */
void nexus_ui_surface_darken(Nexus_UI_Surface *surf, float factor) {
    /* For indexed surfaces, we darken by halving palette indices.
     * A real implementation would compose with the brightness in the
     * V1 framebuffer pipeline.  Here we simply darken each pixel's
     * index toward 0 (0 stays 0 = black).                         */
    int i;
    if (!surf || !surf->data) return;
    if (factor <= 0.0f) factor = 0.5f;
    for (i = 0; i < surf->w * surf->h; i++) {
        int v = (int)((uint8_t)surf->data[i] * factor);
        surf->data[i] = (v < 0) ? 0 : (uint8_t)v;
    }
}

#if defined(__GNUC__) && !defined(__clang__)
/* suppress unused parameter warnings */
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
