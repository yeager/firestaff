
#include "dm1_v2_minimap.h"
#include <string.h>

void dm1_v2_minimap_init(DM1_V2_Minimap *mm, int map_w, int map_h) {
    if (!mm) return;
    memset(mm, 0, sizeof(*mm));
    mm->map_width = map_w > MINIMAP_MAX_MAP ? MINIMAP_MAX_MAP : map_w;
    mm->map_height = map_h > MINIMAP_MAX_MAP ? MINIMAP_MAX_MAP : map_h;
    mm->visible = 1;
    mm->opacity = 0.6f;
}

void dm1_v2_minimap_update(DM1_V2_Minimap *mm, int px, int py, int dir) {
    if (!mm) return;
    mm->party_x = px; mm->party_y = py; mm->party_dir = dir;
    dm1_v2_minimap_reveal(mm, px, py);
    /* Reveal adjacent squares */
    if (px > 0) dm1_v2_minimap_reveal(mm, px-1, py);
    if (px < mm->map_width-1) dm1_v2_minimap_reveal(mm, px+1, py);
    if (py > 0) dm1_v2_minimap_reveal(mm, px, py-1);
    if (py < mm->map_height-1) dm1_v2_minimap_reveal(mm, px, py+1);
}

void dm1_v2_minimap_reveal(DM1_V2_Minimap *mm, int x, int y) {
    if (mm && x >= 0 && x < mm->map_width && y >= 0 && y < mm->map_height)
        mm->explored[y][x] = 1;
}

void dm1_v2_minimap_render(const DM1_V2_Minimap *mm, uint32_t *rgba, int sw, int sh) {
    int mx, my, cell_size, ox, oy;
    if (!mm || !rgba || !mm->visible) return;

    cell_size = MINIMAP_SIZE / mm->map_width;
    if (cell_size < 1) cell_size = 1;
    ox = sw - MINIMAP_SIZE - 8; /* top-right corner */
    oy = 8;

    for (my = 0; my < mm->map_height; my++) {
        for (mx = 0; mx < mm->map_width; mx++) {
            int px0 = ox + mx * cell_size;
            int py0 = oy + my * cell_size;
            uint32_t color;
            int px2, py2;

            if (!mm->explored[my][mx]) {
                color = 0x80000000; /* dark unexplored */
            } else if (mx == mm->party_x && my == mm->party_y) {
                color = 0xFF00FF00; /* green party marker */
            } else {
                color = 0xC0404040; /* explored gray */
            }

            for (py2 = py0; py2 < py0 + cell_size && py2 < sh; py2++) {
                for (px2 = px0; px2 < px0 + cell_size && px2 < sw; px2++) {
                    if (px2 >= 0 && py2 >= 0) {
                        /* Alpha blend */
                        uint32_t bg = rgba[py2 * sw + px2];
                        float a = (float)((color >> 24) & 0xFF) / 255.0f * mm->opacity;
                        int br = (bg>>16)&0xFF, bg2=(bg>>8)&0xFF, bb=bg&0xFF;
                        int fr = (color>>16)&0xFF, fg=(color>>8)&0xFF, fb=color&0xFF;
                        int rr = (int)(fr*a + br*(1-a));
                        int gg = (int)(fg*a + bg2*(1-a));
                        int bbb = (int)(fb*a + bb*(1-a));
                        rgba[py2 * sw + px2] = 0xFF000000|(rr<<16)|(gg<<8)|bbb;
                    }
                }
            }
        }
    }

    /* Party direction indicator */
    {
        int arrow_x = ox + mm->party_x * cell_size + cell_size/2;
        int arrow_y = oy + mm->party_y * cell_size + cell_size/2;
        int ddx[4] = {0, 1, 0, -1}; /* N E S W */
        int ddy[4] = {-1, 0, 1, 0};
        int ax = arrow_x + ddx[mm->party_dir & 3] * cell_size;
        int ay = arrow_y + ddy[mm->party_dir & 3] * cell_size;
        if (ax >= 0 && ax < sw && ay >= 0 && ay < sh)
            rgba[ay * sw + ax] = 0xFFFFFF00; /* yellow direction */
    }
}

