/*
 * csb_v22_inplace_draw_pc34.c
 *
 * CSB V2.2 has no authenticated replacement-art corpus yet.  In particular,
 * an FSV22C cache is generated from host PNG files, not decoded from an
 * original CSB release.  It must therefore never be opened by the production
 * renderer: accepting it would replace verified F0128 pixels with synthetic
 * material.
 *
 * This is an intentionally narrow no-draw boundary.  The public API remains
 * so M11 and the source-owned F0128 handoff can keep their stable contracts;
 * every replacement query returns no material until an original-data decoder,
 * palette receipt and pixel-parity evidence are added together.
 *
 * Source-lock: ReDMCSB DUNVIEW.C F0111/F0115/F0128, CSBWin Viewport.cpp,
 * and TODO.md CSB-ORIGINAL-REPLACE-001.
 */

#include "csb_v22_inplace_draw_pc34.h"

#include <string.h>

static uint8_t g_v22_palette_rgb6[256][3];
static int g_v22_palette_active;

int csb_v22_inplace_draw_init(void)
{
    /* Do not inspect v22_inplace_cache.bin.  It is a generated host-art
     * format and has no original-CSB provenance. */
    return 0;
}

void csb_v22_inplace_draw_shutdown(void)
{
    csb_v22_inplace_draw_clear_indexed_palette();
}

int csb_v22_inplace_draw_active(void)
{
    return 0;
}

int csb_v22_inplace_draw_set_indexed_palette_rgb6(
    const uint8_t rgb6[256][3])
{
    if (!rgb6) {
        return 0;
    }
    memcpy(g_v22_palette_rgb6, rgb6, sizeof(g_v22_palette_rgb6));
    g_v22_palette_active = 1;
    return 1;
}

void csb_v22_inplace_draw_clear_indexed_palette(void)
{
    memset(g_v22_palette_rgb6, 0, sizeof(g_v22_palette_rgb6));
    g_v22_palette_active = 0;
}

const uint32_t *csb_v22_inplace_get_bitmap_by_id(const char *category,
                                                  const char *asset_id,
                                                  int *out_w,
                                                  int *out_h)
{
    (void)category;
    (void)asset_id;
    if (out_w) {
        *out_w = 0;
    }
    if (out_h) {
        *out_h = 0;
    }
    return NULL;
}

int csb_v22_inplace_render_f0128_command(
    const CSB_V1_ViewportRuntimeDrawCommandPc34 *source_command,
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height)
{
    (void)source_command;
    (void)framebuffer;
    (void)framebuffer_width;
    (void)framebuffer_height;
    (void)g_v22_palette_active;
    /* Retain the source-owned F0128 result byte-for-byte. */
    return 0;
}

const char *csb_v22_inplace_draw_source_evidence(void)
{
    return "ReDMCSB DUNVIEW.C F0111/F0115/F0128; CSBWin/Viewport.cpp; "
           "TODO.md CSB-ORIGINAL-REPLACE-001: generated "
           "v22_inplace_cache.bin is explicitly rejected until original "
           "CSB material, palette and pixel-parity evidence exist.";
}
