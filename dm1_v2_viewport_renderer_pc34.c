#include "dm1_v2_viewport_renderer_pc34.h"
#include <stddef.h>
#include <string.h>

static uint8_t clamp_u8(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)val;
}

/* Source-locked V2 viewport material metadata.
 *
 * ReDMCSB WIP20210206 anchors:
 *   DEFS.H:2407 names C000_DERIVED_BITMAP_VIEWPORT as a 224x136 derived bitmap.
 *   DEFS.H:2478/2484 defines C112_BYTE_WIDTH_VIEWPORT and C136_HEIGHT_VIEWPORT.
 *   DUNVIEW.C:734-753 defines G0188_aauc_Graphic558_FieldAspects for D3C..D0R.
 *   DUNVIEW.C:2968-2971 clears 37 black lines, copies a 224x29 ceiling band,
 *     then copies a 224x70 floor band.
 * Existing V1 parity module dm1_v1_viewport_3d_pc34_compat.c keeps the original
 * draw behavior; this table is V2-only metadata for the modern material pass.
 */
static const DM1_V2_FieldAspect s_field_aspects[DM1_V2_FIELD_ASPECT_COUNT] = {
    {0, 63, 0x8A, 0xFF,  76,  51,  0, 64}, /* D3C */
    {0, 63, 0x0A, 0x80,  84,  51, 11, 64}, /* D3L */
    {0, 63, 0x0A, 0x00,  85,  51,  0, 64}, /* D3R */
    {0, 60, 0x8A, 0xFF, 104,  71,  0, 64}, /* D2C */
    {0, 63, 0x0A, 0x81,  80,  71,  5, 64}, /* D2L, MEDIA488 PC */
    {0, 63, 0x0A, 0x01,  80,  71,  0, 64}, /* D2R, MEDIA488 PC */
    {0, 61, 0x8A, 0xFF, 160, 111,  0, 64}, /* D1C */
    {0, 63, 0x0A, 0x82,  59, 111,  0, 64}, /* D1L */
    {0, 63, 0x0A, 0x02,  59, 111,  0, 64}, /* D1R */
    {0, 59, 0x8A, 0xFF, 224, 136,  0, 64}, /* D0C */
    {0, 63, 0x0A, 0x83,  32, 136,  0, 64}, /* D0L */
    {0, 63, 0x0A, 0x03,  32, 136,  0, 64}, /* D0R */
};

static const int16_t s_wall_set_default[DM1_V2_WALL_SET_COUNT] = {
    -17, -16, -15, -14, -13,
     -9,  -8, -12, -11, -10,
     -4,  -3,  -7,  -6,  -5
};

int dm1_v2_vp_source_width(void) {
    return DM1_V2_VIEWPORT_W;
}

int dm1_v2_vp_source_height(void) {
    return DM1_V2_VIEWPORT_H;
}

int dm1_v2_vp_source_byte_width(void) {
    return DM1_V2_VIEWPORT_BYTE_W;
}

DM1_V2_ViewMaterial dm1_v2_vp_material_at(int x, int y) {
    if (x < 0 || x >= DM1_V2_VIEWPORT_W || y < 0 || y >= DM1_V2_VIEWPORT_H) {
        return DM1_V2_VIEW_MATERIAL_OUT_OF_BOUNDS;
    }
    if (y < DM1_V2_VIEWPORT_CEILING_H) {
        return DM1_V2_VIEW_MATERIAL_CEILING;
    }
    if (y < DM1_V2_VIEWPORT_BLACK_AREA_H) {
        return DM1_V2_VIEW_MATERIAL_BLACK;
    }
    if (y >= DM1_V2_VIEWPORT_FLOOR_Y) {
        return DM1_V2_VIEW_MATERIAL_FLOOR;
    }
    return DM1_V2_VIEW_MATERIAL_WALL;
}

const DM1_V2_FieldAspect* dm1_v2_vp_field_aspect(DM1_V2_FieldAspectId id) {
    if (id < 0 || id >= DM1_V2_FIELD_ASPECT_COUNT) return NULL;
    return &s_field_aspects[id];
}

int16_t dm1_v2_vp_wall_set_default(int idx) {
    if (idx < 0 || idx >= DM1_V2_WALL_SET_COUNT) return 0;
    return s_wall_set_default[idx];
}

void dm1_v2_vp_init(DM1_V2_ViewportState* vp) {
    if (!vp) return;
    
    // Zero out the entire state
    memset(vp, 0, sizeof(DM1_V2_ViewportState));
    
    // Initialize light defaults
    // Fog density increasing with depth
    vp->light.fogDensity[0] = 0;
    vp->light.fogDensity[1] = 64;
    vp->light.fogDensity[2] = 128;
    vp->light.fogDensity[3] = 192;
    
    vp->light.lightLevel = 128;
    vp->light.torchRadius = 3;
    vp->light.ambientR = 32;
    vp->light.ambientG = 32;
    vp->light.ambientB = 48;
    
    vp->dirty = 1;
    vp->frameCount = 0;
    vp->lastRenderMs = 0;
}

void dm1_v2_vp_begin_scroll(DM1_V2_ViewportState* vp, int dx, int dy, int speed) {
    if (!vp) return;
    
    vp->scroll.scrollTargetX = vp->scroll.scrollOffX + dx;
    vp->scroll.scrollTargetY = vp->scroll.scrollOffY + dy;
    vp->scroll.scrollSpeed = speed;
    vp->scroll.scrollProgress = 0;
    
    vp->dirty = 1;
}

void dm1_v2_vp_tick_scroll(DM1_V2_ViewportState* vp, int dtMs) {
    if (!vp) return;
    
    // Check if already at target
    if (vp->scroll.scrollOffX == vp->scroll.scrollTargetX && 
        vp->scroll.scrollOffY == vp->scroll.scrollTargetY) {
        return;
    }
    
    // Calculate progress increment
    // progress += speed * dtMs / 1000
    int progressInc = (vp->scroll.scrollSpeed * dtMs) / 1000;
    vp->scroll.scrollProgress += progressInc;
    
    // Lerp offset toward target
    // We treat scrollProgress as a counter that determines how much to move
    // For simplicity, we move by speed pixels per second
    int dx = vp->scroll.scrollTargetX - vp->scroll.scrollOffX;
    int dy = vp->scroll.scrollTargetY - vp->scroll.scrollOffY;
    
    // Determine direction and step
    int stepX = 0;
    int stepY = 0;
    
    if (dx > 0) stepX = 1;
    else if (dx < 0) stepX = -1;
    
    if (dy > 0) stepY = 1;
    else if (dy < 0) stepY = -1;
    
    // Move by speed pixels per second
    // pixels to move = speed * dtMs / 1000
    int moveAmount = (vp->scroll.scrollSpeed * dtMs) / 1000;
    
    if (moveAmount <= 0) moveAmount = 1; // Minimum movement
    
    // Move X without overshooting in either direction.
    if (dx != 0) {
        int moveX = stepX * moveAmount;
        if ((stepX > 0 && moveX > dx) ||
            (stepX < 0 && moveX < dx)) {
            moveX = dx;
        }
        vp->scroll.scrollOffX += moveX;
    }
    
    // Move Y without overshooting in either direction.
    if (dy != 0) {
        int moveY = stepY * moveAmount;
        if ((stepY > 0 && moveY > dy) ||
            (stepY < 0 && moveY < dy)) {
            moveY = dy;
        }
        vp->scroll.scrollOffY += moveY;
    }
    
    // Check if arrived
    if (vp->scroll.scrollOffX == vp->scroll.scrollTargetX && 
        vp->scroll.scrollOffY == vp->scroll.scrollTargetY) {
        vp->scroll.scrollProgress = 0;
    }
    
    vp->dirty = 1;
}

int dm1_v2_vp_is_scrolling(const DM1_V2_ViewportState* vp) {
    if (!vp) return 0;
    return (vp->scroll.scrollOffX != vp->scroll.scrollTargetX || 
            vp->scroll.scrollOffY != vp->scroll.scrollTargetY);
}

void dm1_v2_vp_set_pixel(DM1_V2_ViewportState* vp, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!vp) return;
    
    // Bounds check
    if (x < 0 || x >= DM1_V2_VIEWPORT_W || y < 0 || y >= DM1_V2_VIEWPORT_H) {
        return;
    }
    
    vp->framebuffer[y][x].r = r;
    vp->framebuffer[y][x].g = g;
    vp->framebuffer[y][x].b = b;
    vp->framebuffer[y][x].a = a;
    
    vp->dirty = 1;
}

DM1_V2_Color dm1_v2_vp_get_pixel(const DM1_V2_ViewportState* vp, int x, int y) {
    DM1_V2_Color black = {0, 0, 0, 255};
    
    if (!vp) return black;
    
    // Bounds check
    if (x < 0 || x >= DM1_V2_VIEWPORT_W || y < 0 || y >= DM1_V2_VIEWPORT_H) {
        return black;
    }
    
    return vp->framebuffer[y][x];
}

void dm1_v2_vp_clear(DM1_V2_ViewportState* vp, uint8_t r, uint8_t g, uint8_t b) {
    if (!vp) return;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            vp->framebuffer[y][x].r = r;
            vp->framebuffer[y][x].g = g;
            vp->framebuffer[y][x].b = b;
            vp->framebuffer[y][x].a = 255;
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_apply_fog(DM1_V2_ViewportState* vp, int depth) {
    if (!vp) return;
    
    // Clamp depth
    if (depth < 0) depth = 0;
    if (depth >= DM1_V2_MAX_DEPTH) depth = DM1_V2_MAX_DEPTH - 1;
    
    uint8_t density = vp->light.fogDensity[depth];
    uint8_t ambR = vp->light.ambientR;
    uint8_t ambG = vp->light.ambientG;
    uint8_t ambB = vp->light.ambientB;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            DM1_V2_Color* px = &vp->framebuffer[y][x];
            
            // Blend toward ambient by fogDensity/255
            int factor = density;
            int invFactor = 255 - factor;
            
            px->r = (uint8_t)((px->r * invFactor + ambR * factor) / 255);
            px->g = (uint8_t)((px->g * invFactor + ambG * factor) / 255);
            px->b = (uint8_t)((px->b * invFactor + ambB * factor) / 255);
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_apply_light(DM1_V2_ViewportState* vp, int cx, int cy, int radius, uint8_t intensity) {
    if (!vp) return;
    
    int r2 = radius * radius;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int dist2 = dx * dx + dy * dy;
            
            if (dist2 < r2) {
                // Calculate distance factor: 1 - dist/radius
                // Use integer math: factor = (radius*255 - dist*255/radius) / 255
                // Simplified: factor = (r2 - dist2) / r2 * 255
                int dist __attribute__((unused)) = 0;
                // Approximate sqrt using integer math or just use dist2
                // For simplicity, use dist2 based falloff
                int factor = (r2 - dist2) * 255 / r2;
                if (factor < 0) factor = 0;
                if (factor > 255) factor = 255;
                
                // Brighten by intensity * factor
                int brighten = (intensity * factor) / 255;
                
                DM1_V2_Color* px = &vp->framebuffer[y][x];
                
                int newR = px->r + brighten;
                int newG = px->g + brighten;
                int newB = px->b + brighten;
                
                px->r = clamp_u8(newR);
                px->g = clamp_u8(newG);
                px->b = clamp_u8(newB);
            }
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_mark_dirty(DM1_V2_ViewportState* vp) {
    if (!vp) return;
    vp->dirty = 1;
}

int dm1_v2_vp_is_dirty(const DM1_V2_ViewportState* vp) {
    if (!vp) return 0;
    return vp->dirty;
}

void dm1_v2_vp_present(DM1_V2_ViewportState* vp, int32_t nowMs) {
    if (!vp) return;
    
    vp->dirty = 0;
    vp->frameCount++;
    vp->lastRenderMs = nowMs;
}