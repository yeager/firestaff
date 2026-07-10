#include "nexus_v1_rasterizer.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int count_color(const Nexus_Framebuffer *fb, uint8_t color) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->color_buffer[i] == color) ++count;
    }
    return count;
}

static int count_written_depth(const Nexus_Framebuffer *fb) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->z_buffer[i] < 1e30f) ++count;
    }
    return count;
}

int main(void) {
    Nexus_Framebuffer fb;
    Nexus_Camera cam;
    /* 3x2 is deliberately not a power-of-two surface. */
    const uint8_t pixels[6] = {1, 2, 3, 2, 1, 3};
    uint32_t palette[256] = {0};
    uint8_t map[256];
    int written_before;

    nexus_fb_init(&fb);
    nexus_fb_clear(&fb);
    nexus_camera_init(&cam, (Vec3){0.5f, 0.5f, 3.5f}, 0);
    memset(map, 0xff, sizeof(map));
    palette[1] = 0xff102030U;
    palette[2] = 0xff405060U;
    palette[3] = 0xff708090U;
    map[1] = 41;
    map[2] = 42;
    map[3] = 43;
    fb.palette[41] = palette[1];
    fb.palette[42] = palette[2];
    fb.palette[43] = palette[3];

    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 41) > 0 && count_color(&fb, 42) > 0 &&
               count_color(&fb, 43) > 0,
           "wall rasterizer samples all texels from a 3x2 material surface");
    expect(fb.palette[41] == palette[1] && fb.palette[42] == palette[2] &&
               fb.palette[43] == palette[3],
           "material CLUT is preserved through framebuffer remapping");

    nexus_fb_clear(&fb);
    palette[2] = 0x00000000U;
    written_before = count_written_depth(&fb);
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "transparent material texels never use their mapped replacement color");
    expect(count_written_depth(&fb) > written_before,
           "opaque material texels still write depth after transparent clipping");

    nexus_fb_clear(&fb);
    palette[2] = 0xff405060U;
    map[2] = 0xff;
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "undefined palette remaps clip instead of falling back to flat color");

    return failures ? 1 : 0;
}
