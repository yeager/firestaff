
#include "firestaff_asset_pipeline.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "firestaff_l10n.h"
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

static const char *asset_pipeline_game_id(const char *game_subdir) {
    if (!game_subdir) return NULL;
    if (strcmp(game_subdir, "dm1") == 0 ||
        strcmp(game_subdir, "csb") == 0 ||
        strcmp(game_subdir, "dm2") == 0) {
        return game_subdir;
    }
    return NULL;
}

static const M12_AssetRequiredFileStatus *find_required_role(
    const M12_AssetStatus *status,
    const char *game_id,
    const char *role_id) {
    size_t i;
    size_t count;
    if (!status || !game_id || !role_id) return NULL;
    count = M12_AssetStatus_GetRequiredFileCount(status, game_id);
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus *file =
            M12_AssetStatus_GetRequiredFile(status, game_id, i);
        if (file && file->matched && file->roleId &&
            strcmp(file->roleId, role_id) == 0) {
            return file;
        }
    }
    return NULL;
}

static int fs_assets_load_game_by_hash(FS_AssetBundle *bundle,
                                       const char *data_dir,
                                       const char *game_subdir) {
    M12_AssetStatus status;
    const char *game_id = asset_pipeline_game_id(game_subdir);
    const M12_AssetRequiredFileStatus *graphics;
    const M12_AssetRequiredFileStatus *dungeon;
    if (!bundle || !data_dir || !game_id) return -1;

    M12_AssetStatus_ScanGame(&status, data_dir, game_id);
    if (!M12_AssetStatus_GameAvailable(&status, game_id)) {
        return -1;
    }
    graphics = find_required_role(&status, game_id, "graphics");
    dungeon = find_required_role(&status, game_id, "dungeon");
    if (!graphics || !graphics->matchedPath[0]) {
        return -1;
    }
    if (load_file(graphics->matchedPath,
                  &bundle->graphics_data,
                  &bundle->graphics_size) != 0) {
        return -1;
    }
    if (dungeon && dungeon->matchedPath[0] &&
        load_file(dungeon->matchedPath,
                  &bundle->dungeon_data,
                  &bundle->dungeon_size) != 0) {
        free(bundle->graphics_data);
        bundle->graphics_data = NULL;
        bundle->graphics_size = 0;
        return -1;
    }
    bundle->loaded = 1;
    return 0;
}

int fs_assets_load_game(FS_AssetBundle *bundle, const char *data_dir, const char *game_subdir) {
    if (!bundle || !data_dir) return -1;
    memset(bundle, 0, sizeof(*bundle));

    if (fs_assets_load_game_by_hash(bundle, data_dir, game_subdir) == 0) {
        return 0;
    }

    /* Game data is always source-identified by hash.  A file merely named
     * GRAPHICS.DAT or DUNGEON.DAT can be a different port, a corrupt dump,
     * or a test fixture; accepting it here used to turn such bytes into a
     * plausible-looking synthetic runtime.  M12 already supports arbitrary
     * layouts and archive materialization, so there is no production reason
     * to retain filename-only admission. */
    return -1;
}

int fs_assets_load_dm1(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_game(bundle, data_dir, "dm1");
}

int fs_assets_load_csb(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_game(bundle, data_dir, "csb");
}

int fs_assets_load_dm2(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_game(bundle, data_dir, "dm2");
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



static const char *fs_assets_dm1_multilang_dungeon_hash(FS_AssetLanguage lang) {
    switch (lang) {
        case FS_ASSET_LANG_FR:
            return "82c7802122e9cfb2dc117bc9d197f457";
        case FS_ASSET_LANG_DE:
            return "b9487db563ff9e89605c77ee44d17d14";
        case FS_ASSET_LANG_SV:
        case FS_ASSET_LANG_EN:
        default:
            return "766450c940651fc021c92fe5d0d0b3a6";
    }
}

int fs_assets_load_dm1_multilang(FS_AssetBundle *bundle,
    const char *data_dir, FS_AssetLanguage lang)
{
    char path[512];
    if (!bundle || !data_dir) return -1;
    if (lang < 0 || lang >= FS_ASSET_LANG_COUNT) lang = FS_ASSET_LANG_EN;

    memset(bundle, 0, sizeof(*bundle));

    /* Hash-first path for real DM1 multilingual media. Filenames are only a
     * legacy/custom fallback; user-supplied data may be renamed. */
    if (asset_find_by_md5(data_dir,
                          "f934d97e43e1ba6e5159839acbcd0611",
                          path,
                          (int)sizeof(path),
                          8) &&
        load_file(path, &bundle->graphics_data, &bundle->graphics_size) == 0) {
        if (asset_find_by_md5(data_dir,
                              fs_assets_dm1_multilang_dungeon_hash(lang),
                              path,
                              (int)sizeof(path),
                              8) &&
            load_file(path, &bundle->dungeon_data, &bundle->dungeon_size) == 0) {
            bundle->loaded = 1;
            return 0;
        }
        free(bundle->graphics_data);
        bundle->graphics_data = NULL;
        bundle->graphics_size = 0;
    }

    /* Do not fall through to filename-based language media.  The two hash
     * lookups above cover renamed loose files and supported containers; a
     * missing exact-language pair must be reported rather than silently
     * borrowing an English or unrelated data file. */
    return -1;
}

/* Map Firestaff UI language to asset language */
FS_AssetLanguage fs_assets_lang_from_l10n(int l10n_lang) {
    switch (l10n_lang) {
        case FS_LANG_SV: return FS_ASSET_LANG_SV;
        case FS_LANG_DE: return FS_ASSET_LANG_DE;
        case FS_LANG_FR: return FS_ASSET_LANG_FR;
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
    /* Swedish */ {"VAKNA",           "SPELET PAUSAT",     "VILA I FRID",     "GRATTIS"},
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
