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
#include "csb_v1_utility_flow_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"
#include "entrance_mouse_routes_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "main_loop_m11.h"
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

static void drive_csb_entrance_opening(M11_GameViewState *view,
                                       const char *message)
{
    unsigned int i;
    unsigned int ticks =
        20u + ENTRANCE_Compat_GetDoorAnimationStepCount();
    int tick_before;
    if (!view) {
        expect_true(0, message);
        return;
    }
    tick_before = view->csbState.tick_count;
    expect_true(view->csbState.startup_entrance_active == 1 &&
                    view->csbState.startup_entrance_opening_active == 1 &&
                    view->csbState.startup_entrance_opening_delay_ticks == 20,
                "M11 CSB entrance command starts source door-opening phase");
    for (i = 0; i < ticks; ++i) {
        expect_true(M11_GameView_AdvanceIdleTick(view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB entrance door-opening tick redraws");
    }
    expect_true(view->csbState.tick_count == tick_before,
                "M11 CSB entrance door-opening blocks runtime tick aging");
    expect_true(view->csbState.startup_entrance_active == 0 &&
                    view->csbState.startup_entrance_dismissed == 1 &&
                    view->csbState.startup_entrance_opening_active == 0,
                message);
}

static void drive_csb_entrance_to_wait(M11_GameViewState *view,
                                       const char *message)
{
    int guard = 96;
    if (!view) {
        expect_true(0, message);
        return;
    }
    while (guard-- > 0 && view->csbState.startup_entrance_source_step < 4) {
        int tick_before = view->csbState.tick_count;
        expect_true(M11_GameView_AdvanceIdleTick(view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB title/entrance source prelude redraws");
        expect_true(view->csbState.tick_count == tick_before,
                    "M11 CSB title/entrance source prelude blocks runtime tick aging");
    }
    expect_true(view->csbState.startup_entrance_active == 1 &&
                    view->csbState.startup_entrance_source_step == 4,
                message);
}

static void write_be16(unsigned char *buf, size_t off, unsigned short value) {
    buf[off] = (unsigned char)((value >> 8) & 0xffu);
    buf[off + 1u] = (unsigned char)(value & 0xffu);
}

static void write_le16(unsigned char *buf, size_t off, unsigned short value) {
    buf[off] = (unsigned char)(value & 0xffu);
    buf[off + 1u] = (unsigned char)((value >> 8) & 0xffu);
}

static void write_le32(unsigned char *buf, size_t off, unsigned int value) {
    buf[off] = (unsigned char)(value & 0xffu);
    buf[off + 1u] = (unsigned char)((value >> 8) & 0xffu);
    buf[off + 2u] = (unsigned char)((value >> 16) & 0xffu);
    buf[off + 3u] = (unsigned char)((value >> 24) & 0xffu);
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

typedef struct {
    unsigned short entry_index;
    const unsigned char *decoded;
    size_t decoded_size;
} CsbGraphicsEntryFixture;

static unsigned char *build_csbgraphics_entries_compressed(
    const CsbGraphicsEntryFixture *entries,
    size_t entry_count,
    size_t *out_size) {
    unsigned short max_entry = 0u;
    unsigned int count;
    size_t header_size;
    size_t payload_size = 0u;
    size_t payload_cursor;
    size_t i;
    unsigned char **compressed = NULL;
    size_t *compressed_sizes = NULL;
    unsigned char *buf = NULL;

    if (!entries || entry_count == 0u || !out_size) {
        return NULL;
    }
    for (i = 0u; i < entry_count; ++i) {
        if (entries[i].entry_index > max_entry) {
            max_entry = entries[i].entry_index;
        }
    }
    compressed = (unsigned char**)calloc(entry_count, sizeof(compressed[0]));
    compressed_sizes = (size_t*)calloc(entry_count, sizeof(compressed_sizes[0]));
    if (!compressed || !compressed_sizes) {
        free(compressed);
        free(compressed_sizes);
        return NULL;
    }
    for (i = 0u; i < entry_count; ++i) {
        if (!entries[i].decoded || entries[i].decoded_size == 0u ||
            entries[i].decoded_size > 65535u ||
            test_lzw_encode(entries[i].decoded,
                            entries[i].decoded_size,
                            &compressed[i],
                            &compressed_sizes[i]) != 0 ||
            !compressed[i] ||
            compressed_sizes[i] == 0u ||
            compressed_sizes[i] > 65535u) {
            goto cleanup;
        }
        payload_size += compressed_sizes[i];
    }

    count = (unsigned int)max_entry + 1u;
    header_size = 2u + (size_t)count * 4u;
    buf = (unsigned char*)calloc(1u, header_size + payload_size);
    if (!buf) {
        goto cleanup;
    }
    write_be16(buf, 0u, (unsigned short)count);
    for (i = 0u; i < entry_count; ++i) {
        unsigned short id = entries[i].entry_index;
        write_be16(buf, 2u + (size_t)id * 2u,
                   (unsigned short)compressed_sizes[i]);
        write_be16(buf, 2u + (size_t)count * 2u + (size_t)id * 2u,
                   (unsigned short)entries[i].decoded_size);
    }
    payload_cursor = header_size;
    for (unsigned int id = 0u; id < count; ++id) {
        for (i = 0u; i < entry_count; ++i) {
            if (entries[i].entry_index == id) {
                memcpy(buf + payload_cursor, compressed[i], compressed_sizes[i]);
                payload_cursor += compressed_sizes[i];
            }
        }
    }
    *out_size = header_size + payload_size;

cleanup:
    if (compressed) {
        for (i = 0u; i < entry_count; ++i) {
            free(compressed[i]);
        }
    }
    free(compressed);
    free(compressed_sizes);
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

static int count_color_rect(const unsigned char* fb,
                            int stride,
                            int x,
                            int y,
                            int w,
                            int h,
                            unsigned char color) {
    int count = 0;
    int px;
    int py;
    if (!fb || stride <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (py = y; py < y + h; ++py) {
        for (px = x; px < x + w; ++px) {
            if (fb[py * stride + px] == color) {
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

static int count_nonzero_slot_pixels(const M11_AssetSlot* slot) {
    int count = 0;
    size_t i;
    size_t total;
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        return 0;
    }
    total = (size_t)slot->width * (size_t)slot->height;
    for (i = 0u; i < total; ++i) {
        if (slot->pixels[i] != 0u) {
            ++count;
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

static int inject_synthetic_csbgraphics_custom_background(
    CSB_V1_BootProfile *profile) {
    unsigned char skin_def_decoded[42u];
    unsigned char bitmap_decoded[12u];
    unsigned char mask_decoded[86u];
    CsbGraphicsEntryFixture entries[3];
    unsigned char *bytes;
    size_t size = 0u;
    const CSB_V1_DungeonData *dungeon;
    int width;
    int height;
    int level;
    int x;
    int y;

    if (!profile || !profile->runtime.dungeon_handle) {
        return 0;
    }
    dungeon = profile->runtime.dungeon_handle;
    level = profile->runtime.current_level;
    if (level < 0 || level >= dungeon->level_count) {
        return 0;
    }
    width = dungeon->level_widths[level];
    height = dungeon->level_heights[level];
    if (width <= 0 || height <= 0) {
        return 0;
    }

    memset(skin_def_decoded, 0, sizeof(skin_def_decoded));
    write_le16(skin_def_decoded, 0u, 2u);
    write_le16(skin_def_decoded, 2u, 6u);
    write_le16(skin_def_decoded, 4u, 24u);
    write_le16(skin_def_decoded, 6u, 100u);
    write_le16(skin_def_decoded, 14u, 104u);
    write_le16(skin_def_decoded, 24u, 100u);
    write_le16(skin_def_decoded, 32u, 104u);

    memset(bitmap_decoded, 0, sizeof(bitmap_decoded));
    write_le32(bitmap_decoded, 0u, 16u);
    write_le32(bitmap_decoded, 4u, 0x11223344u);
    write_le32(bitmap_decoded, 8u, 0x55667788u);

    memset(mask_decoded, 0, sizeof(mask_decoded));
    write_le32(mask_decoded, 0u, 16u);
    for (x = 0; x < 16; ++x) {
        write_le32(mask_decoded, 4u + (size_t)x * 4u, 68u);
    }
    write_le16(mask_decoded, 68u, 0u);
    write_le16(mask_decoded, 70u, 0u);
    write_le16(mask_decoded, 72u, 0u);
    write_le16(mask_decoded, 74u, 0u);
    write_le16(mask_decoded, 76u, 16u);
    write_le16(mask_decoded, 78u, 1u);
    write_le16(mask_decoded, 80u, 0xffffu);

    entries[0].entry_index = 1u;
    entries[0].decoded = skin_def_decoded;
    entries[0].decoded_size = sizeof(skin_def_decoded);
    entries[1].entry_index = 100u;
    entries[1].decoded = bitmap_decoded;
    entries[1].decoded_size = sizeof(bitmap_decoded);
    entries[2].entry_index = 104u;
    entries[2].decoded = mask_decoded;
    entries[2].decoded_size = sizeof(mask_decoded);
    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
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
             "00000000000000000000000000000001");
    snprintf(profile->csbgraphics_cache.matched_label,
             sizeof(profile->csbgraphics_cache.matched_label),
             "synthetic-m11-custom-background");
    if (csb_v1_csbgraphics_dat_classify(
            profile->csbgraphics_cache.file_buffer,
            profile->csbgraphics_cache.file_size,
            &profile->csbgraphics_cache.index) !=
        CSB_V1_CSBGRAPHICS_CLASSIFY_OK) {
        return 0;
    }
    profile->csbgraphics_scan_attempted = 1;
    profile->csbgraphics_scan_result = CSB_V1_CSBGRAPHICS_DAT_REAL_OK;
    profile->csbgraphics_skin_def_loaded = 1;
    profile->csbgraphics_skin_def_word_count = 7u;
    memset(profile->csbgraphics_skin_def_words, 0,
           sizeof(profile->csbgraphics_skin_def_words));
    profile->csbgraphics_skin_def_words[0] = 100u;
    profile->csbgraphics_skin_def_words[4] = 104u;
    profile->csbgraphics_plan_result =
        csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
            &profile->csbgraphics_cache,
            profile->csbgraphics_skin_def_words,
            profile->csbgraphics_skin_def_word_count,
            &profile->csbgraphics_m11_plan);
    if (profile->csbgraphics_plan_result !=
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
        return 0;
    }

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            (void)csb_v1_skin_cache_set_skin(&profile->runtime.skin_cache,
                                             level,
                                             width,
                                             height,
                                             x,
                                             y,
                                             1u);
        }
    }
    return 1;
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

static void force_csb_menu_available(M12_StartupMenuState* state) {
    if (!state) return;
    state->entries[1].title = "CHAOS STRIKES BACK";
    state->entries[1].gameId = "csb";
    state->entries[1].kind = M12_MENU_ENTRY_GAME;
    state->entries[1].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[1].available = 1;
    state->assetStatus.originalFileCandidateFound = 1;
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.versions[1][0].gameId = "csb";
    state->assetStatus.versions[1][0].versionId = "pc34-en";
    state->assetStatus.versions[1][0].label = "PC 3.4 English";
    state->assetStatus.versions[1][0].shortLabel = "PC34 EN";
    state->assetStatus.versions[1][0].matched = 1;
    state->assetStatus.requiredFileCounts[1] = 2;
    state->assetStatus.requiredFiles[1][0].gameId = "csb";
    state->assetStatus.requiredFiles[1][0].roleId = "graphics";
    state->assetStatus.requiredFiles[1][0].label = "GRAPHICS.DAT";
    state->assetStatus.requiredFiles[1][0].required = 1;
    state->assetStatus.requiredFiles[1][0].matched = 1;
    state->assetStatus.requiredFiles[1][1].gameId = "csb";
    state->assetStatus.requiredFiles[1][1].roleId = "dungeon";
    state->assetStatus.requiredFiles[1][1].label = "DUNGEON.DAT";
    state->assetStatus.requiredFiles[1][1].required = 1;
    state->assetStatus.requiredFiles[1][1].matched = 1;
    state->gameOptions[1].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->view = M12_MENU_VIEW_MAIN;
    state->selectedIndex = 1;
    state->activatedIndex = 1;
    state->launchRequested = 1;
    state->quickResumeLaunchRequested = 0;
}

static int write_synthetic_dm1_import_save(const char *path,
                                           int champion_count) {
    unsigned char buf[1024];
    FILE *f;
    size_t n;
    int i;

    if (!path || champion_count < 1 || champion_count > CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = (unsigned char)champion_count;
    for (i = 0; i < champion_count; ++i) {
        size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START +
                     (size_t)i * (size_t)CSB_V1_DM1_CHAMP_SIZE;
        size_t equip_off = off + (size_t)CSB_V1_DM1_CHAMP_OFF_EQUIP;
        int slot;
        memcpy(buf + off + CSB_V1_DM1_CHAMP_OFF_NAME,
               i == 0 ? "ALPHA   " : "BETA    ",
               8u);
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_HEALTH,
                   (unsigned short)(80 + i));
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH,
                   (unsigned short)(100 + i));
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_STAMINA,
                   (unsigned short)(60 + i));
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA,
                   (unsigned short)(100 + i));
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MANA,
                   (unsigned short)(30 + i));
        write_le16(buf, off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA,
                   (unsigned short)(50 + i));
        buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = (unsigned char)(55 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = (unsigned char)(66 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = (unsigned char)(77 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = (unsigned char)(88 + i);
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; ++slot) {
            write_le16(buf,
                       equip_off + (size_t)slot * 2u,
                       0xffffu);
        }
    }
    f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    n = fwrite(buf, 1u, sizeof(buf), f);
    fclose(f);
    return n == sizeof(buf);
}

static void assert_csb_view_matches_expected_resume(
    const M11_GameViewState* view,
    const CSB_V1_RuntimeProfile* expected,
    const char* label) {
    expect_true(view->csbState.party_x == expected->party_x &&
                    view->csbState.party_y == expected->party_y &&
                    view->csbState.party_dir == expected->party_dir,
                label);
    expect_true(view->csbState.current_level == expected->current_level,
                "M11 CSB mirror state follows resumed current level");
    expect_true(view->csbState.tick_count == (int)expected->tick_count,
                "M11 CSB mirror state follows resumed tick count");
    expect_true(view->world.party.championCount ==
                    expected->party_state.ChampionCount,
                "M11 CSB party mirror exposes imported champion count");
    expect_true(view->world.party.activeChampionIndex ==
                    expected->leader_index,
                "M11 CSB party mirror exposes runtime leader");
    expect_true(view->world.party.mapX == expected->party_x &&
                    view->world.party.mapY == expected->party_y &&
                    view->world.party.direction == expected->party_dir,
                "M11 CSB party mirror follows runtime map pose");
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
    boot.runtime.party_state.Champions[0].CurrentStamina = 80;
    boot.runtime.party_state.Champions[0].MaximumStamina = 90;
    boot.runtime.party_state.Champions[0].CurrentMana = 12;
    boot.runtime.party_state.Champions[0].MaximumMana = 20;
    boot.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_STR]
                                                [CSB_V1_STAT_CUR] = 42;
    boot.runtime.party_state.Champions[0].Statistics[CSB_V1_STAT_STR]
                                                [CSB_V1_STAT_MAX] = 48;
    boot.runtime.party_state.Champions[0].Skills[0] = 3;
    for (int i = 0; i < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++i) {
        boot.runtime.party_state.Champions[0].Portrait[i] =
            (uint8_t)((i * 7 + 0x23) & 0xffu);
    }
    boot.runtime.party_state.Champions[1].CurrentHealth = 100;
    boot.runtime.party_state.Champions[1].MaximumHealth = 100;
    boot.runtime.party_state.Champions[1].CurrentStamina = 70;
    boot.runtime.party_state.Champions[1].MaximumStamina = 75;
    boot.runtime.party_state.Champions[1].Portrait[70 * 8] = 0x03u;
    boot.runtime.party_state.Champions[1].Portrait[70 * 8 + 5] = 0x0cu;
    boot.runtime.party_state.Champions[1].Portrait[231 * 8 + 2] = 0x07u;
    boot.runtime.party_state.Champions[1].Portrait[231 * 8 + 7] = 0x09u;
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
    char dm1_import_tmpl[] = "/tmp/firestaff_csb_m11_dm1import_XXXXXX";
    char save_path[560];
    char quick_save_path[560];
    char roster_save_path[560];
    char csbwin_save_path[560];
    char dm1_import_path[560];
    const char* data_dir = csb_data_dir(fallback);
    CSB_V1_BootProfile preflight;
    CSB_V1_RuntimeProfile expected;
    CSB_V1_RuntimeProfile quick_loaded;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    CSB_V1_BootProfile* profile;

    expect_true(csb_v1_startup_sequence_source_order_valid_pc34(),
                "CSB startup source-order contract is valid");
    expect_true(strstr(csb_v1_startup_sequence_source_evidence_pc34(),
                       "TITLE.C F0437") != NULL &&
                    strstr(csb_v1_startup_sequence_source_evidence_pc34(),
                           "ENTRANCE.C F0438") != NULL,
                "CSB startup evidence names title and entrance sources");
    expect_true(csb_v1_startup_title_total_ticks_pc34() == 53,
                "CSB startup title timing keeps the bounded 53-tick prelude");
    expect_true(csb_v1_startup_title_presents_ticks_pc34() == 30,
                "CSB startup title timing keeps PRESENTS for 30 ticks");
    expect_true(csb_v1_startup_title_stage_for_frame_pc34(0) ==
                    CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
                    csb_v1_startup_title_stage_for_frame_pc34(
                        csb_v1_startup_title_presents_ticks_pc34() + 1) ==
                        CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
                "CSB startup title helper exposes PRESENTS then CHAOS zoom");
    expect_true(csb_v1_startup_entrance_wait_stage_pc34() == 4 &&
                    csb_v1_startup_entrance_pre_open_delay_ticks_pc34() == 20,
                "CSB startup entrance helper exposes wait step and pre-open delay");
    expect_true(csb_v1_startup_entrance_action_for_input_pc34(
                    0,
                    CSB_V1_STARTUP_INPUT_ACCEPT_PC34) ==
                    CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34 &&
                    csb_v1_startup_entrance_action_for_input_pc34(
                        0,
                        CSB_V1_STARTUP_INPUT_ACTION_PC34) ==
                        CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34,
                "CSB startup entrance input maps Accept/Action to dungeon entry");
    expect_true(csb_v1_startup_entrance_action_for_input_pc34(
                    0,
                    CSB_V1_STARTUP_INPUT_DISK_MENU_PC34) ==
                    CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34 &&
                    csb_v1_startup_entrance_action_for_input_pc34(
                        0,
                        CSB_V1_STARTUP_INPUT_BACK_PC34) ==
                        CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34,
                "CSB startup entrance input maps Disk/Back to resume/quit");
    expect_true(csb_v1_startup_entrance_action_for_input_pc34(
                    1,
                    CSB_V1_STARTUP_INPUT_BACK_PC34) ==
                    CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34 &&
                    csb_v1_startup_entrance_action_for_input_pc34(
                        1,
                        CSB_V1_STARTUP_INPUT_ACCEPT_PC34) ==
                        CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34,
                "CSB startup entrance input dismisses credits before commands");

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
    snprintf(dm1_import_path, sizeof(dm1_import_path),
             ".\\firestaff-csb-m11-dm1import.sav");
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
    {
        int fd = mkstemp(dm1_import_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary DM1 import save path\n");
            return 1;
        }
        close(fd);
        snprintf(dm1_import_path, sizeof(dm1_import_path), "%s.sav",
                 dm1_import_tmpl);
        remove(dm1_import_tmpl);
    }
#endif

    memset(&expected, 0, sizeof(expected));
    expect_true(build_runtime_resume_save(data_dir, save_path, &expected),
                "built CSB runtime save fixture from verified assets");
    expect_true(write_synthetic_dm1_import_save(dm1_import_path, 2),
                "built synthetic DM1 save for CSB startup import");

    fill_csb_launch_spec(&spec, data_dir, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB new-game start succeeds");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                view.csbState.startup_entrance_dismissed == 0,
                "M11 CSB new-game start opens source-locked title/entrance");
    expect_true(view.csbState.startup_title_active == 1 &&
                    view.csbState.startup_title_source_step == 1 &&
                    view.csbState.startup_entrance_source_step == 0,
                "M11 CSB new-game start begins at source title prelude");
    expect_true(view.csbState.level_loaded == 1,
                "M11 CSB entrance keeps runtime loaded behind startup screen");
    {
        int tick_before = view.csbState.tick_count;
        unsigned char fb[320 * 200];
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&view, fb, 320, 200);
        expect_true(count_nonzero_rect(fb, 320, 18, 18, 284, 164) > 0,
                    "M11 CSB entrance draws a visible startup screen");
        expect_true(count_color_rect(fb, 320, 18, 18, 284, 1, 11u) < 200,
                    "M11 CSB real entrance does not keep the synthetic debug frame");
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB entrance advances its presentation frame");
        expect_true(view.csbState.tick_count == tick_before &&
                    view.csbState.startup_entrance_frame > 0,
                    "M11 CSB entrance does not tick runtime before confirm");
        expect_true(view.csbState.startup_title_active == 1 &&
                        view.csbState.startup_title_source_step == 1 &&
                        view.csbState.startup_entrance_source_step == 0,
                    "M11 CSB title prelude holds PRESENTS before entrance");
        for (int i = 0; i < 30 && view.csbState.startup_title_active; ++i) {
            expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 CSB title prelude zoom warmup redraws");
            expect_true(view.csbState.tick_count == tick_before,
                        "M11 CSB title prelude zoom warmup blocks runtime ticks");
        }
        expect_true(view.csbState.startup_title_active == 1 &&
                        view.csbState.startup_title_source_step == 2 &&
                        view.csbState.startup_entrance_source_step == 0,
                    "M11 CSB title prelude reaches CHAOS zoom before entrance");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        250,
                        188,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_IGNORED,
                    "M11 CSB title/entrance ignores pointer commands before source wait loop");
        drive_csb_entrance_to_wait(
            &view,
            "M11 CSB entrance reaches source wait loop before command input");
        expect_true(view.csbState.startup_title_active == 0 &&
                        view.csbState.startup_title_source_step == 0,
                    "M11 CSB title prelude completes before entrance command input");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        250,
                        188,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_credits_active == 1 &&
                    view.csbState.startup_entrance_credits_remaining_ticks ==
                        1800 &&
                    view.csbState.startup_entrance_last_command ==
                        M11_ENTRANCE_RUNTIME_COMMAND_DRAW_CREDITS,
                    "M11 CSB entrance credits button opens the startup credits phase");
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&view, fb, 320, 200);
        expect_true(count_nonzero_rect(fb, 320, 0, 0, 320, 200) > 0,
                    "M11 CSB entrance credits phase draws a visible screen");
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_ACCEPT) ==
                        M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_credits_active == 0,
                    "M11 CSB entrance accepts Enter/Action to leave credits");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        250,
                        188,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_credits_active == 1,
                    "M11 CSB entrance can reopen credits after returning");
        for (int i = 0; i < 1799; ++i) {
            expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 CSB credits idle countdown redraws");
        }
        expect_true(view.csbState.tick_count == tick_before &&
                        view.csbState.startup_entrance_credits_active == 1 &&
                        view.csbState.startup_entrance_credits_remaining_ticks == 1,
                    "M11 CSB credits countdown does not tick runtime before timeout");
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_credits_active == 0 &&
                    view.csbState.startup_entrance_credits_remaining_ticks == 0 &&
                    view.csbState.tick_count == tick_before,
                    "M11 CSB credits timeout returns to entrance without runtime tick");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        245,
                        80,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_resume_available == 0 &&
                    view.csbState.startup_entrance_last_command ==
                        M11_ENTRANCE_RUNTIME_COMMAND_RESUME,
                    "M11 CSB entrance resume without save stays on startup");
        expect_true(strcmp(view.lastOutcome, "CSB RESUME UNAVAILABLE") == 0,
                    "M11 CSB entrance resume without save reports unavailable status");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        245,
                        46,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB entrance enter button accepts source-locked pointer command");
        drive_csb_entrance_opening(&view,
                                   "M11 CSB entrance dismisses to dungeon runtime");
        expect_true(view.csbState.startup_entrance_last_command ==
                        M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON,
                    "M11 CSB entrance records the enter-dungeon command");
        expect_true(view.csbState.startup_entrance_bonus_requested == 0,
                    "M11 CSB normal enter does not mark bonus dungeon");
    }
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.csbImportDm1SavePath = dm1_import_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB startup accepts a DM1 utility import path");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_import_available == 1 &&
                    view.csbState.startup_import_champion_count == 2,
                "M11 CSB entrance keeps imported DM1 party ready before dungeon entry");
    expect_true(view.csbState.startup_import_utility_state ==
                    CSB_V1_UTIL_FLOW_DONE,
                "M11 CSB startup import completes the CSB utility flow");
    expect_true(strstr(view.csbState.startup_import_utility_prompt,
                       "CHAOS STRIKES BACK READY") != NULL,
                "M11 CSB startup import exposes the final utility prompt");
    expect_true(view.world.party.championCount == 2 &&
                    memcmp(view.world.party.champions[0].name,
                           "ALPHA   ", 8u) == 0 &&
                    memcmp(view.world.party.champions[1].name,
                           "BETA    ", 8u) == 0,
                "M11 CSB party mirror exposes utility-imported champion names");
    expect_true(view.csbBootProfile != NULL &&
                    ((CSB_V1_BootProfile *)view.csbBootProfile)
                        ->runtime.party_state.ImportedFromDM1 == 1,
                "M11 CSB runtime marks the startup party as imported from DM1");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB utility import reaches source wait loop before utility input");
    {
        unsigned char fb[320 * 200];
        memset(fb, 0, sizeof(fb));
        if (view.assetsAvailable) {
            M11_GameView_Draw(&view, fb, 320, 200);
            expect_true(count_color_rect(fb, 320, 38, 80, 230, 64, 15u) > 20,
                        "M11 CSB real entrance overlays the utility import prompt");
            expect_true(count_color_rect(fb, 320, 36, 104, 244, 12, 12u) > 20,
                        "M11 CSB real entrance overlays the selected utility import row");
            memset(fb, 0, sizeof(fb));
        }
        view.assetsAvailable = 0;
        M11_GameView_Draw(&view, fb, 320, 200);
        expect_true(count_color_rect(fb, 320, 38, 80, 230, 64, 15u) > 20,
                    "M11 CSB fallback entrance renders the utility import prompt");
        expect_true(count_color_rect(fb, 320, 36, 104, 244, 12, 12u) > 20,
                    "M11 CSB fallback entrance renders the selected utility import row");
    }
    expect_true(view.csbState.startup_import_selected_action_index == 0,
                "M11 CSB utility keyboard starts on the IMPORT row");
    {
        int last_command = view.csbState.startup_entrance_last_command;
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        40,
                        92,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.csbState.startup_entrance_active == 1 &&
                        view.csbState.startup_import_selected_action_index == 0 &&
                        view.csbState.startup_entrance_last_command == last_command,
                    "M11 CSB utility prompt click is consumed by utility panel");
    }
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_selected_action_index == 1,
                "M11 CSB utility keyboard DOWN selects LOAD");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_last_command ==
                        M11_ENTRANCE_RUNTIME_COMMAND_RESUME,
                "M11 CSB utility keyboard ACCEPT activates selected LOAD row");
    expect_true(strcmp(view.lastOutcome, "CSB RESUME UNAVAILABLE") == 0,
                "M11 CSB utility keyboard LOAD reports unavailable resume without save");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_selected_action_index == 0,
                "M11 CSB utility keyboard UP returns to IMPORT");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1,
                "M11 CSB utility keyboard ACCEPT on IMPORT stays on startup");
    expect_true(strcmp(view.lastOutcome, "CSB IMPORT READY") == 0,
                "M11 CSB utility keyboard IMPORT reports import-ready status");
    {
        int last_command = view.csbState.startup_entrance_last_command;
        int last_action = view.csbState.startup_import_selected_action_index;
        M11_GameInputResult result = M11_GameView_HandlePointerButton(
            &view,
            40,
            118,
            ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT);
        expect_true(result == M11_GAME_INPUT_IGNORED &&
                        view.csbState.startup_import_selected_action_index ==
                            last_action &&
                        view.csbState.startup_entrance_last_command ==
                            last_command,
                    "M11 CSB utility LOAD row ignores bonus-dungeon pointer mask");
    }
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    40,
                    106,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1,
                "M11 CSB utility IMPORT row stays on the startup menu");
    expect_true(strcmp(view.lastOutcome, "CSB IMPORT READY") == 0,
                "M11 CSB utility IMPORT row reports import-ready status");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    40,
                    142,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1,
                "M11 CSB utility VIEW row stays on the startup menu");
    expect_true(strcmp(view.lastOutcome, "CSB PARTY READY") == 0,
                "M11 CSB utility VIEW row reports party-ready status");
    expect_true(view.csbState.startup_import_preview_active == 1,
                "M11 CSB utility VIEW row enables the imported-party preview");
    {
        unsigned char fb[320 * 200];
        memset(fb, 0, sizeof(fb));
        view.assetsAvailable = 0;
        M11_GameView_Draw(&view, fb, 320, 200);
        expect_true(count_color_rect(fb, 320, 48, 164, 170, 10, 15u) > 20,
                    "M11 CSB utility VIEW preview renders the second imported champion row");
    }
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_import_preview_active == 0,
                "M11 CSB utility Back closes imported-party preview");
    expect_true(strcmp(view.lastOutcome, "CSB IMPORT READY") == 0,
                "M11 CSB utility Back returns to import-ready status");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    40,
                    142,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_preview_active == 1,
                "M11 CSB utility VIEW row reopens preview after Back");
    {
        int last_command = view.csbState.startup_entrance_last_command;
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        52,
                        164,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.csbState.startup_entrance_active == 1 &&
                        view.csbState.startup_import_preview_active == 1 &&
                        view.csbState.startup_entrance_last_command == last_command,
                    "M11 CSB utility preview click is consumed by utility panel");
    }
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_selected_action_index == 0 &&
                    view.csbState.startup_import_preview_active == 0,
                "M11 CSB utility cursor movement closes imported-party preview");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_selected_action_index == 1,
                "M11 CSB utility keyboard returns to LOAD after preview cursor close");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    40,
                    118,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_last_command ==
                        M11_ENTRANCE_RUNTIME_COMMAND_RESUME,
                "M11 CSB utility LOAD row routes through the resume command");
    expect_true(strcmp(view.lastOutcome, "CSB RESUME UNAVAILABLE") == 0,
                "M11 CSB utility LOAD row reports unavailable resume without save");
    expect_true(view.csbState.startup_import_preview_active == 0,
                "M11 CSB utility LOAD row clears the imported-party preview");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    40,
                    130,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB utility START NEW GAME row enters the dungeon");
    drive_csb_entrance_opening(
        &view,
        "M11 CSB imported-party utility start opens into runtime");
    expect_true(view.world.party.championCount == 2,
                "M11 CSB imported party survives the utility start handoff");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON,
                "M11 CSB utility START NEW GAME row records the enter-dungeon command");
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.csbImportDm1SavePath = dm1_import_path;
    spec.entranceResumeSavePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB utility start accepts import plus validated resume path");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_import_available == 1 &&
                    view.csbState.startup_entrance_resume_available == 1,
                "M11 CSB utility start keeps both import and resume available");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB utility resume reaches source wait loop before utility input");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_import_selected_action_index == 1,
                "M11 CSB utility keyboard selects LOAD when resume is available");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB utility LOAD row resumes a validated save");
    drive_csb_entrance_opening(&view,
                               "M11 CSB utility LOAD row opens into runtime");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_RESUME,
                "M11 CSB utility LOAD row records the resume command before loading");
    assert_csb_view_matches_expected_resume(
        &view,
        &expected,
        "M11 CSB utility LOAD row restores the validated resume pose");
    M11_GameView_Shutdown(&view);

    {
        M12_StartupMenuState menu;
        M12_StartupMenu_InitWithDataDir(&menu, data_dir, NULL);
        force_csb_menu_available(&menu);
        menu.csbImportDm1LaunchRequested = 1;
        snprintf(menu.csbImportDm1SavePath,
                 sizeof(menu.csbImportDm1SavePath),
                 "%s",
                 dm1_import_path);
        M11_GameView_Init(&view);
        expect_true(M11_GameView_OpenSelectedMenuEntry(&view, &menu),
                    "M11 CSB menu-entry launch accepts a DM1 utility import path");
        expect_true(view.csbState.startup_entrance_active == 1 &&
                        view.csbState.startup_import_available == 1 &&
                        view.csbState.startup_import_champion_count == 2,
                    "M11 CSB menu-entry launch forwards the DM1 import intent");
        expect_true(view.csbState.startup_import_utility_state ==
                        CSB_V1_UTIL_FLOW_DONE,
                    "M11 CSB menu-entry launch completes the CSB utility flow");
        drive_csb_entrance_to_wait(
            &view,
            "M11 CSB menu-entry import reaches source wait loop before utility input");
        expect_true(strstr(view.csbState.startup_import_utility_prompt,
                           "CHAOS STRIKES BACK READY") != NULL,
                    "M11 CSB menu-entry launch exposes the final utility prompt");
        expect_true(view.world.party.championCount == 2 &&
                        memcmp(view.world.party.champions[0].name,
                               "ALPHA   ", 8u) == 0 &&
                        memcmp(view.world.party.champions[1].name,
                               "BETA    ", 8u) == 0,
                    "M11 CSB menu-entry launch mirrors imported champion names");
        expect_true(view.csbBootProfile != NULL &&
                        ((CSB_V1_BootProfile *)view.csbBootProfile)
                            ->runtime.party_state.ImportedFromDM1 == 1,
                    "M11 CSB menu-entry launch marks imported runtime party");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        245,
                        46,
                        M11_DM1_MOUSE_MASK_LEFT) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB menu-entry imported-party launch enters the dungeon");
        drive_csb_entrance_opening(
            &view,
            "M11 CSB menu-entry imported-party launch opens into runtime");
        expect_true(view.world.party.championCount == 2,
                    "M11 CSB menu-entry imported party survives entrance handoff");
        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);
    }

    fill_csb_launch_spec(&spec, data_dir, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB quit-pointer start fixture succeeds");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB quit-pointer fixture reaches source wait loop before pointer input");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    245,
                    112,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_RETURN_TO_MENU,
                "M11 CSB entrance quit button returns to launcher");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_QUIT,
                "M11 CSB entrance records the source quit command");
    expect_true(view.csbState.startup_entrance_active == 0 &&
                    view.csbState.startup_entrance_dismissed == 1,
                "M11 CSB entrance quit button clears startup state");
    expect_true(strcmp(view.lastOutcome, "BACK TO LAUNCHER") == 0,
                "M11 CSB entrance quit button reports launcher return");
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB Back-key quit start fixture succeeds");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB Back-key fixture reaches source wait loop before input");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                    M11_GAME_INPUT_RETURN_TO_MENU,
                "M11 CSB entrance Back input returns to launcher");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_QUIT,
                "M11 CSB entrance Back input records the source quit command");
    expect_true(view.csbState.startup_entrance_active == 0 &&
                    view.csbState.startup_entrance_dismissed == 1,
                "M11 CSB entrance Back input clears startup state");
    expect_true(strcmp(view.lastOutcome, "BACK TO LAUNCHER") == 0,
                "M11 CSB entrance Back input reports launcher return");
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB bonus-dungeon start fixture succeeds");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB bonus-dungeon fixture reaches source wait loop before pointer input");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    245,
                    46,
                    ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB entrance accepts the source bonus-dungeon button mask");
    drive_csb_entrance_opening(&view,
                               "M11 CSB bonus-dungeon command dismisses to runtime");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_ENTER_BONUS_DUNGEON,
                "M11 CSB entrance records the bonus-dungeon command");
    expect_true(view.csbState.startup_entrance_bonus_requested == 1,
                "M11 CSB entrance preserves the bonus-dungeon request");
    expect_true(view.csbBootProfile != NULL &&
                    csb_v1_runtime_get_load_bonus_dungeon(
                        &((CSB_V1_BootProfile *)view.csbBootProfile)->runtime) == 1,
                "M11 CSB entrance hands C201 through to runtime bonus-dungeon flag");
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.entranceResumeSavePath = quick_save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB new-game start with invalid entrance resume path succeeds");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_resume_available == 0 &&
                    view.csbState.startup_entrance_resume_path[0] == '\0',
                "M11 CSB entrance rejects an invalid resume path before showing Resume as available");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB invalid-resume fixture reaches source wait loop before pointer input");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    245,
                    80,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW &&
                    view.csbState.startup_entrance_active == 1,
                "M11 CSB entrance invalid resume path stays on startup");
    expect_true(strcmp(view.lastOutcome, "CSB RESUME UNAVAILABLE") == 0,
                "M11 CSB entrance invalid resume path reports unavailable status");
    M11_GameView_Shutdown(&view);

    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.entranceResumeSavePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB new-game start with entrance resume path succeeds");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_resume_available == 1,
                "M11 CSB entrance stores validated resume path");
    expect_true(view.csbState.party_x == CSB_V1_START_PARTY_X &&
                    view.csbState.party_y == CSB_V1_START_PARTY_Y &&
                    view.csbState.party_dir == CSB_V1_START_PARTY_DIR,
                "M11 CSB entrance resume validation does not apply the save before Resume");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB entrance resume fixture reaches source wait loop before pointer input");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    245,
                    80,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB entrance resume button loads the validated save path");
    drive_csb_entrance_opening(
        &view,
        "M11 CSB entrance resume dismisses to resumed runtime");
    expect_true(view.csbState.startup_entrance_last_command ==
                    M11_ENTRANCE_RUNTIME_COMMAND_RESUME,
                "M11 CSB entrance records the resume command before loading");
    assert_csb_view_matches_expected_resume(
        &view,
        &expected,
        "M11 CSB entrance Resume follows resumed party pose");
    M11_GameView_Shutdown(&view);

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
    expect_true(view.csbState.startup_entrance_active == 0 &&
                view.csbState.startup_entrance_dismissed == 1,
                "M11 CSB resume skips the new-game entrance gate");
    assert_csb_view_matches_expected_resume(
        &view,
        &expected,
        "M11 CSB mirror state follows resumed party pose");
    expect_true(view.world.party.champions[0].present == 1 &&
                view.world.party.champions[1].present == 1,
                "M11 CSB party mirror marks resumed champions present");
    expect_true(memcmp(view.world.party.champions[0].name, "TESTA", 5) == 0 &&
                memcmp(view.world.party.champions[1].name, "TESTB", 5) == 0,
                "M11 CSB party mirror packs champion names");
    expect_true(view.world.party.champions[0].hp.current == 100 &&
                view.world.party.champions[0].hp.maximum == 100 &&
                view.world.party.champions[0].stamina.current == 80 &&
                view.world.party.champions[0].mana.maximum == 20,
                "M11 CSB party mirror copies champion vitals");
    expect_true(view.world.party.champions[0].attributes[0] == 42 &&
                view.world.party.champions[0].attributeMaximums[0] == 48 &&
                view.world.party.champions[0].skillLevels[0] == 3,
                "M11 CSB party mirror copies stats and skills");
    expect_true(view.world.party.champions[0].portraitBitmapValid == 1 &&
                memcmp(view.world.party.champions[0].portraitBitmap,
                       expected.party_state.Champions[0].Portrait,
                       CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT) == 0,
                "M11 CSB party mirror copies compatible portrait bitmap");
    expect_true(memcmp(expected.party_state.Champions[1].Portrait,
                       view.world.party.champions[1].portraitBitmap,
                       CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT) != 0,
                "CSB wide portrait fixture is not a compatible-prefix copy");
    expect_true(view.world.party.champions[1].portraitBitmapValid == 1 &&
                view.world.party.champions[1].portraitBitmap[70] == 0x3cu &&
                view.world.party.champions[1].portraitBitmap[231] == 0x79u,
                "M11 CSB party mirror compacts wide runtime portrait bitmap");

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
        if (profile->variant_id == CSB_V1_VARIANT_PC34_EN) {
            const M11_AssetSlot* title;
            expect_true(view.assetsAvailable == 1,
                        "M11 CSB PC34 start exposes GRAPHICS.DAT to shared render paths");
            title = M11_AssetLoader_Load(&view.assetLoader, 4u);
            expect_true(title != NULL,
                        "M11 CSB PC34 asset loader can load a real graphic entry");
            if (title) {
                expect_true(title->width == 320u && title->height == 200u,
                            "M11 CSB PC34 graphic 4 has source full-screen dimensions");
                expect_true(count_nonzero_slot_pixels(title) > 0,
                            "M11 CSB PC34 loaded graphic has non-empty pixels");
            }
        }
        if (view.assetsAvailable) {
            expect_true(strcmp(view.assetLoader.graphicsDatPath,
                               profile->graphics_path) == 0,
                        "M11 CSB asset loader uses the CSB boot profile graphics path");
        }
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
            expect_true(inject_synthetic_csbgraphics_custom_background(profile),
                        "test injects CSBgraphics custom-background skin data");
            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(&view, fb, 320, 200);
            expect_true(count_diff_rect(expected_fb, fb, 320, 0, 33, 16, 1) > 0,
                        "M11 CSB draw applies runtime-selected custom-background masks");
            expect_true(fb[32 * 320] == 0,
                        "M11 CSB custom-background draw preserves pixels outside viewport");
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
        expect_true(view.world.party.direction == target_dir,
                    "M11 CSB turn-right updates the M11 party mirror direction");
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_UP) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 CSB movement input reaches the source command bridge");
        expect_true(view.csbState.party_x == profile->runtime.party_x &&
                    view.csbState.party_y == profile->runtime.party_y &&
                    view.csbState.current_level ==
                        profile->runtime.current_level,
                    "M11 CSB movement input keeps state mirrors aligned");
        expect_true(view.world.party.mapX == profile->runtime.party_x &&
                    view.world.party.mapY == profile->runtime.party_y &&
                    view.world.party.mapIndex == profile->runtime.current_level,
                    "M11 CSB movement input keeps party mirror pose aligned");

        {
            unsigned short throw_thing =
                (unsigned short)((THING_TYPE_WEAPON << 10) | 0u);
            int projectile_count_before = profile->runtime.projectiles.count;
            profile->runtime.party_state.Champions[0]
                .Slots[CSB_V1_SLOT_ACTION_HAND] = throw_thing;
            view.world.party.champions[0]
                .inventory[CHAMPION_SLOT_ACTION_HAND] = throw_thing;
            view.world.party.champions[0].stamina.current =
                view.world.party.champions[0].stamina.maximum;
            profile->runtime.party_state.Champions[0].CurrentStamina =
                profile->runtime.party_state.Champions[0].MaximumStamina;
            view.actionDisabledTicks[0] = 0;

            expect_true(M11_GameView_TriggerNonMeleeActionByIndex(
                            &view, 0, DM1_ACTION_THROW) == 1,
                        "M11 CSB startup direct THROW dispatches through CSB runtime");
            expect_true(profile->runtime.projectiles.count ==
                            projectile_count_before + 1,
                        "M11 CSB startup THROW allocates a runtime projectile");
            expect_true(profile->runtime.projectiles
                            .entries[projectile_count_before]
                            .reserved1 == throw_thing,
                        "M11 CSB startup THROW preserves thrown thing identity");
            expect_true(profile->runtime.party_state.Champions[0]
                            .Slots[CSB_V1_SLOT_ACTION_HAND] == THING_NONE,
                        "M11 CSB startup THROW clears runtime action hand");
            expect_true(view.world.party.champions[0]
                            .inventory[CHAMPION_SLOT_ACTION_HAND] ==
                            THING_NONE,
                        "M11 CSB startup THROW clears M11 action-hand mirror");
        }
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
    expect_true(quick_loaded.current_level == view.csbState.current_level,
                "M11 CSB quicksave preserves mirrored current level");
    expect_true(quick_loaded.tick_count == (uint32_t)view.csbState.tick_count,
                "M11 CSB quicksave preserves mirrored tick count");
    if (profile) {
        expect_true(quick_loaded.projectiles.count ==
                        profile->runtime.projectiles.count,
                    "M11 CSB quicksave preserves runtime projectile count");
        if (profile->runtime.projectiles.count > 0) {
            int projectile_index = profile->runtime.projectiles.count - 1;
            expect_true(quick_loaded.projectiles.entries[projectile_index]
                            .reserved1 ==
                            profile->runtime.projectiles
                                .entries[projectile_index]
                                .reserved1,
                        "M11 CSB quicksave preserves thrown thing identity");
        }
    }
    csb_v1_runtime_cleanup(&quick_loaded);
    if (profile) {
        int saved_x = view.csbState.party_x;
        int saved_y = view.csbState.party_y;
        int saved_dir = view.csbState.party_dir;
        int saved_level = view.csbState.current_level;
        int saved_tick = view.csbState.tick_count;
        profile->runtime.party_x = saved_x + 1;
        profile->runtime.party_y = saved_y + 1;
        profile->runtime.party_dir = (saved_dir + 1) & 3;
        profile->runtime.current_level = saved_level + 1;
        csb_v1_dungeon_set_current_level(profile->runtime.current_level);
        profile->runtime.tick_count += 9U;
        profile->runtime.game_time += 9U;
        view.csbState.party_x = profile->runtime.party_x;
        view.csbState.party_y = profile->runtime.party_y;
        view.csbState.party_dir = profile->runtime.party_dir;
        view.csbState.current_level = profile->runtime.current_level;
        view.csbState.tick_count = (int)profile->runtime.tick_count;
        expect_true(M11_GameView_QuickLoad(&view),
                    "M11 CSB quickload restores the CSB runtime save");
        expect_true(view.csbState.party_x == saved_x &&
                    view.csbState.party_y == saved_y &&
                    view.csbState.party_dir == saved_dir,
                    "M11 CSB quickload restores saved party pose");
        expect_true(view.csbState.current_level == saved_level &&
                    profile->runtime.current_level == saved_level &&
                    csb_v1_dungeon_get_current_level() == saved_level,
                    "M11 CSB quickload restores saved current level");
        expect_true(view.csbState.tick_count == saved_tick,
                    "M11 CSB quickload restores saved tick count");
    }

    M11_GameView_Shutdown(&view);
    expect_true(view.csbBootProfile == NULL,
                "M11 shutdown clears CSB boot ownership");

    expect_true(firestaff_test_write_csbwin_resume_fixture(csbwin_save_path, 0),
                "built verified CSBWin resume save fixture");
    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.entranceResumeSavePath = csbwin_save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB entrance start accepts verified CSBWin resume path");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_resume_available == 1,
                "M11 CSB entrance marks verified CSBWin Resume available");
    expect_true(view.csbState.party_x == CSB_V1_START_PARTY_X &&
                    view.csbState.party_y == CSB_V1_START_PARTY_Y &&
                    view.csbState.party_dir == CSB_V1_START_PARTY_DIR,
                "M11 CSB entrance CSBWin validation does not apply the save before Resume");
    drive_csb_entrance_to_wait(
        &view,
        "M11 CSB CSBWin Resume fixture reaches source wait loop before pointer input");
    expect_true(M11_GameView_HandlePointerButton(
                    &view,
                    245,
                    80,
                    M11_DM1_MOUSE_MASK_LEFT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 CSB entrance Resume loads the verified CSBWin path");
    expect_true(view.csbState.party_x == 12 &&
                view.csbState.party_y == 7 &&
                view.csbState.party_dir == 3,
                "M11 CSB entrance CSBWin Resume mirrors GAMEBLOCK2 party pose");
    M11_GameView_Shutdown(&view);

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
    fill_csb_launch_spec(&spec, data_dir, NULL);
    spec.entranceResumeSavePath = roster_save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB entrance start accepts raw CSBGAME roster resume path");
    expect_true(view.csbState.startup_entrance_active == 1 &&
                    view.csbState.startup_entrance_resume_available == 1,
                "M11 CSB entrance marks raw CSBGAME roster Resume available");
    expect_true(view.csbState.party_x == CSB_V1_START_PARTY_X &&
                    view.csbState.party_y == CSB_V1_START_PARTY_Y &&
                    view.csbState.party_dir == CSB_V1_START_PARTY_DIR,
                "M11 CSB entrance CSBGAME validation does not apply the roster before Resume");
    M11_GameView_Shutdown(&view);

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
    remove(dm1_import_path);

    if (g_failures) {
        fprintf(stderr, "CSB V1 M11 startup/resume gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: CSB V1 M11 startup/resume gate");
    return 0;
}
