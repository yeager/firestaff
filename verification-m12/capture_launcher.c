#include "menu_startup_m12.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FB_W 640
#define FB_H 400

static const unsigned char kPalette[16][3] = {
    {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,0,170},{170,85,0},
    {170,170,170},{85,85,85},{85,85,255},{85,255,85},{85,255,255},
    {255,85,85},{255,85,255},{255,255,85},{255,255,255}
};

static void write_ppm(const char* path, const unsigned char* fb, int w, int h) {
    FILE* fp = fopen(path, "wb");
    if (!fp) { perror(path); return; }
    fprintf(fp, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; ++i) {
        unsigned char idx = fb[i] & 0x0F;
        fwrite(kPalette[idx], 1, 3, fp);
    }
    fclose(fp);
    printf("Wrote %s (%dx%d)\n", path, w, h);
}

int main(void) {
    unsigned char fb[FB_W * FB_H];
    M12_StartupMenuState state;
    M12_StartupMenu_Init(&state);
    
    /* Main view */
    M12_StartupMenu_Draw(&state, fb, FB_W, FB_H);
    write_ppm("verification-m12/launcher_main.ppm", fb, FB_W, FB_H);
    
    /* Game options view */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_Draw(&state, fb, FB_W, FB_H);
    write_ppm("verification-m12/launcher_game_options.ppm", fb, FB_W, FB_H);
    
    /* Settings view */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    state.selectedIndex = 3;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_Draw(&state, fb, FB_W, FB_H);
    write_ppm("verification-m12/launcher_settings.ppm", fb, FB_W, FB_H);
    
    return 0;
}
