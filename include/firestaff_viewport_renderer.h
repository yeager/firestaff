
#ifndef FIRESTAFF_VIEWPORT_RENDERER_H
#define FIRESTAFF_VIEWPORT_RENDERER_H

#include "firestaff_bitmap_extract.h"
#include "firestaff_dungeon_viewport_bridge.h"
#include <stdint.h>

/* Full DM1 viewport renderer.
 * Draws the 3D dungeon view using bitmaps from GRAPHICS.DAT.
 *
 * Viewport: 224×136 pixels at (33, 0) in the 320×200 framebuffer.
 * Same as original DM1 PC-34. */

#define VP_X 33
#define VP_Y 0
#define VP_W 224
#define VP_H 136
#define FB_W 320
#define FB_H 200

typedef struct {
    uint8_t framebuffer[FB_W * FB_H];        /* indexed */
    uint32_t rgba_buffer[FB_W * FB_H];       /* RGBA output */
    FS_GraphicsAtlas *atlas;
    uint32_t palette[256];
    int party_x, party_y, party_dir;
} FS_ViewportRenderer;

void fs_vp_init(FS_ViewportRenderer *vp, FS_GraphicsAtlas *atlas);
void fs_vp_render_frame(FS_ViewportRenderer *vp);
void fs_vp_to_rgba(FS_ViewportRenderer *vp);

/* Draw a bitmap from atlas at position in framebuffer */
void fs_vp_draw_bitmap(FS_ViewportRenderer *vp, int bmp_index, int x, int y);

/* Draw scaled bitmap (for distance-based wall sizing) */
void fs_vp_draw_bitmap_scaled(FS_ViewportRenderer *vp, int bmp_index,
    int dx, int dy, int dw, int dh);

#endif

