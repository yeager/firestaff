#include "dm1_v2_viewport_renderer_pc34.h"
#include <string.h>

static uint8_t clamp_u8(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)val;
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
    
    // Move X
    if (dx != 0) {
        int moveX = stepX * moveAmount;
        if (moveX > dx) moveX = dx;
        if (moveX < dx) moveX = dx;
        vp->scroll.scrollOffX += moveX;
    }
    
    // Move Y
    if (dy != 0) {
        int moveY = stepY * moveAmount;
        if (moveY > dy) moveY = dy;
        if (moveY < dy) moveY = dy;
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