/*
 * test_csb_v1_m11_startup_resume_gate.c -- CSB V1 startup/resume M11 gate.
 *
 * Verifies that M11_GameView_Start(gameId="csb") owns the CSB boot profile
 * and loads an optional CSB savePath through the CSB runtime, not through the
 * generic DM1 quick-resume branch.
 *
 * Source-lock:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance/runtime setup)
 *   ReDMCSB LOADSAVE.C F0435 lines 2721-2800 (save restore of GLOBAL_DATA,
 *     GameTime, party map/position/direction, leader and caster)
 */

#include "csb_v1_boot.h"
#include "csbwin_resume_fixture.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;

typedef struct {
    unsigned char *buf;
    size_t cap;
    size_t bit_pos;
} TestBitWriter;

typedef struct {
    unsigned char dict_first[4096];
    unsigned short dict_prefix[4096];
    int dict_count;
    int code_bits;
} TestLZW;

static int test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void write_be16(unsigned char *buf, size_t off, unsigned short value) {
    buf[off] = (unsigned char)((value >> 8) & 0xffu);
    buf[off + 1u] = (unsigned char)(value & 0xffu);
}

static void test_bw_init(TestBitWriter *bw) {
    bw->cap = 1024u;
    bw->buf = (unsigned char*)calloc(1u, bw->cap);
    bw->bit_pos = 0u;
}

static int test_bw_grow(TestBitWriter *bw) {
    size_t old_cap = bw->cap;
    size_t new_cap = bw->cap * 2u;
    unsigned char *new_buf = (unsigned char*)realloc(bw->buf, new_cap);
    if (!new_buf) {
        free(bw->buf);
        bw->buf = NULL;
        bw->cap = 0u;
        return 0;
    }
    memset(new_buf + old_cap, 0, new_cap - old_cap);
    bw->buf = new_buf;
    bw->cap = new_cap;
    return 1;
}

static int test_bw_write_bits(TestBitWriter *bw, unsigned int value, int n_bits) {
    int i;
    for (i = 0; i < n_bits; ++i) {
        size_t bp = bw->bit_pos++;
        size_t byte_idx;
        int bit_in_byte;
        if ((bp >> 3) >= bw->cap && !test_bw_grow(bw)) {
            return 0;
        }
        byte_idx = bp >> 3;
        bit_in_byte = (int)(bp & 7u);
        if (value & (1u << (unsigned int)i)) {
            bw->buf[byte_idx] |= (unsigned char)(1u << (unsigned int)bit_in_byte);
        }
    }
    return 1;
}

static void test_lzw_init(TestLZW *e) {
    int i;
    e->dict_count = DM1_GFX_LZW_FIRST_CODE;
    e->code_bits = 9;
    for (i = 0; i < 256; ++i) {
        e->dict_first[i] = (unsigned char)i;
        e->dict_prefix[i] = 0xffffu;
    }
}

static int test_lzw_find_or_add(TestLZW *e,
                                unsigned short prefix,
                                unsigned char append) {
    int i;
    for (i = DM1_GFX_LZW_FIRST_CODE; i < e->dict_count; ++i) {
        if (e->dict_prefix[i] == prefix && e->dict_first[i] == append) {
            return i;
        }
    }
    if (e->dict_count >= DM1_GFX_LZW_MAX_CODE) {
        return -1;
    }
    e->dict_prefix[e->dict_count] = prefix;
    e->dict_first[e->dict_count] = append;
    ++e->dict_count;
    return -1;
}

static void test_lzw_maybe_grow(TestLZW *e) {
    if (e->dict_count > ((1 << e->code_bits) - 1) && e->code_bits < 12) {
        ++e->code_bits;
    }
}

static int test_lzw_encode(const unsigned char *input,
                           size_t in_size,
                           unsigned char **out_buf,
                           size_t *out_size) {
    TestBitWriter bw;
    TestLZW e;
    unsigned short prefix_code;
    size_t i;

    if (!input || in_size == 0u || !out_buf || !out_size) {
        return -1;
    }
    *out_buf = NULL;
    *out_size = 0u;
    test_bw_init(&bw);
    if (!bw.buf) {
        return -1;
    }
    test_lzw_init(&e);
    if (!test_bw_write_bits(&bw, DM1_GFX_LZW_CLEAR_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    prefix_code = input[0];
    for (i = 1u; i < in_size; ++i) {
        unsigned char next_byte = input[i];
        int existing = test_lzw_find_or_add(&e, prefix_code, next_byte);
        if (existing >= 0) {
            prefix_code = (unsigned short)existing;
        } else {
            if (!test_bw_write_bits(&bw, prefix_code, e.code_bits)) {
                free(bw.buf);
                return -1;
            }
            test_lzw_maybe_grow(&e);
            prefix_code = next_byte;
        }
    }
    if (!test_bw_write_bits(&bw, prefix_code, e.code_bits) ||
        !test_bw_write_bits(&bw, DM1_GFX_LZW_END_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    *out_buf = bw.buf;
    *out_size = (bw.bit_pos + 7u) / 8u;
    return 0;
}

static unsigned char *build_csbgraphics_single_entry(
    unsigned int entry_index,
    const unsigned char *decoded,
    size_t decoded_size,
    size_t *out_size) {
    unsigned char *compressed = NULL;
    size_t compressed_size = 0u;
    unsigned int count = entry_index + 1u;
    size_t header_size = 2u + (size_t)count * 4u;
    unsigned char *buf;

    if (!out_size || !decoded || decoded_size == 0u ||
        decoded_size > 65535u ||
        test_lzw_encode(decoded, decoded_size,
                        &compressed, &compressed_size) != 0 ||
        !compressed || compressed_size == 0u || compressed_size > 65535u) {
        free(compressed);
        return NULL;
    }
    buf = (unsigned char*)calloc(1u, header_size + compressed_size);
    if (!buf) {
        free(compressed);
        return NULL;
    }
    write_be16(buf, 0u, (unsigned short)count);
    write_be16(buf, 2u + (size_t)entry_index * 2u,
               (unsigned short)compressed_size);
    write_be16(buf, 2u + (size_t)count * 2u + (size_t)entry_index * 2u,
               (unsigned short)decoded_size);
    memcpy(buf + header_size, compressed, compressed_size);
    *out_size = header_size + compressed_size;
    free(compressed);
    return buf;
}

static int count_nonzero_rect(const unsigned char* fb,
                              int stride,
                              int x,
                              int y,
                              int w,
                              int h) {
    int count = 0;
    int px;
    int py;
    if (!fb || stride <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (py = y; py < y + h; ++py) {
        for (px = x; px < x + w; ++px) {
            if (fb[py * stride + px] != 0U) {
                ++count;
            }
        }
    }
    return count;
}

static int count_diff_rect(const unsigned char* expected,
                           const unsigned char* actual,
                           int stride,
                           int x,
                           int y,
                           int w,
                           int h) {
    int count = 0;
    int px;
    int py;
    if (!expected || !actual || stride <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (py = y; py < y + h; ++py) {
        for (px = x; px < x + w; ++px) {
            int offset = py * stride + px;
            if (expected[offset] != actual[offset]) {
                ++count;
            }
        }
    }
    return count;
}

static void snapshot_current_csb_grid(uint8_t grid[32 * 32]) {
    const CSB_V1_DungeonData* dungeon = csb_v1_dungeon_get_current();
    int level;
    int width;
    int height;
    int max_w;
    int max_h;
    int x;
    int y;

    memset(grid, 0, 32 * 32);
    if (!dungeon || !dungeon->raw_data || dungeon->level_count <= 0) {
        return;
    }
    level = csb_v1_dungeon_get_current_level();
    if (level < 0 || level >= dungeon->level_count) {
        level = 0;
    }
    width = dungeon->level_widths[level];
    height = dungeon->level_heights[level];
    max_w = width < 32 ? width : 32;
    max_h = height < 32 ? height : 32;
    if (max_w <= 0 || max_h <= 0) {
        return;
    }
    for (y = 0; y < max_h; ++y) {
        for (x = 0; x < max_w; ++x) {
            int square_type = csb_v1_dungeon_get_square_type(dungeon, level, x, y);
            grid[y * 32 + x] = (square_type >= 0) ? (uint8_t)square_type : 0U;
        }
    }
}

static void render_expected_csb_viewport(const CSB_V1_RuntimeProfile* runtime,
                                         unsigned char fb[320 * 200]) {
    CSB_V1_ViewportConfig cfg;
    uint8_t grid[32 * 32];

    memset(fb, 0, 320 * 200);
    snapshot_current_csb_grid(grid);
    memset(&cfg, 0, sizeof(cfg));
    cfg.viewport_pixels = fb;
    cfg.viewport_stride = 320;
    cfg.dungeon_grid = grid;
    cfg.dungeon_width = 32;
    cfg.dungeon_height = 32;
    cfg.wall_set_index = 0;
    csb_v1_viewport_render_frame(&cfg,
                                  runtime->party_dir,
                                  runtime->party_x,
                                  runtime->party_y);
}

static int inject_synthetic_csbgraphics_viewport_override(
    CSB_V1_BootProfile *profile,
    unsigned char expected_pixels[8 * 8]) {
    unsigned char *bytes;
    size_t size = 0u;
    int i;

    if (!profile || !expected_pixels) {
        return 0;
    }
    for (i = 0; i < 8 * 8; ++i) {
        expected_pixels[i] = (unsigned char)((i % 15) + 1);
    }
    bytes = build_csbgraphics_single_entry(73u,
                                           expected_pixels,
                                           8u * 8u,
                                           &size);
    if (!bytes) {
        return 0;
    }

    csb_v1_csbgraphics_dat_real_cache_free(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_dat_real_cache_init(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_m11_runtime_plan_init(&profile->csbgraphics_m11_plan);
    profile->csbgraphics_cache.file_buffer = bytes;
    profile->csbgraphics_cache.file_size = size;
    profile->csbgraphics_cache.loaded = 1;
    snprintf(profile->csbgraphics_cache.resolved_path,
             sizeof(profile->csbgraphics_cache.resolved_path),
             "/synthetic/CSBgraphics.dat");
    snprintf(profile->csbgraphics_cache.matched_md5,
             sizeof(profile->csbgraphics_cache.matched_md5),
             "00000000000000000000000000000000");
    snprintf(profile->csbgraphics_cache.matched_label,
             sizeof(profile->csbgraphics_cache.matched_label),
             "synthetic-m11-draw");
    if (csb_v1_csbgraphics_dat_classify(
            profile->csbgraphics_cache.file_buffer,
            profile->csbgraphics_cache.file_size,
            &profile->csbgraphics_cache.index) !=
        CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
        return 0;
    }
    profile->csbgraphics_scan_attempted = 1;
    profile->csbgraphics_scan_result = CSB_V1_CSBGRAPHICS_DAT_REAL_OK;
    profile->csbgraphics_plan_result =
        csb_v1_csbgraphics_m11_runtime_plan_add_explicit_entry(
            &profile->csbgraphics_cache,
            73u,
            8u,
            8u,
            &profile->csbgraphics_m11_plan);
    return profile->csbgraphics_plan_result ==
           CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

static int write_tiny_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    fputs(bytes, f);
    fclose(f);
    return 1;
}

static int make_temp_csb_root(char root[512], char csb_dir[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_csb_m11_resume_gate_%lu",
             (unsigned long)rand());
    if (TEST_MKDIR(root) != 0) {
        return 0;
    }
#else
    char tmpl[] = "/tmp/firestaff_csb_m11_resume_gate_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
#endif
    snprintf(csb_dir, 512, "%s%s%s", root, TEST_PATH_SEP, "csb");
    if (TEST_MKDIR(csb_dir) != 0) {
        (void)TEST_RMDIR(root);
        return 0;
    }
    return 1;
}

static void remove_temp_csb_root(const char* root, const char* csb_dir) {
    char graphics[512];
    char dungeon[512];
    snprintf(graphics, sizeof(graphics), "%s%sGRAPHICS.DAT",
             csb_dir, TEST_PATH_SEP);
    snprintf(dungeon, sizeof(dungeon), "%s%sDUNGEON.DAT",
             csb_dir, TEST_PATH_SEP);
    remove(graphics);
    remove(dungeon);
    (void)TEST_RMDIR(csb_dir);
    (void)TEST_RMDIR(root);
}

static void fill_csb_launch_spec(M11_GameLaunchSpec* spec,
                                 const char* data_dir,
                                 const char* save_path) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "CHAOS STRIKES BACK";
    spec->gameId = "csb";
    spec->sourceId = "csb";
    spec->dataDir = data_dir;
    spec->savePath = save_path;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static void check_incomplete_required_files_block_m11(const char* label,
                                                      int seed_graphics,
                                                      int seed_dungeon) {
    char root[512];
    char csb_dir[512];
    char path[512];
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    expect_true(make_temp_csb_root(root, csb_dir),
                "created isolated CSB incomplete-data root");
    if (seed_graphics) {
        snprintf(path, sizeof(path), "%s%sGRAPHICS.DAT", csb_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-csb-graphics"),
                    "seeded synthetic CSB GRAPHICS.DAT");
    }
    if (seed_dungeon) {
        snprintf(path, sizeof(path), "%s%sDUNGEON.DAT", csb_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-csb-dungeon"),
                    "seeded synthetic CSB DUNGEON.DAT");
    }

    fill_csb_launch_spec(&spec, root, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0, label);
    expect_true(view.active == 0,
                "M11 incomplete/unverified CSB launch leaves view inactive");
    expect_true(view.csbBootProfile == NULL,
                "M11 incomplete/unverified CSB launch does not retain boot profile");
    expect_true(view.sourceKind != M11_GAME_SOURCE_CSB_BOOT,
                "M11 incomplete/unverified CSB launch does not claim CSB boot source");
    M11_GameView_Shutdown(&view);
    remove_temp_csb_root(root, csb_dir);
}

static const char* csb_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_CSB_V1_DATA_DIR");
    const char* home;
    if (!data_dir || !data_dir[0]) {
        data_dir = getenv("FIRESTAFF_CSB_CANONICAL_DIR");
    }
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static int build_runtime_resume_save(const char* data_dir,
                                     const char* save_path,
                                     CSB_V1_RuntimeProfile* expected) {
    CSB_V1_BootProfile boot;
    csb_v1_boot_profile_init(&boot);
    if (csb_v1_boot_scan_assets(&boot, data_dir) != 0 ||
        !boot.assets_verified) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }
    if (csb_v1_boot_enter_game(&boot) != 0) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }

    boot.runtime.party_x = CSB_V1_START_PARTY_X + 2;
    boot.runtime.party_y = CSB_V1_START_PARTY_Y + 1;
    boot.runtime.party_dir = CSB_V1_DIR_EAST;
    boot.runtime.leader_index = 0;
    boot.runtime.magic_caster_index = 1;
    boot.runtime.tick_count = 7U;
    boot.runtime.game_time = 7U;
    boot.runtime.total_play_ms = 7ULL * (uint64_t)CSB_V1_TICK_MS_NOMINAL;
    boot.runtime.party_state.PartyMapX = boot.runtime.party_x;
    boot.runtime.party_state.PartyMapY = boot.runtime.party_y;
    boot.runtime.party_state.PartyDirection = boot.runtime.party_dir;
    boot.runtime.party_state.LeaderIndex = boot.runtime.leader_index;
    boot.runtime.party_state.MagicCasterIndex = boot.runtime.magic_caster_index;
    boot.runtime.party_state.ChampionCount = 2;
    boot.runtime.party_state.ImportedFromDM1 = 1;
    snprintf(boot.runtime.party_state.Champions[0].Name,
             sizeof(boot.runtime.party_state.Champions[0].Name), "TESTA");
    snprintf(boot.runtime.party_state.Champions[1].Name,
             sizeof(boot.runtime.party_state.Champions[1].Name), "TESTB");
    boot.runtime.party_state.Champions[0].Cell = CSB_V1_CELL_FRONT_LEFT;
    boot.runtime.party_state.Champions[1].Cell = CSB_V1_CELL_RIGHT;
    boot.runtime.party_state.Champions[0].Direction = boot.runtime.party_dir;
    boot.runtime.party_state.Champions[1].Direction = boot.runtime.party_dir;
    boot.runtime.party_state.Champions[0].CurrentHealth = 100;
    boot.runtime.party_state.Champions[0].MaximumHealth = 100;
    boot.runtime.party_state.Champions[1].CurrentHealth = 100;
    boot.runtime.party_state.Champions[1].MaximumHealth = 100;
    boot.runtime.party_state_valid = 1;
    boot.runtime.champion_count = boot.runtime.party_state.ChampionCount;

    if (csb_v1_runtime_save_game_to_path(&boot.runtime, save_path) !=
        CSB_V1_SAVE_OK) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }
    if (expected) {
        *expected = boot.runtime;
        expected->dungeon_handle = NULL;
    }
    csb_v1_boot_cleanup(&boot);
    return 1;
}

static void fill_raw_csbgame_champion(CSB_V1_Champion* champ,
                                      const char* name,
                                      int hp,
                                      int cell) {
    int i;
    if (!champ) return;
    memset(champ, 0, sizeof(*champ));
    snprintf(champ->Name, sizeof(champ->Name), "%s", name);
    champ->CurrentHealth = (int16_t)hp;
    champ->MaximumHealth = (int16_t)hp;
    champ->CurrentStamina = (int16_t)(hp + 12);
    champ->MaximumStamina = (int16_t)(hp + 12);
    champ->CurrentMana = (int16_t)(hp / 2);
    champ->MaximumMana = (int16_t)(hp / 2);
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        champ->Statistics[i][0] = (uint16_t)(20 + i);
        champ->Statistics[i][1] = (uint16_t)(30 + i);
        champ->Statistics[i][2] = (uint16_t)(40 + i);
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        champ->Skills[i] = (uint8_t)(i + 1);
    }
    champ->Cell = (uint8_t)cell;
    champ->Direction = CSB_V1_DIR_EAST;
}

static int write_raw_csbgame_roster_save(const char* path) {
    CSB_V1_PartyState party;
    unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE * 2];
    long len;
    FILE* fp;
    int ok;

    csb_v1_character_init_default(&party);
    party.ChampionCount = 2;
    party.LeaderIndex = 0;
    party.MagicCasterIndex = 0;
    party.PartyMapX = CSB_V1_START_PARTY_X + 4;
    party.PartyMapY = CSB_V1_START_PARTY_Y + 5;
    party.PartyDirection = CSB_V1_DIR_EAST;
    fill_raw_csbgame_champion(&party.Champions[0], "ROSTERA", 96,
                              CSB_V1_CELL_FRONT_LEFT);
    fill_raw_csbgame_champion(&party.Champions[1], "ROSTERB", 88,
                              CSB_V1_CELL_RIGHT);
    len = csb_v1_build_csb_save_buffer(&party, CSB_SAVE_VERSION_V21,
                                       buf, (long)sizeof(buf));
    if (len <= 0) {
        return 0;
    }

    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    ok = fwrite(buf, 1u, (size_t)len, fp) == (size_t)len &&
         fclose(fp) == 0;
    return ok;
}

int main(void) {
    char fallback[512];
    char save_tmpl[] = "/tmp/firestaff_csb_m11_resume_XXXXXX";
    char quick_save_tmpl[] = "/tmp/firestaff_csb_m11_quicksave_XXXXXX";
    char roster_save_tmpl[] = "/tmp/firestaff_csb_m11_roster_XXXXXX";
    char csbwin_save_tmpl[] = "/tmp/firestaff_csb_m11_csbwin_XXXXXX";
    char save_path[560];
    char quick_save_path[560];
    char roster_save_path[560];
    char csbwin_save_path[560];
    const char* data_dir = csb_data_dir(fallback);
    CSB_V1_BootProfile preflight;
    CSB_V1_RuntimeProfile expected;
    CSB_V1_RuntimeProfile quick_loaded;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    CSB_V1_BootProfile* profile;

    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when GRAPHICS.DAT is present without DUNGEON.DAT",
        1, 0);
    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when DUNGEON.DAT is present without GRAPHICS.DAT",
        0, 1);
    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when required filenames exist but hashes are unknown",
        1, 1);

    if (!data_dir || !data_dir[0]) {
        puts("skip: no CSB data directory configured");
        return g_failures == 0 ? 0 : 1;
    }

    csb_v1_boot_profile_init(&preflight);
    if (csb_v1_boot_scan_assets(&preflight, data_dir) != 0 ||
        !preflight.assets_verified) {
        printf("skip: no hash-verified CSB V1 profile at %s\n", data_dir);
        csb_v1_boot_cleanup(&preflight);
        return g_failures == 0 ? 0 : 1;
    }
    csb_v1_boot_cleanup(&preflight);

#ifdef _WIN32
    snprintf(save_path, sizeof(save_path), ".\\firestaff-csb-m11-resume.sav");
    snprintf(quick_save_path, sizeof(quick_save_path),
             ".\\firestaff-csb-m11-quicksave.sav");
    snprintf(roster_save_path, sizeof(roster_save_path),
             ".\\firestaff-csb-m11-roster.sav");
    snprintf(csbwin_save_path, sizeof(csbwin_save_path),
             ".\\firestaff-csb-m11-csbwin.sav");
#else
    {
        int fd = mkstemp(save_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary save path\n");
            return 1;
        }
        close(fd);
        snprintf(save_path, sizeof(save_path), "%s.sav", save_tmpl);
        remove(save_tmpl);
    }
    {
        int fd = mkstemp(quick_save_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary quicksave path\n");
            return 1;
        }
        close(fd);
        snprintf(quick_save_path, sizeof(quick_save_path), "%s.sav",
                 quick_save_tmpl);
        remove(quick_save_tmpl);
    }
    {
        int fd = mkstemp(roster_save_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary roster save path\n");
            return 1;
        }
        close(fd);
        snprintf(roster_save_path, sizeof(roster_save_path), "%s.sav",
                 roster_save_tmpl);
        remove(roster_save_tmpl);
    }
    {
        int fd = mkstemp(csbwin_save_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary CSBWin save path\n");
            return 1;
        }
        close(fd);
        snprintf(csbwin_save_path, sizeof(csbwin_save_path), "%s.sav",
                 csbwin_save_tmpl);
        remove(csbwin_save_tmpl);
    }
#endif

    memset(&expected, 0, sizeof(expected));
    expect_true(build_runtime_resume_save(data_dir, save_path, &expected),
                "built CSB runtime save fixture from verified assets");

    fill_csb_launch_spec(&spec, data_dir, save_path);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB verified-profile resume start succeeds");
    expect_true(view.active == 1, "M11 CSB view is active");
    expect_true(view.startedFromLauncher == 1, "M11 marks CSB launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_CSB_BOOT,
                "M11 source kind is CSB boot");
    expect_true(strcmp(view.sourceId, "csb") == 0,
                "M11 source id is csb");
    expect_true(view.csbBootProfile != NULL,
                "M11 stores a CSB boot profile");
    expect_true(view.csbState.level_loaded == 1,
                "M11 CSB mirror state reports level loaded");
    expect_true(view.csbState.party_x == expected.party_x &&
                view.csbState.party_y == expected.party_y &&
                view.csbState.party_dir == expected.party_dir,
                "M11 CSB mirror state follows resumed party pose");
    expect_true(view.csbState.tick_count == (int)expected.tick_count,
                "M11 CSB mirror state follows resumed tick count");

    profile = (CSB_V1_BootProfile*)view.csbBootProfile;
    if (profile) {
        expect_true(profile->assets_verified == 1,
                    "CSB boot profile remains hash verified");
        expect_true(strcmp(profile->game_id, "csb") == 0,
                    "CSB boot profile game id is csb");
        expect_true(profile->csbgraphics_scan_attempted == 1,
                    "CSB boot profile attempts CSBgraphics startup scan");
        expect_true(csb_v1_boot_csbgraphics_cache(profile) ==
                        &profile->csbgraphics_cache &&
                    csb_v1_boot_csbgraphics_m11_plan(profile) ==
                        &profile->csbgraphics_m11_plan,
                    "CSB boot profile owns CSBgraphics cache and M11 plan");
        expect_true(profile->csbgraphics_scan_result ==
                        CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND ||
                    profile->csbgraphics_scan_result ==
                        CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
                    "CSBgraphics startup scan is skip-safe or loaded");
        expect_true(profile->runtime.party_x == expected.party_x &&
                    profile->runtime.party_y == expected.party_y &&
                    profile->runtime.party_dir == expected.party_dir,
                    "CSB runtime restored party pose from savePath");
        expect_true(profile->runtime.magic_caster_index ==
                    expected.magic_caster_index,
                    "CSB runtime restored magic caster from savePath");
        expect_true(profile->runtime.game_time == expected.game_time,
                    "CSB runtime restored game time from savePath");
    }

    {
        unsigned char fb[320 * 200];
        unsigned char expected_fb[320 * 200];
        unsigned char override_pixels[8 * 8];
        memset(fb, 0, sizeof(fb));
        if (profile) {
            render_expected_csb_viewport(&profile->runtime, expected_fb);
        } else {
            memset(expected_fb, 0, sizeof(expected_fb));
        }
        M11_GameView_Draw(&view, fb, 320, 200);
        expect_true(count_diff_rect(expected_fb, fb, 320, 0, 33, 224, 136) == 0,
                    "M11 CSB draw matches the direct source viewport frame");
        expect_true(count_nonzero_rect(fb, 320, 18, 18, 160, 12) == 0,
                    "M11 CSB draw no longer uses the boot handoff text path");
        if (profile) {
            expect_true(inject_synthetic_csbgraphics_viewport_override(
                            profile, override_pixels),
                        "test injects a planned CSBgraphics viewport override");
            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(&view, fb, 320, 200);
            expect_true(fb[33 * 320] == override_pixels[0],
                        "M11 CSB draw applies ready CSBgraphics plan entries");
            expect_true(fb[32 * 320] == 0,
                        "M11 CSBgraphics draw preserves pixels outside route");
        }
    }

    if (profile) {
        int old_dir = view.csbState.party_dir & 3;
        int target_dir = (old_dir + 1) & 3;
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_TURN_RIGHT) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB input dispatches turn-right through the CSB bridge");
        expect_true(view.csbState.party_dir == target_dir &&
                    profile->runtime.party_dir == target_dir &&
                    profile->runtime.party_state.PartyDirection == target_dir,
                    "M11 CSB turn-right updates mirrored and runtime party direction");
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_UP) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB movement input reaches the source command bridge");
        expect_true(view.csbState.party_x == profile->runtime.party_x &&
                    view.csbState.party_y == profile->runtime.party_y,
                    "M11 CSB movement input keeps state mirrors aligned");
    }

    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "CSB M11 idle tick dispatches through the CSB runtime boundary");
    expect_true(view.csbState.tick_count == (int)expected.tick_count + 1,
                "CSB M11 mirror tick advances once");

    expect_true(test_setenv("FIRESTAFF_QUICKSAVE_PATH", quick_save_path) == 0,
                "test fixture sets explicit CSB quicksave path");
    expect_true(M11_GameView_QuickSave(&view),
                "M11 CSB quicksave writes a CSB runtime save");
    memset(&quick_loaded, 0, sizeof(quick_loaded));
    csb_v1_runtime_init(&quick_loaded, NULL);
    expect_true(csb_v1_runtime_load_game_from_path(&quick_loaded,
                                                   quick_save_path) ==
                    CSB_V1_LOAD_OK,
                "M11 CSB quicksave reloads through the CSB runtime loader");
    expect_true(quick_loaded.party_x == view.csbState.party_x &&
                quick_loaded.party_y == view.csbState.party_y &&
                quick_loaded.party_dir == view.csbState.party_dir,
                "M11 CSB quicksave preserves mirrored party pose");
    expect_true(quick_loaded.tick_count == (uint32_t)view.csbState.tick_count,
                "M11 CSB quicksave preserves mirrored tick count");
    csb_v1_runtime_cleanup(&quick_loaded);
    if (profile) {
        int saved_x = view.csbState.party_x;
        int saved_y = view.csbState.party_y;
        int saved_dir = view.csbState.party_dir;
        int saved_tick = view.csbState.tick_count;
        profile->runtime.party_x = saved_x + 1;
        profile->runtime.party_y = saved_y + 1;
        profile->runtime.party_dir = (saved_dir + 1) & 3;
        profile->runtime.tick_count += 9U;
        profile->runtime.game_time += 9U;
        view.csbState.party_x = profile->runtime.party_x;
        view.csbState.party_y = profile->runtime.party_y;
        view.csbState.party_dir = profile->runtime.party_dir;
        view.csbState.tick_count = (int)profile->runtime.tick_count;
        expect_true(M11_GameView_QuickLoad(&view),
                    "M11 CSB quickload restores the CSB runtime save");
        expect_true(view.csbState.party_x == saved_x &&
                    view.csbState.party_y == saved_y &&
                    view.csbState.party_dir == saved_dir,
                    "M11 CSB quickload restores saved party pose");
        expect_true(view.csbState.tick_count == saved_tick,
                    "M11 CSB quickload restores saved tick count");
    }

    M11_GameView_Shutdown(&view);
    expect_true(view.csbBootProfile == NULL,
                "M11 shutdown clears CSB boot ownership");

    expect_true(firestaff_test_write_csbwin_resume_fixture(csbwin_save_path, 0),
                "built verified CSBWin resume save fixture");
    fill_csb_launch_spec(&spec, data_dir, csbwin_save_path);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB CSBWin verified-body resume start succeeds");
    expect_true(view.active == 1,
                "M11 CSB CSBWin resume leaves view active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_CSB_BOOT,
                "M11 CSBWin resume source kind is CSB boot");
    expect_true(view.csbState.party_x == 12 &&
                view.csbState.party_y == 7 &&
                view.csbState.party_dir == 3,
                "M11 CSBWin resume mirrors GAMEBLOCK2 party pose");
    expect_true(M11_GameView_GetV1LeaderHandThing(&view) == 0x4321u,
                "M11 CSBWin resume mirrors object-in-hand into leader hand");
    profile = (CSB_V1_BootProfile*)view.csbBootProfile;
    if (profile) {
        expect_true(profile->runtime.game_time == 0x01020304u,
                    "M11 CSBWin resume applies GAMEBLOCK2 game time");
        expect_true(profile->runtime.party_state_valid == 1 &&
                    profile->runtime.party_state.ChampionCount == 2 &&
                    strcmp(profile->runtime.party_state.Champions[0].Name,
                           "TIGGY") == 0 &&
                    strcmp(profile->runtime.party_state.Champions[1].Name,
                           "BORIS") == 0,
                    "M11 CSBWin resume applies champion summaries");
        expect_true(profile->runtime.csbwin_runtime_item16_count == 2u,
                    "M11 CSBWin resume materializes ITEM16 summaries");
        expect_true(profile->runtime.timeline_queue.eventCount == 3,
                    "M11 CSBWin resume materializes timer queue");
    }
    expect_true(test_setenv("FIRESTAFF_QUICKSAVE_PATH", csbwin_save_path) == 0,
                "test fixture points F9 at CSBWin resume save");
    M11_GameView_ClearV1LeaderHandObject(&view);
    expect_true(M11_GameView_GetV1LeaderHandThing(&view) == THING_NONE,
                "test clears CSBWin mirrored leader hand before F9");
    if (profile) {
        profile->runtime.csbwin_gameblock2_summary_valid = 0;
        profile->runtime.csbwin_object_in_hand = THING_NONE;
    }
    expect_true(M11_GameView_QuickLoad(&view),
                "M11 CSB F9 quickload accepts verified CSBWin save");
    expect_true(M11_GameView_GetV1LeaderHandThing(&view) == 0x4321u,
                "M11 CSB F9 quickload restores CSBWin object-in-hand");
    M11_GameView_Shutdown(&view);
    expect_true(view.csbBootProfile == NULL,
                "M11 shutdown clears CSBWin resume boot ownership");

    expect_true(firestaff_test_write_csbwin_resume_fixture(csbwin_save_path, 1),
                "built corrupt CSBWin resume save fixture");
    fill_csb_launch_spec(&spec, data_dir, csbwin_save_path);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "M11 CSB corrupt CSBWin resume start fails closed");
    expect_true(view.active == 0,
                "M11 corrupt CSBWin resume leaves view inactive");
    expect_true(view.csbBootProfile == NULL,
                "M11 corrupt CSBWin resume releases boot profile");
    M11_GameView_Shutdown(&view);

    expect_true(write_raw_csbgame_roster_save(roster_save_path),
                "built raw CSBGAME roster save fixture");
    fill_csb_launch_spec(&spec, data_dir, roster_save_path);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB raw CSBGAME roster resume start succeeds");
    expect_true(view.active == 1,
                "M11 CSB raw CSBGAME roster resume leaves view active");
    expect_true(view.csbState.party_x == CSB_V1_START_PARTY_X &&
                view.csbState.party_y == CSB_V1_START_PARTY_Y &&
                view.csbState.party_dir == CSB_V1_START_PARTY_DIR,
                "M11 CSB raw CSBGAME roster resume preserves boot pose");
    profile = (CSB_V1_BootProfile*)view.csbBootProfile;
    if (profile) {
        expect_true(profile->runtime.champion_count == 2,
                    "M11 CSB raw CSBGAME roster resume imports champion count");
        expect_true(profile->runtime.party_state.ImportSource ==
                    CSB_SAVE_IMPORT_SOURCE,
                    "M11 CSB raw CSBGAME roster resume stamps import source");
        expect_true(strcmp(profile->runtime.party_state.Champions[0].Name,
                           "ROSTERA") == 0 &&
                    strcmp(profile->runtime.party_state.Champions[1].Name,
                           "ROSTERB") == 0,
                    "M11 CSB raw CSBGAME roster resume imports champion names");
        expect_true(profile->runtime.party_state.PartyMapX ==
                    profile->runtime.party_x &&
                    profile->runtime.party_state.PartyMapY ==
                    profile->runtime.party_y &&
                    profile->runtime.party_state.PartyDirection ==
                    profile->runtime.party_dir,
                    "M11 CSB raw CSBGAME roster resume reanchors imported party");
    }
    M11_GameView_Shutdown(&view);
    expect_true(view.csbBootProfile == NULL,
                "M11 shutdown clears raw CSBGAME roster boot ownership");

    remove(save_path);
    remove(quick_save_path);
    remove(roster_save_path);
    remove(csbwin_save_path);

    if (g_failures) {
        fprintf(stderr, "CSB V1 M11 startup/resume gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: CSB V1 M11 startup/resume gate");
    return 0;
}
