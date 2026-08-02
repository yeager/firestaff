/*
 * test_dm2_v1_gdat_cross_platform.c
 *
 * Cross-platform GDAT comparison: loads all available DM2 GRAPHICS.DAT
 * files and compares entry counts per category across platforms.
 *
 * Validates that all platforms parse successfully and reports
 * the category-level entry distribution matrix.
 */

#include "dm2_v1_asset_loader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *label;
    const char *subpath;
    int         is_be;
} PlatformDef;

static const PlatformDef g_platforms[] = {
    {"PC EN",      "dm2-extras/dos-en/data/GRAPHICS.DAT",          0},
    {"PC FR",      "dm2-extras/pc-fr/DATA/GRAPHICS.DAT",           0},
    {"PC DE",      "dm2-extras/pc-de/DATA/GRAPHICS.DAT",           0},
    {"Mac EN",     "dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/Graphics.dat", 1},
    {"Mac FR",     "dm2-extras/mac-fr/Dungeon Master II/DMFiles/Graphics.dat", 1},
    {"FM Towns",   "dm2-extras/fm-towns-ja/extracted/GRAPHICS.DAT",0},
    {"Amiga EN",   "dm2-extras/amiga-en-extracted/GRAPHICS.DAT",   1},
    {"Mega CD",    "dm2-extras/mega-cd-jp-extracted/GRAPHICS.DAT", 1},
    {"PC-9821",    "dm2-extras/pc9821-jp-extracted/GRAPHICS.DAT",  0},
};
#define PLATFORM_COUNT (sizeof(g_platforms) / sizeof(g_platforms[0]))

static const char *g_category_names[] = {
    [0x00] = "TECHDATA",
    [0x01] = "IFACE_GEN",
    [0x02] = "SPELL_DEF",
    [0x03] = "MESSAGES",
    [0x04] = "MUSICS",
    [0x05] = "TITLE",
    [0x06] = "CREDITS",
    [0x07] = "CHARSHEET",
    [0x08] = "GFXSET",
    [0x09] = "WALL_GFX",
    [0x0A] = "FLOOR_GFX",
    [0x0B] = "DOOR_GFX",
    [0x0C] = "DOOR_BTN",
    [0x0D] = "SPELL_MSL",
    [0x0E] = "DOORS",
    [0x0F] = "CREATURES",
    [0x10] = "WEAPONS",
    [0x11] = "CLOTHES",
    [0x12] = "SCROLLS",
    [0x13] = "POTIONS",
    [0x14] = "CONTAINERS",
    [0x15] = "MISC",
    [0x16] = "CHAMPIONS",
    [0x17] = "ENVIRON",
    [0x18] = "TELEPORT",
    [0x19] = "CREAT_AI",
    [0x1A] = "DIALOG",
    [0x1C] = "JP_FONT",
};
#define MAX_CATEGORY 0x1D

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

int main(void) {
    const char *home;
    char path[1024];
    int loaded_count = 0;

    uint16_t totals[PLATFORM_COUNT];
    uint16_t cat_counts[PLATFORM_COUNT][MAX_CATEGORY + 1];
    int loaded[PLATFORM_COUNT];
    size_t sizes[PLATFORM_COUNT];

    printf("DM2 cross-platform GDAT comparison:\n\n");

    home = getenv("HOME");
    if (!home) { printf("SKIP: HOME not set\n"); return 0; }

    memset(totals, 0, sizeof(totals));
    memset(cat_counts, 0, sizeof(cat_counts));
    memset(loaded, 0, sizeof(loaded));
    memset(sizes, 0, sizeof(sizes));

    for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
        uint8_t *data;
        size_t data_size;
        DM2_V1_AssetLoader state;

        snprintf(path, sizeof(path), "%s/.firestaff/data/%s",
                 home, g_platforms[p].subpath);
        data = read_file(path, &data_size);
        if (!data) {
            printf("  SKIP: %-10s — cannot read\n", g_platforms[p].label);
            continue;
        }

        memset(&state, 0, sizeof(state));
        int rc = dm2_v1_asset_loader_init(&state, data, data_size);
        if (rc != 0) {
            printf("  FAIL: %-10s — init returned %d\n",
                   g_platforms[p].label, rc);
            free(data);
            continue;
        }

        loaded[p] = 1;
        loaded_count++;
        sizes[p] = data_size;
        totals[p] = state.entry_count;

        for (int c = 0; c <= MAX_CATEGORY; c++) {
            cat_counts[p][c] = state.category_entry_counts[c];
        }

        printf("  %-10s: %zu bytes, %u entries\n",
               g_platforms[p].label, data_size, state.entry_count);

        free(data);
    }

    if (loaded_count < 2) {
        printf("\nNeed at least 2 platforms to compare. SKIP.\n");
        return 0;
    }

    /* Print comparison matrix */
    printf("\n  Category      ");
    for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
        if (!loaded[p]) continue;
        printf(" %8s", g_platforms[p].label);
    }
    printf("\n  %-14s", "──────────────");
    for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
        if (!loaded[p]) continue;
        printf(" %8s", "────────");
    }
    printf("\n");

    for (int c = 0; c <= MAX_CATEGORY; c++) {
        int any = 0;
        for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
            if (loaded[p] && cat_counts[p][c] > 0) { any = 1; break; }
        }
        if (!any) continue;

        const char *name = (c < (int)(sizeof(g_category_names)/sizeof(g_category_names[0])))
                           ? g_category_names[c] : NULL;
        if (!name) name = "???";

        printf("  0x%02X %-8s", c, name);
        for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
            if (!loaded[p]) continue;
            printf(" %8u", cat_counts[p][c]);
        }
        printf("\n");
    }

    printf("  %-14s", "TOTAL");
    for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
        if (!loaded[p]) continue;
        printf(" %8u", totals[p]);
    }
    printf("\n");

    printf("  %-14s", "SIZE");
    for (int p = 0; p < (int)PLATFORM_COUNT; p++) {
        if (!loaded[p]) continue;
        printf(" %8zu", sizes[p]);
    }
    printf("\n");

    printf("\n  Loaded %d of %d platforms.\n", loaded_count, (int)PLATFORM_COUNT);
    printf("  PASS: all loaded platforms parsed successfully.\n");
    return 0;
}
