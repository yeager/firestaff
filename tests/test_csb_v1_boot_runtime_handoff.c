/*
 * test_csb_v1_boot_runtime_handoff.c
 *
 * CSB V1 Boot → Runtime Handoff regression
 *
 * Verifies that csb_v1_boot_enter_game() actually completes the
 * profile-to-runtime handoff. Before this fix, enter_game() set the
 * runtime path strings and CSB_STATE_TITLE, but did NOT:
 *   1. Set runtime.dungeon_asset.kind
 *   2. Copy entrance_map_index / start_map_index from the boot profile
 *   3. Load DUNGEON.DAT into the runtime (dungeon_handle != NULL)
 *   4. Release the loaded dungeon through csb_v1_boot_cleanup()
 *
 * The handoff gap left the runtime in a TITLE state with no live
 * dungeon, forcing the game-view to do a second hash search via
 * csb_v1_runtime_boot(). This test exercises the in-place handoff with
 * a synthetic DUNGEON.DAT so we can prove the boundary works end-to-end.
 *
 * Source-locks (matches src/csb/csb_v1_boot.c citation block):
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon
 *   ReDMCSB LOADSAVE.C F0435 lines 1940-1944 new-game map 0
 *   ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 current-map globals
 *   CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "entrance_mouse_routes_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define TEST_LZW_CLEAR_CODE 256
#define TEST_LZW_END_CODE 257
#define TEST_LZW_FIRST_CODE 258
#define TEST_LZW_MAX_CODE 4096
#define TEST_CSB_OBJECT_NAMES_INDEX 564u
#define TEST_CSB_OBJECT_NAME_COUNT 199

typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t bit_pos;
} TestBitWriter;

typedef struct {
    uint8_t dict_first[TEST_LZW_MAX_CODE];
    uint16_t dict_prefix[TEST_LZW_MAX_CODE];
    int dict_count;
    int code_bits;
} TestLZW;

typedef struct {
    int utility_panel_count;
    int closed_doors_count;
    int fallback_text_count;
    int last_waiting_for_input;
    int last_menu_option_count;
    int last_surface;
} TestHudMenuDrawProbe;

typedef struct {
    int draw_title_count;
    int clear_black_count;
    int draw_full_surface_count;
    int draw_full_surface_result;
    int draw_opening_frame_count;
    int draw_opening_frame_result;
    int draw_closed_doors_count;
    int draw_door_fallback_count;
    int draw_fallback_text_count;
    int draw_utility_panel_count;
    int last_surface;
} TestStartupRenderProbe;

static void hud_probe_draw_utility_panel(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestHudMenuDrawProbe *probe = (TestHudMenuDrawProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->utility_panel_count;
    probe->last_waiting_for_input = plan->waiting_for_input;
    probe->last_menu_option_count = plan->menu_option_count;
    probe->last_surface = (int)plan->surface;
}

static void hud_probe_draw_closed_doors(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestHudMenuDrawProbe *probe = (TestHudMenuDrawProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->closed_doors_count;
    probe->last_waiting_for_input = plan->waiting_for_input;
    probe->last_menu_option_count = plan->menu_option_count;
    probe->last_surface = (int)plan->surface;
}

static void hud_probe_draw_fallback_text(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestHudMenuDrawProbe *probe = (TestHudMenuDrawProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->fallback_text_count;
    probe->last_waiting_for_input = plan->waiting_for_input;
    probe->last_menu_option_count = plan->menu_option_count;
    probe->last_surface = (int)plan->surface;
}

static void hud_probe_executor_init(
    CSB_V1_StartupRenderExecutor_PC34 *executor,
    TestHudMenuDrawProbe *probe)
{
    memset(executor, 0, sizeof(*executor));
    executor->user = probe;
    executor->draw_utility_panel = hud_probe_draw_utility_panel;
    executor->draw_closed_doors = hud_probe_draw_closed_doors;
    executor->draw_fallback_text = hud_probe_draw_fallback_text;
}

static int render_probe_draw_title(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return 0;
    }
    ++probe->draw_title_count;
    probe->last_surface = (int)plan->surface;
    return 1;
}

static void render_probe_clear_black(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->clear_black_count;
    probe->last_surface = (int)plan->surface;
}

static int render_probe_draw_full_surface(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return 0;
    }
    ++probe->draw_full_surface_count;
    probe->last_surface = (int)plan->surface;
    return probe->draw_full_surface_result;
}

static int render_probe_draw_opening_frame(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return 0;
    }
    ++probe->draw_opening_frame_count;
    probe->last_surface = (int)plan->surface;
    return probe->draw_opening_frame_result;
}

static void render_probe_draw_closed_doors(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->draw_closed_doors_count;
    probe->last_surface = (int)plan->surface;
}

static void render_probe_draw_door_fallback(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->draw_door_fallback_count;
    probe->last_surface = (int)plan->surface;
}

static void render_probe_draw_fallback_text(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->draw_fallback_text_count;
    probe->last_surface = (int)plan->surface;
}

static void render_probe_draw_utility_panel(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    ++probe->draw_utility_panel_count;
    probe->last_surface = (int)plan->surface;
}

static void render_probe_executor_init(
    CSB_V1_StartupRenderExecutor_PC34 *executor,
    TestStartupRenderProbe *probe)
{
    memset(executor, 0, sizeof(*executor));
    memset(probe, 0, sizeof(*probe));
    executor->user = probe;
    executor->draw_title = render_probe_draw_title;
    executor->clear_black = render_probe_clear_black;
    executor->draw_full_surface = render_probe_draw_full_surface;
    executor->draw_opening_frame = render_probe_draw_opening_frame;
    executor->draw_closed_doors = render_probe_draw_closed_doors;
    executor->draw_door_fallback = render_probe_draw_door_fallback;
    executor->draw_fallback_text = render_probe_draw_fallback_text;
    executor->draw_utility_panel = render_probe_draw_utility_panel;
    probe->draw_full_surface_result = 1;
    probe->draw_opening_frame_result = 1;
}

static void write_le16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)(value & 0xffu);
    buf[off + 1u] = (uint8_t)((value >> 8) & 0xffu);
}

static void bw_init(TestBitWriter *bw)
{
    bw->cap = 1024u;
    bw->buf = (uint8_t *)calloc(1u, bw->cap);
    bw->bit_pos = 0u;
}

static int bw_grow(TestBitWriter *bw)
{
    size_t old_cap = bw->cap;
    size_t new_cap = old_cap * 2u;
    uint8_t *new_buf = (uint8_t *)realloc(bw->buf, new_cap);
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

static int bw_write_bits(TestBitWriter *bw, uint32_t value, int n_bits)
{
    int i;
    for (i = 0; i < n_bits; ++i) {
        size_t bp = bw->bit_pos++;
        size_t byte_idx;
        int bit_in_byte;
        if ((bp >> 3) >= bw->cap && !bw_grow(bw)) {
            return 0;
        }
        byte_idx = bp >> 3;
        bit_in_byte = (int)(bp & 7u);
        if (value & (1u << (uint32_t)i)) {
            bw->buf[byte_idx] |= (uint8_t)(1u << (uint32_t)bit_in_byte);
        }
    }
    return 1;
}

static void test_lzw_init(TestLZW *e)
{
    int i;
    e->dict_count = TEST_LZW_FIRST_CODE;
    e->code_bits = 9;
    for (i = 0; i < 256; ++i) {
        e->dict_first[i] = (uint8_t)i;
        e->dict_prefix[i] = 0xffffu;
    }
}

static int test_lzw_find_or_add(TestLZW *e, uint16_t prefix, uint8_t append)
{
    int i;
    for (i = TEST_LZW_FIRST_CODE; i < e->dict_count; ++i) {
        if (e->dict_prefix[i] == prefix && e->dict_first[i] == append) {
            return i;
        }
    }
    if (e->dict_count >= TEST_LZW_MAX_CODE) {
        return -1;
    }
    e->dict_prefix[e->dict_count] = prefix;
    e->dict_first[e->dict_count] = append;
    ++e->dict_count;
    return -1;
}

static void test_lzw_maybe_grow(TestLZW *e)
{
    if (e->dict_count > ((1 << e->code_bits) - 1) && e->code_bits < 12) {
        ++e->code_bits;
    }
}

static int test_lzw_encode(const uint8_t *input, size_t in_size,
                           uint8_t **out_buf, size_t *out_size)
{
    TestBitWriter bw;
    TestLZW e;
    uint16_t prefix_code;
    size_t i;

    if (!input || !out_buf || !out_size || in_size == 0u) {
        return -1;
    }
    *out_buf = NULL;
    *out_size = 0u;
    bw_init(&bw);
    if (!bw.buf) {
        return -1;
    }
    test_lzw_init(&e);
    if (!bw_write_bits(&bw, TEST_LZW_CLEAR_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    prefix_code = input[0];
    for (i = 1u; i < in_size; ++i) {
        uint8_t next_byte = input[i];
        int existing = test_lzw_find_or_add(&e, prefix_code, next_byte);
        if (existing >= 0) {
            prefix_code = (uint16_t)existing;
        } else {
            if (!bw_write_bits(&bw, prefix_code, e.code_bits)) {
                free(bw.buf);
                return -1;
            }
            test_lzw_maybe_grow(&e);
            prefix_code = next_byte;
        }
    }
    if (!bw_write_bits(&bw, prefix_code, e.code_bits) ||
        !bw_write_bits(&bw, TEST_LZW_END_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    *out_buf = bw.buf;
    *out_size = (bw.bit_pos + 7u) / 8u;
    return 0;
}

static int build_m564_object_name_stream(uint8_t *buf, size_t buf_size,
                                         size_t *out_size)
{
    size_t pos = 0u;
    int i;
    for (i = 0; i < TEST_CSB_OBJECT_NAME_COUNT; i++) {
        char name[32];
        size_t len;
        size_t j;
        snprintf(name, sizeof(name), "%c", (char)('A' + (i % 26)));
        if (i == 0) {
            snprintf(name, sizeof(name), "%s", "DAGGER");
        } else if (i == 7) {
            snprintf(name, sizeof(name), "%s", "SOURCE TORCH");
        }
        len = strlen(name);
        if (len == 0u || pos + len > buf_size) {
            return -1;
        }
        for (j = 0u; j < len; j++) {
            uint8_t ch = (uint8_t)name[j];
            if (j + 1u == len) {
                ch |= 0x80u;
            }
            buf[pos++] = ch;
        }
    }
    if (out_size) {
        *out_size = pos;
    }
    return 0;
}

static int write_synthetic_graphics_dat_with_m564(const char *path)
{
    uint8_t decoded[4096];
    size_t decoded_size = 0u;
    uint8_t *compressed = NULL;
    size_t compressed_size = 0u;
    uint16_t count = (uint16_t)(TEST_CSB_OBJECT_NAMES_INDEX + 1u);
    size_t header_size = 4u + (size_t)count * 8u;
    uint8_t *file_bytes;
    FILE *f;
    size_t n;

    if (build_m564_object_name_stream(decoded, sizeof(decoded),
                                      &decoded_size) != 0 ||
        test_lzw_encode(decoded, decoded_size, &compressed,
                        &compressed_size) != 0 ||
        compressed_size == 0u || compressed_size > 65535u ||
        decoded_size > 65535u) {
        free(compressed);
        return -1;
    }
    file_bytes = (uint8_t *)calloc(1u, header_size + compressed_size);
    if (!file_bytes) {
        free(compressed);
        return -1;
    }
    write_le16(file_bytes, 0u, 0x8001u);
    write_le16(file_bytes, 2u, count);
    write_le16(file_bytes, 4u + TEST_CSB_OBJECT_NAMES_INDEX * 2u,
               (uint16_t)compressed_size);
    write_le16(file_bytes,
               4u + (size_t)count * 2u + TEST_CSB_OBJECT_NAMES_INDEX * 2u,
               (uint16_t)decoded_size);
    write_le16(file_bytes,
               4u + (size_t)count * 4u + TEST_CSB_OBJECT_NAMES_INDEX * 4u,
               1u);
    write_le16(file_bytes,
               4u + (size_t)count * 4u + TEST_CSB_OBJECT_NAMES_INDEX * 4u + 2u,
               (uint16_t)decoded_size);
    memcpy(file_bytes + header_size, compressed, compressed_size);

    f = fopen(path, "wb");
    if (!f) {
        free(file_bytes);
        free(compressed);
        return -1;
    }
    n = fwrite(file_bytes, 1u, header_size + compressed_size, f);
    fclose(f);
    free(file_bytes);
    free(compressed);
    return (n == header_size + compressed_size) ? 0 : -1;
}

/* Build a minimal valid CSB V1 DUNGEON.DAT buffer. Mirrors the
 * synthetic builder in test_csb_v1_phase7_verification.c so the
 * fixture shape matches the legacy loader (square_bytes == 2,
 * column-major 16-bit records, ReDMCSB DUNGEON.C F0151).
 *
 * Header layout (CSB V1 legacy synthetic fixture):
 *   0..1  : level_count (LE16) = 1
 *   2..3  : ignored by the legacy loader
 *   4     : level 0 width  (uint8)
 *   5     : level 0 height (uint8)
 *   6..9  : level 0 absolute byte offset to squares (LE32)
 *   10..  : squares, column-major, 2 bytes each (low byte = type)
 */
static int build_synthetic_dungeon(uint8_t *buf, int buf_size,
                                    uint8_t square_type_1_1)
{
    if (!buf || buf_size < 28) return -1;
    memset(buf, 0, (size_t)buf_size);
    buf[0] = 1; buf[1] = 0;             /* level_count = 1 */
    buf[2] = 16; buf[3] = 0;            /* ignored padding (matches existing fixture) */
    buf[4] = 3; buf[5] = 3;             /* level 0 width=3, height=3 */
    buf[6] = 10; buf[7] = 0;            /* level 0 absolute square offset = 10 */
    buf[8] = 0; buf[9] = 0;
    /* 3x3 squares, column-major, 2 bytes each.
     * Cell (x,y) lives at offset 10 + (x*3 + y) * 2.
     * Row 0: walls at (0,0),(1,0),(2,0) */
    buf[10] = 1; buf[11] = 0;
    buf[12] = 1; buf[13] = 0;
    buf[14] = 1; buf[15] = 0;
    /* Row 1: wall at (0,1), marker at (1,1), wall at (2,1) */
    buf[16] = 1; buf[17] = 0;
    buf[18] = square_type_1_1; buf[19] = 0;
    buf[20] = 1; buf[21] = 0;
    /* Row 2: walls */
    buf[22] = 1; buf[23] = 0;
    buf[24] = 1; buf[25] = 0;
    buf[26] = 1; buf[27] = 0;
    return 0;
}

static int write_synthetic_dungeon(const char *path, uint8_t square_type_1_1)
{
    uint8_t buf[32];
    FILE *f;
    size_t n;
    if (build_synthetic_dungeon(buf, (int)sizeof(buf), square_type_1_1) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

static int write_synthetic_dm1_save_for_utility_flow(const char *path)
{
    uint8_t buf[1024];
    FILE *f;
    size_t n;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 1;

    /* One living champion. Keep structure minimal, but include all
     * required stat offsets for import consistency. */
    memcpy((char *)buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_NAME,
           "ALPHA   ", 8);
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_HEALTH] = 80;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_HEALTH + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = 100;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STAMINA] = 60;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STAMINA + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 100;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MANA] = 30;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MANA + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_MANA] = 50;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_MANA + 1] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STR] = 55;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_DEX] = 66;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_WIS] = 77;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_VIT] = 88;

    {
        size_t equip_off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START +
                           (size_t)CSB_V1_DM1_CHAMP_OFF_EQUIP;
        int slot;
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; slot++) {
            buf[equip_off + (size_t)slot * 2u] = 0xFFu;
            buf[equip_off + (size_t)slot * 2u + 1u] = 0xFFu;
        }
    }

    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

static int build_synthetic_dm1_party_buffer(uint8_t *buf, size_t buf_size,
                                            int champion_count)
{
    int i;
    if (!buf || buf_size < 1024 || champion_count < 1 ||
        champion_count > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    memset(buf, 0, buf_size);
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = (uint8_t)champion_count;
    for (i = 0; i < champion_count; i++) {
        size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START +
                     (size_t)i * (size_t)CSB_V1_DM1_CHAMP_SIZE;
        size_t equip_off = off + (size_t)CSB_V1_DM1_CHAMP_OFF_EQUIP;
        int slot;

        memcpy((char *)buf + off + CSB_V1_DM1_CHAMP_OFF_NAME,
               i == 0 ? "ALPHA   " : "BETA    ", 8);
        buf[off + CSB_V1_DM1_CHAMP_OFF_HEALTH] = (uint8_t)(80 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = (uint8_t)(100 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_STAMINA] = (uint8_t)(60 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = (uint8_t)(100 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MANA] = (uint8_t)(30 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA] = (uint8_t)(50 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = (uint8_t)(55 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = (uint8_t)(66 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = (uint8_t)(77 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = (uint8_t)(88 + i);
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; slot++) {
            buf[equip_off + (size_t)slot * 2u] = 0xFFu;
            buf[equip_off + (size_t)slot * 2u + 1u] = 0xFFu;
        }
    }
    return 0;
}

static void test_utility_flow_new_game_handoff_preserves_leader_index(void)
{
    CSB_V1_UtilFlowContext ctx;
    CSB_V1_UtilMenuLayout layout;
    CSB_V1_PartyState party;
    const char *save_path = "/tmp/firestaff-csb-v1-utility-flow-leader.sav";

    CHECK(write_synthetic_dm1_save_for_utility_flow(save_path) == 0,
          "synthetic DM1 save written for utility flow handoff test");

    csb_v1_util_flow_init(&ctx);
    csb_v1_util_flow_set_dm1_path(&ctx, save_path);
    csb_v1_util_flow_mark_utility_disk_verified(&ctx, 1);

    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "utility flow INIT enters the utility disk prompt");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_INSERT_DISK,
          "utility flow reaches INSERT_DISK before verification");
    CHECK(strstr(csb_v1_util_flow_prompt(&ctx),
                 "CHAOS STRIKES BACK UTILITY DISK") != NULL,
          "utility flow exposes the source utility disk prompt");
    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "utility flow advances from disk prompt to verification");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_VERIFY_DISK,
          "utility flow reaches VERIFY_DISK");
    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "verified startup utility disk advances to DISK_OK");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_DISK_OK,
          "utility flow preserves source-visible DISK_OK boundary");
    CHECK(ctx.disk_result == CSB_V1_UTIL_DISK_OK,
          "utility flow records an OK disk result from startup verification");
    CHECK(strstr(csb_v1_util_flow_prompt(&ctx),
                 "THAT'S THE CHAOS STRIKES BACK UTILITY DISK") != NULL,
          "utility flow exposes the source disk-ok prompt");
    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "utility flow advances from DISK_OK to action selection");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_SELECT_ACTION,
          "utility flow reaches SELECT_ACTION from INIT path");
    CHECK(strstr(csb_v1_util_flow_prompt(&ctx),
                 "IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE") != NULL &&
              strstr(csb_v1_util_flow_prompt(&ctx),
                     "START NEW GAME") != NULL,
          "utility flow exposes the source action menu text");
    memset(&layout, 0, sizeof(layout));
    CHECK(csb_v1_util_flow_menu_layout(&ctx, &layout) == 1,
          "utility flow exposes a source-space action menu layout");
    CHECK(layout.row_count == CSB_V1_UTIL_MENU_ROW_COUNT &&
              layout.rows[0].action == CSB_V1_UTIL_ACTION_IMPORT &&
              layout.rows[1].action == CSB_V1_UTIL_ACTION_LOAD &&
              layout.rows[2].action == CSB_V1_UTIL_ACTION_NEW &&
              layout.rows[3].action == CSB_V1_UTIL_ACTION_VIEW,
          "utility flow layout preserves ReDMCSB action row order");
    CHECK(layout.rows[0].selected == 1 &&
              layout.rows[0].x > 0 &&
              layout.rows[0].y > 0 &&
              layout.rows[0].w > 0 &&
              layout.rows[0].h > 0,
          "utility flow layout marks the selected import row with a source rectangle");
    CHECK(csb_v1_util_flow_action_at_point(
              &ctx,
              layout.rows[2].x + 1,
              layout.rows[2].y + 1) == CSB_V1_UTIL_ACTION_NEW,
          "utility flow hit-test maps row coordinates to source actions");
    CHECK(csb_v1_util_flow_action_at_point(&ctx, 0, 0) ==
              CSB_V1_UTIL_ACTION_EXIT,
          "utility flow hit-test rejects points outside the source action menu");
    csb_v1_util_flow_set_action(&ctx, CSB_V1_UTIL_ACTION_IMPORT);
    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "utility flow import action enters IMPORT_CHAMPIONS");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS,
          "utility flow reaches IMPORT_CHAMPIONS from action selection");

    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "utility flow import step parses DM1 save");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_CONFIRM_IMPORT,
          "utility flow enters CONFIRM_IMPORT after successful import");
    CHECK(strstr(csb_v1_util_flow_prompt(&ctx),
                 "IMPORT THESE CHAMPIONS") != NULL,
          "utility flow exposes the import confirmation prompt");

    csb_v1_util_flow_confirm_import(&ctx, 1);
    CHECK(csb_v1_util_flow_step(&ctx) == 0,
          "confirmed import advances utility flow to NEW_GAME");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_NEW_GAME,
          "utility flow reaches NEW_GAME state");

    CHECK(csb_v1_util_flow_step(&ctx) == 1,
          "new-game step completes the utility flow");
    CHECK(ctx.state == CSB_V1_UTIL_FLOW_DONE,
          "utility flow state is DONE after launch transition");

    memset(&party, 0, sizeof(party));
    CHECK(csb_v1_util_flow_get_party(&ctx, &party) == 1,
          "get_party returns one champion after NEW_GAME");
    CHECK(party.ChampionCount == 1,
          "party.ChampionCount is preserved through NEW_GAME handoff");
    CHECK(party.LeaderIndex == 0,
          "LeaderIndex is preserved through NEW_GAME handoff");
    CHECK(party.ImportedFromDM1 == 1,
          "ImportedFromDM1 is preserved through utility flow handoff");
    CHECK(memcmp(party.Champions[0].Name, "ALPHA   ", 8u) == 0,
          "full champion name is preserved through utility flow handoff");
}

static void test_utility_flow_load_game_uses_runtime_loader(void)
{
    CSB_V1_UtilFlowContext ctx;
    CSB_V1_RuntimeProfile writer;
    CSB_V1_PartyState imported;
    CSB_V1_PartyState loaded_party;
    uint8_t save_buf[1024];
    uint8_t bad_header[512];
    const char *bad_path = "/tmp/firestaff-csb-v1-utility-load-bad.sav";
    const char *good_path = "/tmp/firestaff-csb-v1-utility-load-good.sav";
    FILE *f;

    memset(bad_header, 0, sizeof(bad_header));
    bad_header[0] = (uint8_t)(CSB_V1_SAVE_MAGIC_CSB & 0xffu);
    bad_header[1] = (uint8_t)((CSB_V1_SAVE_MAGIC_CSB >> 8) & 0xffu);
    bad_header[2] = (uint8_t)((CSB_V1_SAVE_MAGIC_CSB >> 16) & 0xffu);
    bad_header[3] = (uint8_t)((CSB_V1_SAVE_MAGIC_CSB >> 24) & 0xffu);
    f = fopen(bad_path, "wb");
    CHECK(f != NULL, "bad utility LOAD fixture opens for write");
    if (f) {
        CHECK(fwrite(bad_header, 1, sizeof(bad_header), f) ==
                  sizeof(bad_header),
              "bad utility LOAD fixture writes a magic-only header");
        fclose(f);
    }

    csb_v1_util_flow_init(&ctx);
    csb_v1_util_flow_mark_utility_disk_verified(&ctx, 1);
    csb_v1_util_flow_set_csb_path(&ctx, bad_path);
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_INSERT_DISK,
          "utility LOAD negative path reaches INSERT_DISK");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_VERIFY_DISK,
          "utility LOAD negative path reaches VERIFY_DISK");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_DISK_OK,
          "utility LOAD negative path accepts verified utility disk");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_SELECT_ACTION,
          "utility LOAD negative path reaches SELECT_ACTION");
    csb_v1_util_flow_set_action(&ctx, CSB_V1_UTIL_ACTION_LOAD);
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_LOAD_GAME,
          "utility LOAD negative path enters LOAD_GAME");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_ERROR &&
              ctx.last_error == -5,
          "utility LOAD rejects magic-only bytes through runtime loader");

    CHECK(build_synthetic_dm1_party_buffer(save_buf, sizeof(save_buf), 2) == 0,
          "utility LOAD builds a two-champion DM1 party buffer");
    CHECK(csb_v1_character_import_dm1_buffer(&imported, save_buf,
                                             (int)sizeof(save_buf)) == 2,
          "utility LOAD imports a two-champion party before save");
    imported.LeaderIndex = 1;
    imported.PartyMapX = 7;
    imported.PartyMapY = 9;
    imported.PartyDirection = CSB_V1_DIR_SOUTH;

    csb_v1_runtime_init(&writer, NULL);
    CHECK(csb_v1_runtime_set_party_state(&writer, &imported) == 0,
          "utility LOAD writer runtime accepts the party snapshot");
    writer.party_x = 7;
    writer.party_y = 9;
    writer.party_dir = CSB_V1_DIR_SOUTH;
    writer.party_state.PartyMapX = 7;
    writer.party_state.PartyMapY = 9;
    writer.party_state.PartyDirection = CSB_V1_DIR_SOUTH;
    writer.game_time = 1234u;
    CHECK(csb_v1_runtime_save_game_to_path(&writer, good_path) == 0,
          "utility LOAD writes a real Firestaff CSB runtime save");
    csb_v1_runtime_cleanup(&writer);

    csb_v1_util_flow_init(&ctx);
    csb_v1_util_flow_mark_utility_disk_verified(&ctx, 1);
    csb_v1_util_flow_set_csb_path(&ctx, good_path);
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_INSERT_DISK,
          "utility LOAD positive path reaches INSERT_DISK");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_VERIFY_DISK,
          "utility LOAD positive path reaches VERIFY_DISK");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_DISK_OK,
          "utility LOAD positive path accepts verified utility disk");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_SELECT_ACTION,
          "utility LOAD positive path reaches SELECT_ACTION");
    csb_v1_util_flow_set_action(&ctx, CSB_V1_UTIL_ACTION_LOAD);
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_LOAD_GAME,
          "utility LOAD positive path enters LOAD_GAME");
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_NEW_GAME,
          "utility LOAD promotes a runtime-validated save to NEW_GAME");
    CHECK(csb_v1_util_flow_step(&ctx) == 1 &&
              ctx.state == CSB_V1_UTIL_FLOW_DONE,
          "utility LOAD completes after runtime-validated save");
    memset(&loaded_party, 0, sizeof(loaded_party));
    CHECK(csb_v1_util_flow_get_party(&ctx, &loaded_party) == 2,
          "utility LOAD exposes loaded party through get_party");
    CHECK(loaded_party.ChampionCount == 2 &&
              loaded_party.LeaderIndex == 1,
          "utility LOAD preserves champion count and leader index");
    CHECK(loaded_party.PartyMapX == 7 &&
              loaded_party.PartyMapY == 9 &&
              loaded_party.PartyDirection == CSB_V1_DIR_SOUTH,
          "utility LOAD preserves saved party pose");
    CHECK(memcmp(loaded_party.Champions[1].Name, "BETA", 4u) == 0,
          "utility LOAD preserves loaded champion identity");
}

static void test_enter_game_with_verified_profile_loads_dungeon(void)
{
    CSB_V1_BootProfile p;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 dispatch;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    char save_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-test";
    int mkdir_ok = (TEST_MKDIR(tmp_dir) == 0) || 1; /* best-effort */
    uint32_t adapter_game_time = 0U;

    memset(&p, 0, sizeof(p));
    (void)mkdir_ok;
    csb_v1_boot_profile_init(&p);

    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written to temp path");

    /* Simulate a hash-verified boot scan by populating the fields the
     * scanner would normally set. The handoff logic only reads these
     * fields and does not re-hash, so this is a faithful model. */
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de"); /* CSB PC 3.4 EN */
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds for a hash-verified boot profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state advances to RUNTIME_READY after enter_game");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime state machine is CSB_STATE_TITLE (ReDMCSB ENTRANCE.C F0806)");
    CHECK(p.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime inherits the boot profile variant id");
    CHECK(p.runtime.difficulty == CSB_V1_DIFFICULTY_HARD,
          "runtime keeps the CSB V1 default difficulty");
    CHECK(p.runtime.entrance_map_index == 255U,
          "runtime entrance map matches boot profile (C255_MAP_INDEX_ENTRANCE)");
    CHECK(p.runtime.start_map_index == 0U,
          "runtime start map is 0 (ReDMCSB LOADSAVE.C F0435)");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "runtime dungeon_asset.path points at the boot profile path");
    CHECK(p.runtime.dungeon_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE,
          "runtime dungeon_asset.kind is NONE (dungeon, not graphics)");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path,
          "runtime graphics_asset.path points at the boot profile path");
    CHECK(p.runtime.graphics_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
          "runtime graphics_asset.kind matches the boot profile kind");
    CHECK(p.runtime.dungeon_handle != NULL,
          "dungeon_handle is non-NULL: DUNGEON.DAT was loaded during handoff");
    CHECK(csb_v1_dungeon_get_current() == p.runtime.dungeon_handle,
          "csb_v1_dungeon_get_current() returns the loaded handle");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "current level defaults to 0 after handoff");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1,
          "chaos magic is initialized after handoff (ReDMCSB CASTER.C F0211)");

    /* M11 receives this already-handoff runtime profile and advances it
     * through the same deterministic V1 tick path.  Keep the transition
     * intentionally small: one harmless timeline event at game_time 0,
     * one 55ms tick, no CSB_STATE_GAME/DUNGEON claim.
     * Source-lock: ReDMCSB GAMELOOP.C F0002 lines 69-124 calls
     * F0261_TIMELINE_Process_CPSEF before G0313_ul_GameTime++, and
     * TIMELINE.C F0240 lines 702-708 expires events whose time is <= the
     * current game time. */
    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_PLAY_SOUND;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 0);
    ev.b_mapX = 1;
    ev.b_mapY = 1;
    CHECK(csb_v1_runtime_add_timeline_event(&p.runtime, &ev) >= 0,
          "post-handoff runtime accepts one timeline event at game_time 0");
    CHECK(p.runtime.timeline_queue.eventCount == 1,
          "post-handoff timeline queue contains one pending event before tick");
    CHECK(csb_v1_runtime_tick_v1(&p.runtime) == 1,
          "post-handoff runtime fires exactly one deterministic V1 tick");
    CHECK(p.runtime.tick_count == 1U && p.runtime.game_time == 1U,
          "post-handoff tick advances tick_count/game_time by one safe step");
    CHECK(p.runtime.game_ticks == CSB_V1_TICK_MS_NOMINAL &&
              p.runtime.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "post-handoff tick records one nominal 55ms quantum");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&p.runtime, &dispatch) == 1,
          "post-handoff tick dispatches the queued timeline event");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_SOUND,
          "post-handoff timeline dispatch stays on the sound boundary");
    CHECK(dispatch.records[0].mapX == 1 && dispatch.records[0].mapY == 1,
          "post-handoff timeline dispatch preserves event coordinates");
    CHECK(p.runtime.timeline_queue.eventCount == 0,
          "post-handoff timeline queue is empty after the single tick");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "post-handoff tick does not claim a broader CSB gameplay state");

    snprintf(save_path, sizeof(save_path), "%s/boot-profile-adapter.fsav",
             tmp_dir);
    CHECK(csb_v1_boot_runtime_save_game_to_path_pc34(
              &p,
              save_path,
              &adapter_game_time) == CSB_V1_SAVE_OK &&
              adapter_game_time == p.runtime.game_time,
          "boot-profile save adapter writes CSB runtime save and reports game time");
    p.runtime.party_x = 3;
    p.runtime.party_y = 4;
    p.runtime.party_dir = CSB_V1_DIR_WEST;
    CHECK(csb_v1_boot_runtime_load_game_from_path_pc34(
              &p,
              save_path,
              &adapter_game_time) == CSB_V1_LOAD_OK &&
              adapter_game_time == p.runtime.game_time &&
              p.runtime.party_x != 3 &&
              p.runtime.party_y != 4 &&
              p.runtime.party_dir != CSB_V1_DIR_WEST,
          "boot-profile load adapter restores CSB runtime save and reports game time");
    CHECK(csb_v1_boot_runtime_tick_pc34(
              &p,
              &adapter_game_time) == 1 &&
              adapter_game_time == p.runtime.game_time,
          "boot-profile tick adapter advances CSB runtime and reports game time");

    /* Cleanup: the boot profile owns the handoff runtime and must clear the
     * global current-dungeon context that mirrors ReDMCSB's current map globals.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755. */
    csb_v1_boot_cleanup(&p);
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "boot cleanup returns the profile to PROFILE_READY");
    CHECK(p.runtime.dungeon_handle == NULL,
          "boot cleanup clears the owned dungeon handle");
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon singleton");
}

static void test_enter_game_preserves_imported_party_and_switches_leader(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_PartyState imported;
    CSB_V1_PartyState runtime_party;
    CSB_V1_RuntimePartyMirrorReceipt_PC34 party_receipt;
    CSB_V1_RuntimeM11MirrorReceipt_PC34 mirror_receipt;
    uint8_t save_buf[1024];
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-party";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written for imported party handoff");
    CHECK(build_synthetic_dm1_party_buffer(save_buf, sizeof(save_buf), 2) == 0,
          "synthetic two-champion DM1 save buffer built");
    CHECK(csb_v1_character_import_dm1_buffer(&imported, save_buf,
                                             (int)sizeof(save_buf)) == 2,
          "DM1 buffer import yields two CSB champions");
    CHECK(imported.LeaderIndex == 0,
          "imported party starts with first living champion as leader");
    imported.PartyDirection = CSB_V1_DIR_EAST;
    imported.Champions[0].Slots[CSB_V1_SLOT_READY_HAND] = 0x1234u;
    imported.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] = 0x2345u;
    imported.Champions[0].Portrait[0] = 0xabu;

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_set_imported_party(&p, &imported) == 0,
          "boot profile accepts imported CSB party before handoff");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds with verified assets and imported party");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state is RUNTIME_READY before leader switch");
    CHECK(p.runtime.dungeon_handle != NULL,
          "verified boot handoff loaded DUNGEON.DAT before leader switch");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 2,
          "runtime party snapshot is visible after boot handoff");
    CHECK(runtime_party.ChampionCount == 2,
          "runtime party keeps imported champion count");
    CHECK(runtime_party.ImportedFromDM1 == 1,
          "runtime party keeps DM1 import provenance");
    CHECK(runtime_party.LeaderIndex == 0 && p.runtime.leader_index == 0,
          "runtime leader starts from imported first living champion");
    CHECK(runtime_party.Champions[0].CurrentHealth == 80 &&
              runtime_party.Champions[0].MaximumHealth == 100,
          "runtime champion 0 keeps imported health current/max");
    CHECK(runtime_party.Champions[0].CurrentStamina == 60 &&
              runtime_party.Champions[0].MaximumStamina == 100,
          "runtime champion 0 keeps imported stamina current/max");
    CHECK(runtime_party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] == 55 &&
              runtime_party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] == 55,
          "runtime champion 0 keeps imported STR current/max");
    CHECK(runtime_party.Champions[1].Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR] == 67 &&
              runtime_party.Champions[1].Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_MAX] == 67,
          "runtime champion 1 keeps imported DEX current/max");
    CHECK(csb_v1_runtime_party_mirror_receipt_from_profile_pc34(
              &p.runtime,
              &party_receipt) == 1 && party_receipt.valid,
          "CSB runtime owns the M11 party mirror receipt");
    CHECK(party_receipt.party.championCount == 2 &&
              party_receipt.party.mapX == p.runtime.party_x &&
              party_receipt.party.mapY == p.runtime.party_y &&
              party_receipt.party.direction == (p.runtime.party_dir & 3),
          "party mirror receipt carries pose and champion count");
    CHECK(party_receipt.party.activeChampionIndex == 0 &&
              party_receipt.party.champions[0].present == 1,
          "party mirror receipt carries active leader and present champion");
    CHECK(memcmp(party_receipt.party.champions[0].name, "ALPHA   ", 8u) == 0,
          "party mirror receipt packs imported champion name");
    CHECK(party_receipt.party.champions[0].hp.current == 80 &&
              party_receipt.party.champions[0].hp.maximum == 100 &&
              party_receipt.party.champions[0].hp.shifted == 200,
          "party mirror receipt carries imported health stat");
    CHECK(party_receipt.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] == 55,
          "party mirror receipt carries imported strength");
    CHECK(party_receipt.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] == 0x1234u &&
              party_receipt.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] == 0x2345u,
          "party mirror receipt maps CSB hand slots to shared champion slots");
    CHECK(party_receipt.party.champions[0].portraitBitmapValid == 1 &&
              party_receipt.party.champions[0].portraitBitmap[0] == 0xabu,
          "party mirror receipt carries compatible imported portrait bytes");
    CHECK(csb_v1_boot_runtime_write_inventory_slot_pc34(
              &p,
              0,
              CSB_V1_SLOT_ACTION_HAND,
              0x3456u) == 1,
          "boot runtime inventory facade writes champion action hand");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 2 &&
              runtime_party.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] ==
                  0x3456u,
          "runtime party observes boot-owned inventory slot write");
    CHECK(csb_v1_boot_runtime_write_champion_vitals_pc34(
              &p,
              0,
              71,
              52,
              33) == 1,
          "boot runtime vitals facade writes champion vitals");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 2 &&
              runtime_party.Champions[0].CurrentHealth == 71 &&
              runtime_party.Champions[0].CurrentStamina == 52 &&
              runtime_party.Champions[0].CurrentMana == 33,
          "runtime party observes boot-owned champion vitals write");
    CHECK(csb_v1_boot_runtime_write_leader_hand_pc34(&p, 0x4567u) == 1,
          "boot runtime leader-hand facade writes transient leader hand");
    CHECK(p.runtime.party_state.LeaderHandThing == 0x4567u,
          "runtime party observes boot-owned leader-hand write");
    CHECK(csb_v1_boot_runtime_read_container_slots_pc34(
              &p,
              THING_NONE,
              NULL) < 0,
          "boot runtime container facade rejects invalid container thing");
    CHECK(csb_v1_boot_runtime_object_allowed_slots_pc34(&p, THING_NONE) == 0,
          "boot runtime object-slot facade rejects invalid object thing");
    /* CSBWin stores transient object-in-hand separately from champion ready-hand slots. */
    p.runtime.party_state.LeaderHandThing = 0x1234u;
    CHECK(csb_v1_runtime_m11_mirror_receipt_from_profile_pc34(
              &p.runtime,
              &mirror_receipt) == 1 && mirror_receipt.valid,
          "CSB runtime owns combined M11 mirror receipt");
    CHECK(mirror_receipt.view.level_loaded == 1 &&
              mirror_receipt.view.party_x == p.runtime.party_x &&
              mirror_receipt.view.party_y == p.runtime.party_y &&
              mirror_receipt.party.party.championCount == 2 &&
              mirror_receipt.party.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] == 0x1234u &&
              mirror_receipt.leader_hand_present &&
              mirror_receipt.leader_hand_thing == 0x1234u,
          "combined M11 mirror receipt carries view state, party mirror and leader hand");
    memset(&mirror_receipt, 0, sizeof(mirror_receipt));
    CHECK(csb_v1_boot_runtime_m11_mirror_receipt_pc34(
              &p,
              &mirror_receipt) == 1 && mirror_receipt.valid &&
              mirror_receipt.view.level_loaded == 1 &&
              mirror_receipt.party.party.championCount == 2 &&
              mirror_receipt.leader_hand_thing == 0x1234u,
          "boot runtime owns the M11 mirror receipt facade");
    CHECK(csb_v1_boot_runtime_trigger_front_wall_ornament_click_pc34(
              NULL,
              0x1234u,
              NULL) == 0,
          "boot front-wall ornament facade rejects NULL profile safely");

    CHECK(csb_v1_runtime_set_leader(&p.runtime, 1) == 0,
          "runtime leader switch to second imported champion succeeds");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 2,
          "runtime party snapshot remains available after leader switch");
    CHECK(runtime_party.LeaderIndex == 1 && p.runtime.leader_index == 1,
          "runtime leader index changes to champion 1 after source-locked switch");
    CHECK(runtime_party.Champions[1].Direction == CSB_V1_DIR_EAST,
          "selected leader direction aligns to party direction (CLIKCHAM.C F0368)");
    CHECK(runtime_party.Champions[1].CurrentHealth == 81 &&
              runtime_party.Champions[1].MaximumHealth == 101,
          "leader switch preserves imported champion 1 health current/max");
    CHECK(runtime_party.Champions[1].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] == 56 &&
              runtime_party.Champions[1].Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR] == 78,
          "leader switch preserves imported champion 1 STR/WIS current stats");

    csb_v1_boot_cleanup(&p);
}

static void test_startup_real_asset_receipt_is_skip_safe_and_deterministic(void)
{
    CSB_V1_StartupRealReceipt receipt;
    const char *tmp_dir = "/tmp/firestaff-csb-v1-startup-real-receipt";
    const char *missing_dir = "/tmp/firestaff-csb-v1-startup-real-missing";
    char expected_hash[CSB_V1_STARTUP_REAL_HASH_HEX_CAP];

    (void)TEST_MKDIR(tmp_dir);
    csb_v1_startup_real_receipt_init(&receipt);
    CHECK(receipt.variant_id == CSB_V1_VARIANT_UNKNOWN &&
              receipt.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE &&
              !receipt.matched &&
              receipt.receipt_hash == 0u,
          "startup real-asset receipt initializes to skip-safe empty state");
    CHECK(csb_v1_startup_real_scan_and_receipt(
              NULL,
              4,
              &receipt) == CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR,
          "startup real-asset receipt rejects missing data root");
    CHECK(csb_v1_startup_real_scan_and_receipt(
              missing_dir,
              4,
              &receipt) == CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR,
          "startup real-asset receipt rejects absent data root");
    CHECK(csb_v1_startup_real_scan_and_receipt(
              tmp_dir,
              7,
              &receipt) == CSB_V1_STARTUP_REAL_OK &&
              !receipt.matched &&
              receipt.max_depth == 7 &&
              strcmp(receipt.asset_root, tmp_dir) == 0 &&
              receipt.receipt_hash == 0u &&
              receipt.receipt_hash_hex[0] == '\0',
          "startup real-asset receipt is skip-safe when no CSB pair is staged");

    csb_v1_startup_real_receipt_init(&receipt);
    snprintf(receipt.asset_root, sizeof(receipt.asset_root), "%s", tmp_dir);
    snprintf(receipt.graphics_path,
             sizeof(receipt.graphics_path),
             "%s/GRAPHICS.DAT",
             tmp_dir);
    snprintf(receipt.dungeon_path,
             sizeof(receipt.dungeon_path),
             "%s/DUNGEON.DAT",
             tmp_dir);
    snprintf(receipt.graphics_md5,
             sizeof(receipt.graphics_md5),
             "%s",
             "61fbfd56887c94adc26888a9491c6611");
    snprintf(receipt.dungeon_md5,
             sizeof(receipt.dungeon_md5),
             "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    receipt.graphics_size_bytes = 123456u;
    receipt.dungeon_size_bytes = 654321u;
    receipt.variant_id = CSB_V1_VARIANT_PC34_EN;
    receipt.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    receipt.max_depth = 4;
    receipt.assets_verified = 1;
    receipt.graphics_verified = 1;
    receipt.dungeon_verified = 1;
    receipt.matched = 1;
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 1 &&
              receipt.receipt_hash != 0u &&
              strlen(receipt.receipt_hash_hex) == 16u,
          "startup real-asset receipt packages deterministic capture hash");
    snprintf(expected_hash, sizeof(expected_hash), "%s",
             receipt.receipt_hash_hex);
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 1 &&
              strcmp(receipt.receipt_hash_hex, expected_hash) == 0,
          "startup real-asset receipt recompute validates packaged proof");
    receipt.graphics_size_bytes++;
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 0,
          "startup real-asset receipt detects changed packaged metadata");
    CHECK(csb_v1_startup_real_receipt_from_profile_fields(
              tmp_dir,
              "/tmp/firestaff-csb-v1-startup-real-receipt/GRAPHICS.DAT",
              "/tmp/firestaff-csb-v1-startup-real-receipt/DUNGEON.DAT",
              "61fbfd56887c94adc26888a9491c6611",
              "6695d2acebce49f95db1d8f3a5c733de",
              0u,
              0u,
              CSB_V1_VARIANT_PC34_EN,
              CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
              4,
              1,
              1,
              1,
              &receipt) == 1 &&
              receipt.matched &&
              receipt.receipt_hash != 0u &&
              strcmp(receipt.asset_root, tmp_dir) == 0,
          "startup real-asset receipt packages verified boot profile fields");
    CHECK(strcmp(csb_v1_startup_real_result_name(
                     CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN),
                 "CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN") == 0,
          "startup real-asset receipt exposes stable result names");
}

static void test_enter_game_loads_m564_object_names_from_graphics_dat(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-m564-object-names";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written for M564 boot handoff");
    CHECK(write_synthetic_graphics_dat_with_m564(graphics_path) == 0,
          "synthetic GRAPHICS.DAT written with LZW-compressed M564 object names");

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds with a GRAPHICS.DAT M564 payload");
    CHECK(p.runtime.object_name_table_valid == 1,
          "runtime owns the decoded C199 M564 object-name table after boot");
    CHECK(strcmp(p.runtime.object_names[0], "DAGGER") == 0,
          "M564 entry 0 decodes as DAGGER");
    CHECK(strcmp(p.runtime.object_names[7], "SOURCE TORCH") == 0,
          "M564 entry 7 decodes as SOURCE TORCH");

    csb_v1_boot_cleanup(&p);
}

static void test_enter_game_rotate_party_aligns_champion_state(void)
{
    /* This is the narrow follow-up boundary for the green leader-switch
     * runtime gate (test_enter_game_preserves_imported_party_and_switches_leader).
     * ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines 117-130 is the
     * single source of truth for the party-rotation invariant:
     *   delta = (target_dir - party_dir) mod 4
     *   for every champion in the party:
     *     Cell      = (Cell      + delta) mod 4
     *     Direction = (Direction + delta) mod 4
     *   party_dir = target_dir
     * The runtime must reproduce that invariant deterministically over the
     * imported party snapshot that the boot handoff captured, without
     * touching dungeon geometry, hand objects, or the F0292 redraw stack.
     * Source-lock: ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection
     * lines 117-130 (MEDIA182 C source). */
    CSB_V1_BootProfile p;
    CSB_V1_PartyState imported;
    CSB_V1_PartyState runtime_party;
    uint8_t save_buf[1024];
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-rotation-followup";
    int i;
    int rotations_tested;

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written for party rotation follow-up");
    CHECK(build_synthetic_dm1_party_buffer(save_buf, sizeof(save_buf), 4) == 0,
          "synthetic four-champion DM1 save buffer built for rotation test");
    CHECK(csb_v1_character_import_dm1_buffer(&imported, save_buf,
                                             (int)sizeof(save_buf)) == 4,
          "DM1 buffer import yields four CSB champions");
    imported.PartyDirection = CSB_V1_DIR_NORTH;

    /* Seed deterministic, non-zero Cell/Direction for every champion so
     * the rotation math is observable.  Champion i starts at:
     *   Cell      = (i + 1) mod 4     (so 1, 2, 3, 0)
     *   Direction = (i + 2) mod 4     (so 2, 3, 0, 1)
     * PartyDirection = NORTH (0). */
    for (i = 0; i < imported.ChampionCount; i++) {
        imported.Champions[i].Cell      = (uint8_t)((i + 1) & 3);
        imported.Champions[i].Direction = (uint8_t)((i + 2) & 3);
    }

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_set_imported_party(&p, &imported) == 0,
          "boot profile accepts the four-champion party before handoff");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds for the rotation follow-up path");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state advances to RUNTIME_READY before rotation");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_NORTH,
          "runtime party_dir starts at NORTH (matches PartyDirection)");

    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is visible after boot handoff");
    /* Each champion's seeded Cell/Direction must survive handoff unchanged
     * because F0284 only fires on a real rotation, not on import. */
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        CHECK(runtime_party.Champions[i].Cell ==
                  (uint8_t)((i + 1) & 3),
                  "imported Cell survives handoff (no rotation yet)");
        CHECK(runtime_party.Champions[i].Direction ==
                  (uint8_t)((i + 2) & 3),
                  "imported Direction survives handoff (no rotation yet)");
    }

    /* F0284 guard: rotation to the same direction is a deterministic
     * no-op and must not perturb any champion state. */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_NORTH) == 0,
          "rotate_party(NORTH) is a no-op when already facing NORTH");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_NORTH,
          "party_dir stays NORTH after same-direction no-op");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "party snapshot still has four champions after no-op rotation");
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        CHECK(runtime_party.Champions[i].Cell ==
                  (uint8_t)((i + 1) & 3),
                  "Cell is unchanged by same-direction no-op rotation");
        CHECK(runtime_party.Champions[i].Direction ==
                  (uint8_t)((i + 2) & 3),
                  "Direction is unchanged by same-direction no-op rotation");
    }

    /* Real rotation NORTH -> EAST (delta=+1).  Every champion's Cell
     * and Direction must advance by +1 mod 4 (matches F0284 line 124-125
     * for the MEDIA182 C source). */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_EAST) == 0,
          "rotate_party(EAST) succeeds after verified boot handoff");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_EAST,
          "party_dir advances to EAST after rotation");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "party snapshot is live after NORTH->EAST rotation");
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        CHECK(runtime_party.Champions[i].Cell ==
                  (uint8_t)(((i + 1) + 1) & 3),
                  "Cell advances by +1 mod 4 (CHAMPION.C F0284 delta=+1)");
        CHECK(runtime_party.Champions[i].Direction ==
                  (uint8_t)(((i + 2) + 1) & 3),
                  "Direction advances by +1 mod 4 (CHAMPION.C F0284 delta=+1)");
    }

    /* Real rotation EAST -> WEST (delta=+2).  Every Cell/Direction
     * advances by +2 mod 4 on top of the previous +1 offset. */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_WEST) == 0,
          "rotate_party(WEST) succeeds after EAST pivot");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_WEST,
          "party_dir advances to WEST after rotation");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "party snapshot is live after EAST->WEST rotation");
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        CHECK(runtime_party.Champions[i].Cell ==
                  (uint8_t)(((i + 1) + 1 + 2) & 3),
                  "Cell advances by +2 mod 4 on top of the +1 offset "
                  "(CHAMPION.C F0284 delta=+2)");
        CHECK(runtime_party.Champions[i].Direction ==
                  (uint8_t)(((i + 2) + 1 + 2) & 3),
                  "Direction advances by +2 mod 4 on top of the +1 offset "
                  "(CHAMPION.C F0284 delta=+2)");
    }

    /* Real rotation WEST -> SOUTH (delta=+3 mod 4, equivalent to -1).
     * This exercises the F0284 negative-delta wrap branch
     * (delta += 4 when target < current, lines 120-122). */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_SOUTH) == 0,
          "rotate_party(SOUTH) succeeds after WEST pivot");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_SOUTH,
          "party_dir advances to SOUTH after rotation");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "party snapshot is live after WEST->SOUTH rotation");
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        CHECK(runtime_party.Champions[i].Cell ==
                  (uint8_t)(((i + 1) + 1 + 2 + 3) & 3),
                  "Cell advances by +3 mod 4 on top of the cumulative "
                  "offset (CHAMPION.C F0284 delta=+3)");
        CHECK(runtime_party.Champions[i].Direction ==
                  (uint8_t)(((i + 2) + 1 + 2 + 3) & 3),
                  "Direction advances by +3 mod 4 on top of the cumulative "
                  "offset (CHAMPION.C F0284 delta=+3)");
    }

    /* Rotation tolerance: 4 successive rotations of +1 each must return
     * party_dir to its starting value and bring every champion's Cell and
     * Direction back to the pre-cycle state (delta accumulator is mod 4).
     * Starting point is party_dir = SOUTH (2); we step
     * SOUTH -> WEST -> NORTH -> EAST -> SOUTH. */
    {
        CSB_V1_PartyState pre_cycle;
        memset(&pre_cycle, 0, sizeof(pre_cycle));
        CHECK(csb_v1_runtime_get_party_state(&p.runtime, &pre_cycle) == 4,
              "party snapshot captured before full rotation cycle");
        rotations_tested = 0;
        for (i = 0; i < 4; i++) {
            int target = (CSB_V1_DIR_SOUTH + 1 + i) & 3;
            CHECK(csb_v1_runtime_rotate_party(&p.runtime, target) == 0,
                  "four-step rotation cycle keeps rotate_party returning 0");
            rotations_tested++;
        }
        CHECK(rotations_tested == 4,
              "all four step-rotation steps ran without rejection");
        CHECK(p.runtime.party_dir == CSB_V1_DIR_SOUTH,
              "party_dir returns to SOUTH after SOUTH->W->N->E->SOUTH cycle");
        CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
              "party snapshot is live after full rotation cycle");
        for (i = 0; i < runtime_party.ChampionCount; i++) {
            CHECK(runtime_party.Champions[i].Cell == pre_cycle.Champions[i].Cell,
                  "Cell returns to pre-cycle value after full rotation cycle");
            CHECK(runtime_party.Champions[i].Direction ==
                      pre_cycle.Champions[i].Direction,
                  "Direction returns to pre-cycle value after full rotation cycle");
        }
    }

    /* Leader-switch after rotation must follow the rotated party_dir
     * (CLIKCHAM.C F0368 line 67: Champion.Direction = G0308_i_PartyDirection).
     * Champion 2 is selected here; we re-read its Direction and expect
     * the most recently committed party_dir regardless of pre-rotation
     * seed.  This cross-cuts the F0284 boundary with the existing
     * F0368 leader-switch boundary. */
    CHECK(csb_v1_runtime_set_leader(&p.runtime, 2) == 0,
          "leader switch to champion 2 succeeds after full rotation cycle");
    CHECK(p.runtime.leader_index == 2,
          "runtime leader index points at champion 2");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_SOUTH,
          "party_dir stays at SOUTH after the full rotation cycle and "
          "post-cycle leader switch");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "party snapshot is live after post-rotation leader switch");
    CHECK(runtime_party.Champions[2].Direction ==
              (uint8_t)p.runtime.party_dir,
          "post-rotation leader direction is aligned to party_dir "
          "(CLIKCHAM.C F0368 line 67 over CHAMPION.C F0284)");

    /* Argument validation: out-of-range and missing-party rejections. */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, 4) == -1,
          "rotate_party rejects target_dir=4 (out of normalized range)");
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, -1) == -1,
          "rotate_party rejects negative target_dir");
    CHECK(csb_v1_runtime_rotate_party(NULL, CSB_V1_DIR_NORTH) == -1,
          "rotate_party rejects NULL profile");

    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_NORTH) == -1,
          "rotate_party is unavailable after boot_cleanup clears the party");
}

static void test_enter_game_with_missing_dungeon_path_keeps_runtime_safe(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-missing-dng";
    char bogus_dungeon[ASSET_PATH_MAX];

    (void)TEST_MKDIR(tmp_dir); /* best-effort */
    snprintf(bogus_dungeon, sizeof(bogus_dungeon),
             "%s/this-file-does-not-exist.DAT", tmp_dir);
    (void)dungeon_path;

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);

    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", bogus_dungeon);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game still succeeds when the verified path is unreadable");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state still advances to RUNTIME_READY");
    CHECK(p.runtime.dungeon_handle == NULL,
          "dungeon_handle stays NULL when the path cannot be opened");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "dungeon_asset.path still points at the verified path even if load fails");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime state remains CSB_STATE_TITLE");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1,
          "chaos magic is still initialized (handoff is tolerant)");

    /* Free the global dungeon singleton if the loader left anything set. */
    csb_v1_dungeon_set_current(NULL);
}

static void test_enter_game_runtime_handoff_is_idempotent(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-idempotent";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 7) == 0,
          "synthetic DUNGEON.DAT written for idempotence test");

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0, "first enter_game succeeds");
    CSB_V1_DungeonData *first_handle = p.runtime.dungeon_handle;
    const CSB_V1_DungeonData *first_global = csb_v1_dungeon_get_current();
    CHECK(first_handle != NULL, "first handoff produced a heap dungeon handle");
    CHECK(first_global == first_handle, "singleton matches the heap handle");
    csb_v1_dungeon_set_current_level(2);
    CHECK(csb_v1_dungeon_get_current_level() == 2,
          "test fixture moved the current level before re-entering");

    /* A second enter_game() on the same profile must re-load and
     * replace the previous live context (no double-free, no stale level). */
    CHECK(csb_v1_boot_enter_game(&p) == 0, "second enter_game succeeds");
    CHECK(p.runtime.dungeon_handle != NULL, "second handoff produced a live handle");
    CHECK(csb_v1_dungeon_get_current() == p.runtime.dungeon_handle,
          "singleton now points at the second handle");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "second handoff resets current level to the source-locked new-game map");

    csb_v1_boot_cleanup(&p);
    CHECK(p.runtime.dungeon_handle == NULL,
          "cleanup after repeated handoff clears the latest handle");
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "cleanup after repeated handoff clears the singleton");
}

static void test_enter_game_rejects_partial_or_misrouted_profiles(void)
{
    CSB_V1_BootProfile p;
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-guard";

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 0;
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s/DUNGEON.DAT", tmp_dir);

    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects stale aggregate readiness without DUNGEON proof");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "partial proof rejection leaves profile state unchanged");
    CHECK(p.runtime.dungeon_handle == NULL,
          "partial proof rejection does not load or attach a dungeon");

    p.dungeon_verified = 1;
    p.graphics_path[0] = '\0';
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects verified flags without a graphics path");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "missing path rejection leaves profile state unchanged");

    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    snprintf(p.game_id, sizeof(p.game_id), "%s", "dm1");
    p.runtime.chaos_magic.magic_initialized = 77;
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects a non-CSB profile routed to the CSB runtime");
    CHECK(p.runtime.chaos_magic.magic_initialized == 77,
          "misrouted profile rejection does not rebuild runtime state");
}

static void test_enter_game_v2_profile_labels_do_not_change_v1_handoff(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_RuntimeStartupRuntimePlan_PC34 runtime_plan;
    CSB_V1_StartupRuntimePlan_PC34 startup_plan;
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 runtime_receipt;
    char dungeon_path[ASSET_PATH_MAX];
    char bonus_dungeon_path[ASSET_PATH_MAX];
    char runtime_save_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v2-profile-fallback-guard";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(bonus_dungeon_path, sizeof(bonus_dungeon_path), "%s/DUNGEONB.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written for CSB V2 profile fallback guard");
    CHECK(write_synthetic_dungeon(bonus_dungeon_path, 7) == 0,
          "synthetic DUNGEONB.DAT written for CSB bonus-dungeon handoff");

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/CSBGRAPH.DAT", tmp_dir);
    snprintf(p.version_id, sizeof(p.version_id), "%s", "csb-v2.1-upscaled-selected");
    snprintf(p.variant_label, sizeof(p.variant_label), "%s", "CSB V2.1 Upscaled");
    snprintf(p.media_ref, sizeof(p.media_ref), "%s", "presentation-v2-profile");
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_ST21_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "verified CSB V2-labeled profile falls back through the V1 handoff");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "V2-labeled profile reaches the V1 runtime-ready state");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "V2-labeled profile still enters the source-locked V1 title state");
    CHECK(p.runtime.variant_id == CSB_V1_VARIANT_ST21_EN,
          "V2-labeled profile preserves the CSB V1 media variant");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path &&
              p.runtime.dungeon_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE,
          "V2-labeled profile hands off only the verified V1 DUNGEON.DAT path");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path &&
              p.runtime.graphics_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF,
          "V2-labeled profile hands off only the verified V1 graphics archive path");
    CHECK(p.runtime.dungeon_handle != NULL,
          "V2-labeled profile loads the V1 dungeon during handoff");
    CHECK(p.runtime.entrance_map_index == 255U && p.runtime.start_map_index == 0U,
          "V2-labeled profile keeps the V1 entrance/start map boundary");
    CHECK(csb_v1_runtime_get_load_bonus_dungeon(&p.runtime) == 0,
          "V1 runtime starts with the bonus-dungeon load flag clear");
    CHECK(csb_v1_runtime_try_load_bonus_dungeon(&p.runtime) == 0,
          "runtime does not load a bonus dungeon while the source flag is clear");
    CHECK(p.runtime.dungeon_path == p.dungeon_path,
          "clear bonus flag leaves the normal verified dungeon path active");
    CHECK(csb_v1_runtime_set_load_bonus_dungeon(&p.runtime, 1) == 1,
          "runtime accepts source C201 bonus-dungeon load request");
    CHECK(csb_v1_runtime_get_load_bonus_dungeon(&p.runtime) == 1,
          "runtime exposes source C201 bonus-dungeon load request");
    CHECK(csb_v1_runtime_try_load_bonus_dungeon(&p.runtime) == 1,
          "runtime loads sibling DUNGEONB.DAT when C201 requested bonus dungeon");
    CHECK(csb_v1_runtime_get_bonus_dungeon_path(&p.runtime) != NULL &&
              strcmp(csb_v1_runtime_get_bonus_dungeon_path(&p.runtime),
                     bonus_dungeon_path) == 0,
          "runtime records the loaded bonus-dungeon path");
    CHECK(strcmp(p.runtime.dungeon_path, bonus_dungeon_path) == 0,
          "runtime active dungeon path points at the loaded bonus dungeon");
    CHECK(csb_v1_runtime_set_load_bonus_dungeon(&p.runtime, 0) == 1 &&
              csb_v1_runtime_get_load_bonus_dungeon(&p.runtime) == 0,
          "runtime can clear the bonus-dungeon load request for normal enter");

    memset(&runtime_plan, 0, sizeof(runtime_plan));
    runtime_plan.kind = CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_DUNGEON_PC34;
    runtime_plan.set_bonus_dungeon = 1;
    runtime_plan.bonus_dungeon = 0;
    CHECK(csb_v1_runtime_apply_startup_runtime_plan_pc34(
              &p.runtime,
              &runtime_plan,
              NULL,
              &runtime_receipt) == 1 &&
              runtime_receipt.bonus_requested_changed &&
              runtime_receipt.bonus_requested == 0 &&
              csb_v1_runtime_get_load_bonus_dungeon(&p.runtime) == 0,
          "runtime startup plan execution owns normal dungeon request");

    memset(&runtime_plan, 0, sizeof(runtime_plan));
    runtime_plan.kind =
        CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_BONUS_DUNGEON_PC34;
    runtime_plan.set_bonus_dungeon = 1;
    runtime_plan.bonus_dungeon = 1;
    CHECK(csb_v1_runtime_apply_startup_runtime_plan_pc34(
              &p.runtime,
              &runtime_plan,
              NULL,
              &runtime_receipt) == 1 &&
              runtime_receipt.bonus_requested_changed &&
              runtime_receipt.bonus_requested == 1 &&
              runtime_receipt.bonus_dungeon_loaded &&
              runtime_receipt.sync_profile_state,
          "runtime startup plan execution owns bonus dungeon load");

    snprintf(runtime_save_path,
             sizeof(runtime_save_path),
             "%s/runtime-plan-resume.fsav",
             tmp_dir);
    p.runtime.party_x = 12;
    p.runtime.party_y = 13;
    p.runtime.party_dir = 2;
    CHECK(csb_v1_runtime_save_game_to_path(&p.runtime,
                                           runtime_save_path) == 0,
          "runtime startup plan fixture writes a resumable CSB save");
    p.runtime.party_x = 1;
    p.runtime.party_y = 1;
    p.runtime.party_dir = 0;
    memset(&runtime_plan, 0, sizeof(runtime_plan));
    runtime_plan.kind = CSB_V1_RUNTIME_STARTUP_PLAN_RESUME_PC34;
    runtime_plan.requires_resume_load = 1;
    CHECK(csb_v1_runtime_apply_startup_runtime_plan_pc34(
              &p.runtime,
              &runtime_plan,
              runtime_save_path,
              &runtime_receipt) == 1 &&
              runtime_receipt.resume_available &&
              runtime_receipt.resume_loaded &&
              runtime_receipt.sync_profile_state &&
              runtime_receipt.sync_leader_hand &&
              p.runtime.party_x == 12 &&
              p.runtime.party_y == 13 &&
              p.runtime.party_dir == 2,
          "runtime startup plan execution owns entrance resume load");

    memset(&startup_plan, 0, sizeof(startup_plan));
    startup_plan.kind =
        CSB_V1_STARTUP_RUNTIME_PLAN_ENTER_BONUS_DUNGEON_PC34;
    startup_plan.set_bonus_dungeon = 1;
    startup_plan.bonus_dungeon = 1;
    CHECK(csb_v1_runtime_apply_startup_sequence_plan_pc34(
              &p.runtime,
              &startup_plan,
              NULL,
              &runtime_receipt) == 1 &&
              runtime_receipt.bonus_requested_changed &&
              runtime_receipt.bonus_requested == 1,
          "runtime owns CSB startup-sequence plan adapter for bonus dungeon");

    memset(&startup_plan, 0, sizeof(startup_plan));
    startup_plan.kind = CSB_V1_STARTUP_RUNTIME_PLAN_RESUME_PC34;
    startup_plan.requires_resume_load = 1;
    p.runtime.party_x = 3;
    p.runtime.party_y = 4;
    p.runtime.party_dir = 1;
    CHECK(csb_v1_runtime_apply_startup_sequence_plan_pc34(
              &p.runtime,
              &startup_plan,
              runtime_save_path,
              &runtime_receipt) == 1 &&
              runtime_receipt.resume_available &&
              runtime_receipt.resume_loaded &&
              p.runtime.party_x == 12 &&
              p.runtime.party_y == 13 &&
              p.runtime.party_dir == 2,
          "runtime owns CSB startup-sequence plan adapter for resume");

    csb_v1_boot_cleanup(&p);
}

static void test_runtime_import_dm1_party_path_owns_utility_handoff(void)
{
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_PartyState party;
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 receipt;
    uint8_t save_buf[1024];
    const char *path = "/tmp/firestaff-csb-v1-runtime-dm1-import.sav";
    FILE *f;
    int imported_count = 0;
    int utility_state = -1;
    char utility_prompt[128];

    CHECK(build_synthetic_dm1_party_buffer(save_buf, sizeof(save_buf), 2) == 0,
          "runtime DM1 import builds a two-champion source save buffer");
    f = fopen(path, "wb");
    CHECK(f != NULL, "runtime DM1 import fixture opens for write");
    if (f) {
        CHECK(fwrite(save_buf, 1u, sizeof(save_buf), f) == sizeof(save_buf),
              "runtime DM1 import fixture writes the full source buffer");
        fclose(f);
    }

    csb_v1_runtime_init(&runtime, NULL);
    utility_prompt[0] = '\0';
    CHECK(csb_v1_runtime_import_dm1_party_path(&runtime,
                                               path,
                                               &imported_count,
                                               &utility_state,
                                               utility_prompt,
                                               sizeof(utility_prompt)) == 1,
          "runtime imports a DM1 party through the CSB utility handoff");
    CHECK(imported_count == 2,
          "runtime DM1 import reports imported champion count");
    CHECK(utility_state == (int)CSB_V1_UTIL_FLOW_DONE,
          "runtime DM1 import exposes completed utility state");
    CHECK(strstr(utility_prompt, "READY") != NULL,
          "runtime DM1 import exposes the final utility prompt");
    memset(&party, 0, sizeof(party));
    CHECK(csb_v1_runtime_get_party_state(&runtime, &party) == 2,
          "runtime DM1 import commits party state to runtime");
    CHECK(party.ImportedFromDM1 == 1 &&
              party.ChampionCount == 2 &&
              memcmp(party.Champions[1].Name, "BETA", 4u) == 0,
          "runtime DM1 import preserves champion identity and provenance");

    csb_v1_runtime_cleanup(&runtime);
    csb_v1_runtime_init(&runtime, NULL);
    CHECK(csb_v1_runtime_apply_startup_handoff_pc34(
              &runtime,
              NULL,
              path,
              &receipt) == 1,
          "runtime startup handoff owns DM1 import execution");
    CHECK(receipt.kind ==
              CSB_V1_RUNTIME_STARTUP_HANDOFF_IMPORT_DM1_PC34 &&
              receipt.import_attempted &&
              receipt.import_succeeded &&
              receipt.import_champion_count == 2,
          "runtime startup handoff receipt reports import result");
    CHECK(receipt.import_utility_state == (int)CSB_V1_UTIL_FLOW_DONE &&
              strstr(receipt.import_utility_prompt, "READY") != NULL,
          "runtime startup handoff receipt carries utility state");
    {
        CSB_V1_BootProfile boot;
        CSB_V1_StartupSessionOptions_PC34 options;
        CSB_V1_RuntimeStartupSessionStateReceipt_PC34 state_receipt;
        csb_v1_runtime_startup_session_state_receipt_init_pc34(&state_receipt);
        CHECK(!state_receipt.entrance_resume_available &&
                  !state_receipt.import_available &&
                  state_receipt.import_utility_state ==
                      (int)CSB_V1_UTIL_FLOW_INIT &&
                  state_receipt.entrance_resume_path[0] == '\0' &&
                  state_receipt.import_dm1_save_path[0] == '\0',
              "runtime session state receipt has safe startup defaults");
        CHECK(csb_v1_runtime_build_startup_session_options_pc34(
                  &runtime,
                  &receipt,
                  path,
                  NULL,
                  &options) == 1,
              "runtime builds startup session options from import handoff");
        CHECK(options.import_available &&
                  options.import_champion_count == 2 &&
                  options.import_utility_state ==
                      (int)CSB_V1_UTIL_FLOW_DONE &&
	                  strcmp(options.import_dm1_save_path, path) == 0 &&
	                  strstr(options.import_utility_prompt, "READY") != NULL,
	              "runtime session options publish imported DM1 party");
        CHECK(csb_v1_runtime_build_startup_session_state_receipt_pc34(
                  &runtime,
                  &receipt,
                  path,
                  NULL,
                  &state_receipt) == 1 &&
                  state_receipt.import_available &&
                  state_receipt.import_champion_count == 2 &&
                  state_receipt.import_selected_action_index == 0 &&
                  state_receipt.import_preview_active == 0 &&
                  state_receipt.import_utility_state ==
                      (int)CSB_V1_UTIL_FLOW_DONE &&
                  strcmp(state_receipt.import_dm1_save_path, path) == 0 &&
                  strstr(state_receipt.import_utility_prompt, "READY") != NULL,
              "runtime session state receipt mirrors M11 startup import fields");
        csb_v1_boot_profile_init(&boot);
        CHECK(csb_v1_boot_apply_startup_handoff_pc34(
                  &boot,
                  NULL,
                  path,
                  &receipt) == 1,
              "boot profile owns startup handoff adapter");
        CHECK(receipt.kind ==
                  CSB_V1_RUNTIME_STARTUP_HANDOFF_IMPORT_DM1_PC34 &&
                  receipt.import_succeeded &&
                  receipt.import_champion_count == 2,
              "boot startup handoff receipt reports import result");
        CHECK(csb_v1_boot_build_startup_session_state_receipt_pc34(
                  &boot,
                  &receipt,
                  path,
                  NULL,
                  &state_receipt) == 1 &&
                  state_receipt.import_available &&
                  state_receipt.import_champion_count == 2 &&
                  strcmp(state_receipt.import_dm1_save_path, path) == 0,
              "boot profile owns startup session state receipt adapter");
        {
            CSB_V1_BootProfile boot2;
            CSB_V1_BootStartupLaunchReceipts_PC34 launch_receipts;
            csb_v1_boot_profile_init(&boot2);
            CHECK(csb_v1_boot_build_startup_launch_receipts_pc34(
                      &boot2,
                      NULL,
                      path,
                      NULL,
                      &launch_receipts) == 1 &&
                      launch_receipts.handoff.kind ==
                          CSB_V1_RUNTIME_STARTUP_HANDOFF_IMPORT_DM1_PC34 &&
                      launch_receipts.handoff.import_succeeded &&
                      launch_receipts.init_state.command_state.entrance_active &&
                      launch_receipts.init_state.command_state.title_active &&
                      launch_receipts.session_state.import_available &&
                      launch_receipts.session_state.import_champion_count == 2 &&
                      launch_receipts.launch_host_receipt.status_scope &&
                      strcmp(launch_receipts.launch_host_receipt.status_scope,
                             "BOOT") == 0 &&
                      launch_receipts.launch_host_receipt.status &&
                      strcmp(launch_receipts.launch_host_receipt.status,
                             "CSB ENTRANCE") == 0 &&
                      launch_receipts.launch_host_receipt.log_color == 11U &&
                      launch_receipts.launch_host_receipt.log_line &&
                      strcmp(launch_receipts.launch_host_receipt.log_line,
                             "CSB ENTRANCE") == 0 &&
                      strcmp(launch_receipts.session_state.import_dm1_save_path,
                             path) == 0,
                  "boot profile owns combined M11 startup launch receipts, host status, and log");
            csb_v1_boot_cleanup(&boot2);
        }
        {
            CSB_V1_BootStartupLaunch_PC34 launch;
            CSB_V1_BootStartupRuntimeReceipt_PC34 runtime_receipt;
            memset(&launch, 0, sizeof(launch));
            memset(&runtime_receipt, 0xff, sizeof(runtime_receipt));
            CHECK(csb_v1_boot_startup_launch_detach_runtime_pc34(
                      NULL,
                      &runtime_receipt) == 0 &&
                      runtime_receipt.profile == NULL,
                  "boot runtime detach rejects NULL CSB launch and clears receipt");
            launch.profile =
                (CSB_V1_BootProfile *)calloc(1, sizeof(*launch.profile));
            CHECK(launch.profile != NULL,
                  "boot runtime detach fixture allocates CSB profile");
            if (launch.profile) {
                csb_v1_boot_profile_init(launch.profile);
                snprintf(launch.profile->asset_root,
                         sizeof(launch.profile->asset_root),
                         "/tmp");
                snprintf(launch.profile->graphics_md5,
                         sizeof(launch.profile->graphics_md5),
                         "61fbfd56887c8bfe85ba4fb306fc2861");
                snprintf(launch.profile->dungeon_md5,
                         sizeof(launch.profile->dungeon_md5),
                         "6695d2acebce49f95db1d8f3a5c733de");
                snprintf(launch.profile->graphics_path,
                         sizeof(launch.profile->graphics_path),
                         "/tmp/firestaff_csb_GRAPHICS.DAT");
                snprintf(launch.profile->dungeon_path,
                         sizeof(launch.profile->dungeon_path),
                         "/tmp/firestaff_csb_DUNGEON.DAT");
                launch.profile->assets_verified = 1;
                launch.profile->graphics_verified = 1;
                launch.profile->dungeon_verified = 1;
                launch.profile->variant_id = CSB_V1_VARIANT_PC34_EN;
                launch.profile->graphics_kind =
                    CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
                launch.receipts.launch_host_receipt.status_scope = "BOOT";
                launch.receipts.launch_host_receipt.status = "CSB ENTRANCE";
                launch.receipts.launch_host_receipt.log_color = 11U;
                launch.receipts.init_state.command_state.entrance_active = 1;
                launch.receipts.session_state.entrance_resume_available = 1;
                memset(&runtime_receipt, 0, sizeof(runtime_receipt));
                CHECK(csb_v1_boot_startup_launch_detach_runtime_pc34(
                          &launch,
                          &runtime_receipt) == 1 &&
                          runtime_receipt.profile != NULL &&
                          strcmp(runtime_receipt.boot_asset_md5,
                                 "61fbfd56887c8bfe85ba4fb306fc2861") == 0 &&
                          strcmp(runtime_receipt.title,
                                 "CHAOS STRIKES BACK") == 0 &&
                          strcmp(runtime_receipt.source_id,
                                 "csb") == 0 &&
                          runtime_receipt.bind_graphics_to_m11_asset_loader ==
                              1 &&
                          runtime_receipt.load_original_font_from_graphics ==
                              1 &&
                          strcmp(runtime_receipt.graphics_path,
                                 "/tmp/firestaff_csb_GRAPHICS.DAT") == 0 &&
                          strcmp(runtime_receipt.dungeon_path,
                                 "/tmp/firestaff_csb_DUNGEON.DAT") == 0 &&
                          runtime_receipt.real_asset_receipt_valid &&
                          runtime_receipt.real_asset_receipt.matched &&
                          strcmp(runtime_receipt.real_asset_receipt.graphics_path,
                                 "/tmp/firestaff_csb_GRAPHICS.DAT") == 0 &&
                          strcmp(runtime_receipt.real_asset_receipt.dungeon_md5,
                                 "6695d2acebce49f95db1d8f3a5c733de") == 0 &&
                          runtime_receipt.real_asset_receipt.receipt_hash != 0u &&
                          runtime_receipt.receipts.init_state.command_state.entrance_active &&
                          runtime_receipt.receipts.session_state.entrance_resume_available &&
                          runtime_receipt.receipts.launch_host_receipt.status &&
                          strcmp(runtime_receipt.receipts.launch_host_receipt.status,
                                 "CSB ENTRANCE") == 0 &&
                          launch.profile == NULL,
                      "boot runtime detach transfers CSB profile, M11 identity, and launch receipts");
                csb_v1_boot_cleanup(runtime_receipt.profile);
                free(runtime_receipt.profile);
            }
        }
        {
            CSB_V1_BootStartupLaunch_PC34 launch;
            CHECK(csb_v1_boot_startup_launch_alloc_pc34(
                      "/__firestaff_missing_csb_data__",
                      NULL,
                      NULL,
                      NULL,
                      &launch) == 0 &&
                      launch.profile == NULL &&
                      launch.failure_host_receipt.status_scope &&
                      strcmp(launch.failure_host_receipt.status_scope,
                             "BOOT") == 0 &&
                      launch.failure_host_receipt.status &&
                      strcmp(launch.failure_host_receipt.status,
                             "CSB ASSETS MISSING") == 0 &&
                      launch.failure_host_receipt.log_color == 8U &&
                      launch.failure_host_receipt.log_line &&
                      strcmp(launch.failure_host_receipt.log_line,
                             "CSB ASSETS MISSING") == 0,
                  "boot launch allocation owns missing-asset host failure and log");
            csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        }
        csb_v1_boot_cleanup(&boot);
        receipt.direct_resume_loaded = 1;
        CHECK(csb_v1_runtime_build_startup_session_options_pc34(
                  &runtime,
                  &receipt,
                  path,
                  path,
                  &options) == 1 &&
                  !options.import_available &&
                  !options.entrance_resume_available,
              "runtime session options suppress choices after direct resume");
    }
    csb_v1_runtime_cleanup(&runtime);
    remove(path);
}

static void test_runtime_view_state_receipt_owns_scalar_handoff(void)
{
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_RuntimeViewStateReceipt_PC34 receipt;
    CSB_V1_DungeonData dummy_dungeon;

    csb_v1_runtime_init(&runtime, NULL);
    memset(&dummy_dungeon, 0, sizeof(dummy_dungeon));
    runtime.dungeon_handle = &dummy_dungeon;
    runtime.current_level = 6;
    runtime.party_x = 12;
    runtime.party_y = 13;
    runtime.party_dir = 2;
    runtime.tick_count = 77;

    CHECK(csb_v1_runtime_view_state_receipt_from_profile_pc34(
              &runtime,
              &receipt) == 1,
          "runtime builds scalar view-state receipt for M11");
    CHECK(receipt.level_loaded == 1 &&
              receipt.current_level == 6 &&
              receipt.party_x == 12 &&
              receipt.party_y == 13 &&
              receipt.party_dir == 2 &&
              receipt.tick_count == 77,
          "runtime view-state receipt mirrors loaded level, pose and tick");
    runtime.dungeon_handle = NULL;
    csb_v1_runtime_cleanup(&runtime);
}

static void test_runtime_utility_startup_host_facts_wrappers(void)
{
    CSB_V1_BootProfile boot;
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_UtilRenderPlan plan;
    CSB_V1_UtilApplyReceipt receipt;
    CSB_V1_UtilStateReceipt state_receipt;
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 action_receipt;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupActionReceipt_PC34 boot_action_receipt;
    CSB_V1_BootStartupInputRenderReceipt_PC34 input_render_receipt;
    CSB_V1_BootStartupHostDecisionReceipt_PC34 host_decision;
    CSB_V1_UtilRenderPlan receipt_utility_plan;
    CSB_V1_StartupPresentationReceipt_PC34 presentation_receipt;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 route_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 view_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 poisoned_view_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 runtime_view_receipt;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness_receipt;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_draw_receipt;
    CSB_V1_BootStartupInputGateReceipt_PC34 input_gate_receipt;
    CSB_V1_BootStartupCaptureReceipt_PC34 capture_receipt;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 packaged_proof;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 packaged_proof_from_snapshot;
    CSB_V1_BootStartupHostViewReceipt_PC34 host_view_receipt;
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 host_view_draw_receipt;
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 host_input_dispatch;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 host_ownership;
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 visual_sequence;
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 runtime_visual;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 route_hardening;
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 runtime_host_gate;
    CSB_V1_StartupRenderExecutor_PC34 hud_draw_executor;
    CSB_V1_StartupRenderExecutor_PC34 capture_render_executor;
    TestHudMenuDrawProbe hud_draw_probe;
    TestStartupRenderProbe capture_render_probe;
    CSB_V1_StartupRenderPlan_PC34 receipt_title_plan;
    CSB_V1_StartupRenderPlan_PC34 receipt_closed_door_plan;
    CSB_V1_StartupRenderPlan_PC34 runtime_render_plan;
    int packaged_title_ok;
    int enter_menu_x = 244;
    int enter_menu_y = 45;
    const char *resume_path = "/tmp/firestaff-csb-resume.dat";

    csb_v1_boot_profile_init(&boot);
    snprintf(boot.asset_root, sizeof(boot.asset_root), "/tmp");
    snprintf(boot.graphics_path, sizeof(boot.graphics_path),
             "/tmp/firestaff_csb_GRAPHICS.DAT");
    snprintf(boot.dungeon_path, sizeof(boot.dungeon_path),
             "/tmp/firestaff_csb_DUNGEON.DAT");
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5),
             "61fbfd56887c8bfe85ba4fb306fc2861");
    snprintf(boot.dungeon_md5, sizeof(boot.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de");
    boot.assets_verified = 1;
    boot.graphics_verified = 1;
    boot.dungeon_verified = 1;
    boot.variant_id = CSB_V1_VARIANT_PC34_EN;
    boot.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    CHECK(csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
              &boot,
              &visual_sequence) == 1 &&
              visual_sequence.valid &&
              visual_sequence.real_asset_matched &&
              visual_sequence.sequence_capture_hash != 0u &&
              visual_sequence.title_sample_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              visual_sequence.title_all_stages_captured &&
              visual_sequence.title_presents_capture_ready &&
              visual_sequence.title_chaos_zoom_capture_ready &&
              visual_sequence.title_chaos_hold_capture_ready &&
              visual_sequence.title_strikes_back_capture_ready &&
              visual_sequence.closed_door_hud_capture_ready &&
              visual_sequence.utility_hud_capture_ready &&
              visual_sequence.door_opening_delay_capture_ready &&
              visual_sequence.door_opening_frame_capture_ready &&
              visual_sequence.credits_capture_ready &&
              visual_sequence.hud_menu_draw_available &&
              visual_sequence.opening_frame_draw_available &&
              visual_sequence.no_fallback_text_routes &&
              visual_sequence.no_legacy_door_fallback_routes &&
              visual_sequence.source_title_presents_ticks == 60 &&
              visual_sequence.source_title_chaos_zoom_ticks == 18 &&
              visual_sequence.source_title_chaos_hold_ticks == 2 &&
              visual_sequence.source_title_strikes_back_ticks == 1 &&
              visual_sequence.source_door_pre_open_delay_ticks == 20 &&
              visual_sequence.source_door_step_count == 31 &&
              visual_sequence.title_sample_hashes[0] != 0u &&
              visual_sequence.title_sample_hashes[1] != 0u &&
              visual_sequence.title_sample_hashes[2] != 0u &&
              visual_sequence.title_sample_hashes[3] != 0u &&
              visual_sequence.closed_door_hud_hash != 0u &&
              visual_sequence.utility_hud_hash != 0u &&
              visual_sequence.door_opening_delay_hash != 0u &&
              visual_sequence.door_opening_frame_hash != 0u &&
              visual_sequence.credits_hash != 0u &&
              strstr(visual_sequence.source_evidence, "TITLE.C F0437") != NULL,
          "boot startup visual sequence capture covers title, HUD, credits and door opening without fallback routes");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_runtime_visual_capture_receipt_from_profile_pc34(
              &boot,
              &capture_render_executor,
              &runtime_visual) == 1 &&
              runtime_visual.valid &&
              runtime_visual.visual_sequence_valid &&
              runtime_visual.real_asset_matched &&
              runtime_visual.sequence_capture_hash ==
                  visual_sequence.sequence_capture_hash &&
              runtime_visual.runtime_capture_hash != 0u &&
              runtime_visual.title_runtime_consumed &&
              runtime_visual.closed_door_hud_runtime_consumed &&
              runtime_visual.utility_hud_runtime_consumed &&
              runtime_visual.door_opening_delay_runtime_consumed &&
              runtime_visual.door_opening_frame_runtime_consumed &&
              runtime_visual.credits_runtime_consumed &&
              runtime_visual.title_draw_consumed &&
              runtime_visual.closed_door_hud_draw_consumed &&
              runtime_visual.utility_hud_draw_consumed &&
              runtime_visual.door_opening_frame_draw_consumed &&
              runtime_visual.credits_surface_draw_consumed &&
              runtime_visual.no_fallback_callbacks &&
              runtime_visual.no_wrapper_fallback_routes &&
              runtime_visual.draw_consumes_receipt_only &&
              runtime_visual.input_consumes_receipt_only &&
              capture_render_probe.draw_title_count >= 1 &&
              capture_render_probe.draw_full_surface_count >= 4 &&
              capture_render_probe.draw_opening_frame_count >= 1 &&
              capture_render_probe.draw_fallback_text_count == 0 &&
              capture_render_probe.draw_door_fallback_count == 0 &&
              strstr(runtime_visual.source_evidence, "CSBWin") != NULL,
          "boot startup runtime visual capture consumes title, HUD and door-opening through host executor without fallback callbacks");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_runtime_host_capture_gate_receipt_from_profile_pc34(
              &boot,
              &capture_render_executor,
              &runtime_host_gate) == 1 &&
              runtime_host_gate.valid &&
              runtime_host_gate.runtime_visual_valid &&
              runtime_host_gate.visual_sequence_valid &&
              runtime_host_gate.route_hardening_valid &&
              runtime_host_gate.all_runtime_routes_consumed &&
              runtime_host_gate.title_runtime_captured &&
              runtime_host_gate.closed_door_hud_runtime_captured &&
              runtime_host_gate.utility_hud_runtime_captured &&
              runtime_host_gate.door_opening_runtime_captured &&
              runtime_host_gate.credits_runtime_captured &&
              runtime_host_gate.draw_consumes_receipt_only &&
              runtime_host_gate.input_consumes_receipt_only &&
              runtime_host_gate.no_fallback_callbacks &&
              runtime_host_gate.no_wrapper_fallback_routes &&
              runtime_host_gate.sequence_capture_hash ==
                  visual_sequence.sequence_capture_hash &&
              runtime_host_gate.runtime_capture_hash != 0u &&
              runtime_host_gate.route_hardening_hash != 0u &&
              runtime_host_gate.runtime_host_gate_hash != 0u &&
              runtime_host_gate.title_route_hardening.title_route_covered &&
              runtime_host_gate.closed_door_route_hardening
                  .closed_door_hud_route_covered &&
              runtime_host_gate.utility_route_hardening
                  .utility_hud_route_covered &&
              runtime_host_gate.door_opening_route_hardening
                  .door_opening_route_covered &&
              capture_render_probe.draw_fallback_text_count == 0 &&
              capture_render_probe.draw_door_fallback_count == 0,
          "boot startup runtime host gate binds full capture to route hardening without wrapper fallbacks");
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &boot;
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.pending_command = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    snapshot.resume_available = 1;
    snapshot.resume_path = resume_path;
    snapshot.title_active = 1;
    snapshot.title_frame = 0;
    snapshot.title_source_step = 1;
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
              &snapshot,
              0,
              0,
              &capture_render_executor,
              &host_ownership) == 1 &&
              csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
                  &visual_sequence,
                  &host_ownership,
                  &route_hardening) == 1 &&
              route_hardening.valid &&
              route_hardening.title_route_covered &&
              route_hardening.no_fallback_text_route &&
              route_hardening.no_legacy_door_fallback_route &&
              route_hardening.host_draw_consumes_receipt_only &&
              route_hardening.route_hardening_hash != 0u,
          "boot startup route hardening accepts only full-captured title ownership");
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &boot;
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.pending_command = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    snapshot.resume_available = 1;
    snapshot.resume_path = resume_path;
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.utility_prompt = "CHAOS STRIKES BACK READY";
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
              &snapshot,
              0,
              0,
              &capture_render_executor,
              &host_ownership) == 1 &&
              csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
                  &visual_sequence,
                  &host_ownership,
                  &route_hardening) == 1 &&
              route_hardening.valid &&
              route_hardening.utility_hud_route_covered &&
              route_hardening.no_fallback_text_route &&
              capture_render_probe.draw_fallback_text_count == 0,
          "boot startup route hardening accepts only full-captured utility HUD ownership");
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &boot;
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.resume_available = 1;
    snapshot.resume_path = resume_path;
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 3;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
              &snapshot,
              0,
              0,
              &capture_render_executor,
              &host_ownership) == 1 &&
              csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
                  &visual_sequence,
                  &host_ownership,
                  &route_hardening) == 1 &&
              route_hardening.valid &&
              route_hardening.door_opening_route_covered &&
              route_hardening.no_legacy_door_fallback_route &&
              capture_render_probe.draw_door_fallback_count == 0,
          "boot startup route hardening accepts only full-captured door-opening ownership");
    memset(&facts, 0, sizeof(facts));
    facts.boot_profile = &boot;
    facts.utility_overlay_active = 1;
    facts.utility_selected_action_index = 0;
    facts.utility_imported_champion_count = 2;
    facts.utility_preview_active = 0;
    facts.utility_prompt = "CHAOS STRIKES BACK READY";

    CHECK(csb_v1_runtime_util_render_plan_from_startup_host_facts_pc34(
              &facts,
              &plan) == 1,
          "runtime utility render wrapper accepts startup host facts");
    CHECK(plan.menu_row_count == CSB_V1_UTIL_MENU_ROW_COUNT &&
              strstr(plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL,
          "runtime utility render wrapper owns M11 utility facts");

    CHECK(csb_v1_runtime_util_apply_firestaff_input_from_startup_host_facts_pc34(
              &facts,
              2,
              &receipt,
              &state_receipt) == 1,
          "runtime utility keyboard wrapper accepts startup host facts");
    CHECK(state_receipt.selected_action_index == 1 &&
              receipt.result == CSB_V1_UTIL_APPLY_REDRAW,
          "runtime utility keyboard wrapper owns M11 utility facts");

    CHECK(csb_v1_runtime_util_apply_point_from_startup_host_facts_pc34(
              &facts,
              72,
              126,
              &receipt,
              &state_receipt) == 1,
          "runtime utility pointer wrapper accepts startup host facts");
    CHECK(state_receipt.selected_action_index >= 0,
          "runtime utility pointer wrapper owns M11 utility facts");

    facts.utility_selected_action_index = 0;
    CHECK(csb_v1_runtime_util_apply_firestaff_input_from_startup_host_facts_with_action_receipt_pc34(
              &facts,
              2,
              &action_receipt) == 1,
          "runtime utility keyboard action wrapper accepts startup host facts");
    CHECK(action_receipt.util_state_receipt.selected_action_index == 1 &&
              action_receipt.util_receipt.result == CSB_V1_UTIL_APPLY_REDRAW &&
              !action_receipt.entrance_receipt_valid,
          "runtime utility keyboard action wrapper owns redraw receipt");

    facts.utility_selected_action_index = 1;
    CHECK(csb_v1_runtime_util_apply_firestaff_input_from_startup_host_facts_with_action_receipt_pc34(
              &facts,
              9,
              &action_receipt) == 1,
          "runtime utility keyboard action wrapper handles accept");
    CHECK(action_receipt.util_receipt.result ==
              CSB_V1_UTIL_APPLY_ENTRANCE_COMMAND &&
              action_receipt.entrance_receipt_valid,
          "runtime utility keyboard action wrapper chains entrance receipt");

    CHECK(csb_v1_runtime_execute_startup_entrance_firestaff_input_from_host_facts_with_receipts_pc34(
              &facts,
              2,
              &entrance_receipt) == 1,
          "runtime entrance keyboard action wrapper accepts startup host facts");
    CHECK(!entrance_receipt.handled,
          "runtime entrance keyboard action wrapper ignores navigation input");

    CHECK(csb_v1_runtime_execute_startup_entrance_firestaff_input_from_host_facts_with_receipts_pc34(
              &facts,
              9,
              &entrance_receipt) == 1,
          "runtime entrance keyboard action wrapper handles accept input");
    CHECK(entrance_receipt.handled,
          "runtime entrance keyboard action wrapper chains command receipt");

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = 4;
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.utility_prompt = facts.utility_prompt;
    snapshot.resume_available = 1;
    snapshot.resume_path = resume_path;
    snapshot.boot_profile = &boot;
    snapshot.title_active = 1;
    snapshot.title_frame = 0;
    snapshot.title_source_step = 1;
    CHECK(csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
              &snapshot,
              &route_receipt) == 1,
          "boot startup route receipt accepts title snapshot");
    CHECK(route_receipt.valid &&
              route_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              route_receipt.draw_title &&
              !route_receipt.draw_surface &&
              !route_receipt.hud_menu_visible &&
              strcmp(route_receipt.presentation.animation, "csb-title") == 0,
          "boot startup route receipt owns title animation route");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.valid &&
              view_receipt.title_after_swoosh_route &&
              view_receipt.boot_executor_route &&
              view_receipt.render_plan_valid &&
              view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              view_receipt.route_receipt.draw_title &&
              !view_receipt.hud_menu_receipt_ready,
          "boot startup render-view receipt owns post-swoosh CSB title route");
    CHECK(csb_v1_boot_startup_readiness_receipt_from_view_pc34(
              &view_receipt,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              readiness_receipt.startup_active &&
              readiness_receipt.post_ftl_title_active &&
              !readiness_receipt.title_ready &&
              !readiness_receipt.input_ready &&
              !readiness_receipt.hud_menu_ready &&
              readiness_receipt.host_input_blocked &&
              !readiness_receipt.host_startup_input_ready &&
              !readiness_receipt.host_runtime_input_ready &&
              readiness_receipt.host_hud_blocked &&
              !readiness_receipt.host_startup_hud_ready &&
              !readiness_receipt.host_runtime_hud_ready &&
              strcmp(readiness_receipt.animation, "csb-title") == 0 &&
              readiness_receipt.title_presents_visible,
          "boot startup readiness receipt owns post-FTL title-not-ready gate");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.valid &&
              capture_receipt.route_valid &&
              capture_receipt.render_view_valid &&
              capture_receipt.readiness_valid &&
              !capture_receipt.hud_menu_draw_valid &&
              capture_receipt.real_asset_receipt_valid &&
              capture_receipt.real_asset_receipt.matched &&
              strcmp(capture_receipt.real_asset_receipt.graphics_path,
                     "/tmp/firestaff_csb_GRAPHICS.DAT") == 0 &&
              strcmp(capture_receipt.real_asset_receipt.dungeon_md5,
                     "6695d2acebce49f95db1d8f3a5c733de") == 0 &&
              capture_receipt.real_asset_receipt.receipt_hash != 0u &&
              capture_receipt.title_capture_ready &&
              !capture_receipt.hud_menu_capture_ready &&
              capture_receipt.host_input_blocked &&
              capture_receipt.host_hud_blocked &&
              !capture_receipt.startup_input_ready &&
              !capture_receipt.startup_hud_ready &&
              capture_receipt.render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              capture_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              capture_receipt.title_frame == 0 &&
              capture_receipt.title_source_step == 1,
          "boot startup capture receipt packages title block gates");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.valid &&
              host_view_receipt.render_draw.title_draw_ready &&
              host_view_receipt.render_draw.real_asset_matched &&
              host_view_receipt.render_draw.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              host_view_receipt.render_draw.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              host_view_receipt.render_draw.render_plan.asset_command_count == 1 &&
              host_view_receipt.render_draw.render_plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34 &&
              host_view_receipt.render_draw.render_plan.render_command_count == 2 &&
              host_view_receipt.render_draw.render_plan.source_asset_id == 1,
          "boot startup host-view render-draw receipt consumes real-asset title route");
    CHECK(host_view_receipt.render_plan_valid &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              host_view_receipt.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              host_view_receipt.render_plan.asset_command_count == 1 &&
              host_view_receipt.render_plan.render_command_count == 2,
          "boot startup host-view receipt returns full title render plan");
    CHECK(route_receipt.presentation.redmcsb_source_locked &&
              route_receipt.presentation.redmcsb_title_graphic_id == 1 &&
              route_receipt.presentation.redmcsb_presents_ticks == 60 &&
              route_receipt.presentation.redmcsb_chaos_zoom_ticks == 18 &&
              route_receipt.presentation.redmcsb_chaos_hold_ticks == 2 &&
              route_receipt.presentation.redmcsb_strikes_back_ticks == 1 &&
              route_receipt.presentation.redmcsb_entrance_screen_graphic_id ==
                  4 &&
              route_receipt.presentation.redmcsb_credits_graphic_id == 5 &&
              strstr(route_receipt.presentation.redmcsb_title_source,
                     "TITLE.C F0437") != NULL,
          "boot startup presentation receipt carries ReDMCSB title source lock");
    packaged_title_ok =
        csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
            &capture_receipt,
            &packaged_proof) == 1 &&
        packaged_proof.valid &&
        packaged_proof.capture_valid &&
        packaged_proof.real_asset_matched &&
        packaged_proof.real_asset_receipt_hash ==
            capture_receipt.real_asset_receipt.receipt_hash &&
        packaged_proof.packaged_capture_hash != 0u &&
        packaged_proof.route == capture_receipt.render_route &&
        packaged_proof.title_capture_ready ==
            capture_receipt.title_capture_ready &&
        packaged_proof.hud_menu_capture_ready ==
            capture_receipt.hud_menu_capture_ready &&
        packaged_proof.runtime_capture_ready ==
            capture_receipt.runtime_capture_ready &&
        packaged_proof.render_plan_available &&
        !packaged_proof.hud_menu_draw_available &&
        packaged_proof.boot_executor_route &&
        packaged_proof.title_route &&
        !packaged_proof.closed_door_menu_route &&
        !packaged_proof.utility_menu_route &&
        !packaged_proof.opening_door_route &&
        packaged_proof.host_input_blocked &&
        packaged_proof.host_hud_blocked &&
        !packaged_proof.startup_input_ready &&
        !packaged_proof.startup_hud_ready &&
        packaged_proof.title_stage == capture_receipt.title_stage &&
        packaged_proof.title_frame == capture_receipt.title_frame &&
        packaged_proof.selected_command_id ==
            capture_receipt.selected_command_id;
    CHECK(packaged_title_ok,
          "boot startup packaged proof binds title capture route, real assets, and render plan");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
              &snapshot,
              &packaged_proof_from_snapshot) == 1 &&
              packaged_proof_from_snapshot.valid &&
              packaged_proof_from_snapshot.packaged_capture_hash ==
                  packaged_proof.packaged_capture_hash &&
              packaged_proof_from_snapshot.real_asset_receipt_hash ==
                  packaged_proof.real_asset_receipt_hash,
          "boot startup packaged proof snapshot wrapper is deterministic");
    CHECK(packaged_title_ok &&
              strstr(packaged_proof.source_evidence, "TITLE.C") != NULL,
          "boot startup packaged capture proof binds title and real assets");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.startup_active &&
              host_view_receipt.phase[0] != '\0' &&
              strcmp(host_view_receipt.animation, "csb-title") == 0 &&
              host_view_receipt.title_frame == 0 &&
              !host_view_receipt.title_ready &&
              !host_view_receipt.startup_input_ready &&
              !host_view_receipt.startup_hud_menu_ready &&
              host_view_receipt.capture_proof_valid &&
              host_view_receipt.capture_proof.title_route &&
              host_view_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              host_view_receipt.special_palette >= 0 &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.title_draw_ready &&
              host_view_receipt.readiness_valid &&
              host_view_receipt.readiness.host_input_blocked &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34,
          "boot startup host-view receipt consumes title render-draw capture proof");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              capture_render_probe.clear_black_count == 1 &&
              capture_render_probe.draw_title_count == 1 &&
              capture_render_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34,
          "boot startup host-view draw receipt executes title render");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.valid &&
              host_view_draw_receipt.consumed_host_view_only &&
              host_view_draw_receipt.render_executed &&
              !host_view_draw_receipt.hud_menu_executed &&
              host_view_draw_receipt.real_asset_matched &&
              host_view_draw_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              host_view_draw_receipt.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              capture_render_probe.clear_black_count == 1 &&
              capture_render_probe.draw_title_count == 1,
          "boot startup host-view draw receipt consumes title render without legacy plan/HUD split");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.capture_proof.title_route &&
              csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
                  &snapshot,
                  &packaged_proof_from_snapshot) == 1 &&
              packaged_proof_from_snapshot.packaged_capture_hash ==
                  packaged_proof.packaged_capture_hash,
          "boot startup snapshot host-view consumes title capture proof");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_runtime_state_pc34(
              &runtime_view_receipt,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 1 &&
              runtime_view_receipt.title_after_swoosh_route &&
              runtime_view_receipt.render_plan.surface ==
                  view_receipt.render_plan.surface,
          "boot startup runtime-state render-view receipt matches post-swoosh title route");
    CHECK(view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              view_receipt.title_source_step ==
                  CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              view_receipt.title_frame == 0 &&
              view_receipt.title_frame_max ==
                  csb_v1_startup_title_total_ticks_pc34() &&
              view_receipt.title_presents_visible &&
              view_receipt.title_phase_tick == 0 &&
              view_receipt.title_phase_tick_count ==
                  csb_v1_startup_title_presents_ticks_pc34() &&
              view_receipt.title_render_command_count == 2 &&
              view_receipt.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 &&
              view_receipt.title_transparent_color == -1 &&
              view_receipt.title_source_x == 0 &&
              view_receipt.title_source_y == 137 &&
              view_receipt.title_source_w == 320 &&
              view_receipt.title_source_h == 16 &&
              view_receipt.title_dest_x == 0 &&
              view_receipt.title_dest_y == 90 &&
              view_receipt.title_dest_w == 320 &&
              view_receipt.title_dest_h == 16 &&
              !view_receipt.title_chaos_visible &&
              !view_receipt.title_chaos_zoom_visible &&
              !view_receipt.title_chaos_hold_visible &&
              !view_receipt.title_strikes_back_visible,
          "boot startup render-view receipt exposes source PRESENTS title render route");
    snapshot.title_frame = 60;
    snapshot.title_source_step = 2;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              view_receipt.title_source_step == 2 &&
              view_receipt.title_render_command_count == 2 &&
              view_receipt.title_phase_tick == 0 &&
              view_receipt.title_phase_tick_count ==
                  csb_v1_startup_title_chaos_zoom_ticks_pc34() &&
              view_receipt.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34 &&
              ((view_receipt.title_source_x == 136 &&
                view_receipt.title_source_y == 74 &&
                view_receipt.title_source_w == 48 &&
                view_receipt.title_source_h == 12 &&
                view_receipt.title_dest_x == 0 &&
                view_receipt.title_dest_y == 0 &&
                view_receipt.title_dest_w == 320 &&
                view_receipt.title_dest_h == 80) ||
               (view_receipt.title_source_x == 0 &&
                view_receipt.title_source_y == 0 &&
                view_receipt.title_source_w == 320 &&
                view_receipt.title_source_h == 80 &&
                view_receipt.title_dest_x == 136 &&
                view_receipt.title_dest_y == 74 &&
                view_receipt.title_dest_w == 48 &&
                view_receipt.title_dest_h == 12)) &&
              view_receipt.title_chaos_visible &&
              view_receipt.title_chaos_zoom_visible &&
              !view_receipt.title_chaos_hold_visible &&
              !view_receipt.title_presents_visible &&
              !view_receipt.title_strikes_back_visible,
          "boot startup render-view receipt exposes source CHAOS title render route");
    poisoned_view_receipt = view_receipt;
    poisoned_view_receipt.render_plan.asset_commands[0].source_y = 1;
    poisoned_view_receipt.render_plan.asset_commands[0].dest_h = 1;
    poisoned_view_receipt.render_plan.title_source_y = 1;
    poisoned_view_receipt.render_plan.title_dest_h = 1;
    capture_receipt.render_view = poisoned_view_receipt;
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.render_plan_valid &&
              (receipt_title_plan =
                   host_view_receipt.render_draw.render_plan,
               1) &&
              receipt_title_plan.asset_command_count == 1 &&
              receipt_title_plan.asset_commands[0].kind ==
                  CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34 &&
              ((receipt_title_plan.asset_commands[0].source_y == 74 &&
                receipt_title_plan.asset_commands[0].dest_h == 80 &&
                receipt_title_plan.title_source_y == 74 &&
                receipt_title_plan.title_dest_h == 80) ||
               (receipt_title_plan.asset_commands[0].source_y == 0 &&
                receipt_title_plan.asset_commands[0].dest_h == 12 &&
                receipt_title_plan.title_source_y == 0 &&
                receipt_title_plan.title_dest_h == 12)) &&
              receipt_title_plan.render_command_count == 2 &&
              receipt_title_plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              receipt_title_plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34,
          "boot startup host-view title draw consumes render-view receipt fields");
    snapshot.title_frame = 78;
    snapshot.title_source_step = 19;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              view_receipt.title_source_step == 19 &&
              view_receipt.title_chaos_visible &&
              !view_receipt.title_chaos_zoom_visible &&
              view_receipt.title_chaos_hold_visible &&
              view_receipt.title_phase_tick == 0 &&
              view_receipt.title_phase_tick_count ==
                  csb_v1_startup_title_chaos_hold_ticks_pc34() &&
              view_receipt.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34 &&
              view_receipt.title_source_w == 320 &&
              view_receipt.title_source_h == 80,
          "boot startup render-view receipt exposes source CHAOS hold timing");
    snapshot.title_frame = 80;
    snapshot.title_source_step = 20;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              view_receipt.title_render_command_count == 2 &&
              view_receipt.title_phase_tick == 0 &&
              view_receipt.title_phase_tick_count ==
                  csb_v1_startup_title_strikes_back_ticks_pc34() &&
              view_receipt.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 &&
              view_receipt.title_transparent_color == 0 &&
              view_receipt.title_source_x == 0 &&
              view_receipt.title_source_y == 80 &&
              view_receipt.title_source_w == 320 &&
              view_receipt.title_source_h == 57 &&
              view_receipt.title_dest_x == 0 &&
              view_receipt.title_dest_y == 118 &&
              view_receipt.title_dest_w == 320 &&
              view_receipt.title_dest_h == 57 &&
              view_receipt.title_strikes_back_visible &&
              !view_receipt.title_presents_visible &&
              !view_receipt.title_chaos_visible,
          "boot startup render-view receipt exposes source STRIKES BACK title render route");
    snapshot.title_frame = 0;
    snapshot.title_source_step = 1;
    snapshot.utility_overlay_active = 0;
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              9,
              &boot_action_receipt) == 0 &&
              boot_action_receipt.pre_input_route.valid &&
              boot_action_receipt.pre_input_route.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              boot_action_receipt.pre_input_route.draw_title &&
              boot_action_receipt.pre_input_render_view_valid &&
              boot_action_receipt.pre_input_render_view
                  .title_after_swoosh_route &&
              boot_action_receipt.pre_input_render_view
                  .title_presents_visible &&
              !boot_action_receipt.post_input_render_view_valid &&
              boot_action_receipt.menu_input == 9 &&
              boot_action_receipt.startup_input ==
                  CSB_V1_STARTUP_INPUT_ACCEPT_PC34 &&
              boot_action_receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              boot_action_receipt.input_blocked_by_title &&
              !boot_action_receipt.pre_input_route.hud_menu_state.valid,
          "boot startup action receipt captures title render-view before blocked input");
    CHECK(csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
              &boot_action_receipt,
              &host_decision) == 1 &&
              host_decision.valid &&
              host_decision.consumed_input &&
              host_decision.blocked_by_title &&
              !host_decision.redraw_startup &&
              host_decision.pre_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34,
          "boot startup host decision consumes title-block receipt");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
              &snapshot,
              9,
              &input_render_receipt) == 1 &&
              input_render_receipt.valid &&
              input_render_receipt.action_valid &&
              input_render_receipt.host_decision_valid &&
              input_render_receipt.pre_input_readiness_valid &&
              !input_render_receipt.post_input_readiness_valid &&
              !input_render_receipt.hud_menu_draw_valid &&
              input_render_receipt.input_consumed &&
              !input_render_receipt.startup_redraw &&
              !input_render_receipt.startup_hud_draw_ready &&
              input_render_receipt.host_decision.blocked_by_title &&
              input_render_receipt.pre_input_readiness.post_ftl_title_active,
          "boot startup input/render receipt owns title input block");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
              &snapshot,
              9,
              &input_gate_receipt) == 1 &&
              input_gate_receipt.valid &&
              input_gate_receipt.startup_active &&
              !input_gate_receipt.startup_input_ready &&
              input_gate_receipt.host_input_blocked &&
              !input_gate_receipt.should_dispatch_input &&
              input_gate_receipt.should_ignore_input &&
              input_gate_receipt.input_render_valid &&
              input_gate_receipt.input_render.host_decision.blocked_by_title,
          "boot startup input gate owns title-block decision");
    CHECK(csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
              &snapshot,
              9,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.startup_active &&
              !host_input_dispatch.startup_input_ready &&
              host_input_dispatch.host_input_blocked &&
              !host_input_dispatch.should_dispatch_input &&
              host_input_dispatch.should_ignore_input &&
              host_input_dispatch.input_render_valid &&
              host_input_dispatch.input_render.host_decision.blocked_by_title,
          "boot startup host input dispatch receipt owns title-block snapshot");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
              &snapshot,
              1,
              9,
              &capture_render_executor,
              &host_ownership) == 1 &&
              host_ownership.valid &&
              host_ownership.packaged_visual_capture_ready &&
              host_ownership.title_capture_ready &&
              host_ownership.title_draw_ready &&
              host_ownership.render_executed &&
              host_ownership.draw_consumes_receipt_only &&
              host_ownership.host_input_dispatch_valid &&
              host_ownership.host_input_blocked &&
              host_ownership.should_ignore_input &&
              host_ownership.input_consumes_receipt_only &&
              capture_render_probe.draw_title_count == 1,
          "boot startup host ownership receipt binds title capture, draw, and blocked input");
    snapshot.boot_profile = NULL;
    CHECK(csb_v1_boot_startup_render_plan_from_runtime_state_pc34(
              &runtime_render_plan,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 0 &&
              runtime_render_plan.surface == CSB_V1_STARTUP_RENDER_NONE_PC34,
          "boot startup runtime-state render-plan facade rejects unverified raw title route");
    snapshot.boot_profile = &boot;
    snapshot.utility_overlay_active = 1;
    snapshot.title_active = 0;
    snapshot.title_frame = 0;
    snapshot.title_source_step = 0;
    CHECK(csb_v1_boot_startup_presentation_state_receipt_from_snapshot_pc34(
              &snapshot,
              &presentation_receipt) == 1,
          "boot startup presentation receipt facade accepts snapshot");
    CHECK(presentation_receipt.valid &&
              strcmp(presentation_receipt.phase, "csb-entrance-4") == 0 &&
              strcmp(presentation_receipt.animation, "csb-entrance") == 0 &&
              presentation_receipt.accepts_input &&
              presentation_receipt.waiting_for_input &&
              presentation_receipt.menu_option_count == 4 &&
              presentation_receipt.render_command_count == 5 &&
              presentation_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup presentation receipt owns render/menu/input snapshot");
    CHECK(presentation_receipt.redmcsb_source_locked &&
              presentation_receipt.redmcsb_left_door_graphic_id == 2 &&
              presentation_receipt.redmcsb_right_door_graphic_id == 3 &&
              presentation_receipt.redmcsb_pre_open_delay_ticks == 20 &&
              presentation_receipt.redmcsb_door_step_count == 31 &&
              presentation_receipt.redmcsb_closed_door_left_x == 0 &&
              presentation_receipt.redmcsb_closed_door_right_x == 128 &&
              presentation_receipt.redmcsb_closed_door_y == 28 &&
              presentation_receipt.redmcsb_closed_door_w == 128 &&
              presentation_receipt.redmcsb_closed_door_h == 161 &&
              strstr(presentation_receipt.redmcsb_entrance_source,
                     "ENTRANCE.C F0441") != NULL,
          "boot startup presentation receipt carries ReDMCSB entrance source lock");
    CHECK(csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
              &snapshot,
              &route_receipt) == 1,
          "boot startup route receipt accepts entrance snapshot");
    CHECK(route_receipt.valid &&
              route_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              route_receipt.draw_surface &&
              route_receipt.draw_closed_doors &&
              !route_receipt.draw_fallback_text &&
              route_receipt.draw_utility_panel &&
              route_receipt.hud_menu_visible &&
              route_receipt.menu_option_count == 4 &&
              route_receipt.utility_plan_valid &&
              route_receipt.utility_plan.menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              strstr(route_receipt.utility_plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL &&
              route_receipt.hud_menu_state.valid &&
              route_receipt.hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              route_receipt.hud_menu_state.utility_selected_action_index == 0 &&
              route_receipt.hud_menu_state.utility_menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              strstr(route_receipt.hud_menu_state.prompt,
                     "CHAOS STRIKES BACK READY") != NULL &&
              route_receipt.accepts_input,
          "boot startup route receipt owns closed entrance utility HUD/menu plan route");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.utility_menu_route &&
              view_receipt.utility_menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              view_receipt.utility_selected_action_index == 0 &&
              strstr(view_receipt.utility_prompt,
                     "CHAOS STRIKES BACK READY") != NULL,
          "boot startup render-view receipt owns utility HUD/menu route");
    CHECK(csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
              &snapshot,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              readiness_receipt.input_ready &&
              readiness_receipt.hud_menu_ready &&
              readiness_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              !readiness_receipt.host_input_blocked &&
              readiness_receipt.host_startup_input_ready &&
              !readiness_receipt.host_hud_blocked &&
              readiness_receipt.host_startup_hud_ready &&
              readiness_receipt.utility_menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              readiness_receipt.selected_utility_action_index == 0,
          "boot startup readiness receipt owns utility HUD/menu readiness");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.valid &&
              capture_receipt.route_valid &&
              capture_receipt.render_view_valid &&
              capture_receipt.readiness_valid &&
              capture_receipt.hud_menu_draw_valid &&
              !capture_receipt.title_capture_ready &&
              capture_receipt.hud_menu_capture_ready &&
              !capture_receipt.host_input_blocked &&
              !capture_receipt.host_hud_blocked &&
              capture_receipt.startup_input_ready &&
              capture_receipt.startup_hud_ready &&
              capture_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              capture_receipt.selected_utility_action_index == 0 &&
              capture_receipt.selected_utility_action_index ==
                  capture_receipt.hud_menu_draw
                      .selected_utility_action_index &&
              capture_receipt.hud_menu_draw.draw_utility_panel &&
              capture_receipt.hud_menu_draw.option_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              capture_receipt.hud_menu_draw.utility_render_plan_valid,
          "boot startup capture receipt packages utility HUD/menu draw");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              host_view_receipt.render_plan.waiting_for_input &&
              host_view_receipt.render_plan.menu_option_count == 4 &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.hud_menu_draw_ready &&
              host_view_receipt.hud_menu_draw_valid,
          "boot startup snapshot host-view consumes utility render-draw receipt");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.hud_menu_draw_ready &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.hud_menu_draw.draw_utility_panel &&
              host_view_receipt.hud_menu_draw.utility_render_plan_valid &&
              host_view_receipt.readiness_valid &&
              host_view_receipt.readiness.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_view_receipt.capture_proof.utility_menu_route,
          "boot startup host-view receipt packages utility HUD render-draw receipt");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              host_view_draw_receipt.hud_menu_executed == 1 &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_utility_panel_count == 1 &&
              capture_render_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup host-view draw receipt executes utility render");
    memset(&hud_draw_probe, 0, sizeof(hud_draw_probe));
    hud_probe_executor_init(&hud_draw_executor, &hud_draw_probe);
    CHECK(csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
              &snapshot,
              &readiness_receipt) == 1 &&
              csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
                  &host_view_receipt.hud_menu_draw,
                  &readiness_receipt,
                  &hud_draw_executor) == 1 &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.capture_proof.hud_menu_draw_available &&
              host_view_receipt.capture_proof.utility_menu_route &&
              hud_draw_probe.utility_panel_count == 1 &&
              hud_draw_probe.closed_doors_count == 0 &&
              hud_draw_probe.fallback_text_count == 0 &&
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup host-view receipt executes utility HUD/menu draw");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.valid &&
              host_view_draw_receipt.render_executed &&
              host_view_draw_receipt.hud_menu_executed == 1 &&
              host_view_draw_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_view_draw_receipt.consumed_host_view_only &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_utility_panel_count == 1 &&
              capture_render_probe.draw_closed_doors_count == 0,
          "boot startup host-view draw receipt consumes utility render plus HUD without legacy split");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
              &capture_receipt,
              &packaged_proof) == 1 &&
              packaged_proof.valid &&
              packaged_proof.hud_menu_capture_ready &&
              !packaged_proof.title_capture_ready &&
              !packaged_proof.runtime_capture_ready &&
              packaged_proof.render_plan_available &&
              packaged_proof.hud_menu_draw_available &&
              packaged_proof.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              !packaged_proof.title_route &&
              packaged_proof.utility_menu_route &&
              packaged_proof.closed_door_menu_route &&
              !packaged_proof.opening_door_route &&
              packaged_proof.draw_utility_panel &&
              !packaged_proof.draw_closed_doors &&
              packaged_proof.startup_input_ready &&
              packaged_proof.startup_hud_ready &&
              !packaged_proof.host_input_blocked &&
              !packaged_proof.host_hud_blocked &&
              packaged_proof.hud_menu_option_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              packaged_proof.utility_menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              packaged_proof.selected_utility_action_index == 0 &&
              packaged_proof.packaged_capture_hash != 0u,
          "boot startup packaged capture proof binds utility HUD/menu draw");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.startup_active &&
              host_view_receipt.startup_input_ready &&
              host_view_receipt.startup_hud_menu_ready &&
              host_view_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_view_receipt.hud_menu_option_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              host_view_receipt.selected_utility_action_index == 0 &&
              host_view_receipt.capture_proof_valid &&
              host_view_receipt.capture_proof.utility_menu_route,
          "boot startup host-view receipt consumes utility HUD/menu proof");
    poisoned_view_receipt = view_receipt;
    poisoned_view_receipt.route_receipt.utility_plan.menu_row_count = 1;
    poisoned_view_receipt.route_receipt.utility_plan.menu_rows[0].selected = 0;
    poisoned_view_receipt.route_receipt.utility_plan.menu_rows[1].selected = 1;
    poisoned_view_receipt.route_receipt.utility_plan.has_prompt_row = 0;
    poisoned_view_receipt.route_receipt.utility_plan.prompt_row.text[0] = '\0';
    poisoned_view_receipt.route_receipt.utility_plan.preview_active = 1;
    CHECK(csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
              &poisoned_view_receipt,
              &hud_draw_receipt) == 1 &&
              hud_draw_receipt.utility_render_plan_valid &&
              (receipt_utility_plan = hud_draw_receipt.utility_render_plan,
               1) &&
              receipt_utility_plan.menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              receipt_utility_plan.menu_rows[0].selected &&
              !receipt_utility_plan.menu_rows[1].selected &&
              receipt_utility_plan.has_prompt_row &&
              strstr(receipt_utility_plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL &&
              !receipt_utility_plan.preview_active,
          "boot startup utility HUD/menu receipt consumes render-view receipt fields");
    CHECK(csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
              &poisoned_view_receipt,
              &hud_draw_receipt) == 1 &&
              hud_draw_receipt.valid &&
              hud_draw_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              hud_draw_receipt.utility_render_plan_valid &&
              hud_draw_receipt.startup_render_plan_valid &&
              hud_draw_receipt.startup_render_plan.waiting_for_input &&
              hud_draw_receipt.draw_utility_panel &&
              hud_draw_receipt.option_count == CSB_V1_UTIL_MENU_ROW_COUNT &&
              hud_draw_receipt.selected_utility_action_index == 0 &&
              hud_draw_receipt.utility_render_plan.menu_rows[0].selected &&
              !hud_draw_receipt.utility_render_plan.menu_rows[1].selected &&
              strstr(hud_draw_receipt.prompt,
                     "CHAOS STRIKES BACK READY") != NULL,
          "boot startup HUD/menu draw receipt consumes utility render-view receipt");
    memset(&hud_draw_probe, 0, sizeof(hud_draw_probe));
    hud_probe_executor_init(&hud_draw_executor, &hud_draw_probe);
    CHECK(csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
              &hud_draw_receipt,
              &readiness_receipt,
              &hud_draw_executor) == 1 &&
              hud_draw_probe.utility_panel_count == 1 &&
              hud_draw_probe.closed_doors_count == 0 &&
              hud_draw_probe.fallback_text_count == 0 &&
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup HUD/menu executor draws utility panel from readiness-gated receipt");
    CHECK(csb_v1_boot_runtime_util_render_plan_from_snapshot_pc34(
              &snapshot,
              &receipt_utility_plan) == 1 &&
              receipt_utility_plan.menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              receipt_utility_plan.menu_rows[0].selected &&
              !receipt_utility_plan.menu_rows[1].selected &&
              receipt_utility_plan.has_prompt_row &&
              strstr(receipt_utility_plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL,
          "boot startup utility snapshot facade consumes render-view receipt");
    CHECK(csb_v1_boot_runtime_util_render_plan_from_runtime_state_pc34(
              &receipt_utility_plan,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 1 &&
              receipt_utility_plan.menu_rows[0].selected &&
              !receipt_utility_plan.preview_active,
          "boot startup utility runtime-state facade consumes render-view receipt");
    snapshot.utility_overlay_active = 0;
    CHECK(csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
              &snapshot,
              &route_receipt) == 1 &&
              route_receipt.hud_menu_state.valid &&
              route_receipt.hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              route_receipt.hud_menu_visible &&
              !route_receipt.draw_utility_panel &&
              !route_receipt.utility_plan_valid &&
              route_receipt.hud_menu_state.selected_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              route_receipt.hud_menu_state.option_count == 4 &&
              route_receipt.hud_menu_state.resume_option_visible &&
              route_receipt.hud_menu_state.resume_enabled &&
              route_receipt.hud_menu_state.resume_available &&
              !route_receipt.hud_menu_state.resume_option_selected &&
              strcmp(route_receipt.hud_menu_state.resume_path, resume_path) ==
                  0 &&
              strstr(route_receipt.hud_menu_state.prompt, "PRESS ENTER") !=
                  NULL,
          "boot startup route receipt owns closed entrance HUD/menu state and resume gate without utility fallback");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.closed_door_menu_route &&
              view_receipt.hud_menu_receipt_ready &&
              view_receipt.suppress_legacy_utility_fallback &&
              view_receipt.route_receipt.hud_menu_state.resume_available &&
              strcmp(view_receipt.route_receipt.hud_menu_state.resume_path,
                     resume_path) == 0,
          "boot startup render-view receipt owns closed-door HUD/menu and utility fallback gate");
    CHECK(csb_v1_boot_startup_readiness_receipt_from_view_pc34(
              &view_receipt,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              readiness_receipt.input_ready &&
              readiness_receipt.hud_menu_ready &&
              readiness_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              !readiness_receipt.host_input_blocked &&
              readiness_receipt.host_startup_input_ready &&
              !readiness_receipt.host_hud_blocked &&
              readiness_receipt.host_startup_hud_ready &&
              readiness_receipt.hud_menu_option_count == 4 &&
              readiness_receipt.selected_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              readiness_receipt.resume_available &&
              readiness_receipt.suppress_legacy_utility_fallback,
          "boot startup readiness receipt owns closed-door HUD/menu readiness");
    snapshot.entrance_active = 0;
    snapshot.entrance_source_step = 0;
    snapshot.entrance_dismissed = 1;
    snapshot.resume_available = 0;
    snapshot.resume_path = NULL;
    snapshot.runtime_level_loaded = 1;
    snapshot.runtime_map_index = 6;
    snapshot.runtime_party_x = 12;
    snapshot.runtime_party_y = 13;
    snapshot.runtime_party_dir = 2;
    snapshot.runtime_champion_count = 4;
    snapshot.runtime_tick_count = 77;
    CHECK(csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
              &snapshot,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              !readiness_receipt.startup_active &&
              readiness_receipt.title_ready &&
              readiness_receipt.runtime_handoff_ready &&
              readiness_receipt.runtime_viewport_ready &&
              readiness_receipt.runtime_hud_ready &&
              !readiness_receipt.host_input_blocked &&
              !readiness_receipt.host_startup_input_ready &&
              readiness_receipt.host_runtime_input_ready &&
              !readiness_receipt.host_hud_blocked &&
              !readiness_receipt.host_startup_hud_ready &&
              readiness_receipt.host_runtime_hud_ready &&
              readiness_receipt.runtime_level_loaded == 1 &&
              readiness_receipt.runtime_map_index == 6 &&
              readiness_receipt.runtime_party_x == 12 &&
              readiness_receipt.runtime_party_y == 13 &&
              readiness_receipt.runtime_party_dir == 2 &&
              readiness_receipt.runtime_champion_count == 4 &&
              readiness_receipt.runtime_tick_count == 77,
          "boot startup readiness receipt owns runtime HUD handoff after title/menu chain");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.valid &&
              !capture_receipt.route_valid &&
              !capture_receipt.render_view_valid &&
              capture_receipt.readiness_valid &&
              !capture_receipt.hud_menu_draw_valid &&
              !capture_receipt.title_capture_ready &&
              !capture_receipt.hud_menu_capture_ready &&
              capture_receipt.runtime_capture_ready &&
              !capture_receipt.host_input_blocked &&
              !capture_receipt.host_hud_blocked &&
              capture_receipt.render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 &&
              capture_receipt.readiness.runtime_map_index == 6 &&
              capture_receipt.readiness.runtime_champion_count == 4,
          "boot startup capture receipt packages runtime HUD readiness");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
              &snapshot,
              &packaged_proof) == 1 &&
              packaged_proof.valid &&
              packaged_proof.runtime_capture_ready &&
              !packaged_proof.title_capture_ready &&
              !packaged_proof.hud_menu_capture_ready &&
              !packaged_proof.render_plan_available &&
              !packaged_proof.hud_menu_draw_available &&
              packaged_proof.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 &&
              !packaged_proof.boot_executor_route &&
              !packaged_proof.title_route &&
              !packaged_proof.closed_door_menu_route &&
              !packaged_proof.utility_menu_route &&
              !packaged_proof.opening_door_route &&
              packaged_proof.startup_input_ready == 0 &&
              packaged_proof.startup_hud_ready == 0 &&
              !packaged_proof.host_input_blocked &&
              !packaged_proof.host_hud_blocked &&
              packaged_proof.real_asset_matched &&
              packaged_proof.packaged_capture_hash != 0u,
          "boot startup packaged capture proof binds runtime HUD handoff");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              !host_view_receipt.startup_active &&
              host_view_receipt.startup_hud_runtime_ready &&
              host_view_receipt.runtime_handoff_ready &&
              host_view_receipt.runtime_map_index == 6 &&
              host_view_receipt.runtime_party_x == 12 &&
              host_view_receipt.runtime_champion_count == 4 &&
              host_view_receipt.capture_proof_valid &&
              host_view_receipt.capture_proof.runtime_capture_ready,
          "boot startup host-view receipt consumes runtime HUD handoff proof");
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = 4;
    snapshot.entrance_dismissed = 0;
    snapshot.resume_available = 1;
    snapshot.resume_path = resume_path;
    poisoned_view_receipt = view_receipt;
    poisoned_view_receipt.render_plan.waiting_for_input = 0;
    poisoned_view_receipt.render_plan.render_command_count = 1;
    poisoned_view_receipt.render_plan.asset_command_count = 0;
    poisoned_view_receipt.render_plan.menu_option_count = 1;
    poisoned_view_receipt.render_plan.menu_options[0].selected = 0;
    poisoned_view_receipt.render_plan.menu_options[1].selected = 1;
    poisoned_view_receipt.render_plan.fallback_prompt_text = "STALE";
    CHECK(csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
              &poisoned_view_receipt,
              &hud_draw_receipt) == 1 &&
              hud_draw_receipt.startup_render_plan_valid &&
              (receipt_closed_door_plan = hud_draw_receipt.startup_render_plan,
               1) &&
              receipt_closed_door_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              receipt_closed_door_plan.waiting_for_input &&
              receipt_closed_door_plan.render_command_count ==
                  view_receipt.closed_door_render_command_count - 1 &&
              receipt_closed_door_plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34 &&
              receipt_closed_door_plan.asset_command_count ==
                  view_receipt.closed_door_asset_command_count &&
              receipt_closed_door_plan.menu_option_count == 4 &&
              receipt_closed_door_plan.menu_options[0].selected &&
              !receipt_closed_door_plan.menu_options[1].selected &&
              strcmp(receipt_closed_door_plan.fallback_prompt_text,
                     view_receipt.closed_door_prompt) == 0 &&
              strstr(receipt_closed_door_plan.fallback_prompt_text,
                     "PRESS ENTER") != NULL,
          "boot startup closed-door HUD/menu receipt consumes render-view receipt fields");
    CHECK(csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
              &poisoned_view_receipt,
              &hud_draw_receipt) == 1 &&
              hud_draw_receipt.valid &&
              hud_draw_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              hud_draw_receipt.startup_render_plan_valid &&
              !hud_draw_receipt.utility_render_plan_valid &&
              hud_draw_receipt.draw_closed_doors &&
              !hud_draw_receipt.draw_fallback_text &&
              hud_draw_receipt.suppress_legacy_utility_fallback &&
              hud_draw_receipt.option_count == 4 &&
              hud_draw_receipt.selected_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              hud_draw_receipt.resume_enabled &&
              hud_draw_receipt.resume_available &&
              hud_draw_receipt.resume_option_visible &&
              !hud_draw_receipt.resume_option_selected &&
              hud_draw_receipt.startup_render_plan.menu_options[0].selected &&
              strstr(hud_draw_receipt.prompt, "PRESS ENTER") != NULL,
          "boot startup HUD/menu draw receipt consumes closed-door render-view receipt");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.hud_menu_capture_ready &&
              capture_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              capture_receipt.hud_menu_draw_valid &&
              capture_receipt.hud_menu_draw.draw_closed_doors &&
              !capture_receipt.hud_menu_draw.draw_fallback_text &&
              capture_receipt.selected_command_id ==
                  capture_receipt.hud_menu_draw.selected_command_id &&
              capture_receipt.suppress_legacy_utility_fallback ==
                  capture_receipt.hud_menu_draw
                      .suppress_legacy_utility_fallback &&
              capture_receipt.real_asset_receipt_valid &&
              capture_receipt.real_asset_receipt.matched,
          "boot startup capture receipt packages closed-door HUD/menu draw");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              host_view_receipt.render_plan.waiting_for_input &&
              host_view_receipt.render_plan.menu_option_count == 4 &&
              strstr(host_view_receipt.render_plan.fallback_prompt_text,
                     "PRESS ENTER") != NULL &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.hud_menu_draw_ready &&
              host_view_receipt.hud_menu_draw_valid,
          "boot startup snapshot host-view consumes closed-door render-draw receipt");
    CHECK(csb_v1_boot_startup_readiness_receipt_from_view_pc34(
              &view_receipt,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              readiness_receipt.hud_menu_ready &&
              readiness_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34,
          "boot startup HUD/menu executor receives closed-door readiness receipt");
    memset(&hud_draw_probe, 0, sizeof(hud_draw_probe));
    hud_probe_executor_init(&hud_draw_executor, &hud_draw_probe);
    CHECK(csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
              &hud_draw_receipt,
              &readiness_receipt,
              &hud_draw_executor) == 1 &&
              hud_draw_probe.utility_panel_count == 0 &&
              hud_draw_probe.closed_doors_count == 1 &&
              hud_draw_probe.fallback_text_count == 0 &&
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_menu_option_count == 4 &&
              hud_draw_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup HUD/menu executor draws closed-door HUD without fallback text");
    memset(&hud_draw_probe, 0, sizeof(hud_draw_probe));
    hud_probe_executor_init(&hud_draw_executor, &hud_draw_probe);
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.capture_proof.hud_menu_draw_available &&
              host_view_receipt.capture_proof.closed_door_menu_route &&
              !host_view_receipt.capture_proof.draw_fallback_text &&
              host_view_receipt.readiness_valid &&
              host_view_receipt.readiness.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
                  &host_view_receipt.hud_menu_draw,
                  &readiness_receipt,
                  &hud_draw_executor) == 1 &&
              hud_draw_probe.utility_panel_count == 0 &&
              hud_draw_probe.closed_doors_count == 1 &&
              hud_draw_probe.fallback_text_count == 0 &&
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_menu_option_count == 4 &&
              hud_draw_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup host-view receipt executes closed-door HUD/menu draw");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
              &capture_receipt,
              &packaged_proof) == 1 &&
              packaged_proof.valid &&
              packaged_proof.hud_menu_capture_ready &&
              packaged_proof.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              packaged_proof.selected_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              !packaged_proof.title_route &&
              packaged_proof.closed_door_menu_route &&
              !packaged_proof.utility_menu_route &&
              !packaged_proof.opening_door_route &&
              packaged_proof.draw_closed_doors &&
              !packaged_proof.draw_fallback_text &&
              !packaged_proof.draw_utility_panel &&
              packaged_proof.startup_input_ready &&
              packaged_proof.startup_hud_ready &&
              !packaged_proof.host_input_blocked &&
              !packaged_proof.host_hud_blocked &&
              packaged_proof.hud_menu_option_count == 4 &&
              packaged_proof.resume_available &&
              packaged_proof.resume_option_visible &&
              packaged_proof.suppress_legacy_utility_fallback &&
              packaged_proof.render_plan_available &&
              packaged_proof.hud_menu_draw_available &&
              packaged_proof.packaged_capture_hash != 0u &&
              strstr(packaged_proof.source_evidence, "ENTRANCE.C") != NULL,
          "boot startup packaged capture proof binds closed-door HUD/menu draw");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.startup_active &&
              host_view_receipt.startup_input_ready &&
              host_view_receipt.startup_hud_menu_ready &&
              host_view_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              host_view_receipt.hud_menu_option_count == 4 &&
              host_view_receipt.selected_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_plan.render_command_count == 4 &&
              host_view_receipt.render_plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34 &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.hud_menu_draw_ready &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.hud_menu_draw.draw_closed_doors &&
              !host_view_receipt.hud_menu_draw.draw_fallback_text &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              host_view_receipt.capture_proof_valid &&
              host_view_receipt.capture_proof.closed_door_menu_route,
          "boot startup host-view receipt consumes closed-door HUD/menu proof");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              host_view_draw_receipt.hud_menu_executed == 1 &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_closed_doors_count == 1 &&
              capture_render_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup host-view draw receipt executes closed-door render");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.valid &&
              host_view_draw_receipt.render_executed &&
              host_view_draw_receipt.hud_menu_executed == 1 &&
              host_view_draw_receipt.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              host_view_draw_receipt.suppress_legacy_utility_fallback &&
              host_view_draw_receipt.consumed_host_view_only &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_closed_doors_count == 1 &&
              capture_render_probe.draw_fallback_text_count == 0,
          "boot startup host-view draw receipt consumes closed-door render plus HUD without fallback text");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    capture_render_probe.draw_full_surface_result = 0;
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.valid &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_closed_doors_count == 1 &&
              capture_render_probe.draw_door_fallback_count == 0 &&
              capture_render_probe.draw_fallback_text_count == 0,
          "boot startup closed-door host-view receipt refuses fallback when surface assets fail");
    CHECK(host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.render_plan_valid &&
              host_view_receipt.render_draw.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              host_view_receipt.render_draw.render_plan.waiting_for_input,
          "boot startup host-view receipt returns packaged closed-door render plan");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_runtime_state_pc34(
              &runtime_view_receipt,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 1 &&
              runtime_view_receipt.closed_door_menu_route &&
              runtime_view_receipt.suppress_legacy_utility_fallback &&
              runtime_view_receipt.hud_menu_receipt_ready,
          "boot startup runtime-state render-view receipt owns closed-door HUD gate");
    CHECK(csb_v1_boot_startup_render_plan_from_runtime_state_pc34(
              &runtime_render_plan,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 1 &&
              runtime_render_plan.surface ==
                  host_view_receipt.render_draw.render_plan.surface &&
              runtime_render_plan.waiting_for_input ==
                  host_view_receipt.render_draw.render_plan.waiting_for_input,
          "boot startup runtime-state render-plan facade matches host-view packaged plan");
    snapshot.utility_overlay_active = 1;
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 3;
    CHECK(csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
              &snapshot,
              &route_receipt) == 1,
          "boot startup route receipt accepts door-opening snapshot");
    CHECK(route_receipt.valid &&
              route_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_FRAME_PC34 &&
              route_receipt.draw_surface &&
              route_receipt.draw_closed_doors &&
              route_receipt.draw_opening_frame &&
              !route_receipt.hud_menu_visible &&
              !route_receipt.accepts_input,
          "boot startup route receipt owns door-opening render route");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.opening_door_route &&
              !view_receipt.hud_menu_receipt_ready &&
              !view_receipt.suppress_legacy_utility_fallback,
          "boot startup render-view receipt owns door-opening runtime route");
    CHECK(csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
              &snapshot,
              &readiness_receipt) == 1 &&
              readiness_receipt.valid &&
              readiness_receipt.startup_active &&
              !readiness_receipt.input_ready &&
              !readiness_receipt.hud_menu_ready &&
              readiness_receipt.host_input_blocked &&
              readiness_receipt.host_hud_blocked &&
              !readiness_receipt.host_startup_input_ready &&
              !readiness_receipt.host_startup_hud_ready,
          "boot startup readiness receipt blocks host input/HUD during door opening");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.valid &&
              capture_receipt.render_view_valid &&
              !capture_receipt.hud_menu_capture_ready &&
              capture_receipt.host_hud_blocked,
          "boot startup capture receipt packages door-opening gates");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.opening_draw_ready &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
              host_view_receipt.render_plan.opening_step == 3 &&
              host_view_receipt.render_plan.render_command_count >= 4 &&
              host_view_receipt.render_plan.render_commands[3].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34 &&
              host_view_receipt.capture_proof.opening_door_route,
          "boot startup host-view receipt packages door-opening render plan");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              capture_render_probe.draw_opening_frame_count == 1 &&
              capture_render_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34,
          "boot startup host-view draw receipt executes door-opening render");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    capture_render_probe.draw_full_surface_result = 0;
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_opening_frame_count == 0 &&
              capture_render_probe.draw_closed_doors_count == 0 &&
              capture_render_probe.draw_door_fallback_count == 0 &&
              capture_render_probe.draw_fallback_text_count == 0,
          "boot startup door-opening capture plan refuses fallback when surface assets fail");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.opening_draw_ready &&
              host_view_receipt.capture_proof.opening_door_route &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34,
          "boot startup snapshot host-view consumes door-opening capture proof");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
              &capture_receipt,
              &packaged_proof) == 1 &&
              packaged_proof.valid &&
              packaged_proof.render_plan_available &&
              !packaged_proof.hud_menu_draw_available &&
              packaged_proof.opening_door_route &&
              packaged_proof.draw_opening_frame &&
              !packaged_proof.title_route &&
              !packaged_proof.closed_door_menu_route &&
              !packaged_proof.utility_menu_route &&
              packaged_proof.host_input_blocked &&
              packaged_proof.host_hud_blocked &&
              !packaged_proof.startup_input_ready &&
              !packaged_proof.startup_hud_ready &&
              packaged_proof.packaged_capture_hash != 0u,
          "boot startup packaged capture proof binds door-opening animation route");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.startup_active &&
              !host_view_receipt.startup_input_ready &&
              !host_view_receipt.startup_hud_menu_ready &&
              host_view_receipt.capture_proof_valid &&
              host_view_receipt.capture_proof.opening_door_route &&
              host_view_receipt.capture_proof.draw_opening_frame,
          "boot startup host-view receipt consumes door-opening animation proof");
    snapshot.opening_active = 0;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 0;
    snapshot.credits_active = 1;
    snapshot.credits_remaining_ticks =
        csb_v1_startup_entrance_credits_ticks_pc34();
    CHECK(csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
              &snapshot,
              &route_receipt) == 1 &&
              route_receipt.valid &&
              route_receipt.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34 &&
              route_receipt.draw_surface &&
              !route_receipt.draw_fallback_text &&
              !route_receipt.hud_menu_visible,
          "boot startup route receipt owns credits surface without text fallback");
    CHECK(csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
              &snapshot,
              &capture_receipt) == 1 &&
              capture_receipt.valid &&
              capture_receipt.render_view_valid &&
              capture_receipt.render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34 &&
              capture_receipt.host_hud_blocked,
          "boot startup capture receipt packages credits surface gate");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34 &&
              host_view_receipt.render_plan.render_command_count == 2 &&
              host_view_receipt.render_plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34 &&
              host_view_receipt.capture_proof.credits_route &&
              !host_view_receipt.capture_proof.draw_fallback_text,
          "boot startup host-view receipt packages credits as real surface");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    capture_render_probe.draw_full_surface_result = 0;
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.render_executed &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_fallback_text_count == 0,
          "boot startup credits capture plan refuses text fallback when surface assets fail");
    CHECK(csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
              &capture_receipt,
              &packaged_proof) == 1 &&
              packaged_proof.valid &&
              packaged_proof.render_plan_available &&
              packaged_proof.credits_route &&
              !packaged_proof.draw_fallback_text &&
              packaged_proof.packaged_capture_hash != 0u,
          "boot startup packaged capture proof binds credits surface route");
    snapshot.credits_active = 0;
    snapshot.credits_remaining_ticks = 0;
    snapshot.opening_active = 0;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 0;
    CHECK(csb_v1_boot_startup_presentation_state_receipt_from_runtime_state_pc34(
              &presentation_receipt,
              snapshot.title_active,
              snapshot.title_frame,
              snapshot.title_source_step,
              snapshot.entrance_active,
              snapshot.entrance_source_step,
              snapshot.entrance_dismissed,
              snapshot.credits_active,
              snapshot.credits_remaining_ticks,
              snapshot.opening_active,
              snapshot.opening_delay_ticks,
              snapshot.opening_step,
              snapshot.pending_command,
              snapshot.entrance_frame,
              snapshot.utility_overlay_active,
              snapshot.utility_selected_action_index,
              snapshot.utility_imported_champion_count,
              snapshot.utility_preview_active,
              snapshot.utility_prompt,
              snapshot.resume_available,
              snapshot.resume_path,
              snapshot.boot_profile) == 1 &&
              presentation_receipt.valid &&
              presentation_receipt.render_plan.waiting_for_input,
          "boot startup presentation receipt facade accepts runtime fields");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              2,
              &boot_action_receipt) == 1,
          "boot startup action facade accepts utility keyboard input");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 &&
              boot_action_receipt.utility_receipt.util_receipt.result ==
                  CSB_V1_UTIL_APPLY_REDRAW &&
              boot_action_receipt.pre_input_route.valid &&
              boot_action_receipt.pre_input_route.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              boot_action_receipt.pre_input_route.hud_menu_state.valid &&
              boot_action_receipt.pre_input_route.hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              boot_action_receipt.menu_input == 2 &&
              boot_action_receipt.startup_input ==
                  CSB_V1_STARTUP_INPUT_NONE_PC34 &&
              boot_action_receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34 &&
              boot_action_receipt.input_routed_to_utility &&
              !boot_action_receipt.input_routed_to_entrance &&
              boot_action_receipt.pre_input_render_view_valid &&
              boot_action_receipt.pre_input_render_view.closed_door_menu_route &&
              boot_action_receipt.pre_input_render_view.hud_menu_receipt_ready &&
              boot_action_receipt.pre_input_render_view.route_receipt
                      .hud_menu_state.utility_selected_action_index == 0,
          "boot startup action facade keeps utility priority with pre-render proof");
    CHECK(boot_action_receipt.post_input_render_view_valid &&
              boot_action_receipt.input_stays_on_startup &&
              !boot_action_receipt.input_requests_launcher_return &&
              boot_action_receipt.post_input_render_view.closed_door_menu_route &&
              boot_action_receipt.post_input_render_view.route_receipt.valid &&
              boot_action_receipt.post_input_render_view.route_receipt
                      .hud_menu_state.valid &&
              boot_action_receipt.post_input_render_view.route_receipt
                      .hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              boot_action_receipt.post_input_render_view.route_receipt
                      .hud_menu_state.utility_selected_action_index == 1,
          "boot startup utility input carries post-input render/HUD route");
    CHECK(csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
              &boot_action_receipt,
              &host_decision) == 1 &&
              host_decision.valid &&
              host_decision.consumed_input &&
              host_decision.routed_to_utility &&
              !host_decision.routed_to_entrance &&
              host_decision.redraw_startup &&
              host_decision.stays_on_startup &&
              !host_decision.return_to_launcher &&
              host_decision.utility_selected_action_index == 1 &&
              host_decision.pre_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34,
          "boot startup host decision consumes utility redraw receipt");
    CHECK(csb_v1_boot_startup_hud_menu_draw_receipt_from_action_pc34(
              &boot_action_receipt,
              1,
              &hud_draw_receipt) == 1 &&
              hud_draw_receipt.valid &&
              hud_draw_receipt.from_post_input_render_view &&
              hud_draw_receipt.host_decision_valid &&
              hud_draw_receipt.host_decision.redraw_startup &&
              hud_draw_receipt.host_decision.routed_to_utility &&
              hud_draw_receipt.host_decision.utility_selected_action_index == 1 &&
              hud_draw_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              hud_draw_receipt.utility_render_plan_valid &&
              hud_draw_receipt.selected_utility_action_index == 1 &&
              hud_draw_receipt.utility_render_plan.menu_rows[1].selected,
          "boot startup HUD/menu draw receipt consumes post-input host decision receipt");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
              &snapshot,
              2,
              &input_render_receipt) == 1 &&
              input_render_receipt.valid &&
              input_render_receipt.action_valid &&
              input_render_receipt.host_decision_valid &&
              input_render_receipt.post_input_readiness_valid &&
              input_render_receipt.hud_menu_draw_valid &&
              input_render_receipt.draw_from_post_input &&
              input_render_receipt.input_consumed &&
              input_render_receipt.startup_redraw &&
              input_render_receipt.startup_hud_draw_ready &&
              !input_render_receipt.return_to_launcher &&
              input_render_receipt.host_decision.routed_to_utility &&
              input_render_receipt.hud_menu_draw.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              input_render_receipt.hud_menu_draw.selected_utility_action_index ==
                  1 &&
              input_render_receipt.post_input_readiness.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34,
          "boot startup input/render receipt owns utility redraw HUD path");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
              &snapshot,
              2,
              &input_gate_receipt) == 1 &&
              input_gate_receipt.valid &&
              input_gate_receipt.startup_active &&
              input_gate_receipt.startup_input_ready &&
              !input_gate_receipt.host_input_blocked &&
              input_gate_receipt.should_dispatch_input &&
              !input_gate_receipt.should_ignore_input &&
              input_gate_receipt.input_render_valid &&
              input_gate_receipt.input_render.startup_hud_draw_ready,
          "boot startup input gate owns utility HUD redraw decision");
    CHECK(csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
              &snapshot,
              2,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.startup_active &&
              host_input_dispatch.startup_input_ready &&
              !host_input_dispatch.host_input_blocked &&
              host_input_dispatch.should_dispatch_input &&
              !host_input_dispatch.should_ignore_input &&
              host_input_dispatch.input_render_valid &&
              host_input_dispatch.input_render.host_decision.routed_to_utility &&
              host_input_dispatch.input_render.startup_hud_draw_ready,
          "boot startup host input dispatch receipt owns utility redraw snapshot");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
              &snapshot,
              1,
              2,
              &capture_render_executor,
              &host_ownership) == 1 &&
              host_ownership.valid &&
              host_ownership.packaged_visual_capture_ready &&
              host_ownership.hud_menu_capture_ready &&
              host_ownership.utility_menu_draw_ready &&
              host_ownership.render_executed &&
              host_ownership.hud_menu_executed == 1 &&
              host_ownership.draw_consumes_receipt_only &&
              host_ownership.host_input_dispatch_valid &&
              host_ownership.should_dispatch_input &&
              host_ownership.input_redraws_hud_menu &&
              host_ownership.input_consumes_receipt_only &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_utility_panel_count == 1,
          "boot startup host ownership receipt binds utility capture, draw, and input redraw");
    (void)csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
        &snapshot,
        72,
        126,
        ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
        &boot_action_receipt);
    CHECK(boot_action_receipt.pre_input_route.valid &&
              boot_action_receipt.pre_input_route.hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              boot_action_receipt.input_is_pointer &&
              boot_action_receipt.pointer_x == 72 &&
              boot_action_receipt.pointer_y == 126 &&
              boot_action_receipt.pointer_button_mask ==
                  ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT &&
              boot_action_receipt.pointer_left_button &&
              boot_action_receipt.input_routed_to_utility &&
              !boot_action_receipt.input_routed_to_entrance &&
              boot_action_receipt.startup_input ==
                  CSB_V1_STARTUP_INPUT_NONE_PC34,
          "boot startup pointer action carries utility route proof");

    snapshot.utility_overlay_active = 0;
    CHECK(csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &boot_action_receipt) == 1,
          "boot startup pointer action handles entrance menu row");
    CHECK(boot_action_receipt.input_is_pointer &&
              boot_action_receipt.pointer_x == enter_menu_x &&
              boot_action_receipt.pointer_y == enter_menu_y &&
              boot_action_receipt.pointer_left_button &&
              boot_action_receipt.input_routed_to_entrance &&
              !boot_action_receipt.input_routed_to_utility &&
              boot_action_receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              boot_action_receipt.host_receipt_valid &&
              boot_action_receipt.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(boot_action_receipt.host_status_scope, "BOOT") == 0 &&
              strcmp(boot_action_receipt.host_status, "CSB DOORS") == 0 &&
              boot_action_receipt.post_input_render_view_valid &&
              boot_action_receipt.post_input_render_view.opening_door_route,
          "boot startup pointer entrance carries command, host, and post-render route");
    CHECK(csb_v1_boot_runtime_execute_startup_pointer_render_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &input_render_receipt) == 1 &&
              input_render_receipt.valid &&
              input_render_receipt.host_decision_valid &&
              input_render_receipt.post_input_readiness_valid &&
              !input_render_receipt.hud_menu_draw_valid &&
              input_render_receipt.input_consumed &&
              input_render_receipt.startup_redraw &&
              !input_render_receipt.startup_hud_draw_ready &&
              input_render_receipt.host_decision.routed_to_entrance &&
              input_render_receipt.host_decision.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              input_render_receipt.post_input_readiness.host_hud_blocked,
          "boot startup input/render receipt owns pointer door-opening handoff");
    CHECK(csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &input_gate_receipt) == 1 &&
              input_gate_receipt.valid &&
              input_gate_receipt.input_is_pointer &&
              input_gate_receipt.pointer_button_relevant &&
              input_gate_receipt.should_dispatch_input &&
              input_gate_receipt.input_render_valid &&
              input_gate_receipt.input_render.host_decision.routed_to_entrance,
          "boot startup pointer gate owns entrance command dispatch");
    CHECK(csb_v1_boot_startup_host_input_dispatch_pointer_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.input_is_pointer &&
              host_input_dispatch.pointer_button_relevant &&
              host_input_dispatch.startup_active &&
              host_input_dispatch.startup_input_ready &&
              !host_input_dispatch.host_input_blocked &&
              host_input_dispatch.should_dispatch_input &&
              host_input_dispatch.input_render_valid &&
              host_input_dispatch.input_render.host_decision.routed_to_entrance,
          "boot startup host input dispatch receipt owns pointer entrance snapshot");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              9,
              &boot_action_receipt) == 1,
          "boot startup action facade falls back to entrance input");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 &&
              boot_action_receipt.entrance_receipt.handled &&
              boot_action_receipt.pre_input_route.valid &&
              boot_action_receipt.pre_input_route.route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              boot_action_receipt.pre_input_route.hud_menu_state.valid &&
              boot_action_receipt.pre_input_route.hud_menu_state.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 &&
              boot_action_receipt.menu_input == 9 &&
              boot_action_receipt.startup_input ==
                  CSB_V1_STARTUP_INPUT_ACCEPT_PC34 &&
              boot_action_receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              boot_action_receipt.host_receipt_valid &&
              boot_action_receipt.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              strcmp(boot_action_receipt.host_status_scope, "BOOT") == 0 &&
              strcmp(boot_action_receipt.host_status, "CSB DOORS") == 0 &&
              !boot_action_receipt.input_routed_to_utility &&
              boot_action_receipt.input_routed_to_entrance &&
              boot_action_receipt.pre_input_render_view_valid &&
              boot_action_receipt.pre_input_render_view.closed_door_menu_route &&
              boot_action_receipt.pre_input_render_view
                  .suppress_legacy_utility_fallback,
          "boot startup action facade returns entrance receipt with pre-render proof");
    CHECK(boot_action_receipt.post_input_render_view_valid &&
              boot_action_receipt.input_stays_on_startup &&
              !boot_action_receipt.input_requests_launcher_return &&
              boot_action_receipt.post_input_render_view.opening_door_route &&
              boot_action_receipt.post_input_render_view.route_receipt.valid &&
              boot_action_receipt.post_input_render_view.route_receipt.route !=
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              boot_action_receipt.post_input_render_view.route_receipt
                  .draw_closed_doors &&
              !boot_action_receipt.post_input_render_view.route_receipt
                   .accepts_input,
          "boot startup entrance input carries post-input door render route");
    CHECK(csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
              &boot_action_receipt,
              &host_decision) == 1 &&
              host_decision.valid &&
              host_decision.consumed_input &&
              !host_decision.routed_to_utility &&
              host_decision.routed_to_entrance &&
              host_decision.redraw_startup &&
              host_decision.stays_on_startup &&
              host_decision.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              host_decision.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              host_decision.post_render_route ==
                  boot_action_receipt.post_input_render_view
                      .route_receipt.route &&
              host_decision.post_render_route !=
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              strcmp(host_decision.status, "CSB DOORS") == 0,
          "boot startup host decision consumes entrance redraw receipt");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              10,
              &boot_action_receipt) == 1,
          "boot startup action facade handles entrance Back input");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 &&
              boot_action_receipt.input_requests_launcher_return &&
              !boot_action_receipt.input_stays_on_startup &&
              !boot_action_receipt.post_input_render_view_valid &&
              boot_action_receipt.menu_input == 10 &&
              boot_action_receipt.startup_input ==
                  CSB_V1_STARTUP_INPUT_BACK_PC34 &&
              boot_action_receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 &&
              boot_action_receipt.host_receipt_valid &&
              boot_action_receipt.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34 &&
              strcmp(boot_action_receipt.host_status_scope, "RETURN") == 0 &&
              strcmp(boot_action_receipt.host_status, "BACK TO LAUNCHER") == 0 &&
              boot_action_receipt.input_routed_to_entrance &&
              boot_action_receipt.pre_input_render_view_valid &&
              boot_action_receipt.pre_input_render_view.closed_door_menu_route &&
              boot_action_receipt.pre_input_render_view.route_receipt
                  .hud_menu_state.valid,
          "boot startup Back input carries pre-render menu and launcher return");
    CHECK(csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
              &boot_action_receipt,
              &host_decision) == 1 &&
              host_decision.valid &&
              host_decision.consumed_input &&
              host_decision.return_to_launcher &&
              !host_decision.stays_on_startup &&
              !host_decision.redraw_startup &&
              host_decision.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34 &&
              host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 &&
              strcmp(host_decision.status, "BACK TO LAUNCHER") == 0,
          "boot startup host decision consumes launcher-return receipt");
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
              &snapshot,
              10,
              &input_render_receipt) == 1 &&
              input_render_receipt.valid &&
              input_render_receipt.host_decision_valid &&
              input_render_receipt.pre_input_readiness_valid &&
              !input_render_receipt.post_input_readiness_valid &&
              !input_render_receipt.hud_menu_draw_valid &&
              input_render_receipt.input_consumed &&
              input_render_receipt.return_to_launcher &&
              !input_render_receipt.startup_redraw &&
              !input_render_receipt.startup_hud_draw_ready &&
              input_render_receipt.host_decision.return_to_launcher,
          "boot startup input/render receipt owns launcher-return handoff");
    CHECK(csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              0U,
              &input_gate_receipt) == 1 &&
              input_gate_receipt.valid &&
              input_gate_receipt.startup_active &&
              !input_gate_receipt.pointer_button_relevant &&
              !input_gate_receipt.should_dispatch_input &&
              !input_gate_receipt.should_ignore_input &&
              !input_gate_receipt.input_render_valid,
          "boot startup pointer gate ignores irrelevant buttons at CSB boundary");

    csb_v1_boot_cleanup(&boot);
}

int main(void)
{
    printf("=== CSB V1 Boot → Runtime Handoff Regression ===\n\n");
    test_enter_game_with_verified_profile_loads_dungeon();
    test_enter_game_loads_m564_object_names_from_graphics_dat();
    test_enter_game_preserves_imported_party_and_switches_leader();
    test_runtime_import_dm1_party_path_owns_utility_handoff();
    test_runtime_view_state_receipt_owns_scalar_handoff();
    test_runtime_utility_startup_host_facts_wrappers();
    test_startup_real_asset_receipt_is_skip_safe_and_deterministic();
    test_enter_game_rotate_party_aligns_champion_state();
    test_enter_game_with_missing_dungeon_path_keeps_runtime_safe();
    test_enter_game_runtime_handoff_is_idempotent();
    test_enter_game_rejects_partial_or_misrouted_profiles();
    test_enter_game_v2_profile_labels_do_not_change_v1_handoff();
    test_utility_flow_new_game_handoff_preserves_leader_index();
    test_utility_flow_load_game_uses_runtime_loader();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 boot→runtime handoff completes dungeon load, asset metadata, and entrance/start map handoff in one step");
        puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 lines 409-441; LOADSAVE.C F0435 lines 1940-1944; CSBWin/CSBCode.cpp:6800-6950 LoadDungeon");
        puts("ok: CSB utility flow NEW_GAME handoff preserves LeaderIndex in csb_v1_util_flow_get_party()");
        puts("sourceEvidence=ReDMCSB SAVEGAME.C F0100-F0120 import state; ReDMCSB ENTRANCE.C F0806 startup flow");
        puts("ok: CSB V1 verified boot handoff preserves an imported two-champion party and supports a deterministic leader switch");
        puts("sourceEvidence=ReDMCSB LOADSAVE.C F0435 lines 2728-2734 imports party globals; CLIKCHAM.C F0368 lines 51-68; CHAMPION.C F0284 lines 117-130");
        puts("ok: CSB V1 verified boot handoff decodes GRAPHICS.DAT M564 object names into the runtime-owned icon-indexed name table");
        puts("sourceEvidence=ReDMCSB OBJECT.C F0031 M564_GRAPHIC_OBJECT_NAMES; OBJECT.C F0033 icon-indexed object names");
        puts("ok: CSB V1 verified boot handoff rotates an imported four-champion party through the source-locked F0284 delta-mod-4 invariant on every champion's Cell and Direction");
        puts("sourceEvidence=ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines 117-130 (MEDIA182 C source); CLIKCHAM.C F0368 line 67 (leader-switch alignment to G0308_i_PartyDirection)");
        puts("ok: CSB V2 profile labels do not alter V1 runtime handoff paths, media kind, title state, or required dungeon load");
        puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 lines 409-441 selects CSB media; LOADSAVE.C F0435 lines 1936-1944 loads the V1 dungeon");
    }
    return failed == 0 ? 0 : 1;
}
