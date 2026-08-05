
#include "firestaff_viewport_renderer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void fs_vp_init(FS_ViewportRenderer *vp, FS_GraphicsAtlas *atlas) {
    if (!vp) return;
    memset(vp, 0, sizeof(*vp));
    vp->atlas = atlas;
    if (atlas && atlas->palette_loaded)
        memcpy(vp->palette, atlas->palette, sizeof(vp->palette));
}

/* Draw a bitmap from the atlas into the framebuffer */
void fs_vp_draw_bitmap(FS_ViewportRenderer *vp, int bmp_index, int x, int y) {
    int w, h, px, py;
    uint8_t *pixels;

    if (!vp || !vp->atlas) return;
    if (fs_gfx_get_size(vp->atlas, bmp_index, &w, &h) < 0) return;
    if (w <= 0 || h <= 0 || w > 320 || h > 200) return;

    pixels = (uint8_t *)malloc(w * h);
    if (!pixels) return;

    if (fs_gfx_extract_bitmap(vp->atlas, bmp_index, pixels, w * h) < 0) {
        free(pixels); return;
    }

    /* Blit to framebuffer with transparency (index 0 = transparent) */
    for (py = 0; py < h; py++) {
        for (px = 0; px < w; px++) {
            int fbx = x + px, fby = y + py;
            uint8_t c = pixels[py * w + px];
            if (c != 0 && fbx >= 0 && fbx < FB_W && fby >= 0 && fby < FB_H)
                vp->framebuffer[fby * FB_W + fbx] = c;
        }
    }
    free(pixels);
}

/* Draw scaled bitmap using nearest-neighbor */
void fs_vp_draw_bitmap_scaled(FS_ViewportRenderer *vp, int bmp_index,
    int dx, int dy, int dw, int dh)
{
    int w, h, px, py;
    uint8_t *pixels;

    if (!vp || !vp->atlas || dw <= 0 || dh <= 0) return;
    if (fs_gfx_get_size(vp->atlas, bmp_index, &w, &h) < 0) return;
    if (w <= 0 || h <= 0) return;

    pixels = (uint8_t *)malloc(w * h);
    if (!pixels) return;
    if (fs_gfx_extract_bitmap(vp->atlas, bmp_index, pixels, w * h) < 0) {
        free(pixels); return;
    }

    for (py = 0; py < dh; py++) {
        int sy = py * h / dh;
        for (px = 0; px < dw; px++) {
            int sx = px * w / dw;
            int fbx = dx + px, fby = dy + py;
            uint8_t c = pixels[sy * w + sx];
            if (c != 0 && fbx >= 0 && fbx < FB_W && fby >= 0 && fby < FB_H)
                vp->framebuffer[fby * FB_W + fbx] = c;
        }
    }
    free(pixels);
}

/* Render one frame of the dungeon viewport */
void fs_vp_render_frame(FS_ViewportRenderer *vp) {
    VP_SquareInfo cone[VP_VIEW_DEPTH][VP_VIEW_WIDTH];
    int depth, col;

    if (!vp || !vp->atlas) return;

    /* Clear viewport area */
    {
        int y2;
        for (y2 = VP_Y; y2 < VP_Y + VP_H; y2++)
            memset(&vp->framebuffer[y2 * FB_W + VP_X], 0, VP_W);
    }

    /* Build view cone from dungeon data */
    fs_viewport_build_view_cone(vp->party_x, vp->party_y, vp->party_dir, cone);

    /* Render back to front (D3 → D0) for painter's algorithm */
    for (depth = VP_VIEW_DEPTH - 1; depth >= 0; depth--) {
        /* Scale factors based on distance */
        int wall_h = VP_H / (depth + 1);
        int wall_w = VP_W / (3 * (depth + 1));
        int y_offset = (VP_H - wall_h) / 2 + VP_Y;

        for (col = 0; col < VP_VIEW_WIDTH; col++) {
            VP_SquareInfo *sq = &cone[depth][col];
            int x_offset = VP_X + col * (VP_W / 3);

            if (sq->square_type == 0) {
                /* Wall: draw front wall bitmap scaled by distance */
                int bmp = fs_viewport_select_wall_bitmap(0, depth, col);
                if (bmp >= 0 && bmp < vp->atlas->bitmap_count)
                    fs_vp_draw_bitmap_scaled(vp, bmp, x_offset, y_offset, wall_w, wall_h);
                /* Missing wall material is deliberately no-draw.  The
                 * source-backed M11/CSB renderers own floor, ceiling, side
                 * wall and door composition; this legacy bridge must never
                 * invent pixels when a GRAPHICS.DAT record is unavailable. */
            }
        }
    }
}

void fs_vp_to_rgba(FS_ViewportRenderer *vp) {
    int i;
    if (!vp) return;
    for (i = 0; i < FB_W * FB_H; i++)
        vp->rgba_buffer[i] = vp->palette[vp->framebuffer[i]];
}
