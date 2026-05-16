
#include "firestaff_asset_pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char *path, uint8_t **out, int *out_size) {
    FILE *f = fopen(path, "rb");
    long size;
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *out = (uint8_t *)malloc(size);
    if (!*out) { fclose(f); return -1; }
    *out_size = (int)fread(*out, 1, size, f);
    fclose(f);
    return 0;
}

int fs_assets_load_dm1(FS_AssetBundle *bundle, const char *data_dir) {
    char path[512];
    if (!bundle || !data_dir) return -1;
    memset(bundle, 0, sizeof(*bundle));

    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", data_dir);
    if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0) {
        /* Try lowercase */
        snprintf(path, sizeof(path), "%s/graphics.dat", data_dir);
        if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0)
            return -1;
    }

    snprintf(path, sizeof(path), "%s/DUNGEON.DAT", data_dir);
    if (load_file(path, &bundle->dungeon_data, &bundle->dungeon_size) < 0) {
        snprintf(path, sizeof(path), "%s/dungeon.dat", data_dir);
        load_file(path, &bundle->dungeon_data, &bundle->dungeon_size);
    }

    bundle->loaded = 1;
    return 0;
}

int fs_assets_load_csb(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_dm1(bundle, data_dir); /* Same DAT format */
}

int fs_assets_load_dm2(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_dm1(bundle, data_dir); /* Similar format */
}

void fs_assets_free(FS_AssetBundle *bundle) {
    if (!bundle) return;
    free(bundle->graphics_data); bundle->graphics_data = NULL;
    free(bundle->dungeon_data); bundle->dungeon_data = NULL;
    bundle->loaded = 0;
}

void fs_assets_expand_vga_palette(const uint8_t *vga_6bit, uint32_t *rgba_out, int count) {
    int i;
    if (!vga_6bit || !rgba_out) return;
    for (i = 0; i < count; i++) {
        /* VGA 6-bit (0-63) → 8-bit (0-255): multiply by 4 + rounding */
        uint8_t r = (vga_6bit[i * 3 + 0] << 2) | (vga_6bit[i * 3 + 0] >> 4);
        uint8_t g = (vga_6bit[i * 3 + 1] << 2) | (vga_6bit[i * 3 + 1] >> 4);
        uint8_t b = (vga_6bit[i * 3 + 2] << 2) | (vga_6bit[i * 3 + 2] >> 4);
        rgba_out[i] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
}


/* ══════════════════════════════════════════════════════════════════════
 * Multi-language dungeon.dat loader
 *
 * DM1 PC34 Multilingual has per-language dungeon files:
 *   DUNGEON.DAT  = English (default)
 *   DUNGEONF.DAT = French
 *   DUNGEONG.DAT = German
 *
 * The GRAPHICS.DAT is shared across all languages (object names and
 * action names are bitmaps, not language-specific in the multi-lang
 * version).
 *
 * In-game hardcoded text (WAKE UP, GAME FROZEN, etc) is selected
 * at compile time in ReDMCSB but we handle it at runtime via the
 * localization system (firestaff_l10n).
 *
 * Source: ReDMCSB COMMAND.C:2009-2015 (WAKE UP / WECKEN / REVEILLEZ-VOUS)
 *         ReDMCSB COMMAND.C:2393-2399 (GAME FROZEN / SPIEL ANGEHALTEN / JEU BLOQUE)
 *         ReDMCSB OBJECT.C:59 M564_GRAPHIC_OBJECT_NAMES (shared graphic)
 *         PC34 Multilingual: EUDATA/DUNGEON.DAT, DUNGEONF.DAT, DUNGEONG.DAT
 * ══════════════════════════════════════════════════════════════════════ */

typedef enum {
    FS_ASSET_LANG_EN = 0,
    FS_ASSET_LANG_FR,
    FS_ASSET_LANG_DE,
    FS_ASSET_LANG_SV,  /* Swedish (Firestaff original) */
    FS_ASSET_LANG_COUNT
} FS_AssetLanguage;

static const char *g_dungeon_dat_names[FS_ASSET_LANG_COUNT] = {
    "DUNGEON.DAT",   /* English */
    "DUNGEONF.DAT",  /* French */
    "DUNGEONG.DAT",  /* German */
    "DUNGEON.DAT",   /* Swedish (uses EN dungeon + SV runtime strings) */
};

static const char *g_dungeon_dat_names_lower[FS_ASSET_LANG_COUNT] = {
    "dungeon.dat",
    "dungeonf.dat",
    "dungeong.dat",
    "dungeon.dat",   /* Swedish: EN dungeon data */
};

int fs_assets_load_dm1_multilang(FS_AssetBundle *bundle,
    const char *data_dir, FS_AssetLanguage lang)
{
    char path[512];
    const char *dat_name;
    if (!bundle || !data_dir) return -1;
    if (lang < 0 || lang >= FS_ASSET_LANG_COUNT) lang = FS_ASSET_LANG_EN;

    memset(bundle, 0, sizeof(*bundle));

    /* GRAPHICS.DAT is shared across all languages */
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", data_dir);
    if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0) {
        snprintf(path, sizeof(path), "%s/graphics.dat", data_dir);
        if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0)
            return -1;
    }

    /* Load language-specific DUNGEON.DAT */
    dat_name = g_dungeon_dat_names[lang];
    snprintf(path, sizeof(path), "%s/%s", data_dir, dat_name);
    if (load_file(path, &bundle->dungeon_data, &bundle->dungeon_size) < 0) {
        /* Try lowercase */
        dat_name = g_dungeon_dat_names_lower[lang];
        snprintf(path, sizeof(path), "%s/%s", data_dir, dat_name);
        if (load_file(path, &bundle->dungeon_data, &bundle->dungeon_size) < 0) {
            /* Fall back to English */
            if (lang != FS_ASSET_LANG_EN) {
                printf("Firestaff: %s not found, falling back to English\\n", dat_name);
                return fs_assets_load_dm1_multilang(bundle, data_dir, FS_ASSET_LANG_EN);
            }
            return -1;
        }
    }

    bundle->loaded = 1;
    printf("Firestaff: loaded %s + %s\\n",
        "GRAPHICS.DAT", g_dungeon_dat_names[lang]);
    return 0;
}

/* Map Firestaff UI language to asset language */
FS_AssetLanguage fs_assets_lang_from_l10n(int l10n_lang) {
    switch (l10n_lang) {
        case 1: return FS_ASSET_LANG_FR; /* FS_LANG_FR = 3, but asset FR = 1 */
        case 2: return FS_ASSET_LANG_DE; /* FS_LANG_DE = 2, asset DE = 2 */
        default: return FS_ASSET_LANG_EN;
    }
}

/* In-game hardcoded strings — runtime selection instead of #ifdef */
typedef struct {
    const char *wake_up;
    const char *game_frozen;
    const char *you_died;
    const char *victory;
} FS_InGameStrings;

static const FS_InGameStrings g_ingame_strings[FS_ASSET_LANG_COUNT] = {
    /* English */ {"WAKE UP",         "GAME FROZEN",      "REST IN PEACE",    "CONGRATULATIONS"},
    /* French  */ {"REVEILLEZ-VOUS",  "JEU BLOQUE",       "REPOSEZ EN PAIX",  "FELICITATIONS"},
    /* German  */ {"WECKEN",          "SPIEL ANGEHALTEN",  "RUHE IN FRIEDEN", "HERZLICHEN GLÜCKWUNSCH"},
};

const char *fs_assets_ingame_string(FS_AssetLanguage lang, int string_id) {
    if (lang < 0 || lang >= FS_ASSET_LANG_COUNT) lang = FS_ASSET_LANG_EN;
    switch (string_id) {
        case 0: return g_ingame_strings[lang].wake_up;
        case 1: return g_ingame_strings[lang].game_frozen;
        case 2: return g_ingame_strings[lang].you_died;
        case 3: return g_ingame_strings[lang].victory;
        default: return "???";
    }
}
