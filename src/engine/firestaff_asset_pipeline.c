
#include "firestaff_asset_pipeline.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "dm1_v1_atari_st_stx.h"
#include "firestaff_l10n.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#define FIRESTAFF_PIPELINE_GETPID _getpid
#else
#include <unistd.h>
#define FIRESTAFF_PIPELINE_GETPID getpid
#endif

static const char *const g_dm1_atari_st_stx_md5[] = {
    "58286fceb935b18a84413c760464a6ca",
    "3fd743a3aa08706cf1c52a87de37b860",
    "5ee3f90245a1cc54fec84a12b450b4e4",
    "279e322b837e98ea258f0b16a736f9ca",
    "97cc99bf5b594b8260f42d9d81320308",
    "933955a6a596081b4bc62655efeafde5",
    NULL
};

static int load_file_alloc(const char *path, uint8_t **out, int *out_size)
{
    FILE *f;
    long size;
    size_t got;
    if (!path || !out || !out_size) return -1;
    *out = NULL;
    *out_size = 0;
    f = fopen(path, "rb");
    if (!f || fseek(f, 0, SEEK_END) != 0) {
        if (f) fclose(f);
        return -1;
    }
    size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }
    *out = (uint8_t *)malloc((size_t)size);
    if (!*out) {
        fclose(f);
        return -1;
    }
    got = fread(*out, 1u, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size || size > INT_MAX) {
        free(*out);
        *out = NULL;
        return -1;
    }
    *out_size = (int)size;
    return 0;
}

static int load_stx_file_or_virtual(const char *path,
                                    uint8_t **out,
                                    int *out_size)
{
    char temp[256];
    int written;
    int result;
    if (!path || !out || !out_size) return -1;
    if (!strstr(path, "::")) return load_file_alloc(path, out, out_size);
    written = snprintf(temp, sizeof(temp), "/tmp/firestaff-dm1-stx-%ld.stx",
                       (long)FIRESTAFF_PIPELINE_GETPID());
    if (written <= 0 || (size_t)written >= sizeof(temp) ||
        !asset_extract_virtual_path(path, temp)) {
        return -1;
    }
    result = load_file_alloc(temp, out, out_size);
    (void)remove(temp);
    return result;
}

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
    bundle->source_format = FS_ASSET_SOURCE_PC34;
}

int fs_assets_load_dm1_atari_st_stx(FS_AssetBundle *bundle,
                                     const char *data_dir)
{
    char path[ASSET_PATH_MAX];
    int match_index = -1;
    uint8_t *stx_bytes = NULL;
    int stx_size = 0;
    uint8_t *graphics = NULL;
    uint8_t *dungeon = NULL;
    size_t graphics_size = 0U;
    size_t dungeon_size = 0U;
    DM1_V1_AtariStx stx;
    if (!bundle || !data_dir) return -1;
    memset(bundle, 0, sizeof(*bundle));
    if (FSP_FileExists(data_dir)) {
        char md5[33];
        size_t i;
        if (asset_file_md5_hex(data_dir, md5)) {
            for (i = 0U; g_dm1_atari_st_stx_md5[i] != NULL; ++i) {
                if (strcmp(md5, g_dm1_atari_st_stx_md5[i]) == 0) {
                    snprintf(path, sizeof(path), "%s", data_dir);
                    match_index = (int)i;
                    break;
                }
            }
        }
        /* A launcher may pass the original archive itself rather than its
         * extracted STX member.  Keep direct-file selection source-bound by
         * asking the same virtual-aware scanner used for directory roots;
         * do not treat the archive bytes as a disk image. */
        if (match_index < 0) {
            (void)asset_find_by_md5_list(data_dir, g_dm1_atari_st_stx_md5,
                                         path, (int)sizeof(path),
                                         &match_index, 32);
        }
    } else {
        (void)asset_find_by_md5_list(data_dir, g_dm1_atari_st_stx_md5,
                                     path, (int)sizeof(path), &match_index, 32);
    }
    if (match_index < 0 ||
        load_stx_file_or_virtual(path, &stx_bytes, &stx_size) != 0 ||
        !dm1_v1_atari_st_stx_open(stx_bytes, (size_t)stx_size, &stx)) {
        free(stx_bytes);
        return -1;
    }
    graphics = (uint8_t *)malloc(1024U * 1024U);
    dungeon = (uint8_t *)malloc(128U * 1024U);
    if (!graphics || !dungeon ||
        !dm1_v1_atari_st_stx_extract_file(&stx, "GRAPHICS.DAT",
                                           graphics, 1024U * 1024U,
                                           &graphics_size) ||
        !dm1_v1_atari_st_stx_extract_file(&stx, "DUNGEON.DAT",
                                           dungeon, 128U * 1024U,
                                           &dungeon_size) ||
        graphics_size == 0U || dungeon_size == 0U) {
        free(stx_bytes);
        free(graphics);
        free(dungeon);
        return -1;
    }
    free(stx_bytes);
    bundle->graphics_data = graphics;
    bundle->graphics_size = (int)graphics_size;
    bundle->dungeon_data = dungeon;
    bundle->dungeon_size = (int)dungeon_size;
    bundle->source_format = FS_ASSET_SOURCE_DM1_ATARI_ST_STX;
    bundle->loaded = 1;
    return 0;
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
