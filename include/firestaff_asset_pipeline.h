
#ifndef FIRESTAFF_ASSET_PIPELINE_H
#define FIRESTAFF_ASSET_PIPELINE_H

#include <stdint.h>

/* Firestaff Asset Pipeline — load original game data files.
 * Supports DM1, CSB, DM2 data formats.
 *
 * Pipeline: file → decompress → cache → render-ready */

typedef struct {
    uint8_t *graphics_data;
    int graphics_size;
    uint8_t *dungeon_data;
    int dungeon_size;
    uint32_t palette[256];
    int loaded;
    /* Identifies the admitted source container.  Callers that only support
     * the PC34 renderer must reject non-PC bundles instead of guessing. */
    int source_format;
} FS_AssetBundle;

enum {
    FS_ASSET_SOURCE_PC34 = 0,
    FS_ASSET_SOURCE_DM1_ATARI_ST_STX = 1
};

int fs_assets_load_game(FS_AssetBundle *bundle, const char *data_dir, const char *game_subdir);
int fs_assets_load_dm1(FS_AssetBundle *bundle, const char *data_dir);
/* Load a real protected Atari ST DM1 disk image.  The input is found by
 * hash, including inside supported archives; GRAPHICS.DAT and DUNGEON.DAT
 * are extracted from the STX image without filename admission. */
int fs_assets_load_dm1_atari_st_stx(FS_AssetBundle *bundle,
                                     const char *data_dir);
int fs_assets_load_csb(FS_AssetBundle *bundle, const char *data_dir);
int fs_assets_load_dm2(FS_AssetBundle *bundle, const char *data_dir);
void fs_assets_free(FS_AssetBundle *bundle);

/* VGA palette: 6-bit DAC values → 8-bit RGBA */
void fs_assets_expand_vga_palette(const uint8_t *vga_6bit, uint32_t *rgba_out, int count);




/* ── Multi-language support ───────────────────────────────────────── */

typedef enum {
    FS_ASSET_LANG_EN = 0,
    FS_ASSET_LANG_FR,
    FS_ASSET_LANG_DE,
    FS_ASSET_LANG_SV,
    FS_ASSET_LANG_COUNT
} FS_AssetLanguage;

int fs_assets_load_dm1_multilang(FS_AssetBundle *bundle,
    const char *data_dir, FS_AssetLanguage lang);
FS_AssetLanguage fs_assets_lang_from_l10n(int l10n_lang);
const char *fs_assets_ingame_string(FS_AssetLanguage lang, int string_id);

/* In-game string IDs */
#define FS_INGAME_WAKE_UP     0
#define FS_INGAME_GAME_FROZEN 1
#define FS_INGAME_YOU_DIED    2
#define FS_INGAME_VICTORY     3

#endif
