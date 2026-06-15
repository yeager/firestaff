/*
 * firestaff_m12_extras_views_visual_capture.c
 *
 * Visual capture helper for the v2.7.14 M12 launcher extras views.
 * Renders each of the 3 new views (BESTIARY, ITEM ENCYCLOPEDIA,
 * SCREENSHOT GALLERY) into its own PPM file so a human reviewer
 * can compare the before/after stub state.
 */
#include "menu_startup_m12.h"
#include "screenshot_gallery_m12.h"
#include "firestaff_item_encyclopedia.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_ppm(const char* path,
                      const unsigned char* fb,
                      int w, int h) {
    FILE* f = fopen(path, "wb");
    static const unsigned char kVga[16][3] = {
        {  0,  0,  0}, {  0,  0,170}, {  0,170,  0}, {  0,170,170},
        {170,  0,  0}, {170,  0,170}, {170, 85,  0}, {170,170,170},
        { 85, 85, 85}, { 85, 85,255}, { 85,255, 85}, { 85,255,255},
        {255, 85, 85}, {255, 85,255}, {255,255, 85}, {255,255,255},
    };
    int i, idx;
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (i = 0; i < w * h; ++i) {
        idx = fb[i] & 0x0F;
        fwrite(kVga[idx], 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    M12_StartupMenuState state;
    unsigned char* fb;
    int fbW = 1280;
    int fbH = 720;
    int fbSize = fbW * fbH;
    const char* outDir = argc > 1 ? argv[1] : "/tmp/extras_smoke";

    (void)argc; (void)argv;

    fb = (unsigned char*)calloc((size_t)fbSize, 1);
    if (!fb) return 2;

    M12_StartupMenu_Init(&state);
    state.settings.graphicsIndex = M12_PRESENTATION_V22_MODERN;
    /* Verify font data and glyphs are accessible. */
    {
        int i;
        int with_visible = 0;
        extern const char* m12_effective_text_scale_str(void);
        for (i = 0; i < 26; ++i) {
            /* We can't easily call internal m12_find_glyph from
             * here, but we can verify the framebuffer writes
             * work by calling the public draw and checking
             * pixel-level changes. */
        }
        (void)with_visible;
    }
    {
        /* Direct call: m12_draw_text is internal so we can't
         * call it from here.  Just render the changelog and
         * check the framebuffer. */
        extern int M12_Changelog_LineCount(void);
        printf("changelog line count: %d\n", M12_Changelog_LineCount());
    }
    /* M12_StartupMenu_Init calls m12_show_no_game_data_popup which
     * forces state->view = M12_MENU_VIEW_MESSAGE when no game data
     * is present.  Override it to enter the actual view. */
    state.view = M12_MENU_VIEW_MAIN;

    /* First, render the changelog view (an existing working
     * modern draw function) as a baseline to verify the font
     * pipeline is producing output. */
    state.view = M12_MENU_VIEW_CHANGELOG;
    M12_Changelog_Init(&state.changelog);
    memset(fb, 0, (size_t)fbSize);
    M12_StartupMenu_Draw(&state, fb, fbW, fbH);
    {
        int colors[16] = {0};
        int i;
        for (i = 0; i < fbSize; ++i) {
            if (fb[i] < 16) colors[fb[i]]++;
        }
        printf("\n=== Changelog baseline color distribution ===\n");
        for (i = 0; i < 16; ++i) {
            printf("  color[%2d] = %d pixels\n", i, colors[i]);
        }
    }

    /* Also try the MAIN view to see if it produces white text. */
    state.view = M12_MENU_VIEW_MAIN;
    memset(fb, 0, (size_t)fbSize);
    M12_StartupMenu_Draw(&state, fb, fbW, fbH);
    {
        int colors[16] = {0};
        int i;
        for (i = 0; i < fbSize; ++i) {
            if (fb[i] < 16) colors[fb[i]]++;
        }
        printf("\n=== Main view color distribution ===\n");
        for (i = 0; i < 16; ++i) {
            printf("  color[%2d] = %d pixels\n", i, colors[i]);
        }
    }
    {
        char path[256];
        snprintf(path, sizeof(path), "%s/_baseline_changelog.ppm", outDir);
        write_ppm(path, fb, fbW, fbH);
        printf("Wrote %s (baseline)\n", path);
    }

    /* Bestiary */
    state.extrasSelected = M12_EXTRAS_BESTIARY;
    state.view = M12_MENU_VIEW_BESTIARY;
    M12_Bestiary_Init(&state.bestiary);
    memset(fb, 0, (size_t)fbSize);
    /* Marker at a fixed location: paint pixel (10, 10) with a
     * unique color before drawing so we can confirm in the
     * output which view was being rendered. */
    fb[10 * fbW + 10] = 0x0E;  /* bright yellow */
    M12_StartupMenu_Draw(&state, fb, fbW, fbH);
    {
        char path[256];
        snprintf(path, sizeof(path), "%s/bestiary.ppm", outDir);
        write_ppm(path, fb, fbW, fbH);
        printf("Wrote %s (view=%d)\n", path, (int)state.view);
    }

    /* Item Encyclopedia */
    state.extrasSelected = M12_EXTRAS_ITEMS;
    state.view = M12_MENU_VIEW_ITEM_ENCYCLOPEDIA;
    state.itemEncyclopediaSelectedIndex = 0;
    state.itemEncyclopediaScrollOffset = 0;
    state.itemEncyclopediaCategory = 0;
    memset(fb, 0, (size_t)fbSize);
    fb[10 * fbW + 10] = 0x0D;  /* magenta */
    M12_StartupMenu_Draw(&state, fb, fbW, fbH);
    {
        char path[256];
        snprintf(path, sizeof(path), "%s/item_encyclopedia.ppm", outDir);
        write_ppm(path, fb, fbW, fbH);
        printf("Wrote %s (view=%d)\n", path, (int)state.view);
    }

    /* Screenshot Gallery */
    state.extrasSelected = M12_EXTRAS_SCREENSHOTS;
    state.view = M12_MENU_VIEW_SCREENSHOT_GALLERY;
    memset(fb, 0, (size_t)fbSize);
    fb[10 * fbW + 10] = 0x0C;  /* red */
    M12_StartupMenu_Draw(&state, fb, fbW, fbH);
    {
        char path[256];
        snprintf(path, sizeof(path), "%s/screenshot_gallery.ppm", outDir);
        write_ppm(path, fb, fbW, fbH);
        printf("Wrote %s (view=%d entryCount=%d)\n", path,
               (int)state.view, state.screenshotGallery.entryCount);
    }

    free(fb);
    return 0;
}

/* Print color distribution of a framebuffer for debugging. */
static void dump_color_distribution(const unsigned char* fb, int n,
                                    const char* label) {
    int colors[16] = {0};
    int i;
    for (i = 0; i < n; ++i) {
        if (fb[i] < 16) colors[fb[i]]++;
    }
    printf("\n=== %s color distribution ===\n", label);
    for (i = 0; i < 16; ++i) {
        printf("  color[%2d] = %d pixels\n", i, colors[i]);
    }
}
int dummy_anchor(void) {
    dump_color_distribution(NULL, 0, "anchor");
    return 0;
}
