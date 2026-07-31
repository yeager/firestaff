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
 * a source-layout DUNGEON.DAT fixture so we can prove the boundary works
 * end-to-end without admitting the loader's legacy unit-test format.
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
/* ReDMCSB DEFS.H:2394 maps M564_GRAPHIC_OBJECT_NAMES to PC/I34E item 694. */
#define TEST_CSB_OBJECT_NAMES_INDEX 694u
#define TEST_CSB_OBJECT_NAME_COUNT 199
#define TEST_CSB_UTILITY_ADF_BYTES (80u * 2u * 11u * 512u)
#define TEST_CSB_UTILITY_ROOT_OFFSET (880u * 512u)
#define TEST_CSB_UTILITY_NAME_OFFSET 432u

typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t bit_pos;
} TestBitWriter;

static int write_csb_utility_adf_fixture(const char *path)
{
    static const char volume[] = "FTL_CSB_Utility";
    FILE *fp;
    if (!path) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fseek(fp, (long)TEST_CSB_UTILITY_ADF_BYTES - 1L, SEEK_SET) != 0 ||
        fputc(0, fp) == EOF ||
        fseek(fp, (long)(TEST_CSB_UTILITY_ROOT_OFFSET +
                         TEST_CSB_UTILITY_NAME_OFFSET), SEEK_SET) != 0 ||
        fputc((int)(sizeof(volume) - 1u), fp) == EOF ||
        fwrite(volume, 1u, sizeof(volume) - 1u, fp) != sizeof(volume) - 1u) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void set_csb_utility_disk_env(const char *path)
{
#ifdef _WIN32
    (void)_putenv_s("FIRESTAFF_CSB_UTILITY_DISK", path ? path : "");
#else
    if (path) (void)setenv("FIRESTAFF_CSB_UTILITY_DISK", path, 1);
    else (void)unsetenv("FIRESTAFF_CSB_UTILITY_DISK");
#endif
}

typedef struct {
    uint8_t dict_first[TEST_LZW_MAX_CODE];
    uint16_t dict_prefix[TEST_LZW_MAX_CODE];
    int dict_count;
    int code_bits;
} TestLZW;

typedef struct {
    int utility_panel_count;
    int closed_doors_count;
    int last_waiting_for_input;
    int last_menu_option_count;
    int last_surface;
    int last_utility_selected_action_index;
} TestHudMenuDrawProbe;

typedef struct {
    int draw_title_count;
    int clear_black_count;
    int draw_full_surface_count;
    int draw_full_surface_result;
    int draw_opening_frame_count;
    int draw_opening_frame_result;
    int draw_closed_doors_count;
    int draw_utility_panel_count;
    int last_surface;
} TestStartupRenderProbe;

static void hud_probe_draw_utility_panel(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_UtilRenderPlan *utility_plan)
{
    TestHudMenuDrawProbe *probe = (TestHudMenuDrawProbe *)user;
    int i;
    if (!probe || !plan || !utility_plan) {
        return;
    }
    ++probe->utility_panel_count;
    probe->last_waiting_for_input = plan->waiting_for_input;
    probe->last_menu_option_count = plan->menu_option_count;
    probe->last_surface = (int)plan->surface;
    probe->last_utility_selected_action_index = -1;
    for (i = 0; i < utility_plan->menu_row_count; ++i) {
        if (utility_plan->menu_rows[i].selected) {
            probe->last_utility_selected_action_index = i;
            break;
        }
    }
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

static void hud_probe_executor_init(
    CSB_V1_StartupRenderExecutor_PC34 *executor,
    TestHudMenuDrawProbe *probe)
{
    memset(executor, 0, sizeof(*executor));
    executor->user = probe;
    executor->draw_utility_panel = hud_probe_draw_utility_panel;
    executor->draw_closed_doors = hud_probe_draw_closed_doors;
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

static void render_probe_draw_utility_panel(
    void *user,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_UtilRenderPlan *utility_plan)
{
    TestStartupRenderProbe *probe = (TestStartupRenderProbe *)user;
    if (!probe || !plan) {
        return;
    }
    (void)utility_plan;
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
    executor->draw_utility_panel = render_probe_draw_utility_panel;
    probe->draw_full_surface_result = 1;
    probe->draw_opening_frame_result = 1;
}

static void write_be16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)((value >> 8) & 0xffu);
    buf[off + 1u] = (uint8_t)(value & 0xffu);
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
    /* F0479's new-file header is big-endian: marker 80 01, count and tables. */
    file_bytes[0] = 0x80u;
    file_bytes[1] = 0x01u;
    write_be16(file_bytes, 2u, count);
    write_be16(file_bytes, 4u + TEST_CSB_OBJECT_NAMES_INDEX * 2u,
               (uint16_t)compressed_size);
    write_be16(file_bytes,
               4u + (size_t)count * 2u + TEST_CSB_OBJECT_NAMES_INDEX * 2u,
               (uint16_t)decoded_size);
    write_be16(file_bytes,
               4u + (size_t)count * 4u + TEST_CSB_OBJECT_NAMES_INDEX * 4u,
               1u);
    write_be16(file_bytes,
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

/* Build the smallest DUNGEON_HEADER/MAP byte-map that the source loader
 * accepts. ReDMCSB DUNGEON.C F0151 indexes the raw map column-major; the
 * six bytes between MAP and squares are G0284's three column starts. */
static int build_source_layout_dungeon(uint8_t *buf, int buf_size,
                                       uint8_t square_type_1_1)
{
    enum { header = 44, map = 16, column_starts = 6, squares = 9 };
    const int raw_map = header + map + column_starts;

    if (!buf || buf_size < raw_map + squares) return -1;
    memset(buf, 0, (size_t)buf_size);
    buf[4] = 1;                         /* DUNGEON_HEADER.MapCount */
    buf[8] = 0x00;                      /* InitialPartyLocation: (0,0,N) */
    /* MAP[0]: offset 0; bit A = map 0, width 3, height 3. */
    buf[header + 8] = 0x80;
    buf[header + 9] = 0x10;
    /* 3x3 source byte-map, column-major. */
    buf[raw_map + 0] = 1;
    buf[raw_map + 1] = 1;
    buf[raw_map + 2] = 1;
    buf[raw_map + 3] = 1;
    buf[raw_map + 4] = square_type_1_1;
    buf[raw_map + 5] = 1;
    buf[raw_map + 6] = 1;
    buf[raw_map + 7] = 1;
    buf[raw_map + 8] = 1;
    return 0;
}

static int write_source_layout_dungeon(const char *path, uint8_t square_type_1_1)
{
    uint8_t buf[80];
    FILE *f;
    size_t n;
    if (build_source_layout_dungeon(buf, (int)sizeof(buf), square_type_1_1) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

/* Deliberately old Firestaff-only unit-fixture shape.  It remains useful to
 * prove that boot never promotes a parser-compatible test buffer into a live
 * CSB dungeon. */
static int write_legacy_fixture_dungeon(const char *path)
{
    uint8_t buf[28] = { 0 };
    FILE *f;

    buf[0] = 1;
    buf[2] = 16;
    buf[4] = 3;
    buf[5] = 3;
    buf[6] = 10;
    memset(buf + 10, 1, sizeof(buf) - 10);
    f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(buf, 1u, sizeof(buf), f) != sizeof(buf)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
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

static void test_utility_import_confirmation_is_transactional(void)
{
    CSB_V1_UtilFlowContext ctx;
    const char *save_path = "/tmp/firestaff-csb-v1-utility-confirm.sav";

    CHECK(write_synthetic_dm1_save_for_utility_flow(save_path) == 0,
          "utility confirmation fixture writes a bounded DM1 save");
    csb_v1_util_flow_init(&ctx);
    snprintf(ctx.imported_party.Champions[0].Name,
             sizeof(ctx.imported_party.Champions[0].Name), "%s", "KEEP");
    ctx.imported_party.ChampionCount = 1;
    ctx.imported_champion_count = 1;
    ctx.state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
    csb_v1_util_flow_set_dm1_path(&ctx, save_path);

    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_CONFIRM_IMPORT &&
              ctx.pending_import_active &&
              ctx.pending_import_champion_count == 1,
          "utility import stages a validated candidate for confirmation");
    CHECK(ctx.imported_party.ChampionCount == 1 &&
              strcmp(ctx.imported_party.Champions[0].Name, "KEEP") == 0 &&
              strcmp(ctx.pending_import_party.Champions[0].Name, "ALPHA   ") == 0,
          "utility preview cannot replace the committed party before confirm");
    CHECK(csb_v1_util_flow_cancel_to_menu(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_SELECT_ACTION &&
              !ctx.pending_import_active &&
              ctx.pending_import_champion_count == 0 &&
              ctx.imported_party.ChampionCount == 1 &&
              strcmp(ctx.imported_party.Champions[0].Name, "KEEP") == 0,
          "utility cancel rolls the preview candidate back without party mutation");

    ctx.state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
    ctx.action = CSB_V1_UTIL_ACTION_IMPORT;
    CHECK(csb_v1_util_flow_step(&ctx) == 0 && ctx.pending_import_active,
          "utility import can stage a fresh candidate after cancel");
    csb_v1_util_flow_confirm_import(&ctx, 1);
    CHECK(csb_v1_util_flow_step(&ctx) == 0 &&
              ctx.state == CSB_V1_UTIL_FLOW_NEW_GAME &&
              !ctx.pending_import_active &&
              ctx.imported_party.ChampionCount == 1 &&
              strcmp(ctx.imported_party.Champions[0].Name, "ALPHA   ") == 0,
          "utility confirmation atomically commits the staged original-save party");
    remove(save_path);
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
    writer.timeline_queue.gameTick = writer.game_time;
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
    char csbwin_save_path[ASSET_PATH_MAX];
    uint8_t csbwin_save_buf[1024];
    size_t csbwin_save_size = 0u;
    FILE *csbwin_save_file = NULL;
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 save_import_receipt;
    CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 dsa_handoff_receipt;
    CSB_V1_BootOriginalSaveRuntimeReceipt_PC34 original_save_receipt;
    CSB_V1_F0240_FirstEventExpiredReceipt f0240_receipt;
    CSB_V1_F0261_ProcessTickReceipt f0261_receipt;
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-test";
    int mkdir_ok = (TEST_MKDIR(tmp_dir) == 0) || 1; /* best-effort */
    uint32_t adapter_game_time = 0U;

    memset(&p, 0, sizeof(p));
    (void)mkdir_ok;
    csb_v1_boot_profile_init(&p);

    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_source_layout_dungeon(dungeon_path, 2) == 0,
          "source-layout DUNGEON.DAT written to temp path");

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
    CHECK(csb_v1_runtime_f0240_is_first_event_expired(
              &p.runtime, &f0240_receipt) == 1 &&
              f0240_receipt.valid == 1 &&
              f0240_receipt.expired == 1 &&
              f0240_receipt.event_count == 1 &&
              f0240_receipt.first_event_time == 0U &&
              f0240_receipt.game_time == 0U &&
              f0240_receipt.first_event_type == DM1_EVENT_PLAY_SOUND &&
              strcmp(f0240_receipt.status, "expired-first-event") == 0,
          "CSB F0240 receipt expires the first live heap event at game_time 0");
    ev.map_time = DM1_MAP_TIME_MAKE(0, 5);
    CHECK(csb_v1_runtime_add_timeline_event(&p.runtime, &ev) >= 0,
          "post-handoff runtime accepts one future timeline event");
    CHECK(csb_v1_runtime_f0240_is_first_event_expired(
              &p.runtime, &f0240_receipt) == 1 &&
              f0240_receipt.expired == 1 &&
              f0240_receipt.first_event_time == 0U,
          "CSB F0240 reads only the source heap root, not a later future event");
    CHECK(csb_v1_runtime_f0261_process_tick(
              &p.runtime, &f0261_receipt) == 1 &&
              f0261_receipt.valid == 1 &&
              f0261_receipt.tick_fired == 1 &&
              f0261_receipt.pre_event_count == 2 &&
              f0261_receipt.post_event_count == 1 &&
              f0261_receipt.dispatched_count == 1 &&
              f0261_receipt.first_event_time == 0U &&
              f0261_receipt.first_event_type == DM1_EVENT_PLAY_SOUND &&
              f0261_receipt.game_time_before == 0U &&
              f0261_receipt.game_time_after == 1U &&
              strcmp(f0261_receipt.status,
                     "processed-expired-events") == 0,
          "CSB F0261 receipt drains expired events through the live runtime tick");
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
    CHECK(p.runtime.timeline_queue.eventCount == 1,
          "post-handoff timeline queue keeps the future event after the first tick");
    CHECK(csb_v1_runtime_f0261_process_tick(
              &p.runtime, &f0261_receipt) == 1 &&
              f0261_receipt.valid == 1 &&
              f0261_receipt.tick_fired == 1 &&
              f0261_receipt.pre_event_count == 1 &&
              f0261_receipt.post_event_count == 1 &&
              f0261_receipt.dispatched_count == 0 &&
              f0261_receipt.first_event_time == 5U &&
              f0261_receipt.game_time_before == 1U &&
              f0261_receipt.game_time_after == 2U &&
              strcmp(f0261_receipt.status,
                     "processed-no-expired-events") == 0,
          "CSB F0261 receipt preserves future live timeline events");
    CHECK(csb_v1_runtime_f0240_is_first_event_expired(
              &p.runtime, &f0240_receipt) == 1 &&
              f0240_receipt.valid == 1 &&
              f0240_receipt.expired == 0 &&
              f0240_receipt.event_count == 1 &&
              f0240_receipt.first_event_time == 5U &&
              f0240_receipt.game_time == 2U &&
              strcmp(f0240_receipt.status, "waiting-first-event") == 0,
          "CSB F0240 waits when the first source event is still in the future");
    p.runtime.timeline_queue.timeline[0] = (uint16_t)DM1_EVENT_MAX_COUNT;
    CHECK(csb_v1_runtime_f0240_is_first_event_expired(
              &p.runtime, &f0240_receipt) == 0 &&
              strcmp(f0240_receipt.status,
                     "malformed-first-event-index") == 0,
          "CSB F0240 rejects a malformed heap root instead of fabricating an event");
    p.runtime.timeline_queue.eventCount = 0;
    CHECK(csb_v1_runtime_f0240_is_first_event_expired(
              &p.runtime, &f0240_receipt) == 1 &&
              f0240_receipt.valid == 1 &&
              f0240_receipt.expired == 0 &&
              strcmp(f0240_receipt.status, "empty-timeline") == 0,
          "CSB F0240 treats an empty live timeline as non-expired");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "post-handoff tick does not claim a broader CSB gameplay state");

    /* The handed-off runtime also accepts the wall-clock accumulator tick
     * API: one additional nominal 55ms quantum fires exactly one V1 tick
     * through csb_v1_runtime_tick/csb_v1_runtime_tick_due on the same
     * profile the F0261 receipt path just advanced. */
    CHECK(csb_v1_runtime_tick_due(&p.runtime, 0U) == 0,
          "accumulator tick is not due before another quantum banks");
    csb_v1_runtime_tick(&p.runtime, CSB_V1_TICK_MS_NOMINAL);
    CHECK(p.runtime.tick_count == 3U && p.runtime.game_time == 3U,
          "csb_v1_runtime_tick fires one banked quantum on the handed-off runtime");
    CHECK(p.runtime.total_play_ms == CSB_V1_TICK_MS_NOMINAL * 3U,
          "csb_v1_runtime_tick accumulates wall time across both tick APIs");
    CHECK(csb_v1_runtime_tick_due(&p.runtime, 0U) == 0,
          "accumulator tick drains the banked quantum");

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
    snprintf(csbwin_save_path,
             sizeof(csbwin_save_path),
             "%s/csbgame.dat",
             tmp_dir);
    csbwin_save_size = csb_v1_csbwin_save_loader_boundary_build_fixture(
        CSB_V1_CSBWIN_SHAPE_CSBGAME_V20,
        csbwin_save_buf,
        sizeof(csbwin_save_buf));
    CHECK(csbwin_save_size > 0u,
          "CSBWin CSBGAME fixture builds for boot save/import receipt");
    csbwin_save_file = fopen(csbwin_save_path, "wb");
    CHECK(csbwin_save_file != NULL,
          "CSBWin CSBGAME fixture opens for boot receipt");
    if (csbwin_save_file) {
        CHECK(fwrite(csbwin_save_buf, 1u, csbwin_save_size,
                     csbwin_save_file) == csbwin_save_size,
              "CSBWin CSBGAME fixture writes for boot receipt");
        fclose(csbwin_save_file);
    }
    CHECK(csb_v1_boot_runtime_save_import_receipt_pc34(
              &p,
              "/tmp/firestaff-csb-v1-runtime-dm1-import.sav",
              save_path,
              csbwin_save_path,
              &save_import_receipt) == 1 &&
              save_import_receipt.valid &&
              save_import_receipt.runtime_ready &&
              save_import_receipt.save_root_bound &&
              save_import_receipt.save_adapter_available &&
              save_import_receipt.load_adapter_available &&
              save_import_receipt.tick_adapter_available &&
              save_import_receipt.resume_path_present &&
              save_import_receipt.dm1_import_path_present &&
              save_import_receipt.csbwin_path_present &&
              save_import_receipt.csbwin_filename_candidate &&
              save_import_receipt.csbwin_should_attempt_import &&
              save_import_receipt.csbwin_contract_match &&
              save_import_receipt.csbwin_shape ==
                  CSB_V1_CSBWIN_SHAPE_CSBGAME_V20 &&
              save_import_receipt.csbwin_file_kind ==
                  CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_DAT &&
              !save_import_receipt.csbwin_dsa_corpus_positive &&
              !save_import_receipt.csbwin_dsa_runtime_handoff_ready &&
              !save_import_receipt.csbwin_dsa_extended_tail_valid &&
              !save_import_receipt.csbwin_dsa_section_valid &&
              !save_import_receipt.csbwin_dsa_has_runtime_actions &&
              !save_import_receipt.csbwin_dsa_gameblock1_valid &&
              strcmp(save_import_receipt.csbwin_dsa_decision_label,
                     "reject_dsa_corpus_no_extended_features") == 0 &&
              strstr(save_import_receipt.source_evidence,
                     "LOADSAVE.C F0433/F0435") != NULL,
          "boot save/import receipt owns runtime save, resume, DM1 import, CSBWin CSBGAME, and fail-closed DSA corpus gates");
    CHECK(csb_v1_boot_runtime_load_original_save_receipt_pc34(
              &p, save_path, &original_save_receipt) == 1 &&
              original_save_receipt.valid &&
              original_save_receipt.native_csb_header_valid &&
              original_save_receipt.runtime_load_succeeded &&
              original_save_receipt.runtime_dungeon_ready &&
              original_save_receipt.runtime_party_ready &&
              original_save_receipt.runtime_champion_count_after >= 0 &&
              original_save_receipt.native_header_fnv1a != 0u &&
              strcmp(original_save_receipt.dungeon_md5, p.dungeon_md5) == 0 &&
              original_save_receipt.source_identity_hash != 0u &&
              strcmp(original_save_receipt.save_path, save_path) == 0 &&
              strstr(original_save_receipt.source_evidence,
                     "LOADSAVE.C F0435") != NULL,
          "native CSB save handoff is source-owned and cannot use CSBWin fallback");
    CHECK(csb_v1_boot_runtime_import_csbwin_save_from_path_pc34(
              &p,
              csbwin_save_path,
              &save_import_receipt) == 1 &&
              save_import_receipt.valid &&
              save_import_receipt.csbwin_runtime_load_attempted &&
              save_import_receipt.csbwin_runtime_load_succeeded &&
              save_import_receipt.csbwin_runtime_load_code ==
                  CSB_V1_LOAD_OK &&
              save_import_receipt.csbwin_shape ==
                  CSB_V1_CSBWIN_SHAPE_CSBGAME_V20 &&
              save_import_receipt.csbwin_file_kind ==
                  CSB_V1_CSBWIN_SAVE_FILE_CSBGAME_DAT &&
              save_import_receipt.runtime_party_loaded_after &&
              save_import_receipt.runtime_import_source_after ==
                  CSB_SAVE_IMPORT_SOURCE &&
              save_import_receipt.runtime_champion_count_after > 0 &&
              save_import_receipt.runtime_champion_count_after <=
                  CSB_V1_MAX_CHAMPIONS &&
              save_import_receipt.runtime_game_time_after ==
                  p.runtime.game_time,
          "boot CSBWin import path runs the runtime loader and publishes imported party state");
    CHECK(csb_v1_boot_runtime_dsa_save_handoff_receipt_pc34(
              &save_import_receipt,
              &dsa_handoff_receipt) == 0 &&
              !dsa_handoff_receipt.valid &&
              dsa_handoff_receipt.save_import_receipt_consumed &&
              dsa_handoff_receipt.runtime_load_consumed &&
              !dsa_handoff_receipt.dsa_corpus_positive &&
              !dsa_handoff_receipt.dsa_runtime_handoff_ready &&
              dsa_handoff_receipt.runtime_party_loaded &&
              dsa_handoff_receipt.runtime_import_source_after ==
                  CSB_SAVE_IMPORT_SOURCE &&
              strcmp(dsa_handoff_receipt.decision_label,
                     "reject_dsa_corpus_no_extended_features") == 0 &&
              strstr(dsa_handoff_receipt.source_evidence,
                     "Extended Features DSA") != NULL,
          "loader-ready CSBGAME import cannot masquerade as a DSA save-runtime handoff");

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
    CHECK(write_source_layout_dungeon(dungeon_path, 2) == 0,
          "source-layout DUNGEON.DAT written for imported party handoff");
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
              mirror_receipt.view.current_map_difficulty == 0 &&
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
    CHECK(runtime_party.Champions[1].Direction == CSB_V1_DIR_NORTH,
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
    CSB_V1_StartupRealReceipt reissue_source;
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
    snprintf(receipt.graphics_md5, sizeof(receipt.graphics_md5), "%s",
             "00000000000000000000000000000000");
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 0 &&
              receipt.receipt_hash == 0u,
          "startup real-asset receipt rejects graphics MD5 drift before session publication");
    snprintf(receipt.graphics_md5, sizeof(receipt.graphics_md5), "%s",
             "61fbfd56887c94adc26888a9491c6611");
    snprintf(receipt.dungeon_md5, sizeof(receipt.dungeon_md5), "%s",
             "00000000000000000000000000000000");
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 0 &&
              receipt.receipt_hash == 0u,
          "startup real-asset receipt rejects Dungeon MD5 drift before session publication");
    snprintf(receipt.dungeon_md5, sizeof(receipt.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    CHECK(csb_v1_startup_real_receipt_recompute_hash(&receipt) == 0 &&
              receipt.receipt_hash == 0u,
          "startup real-asset receipt requires fresh admission after MD5 drift");
    reissue_source = receipt;
    CHECK(csb_v1_startup_real_receipt_from_profile_fields(
              tmp_dir, reissue_source.graphics_path,
              reissue_source.dungeon_path, reissue_source.graphics_md5,
              reissue_source.dungeon_md5, reissue_source.graphics_size_bytes,
              reissue_source.dungeon_size_bytes, reissue_source.variant_id,
              reissue_source.graphics_kind, reissue_source.max_depth,
              reissue_source.assets_verified,
              reissue_source.graphics_verified,
              reissue_source.dungeon_verified, &receipt) == 1 &&
              receipt.receipt_hash != 0u && !receipt.invalidated,
          "startup real-asset scanner owner reissues only the original verified MD5 pair");
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
    CHECK(write_source_layout_dungeon(dungeon_path, 2) == 0,
          "source-layout DUNGEON.DAT written for M564 boot handoff");
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

static void test_enter_game_loads_real_m564_object_names_when_supplied(void)
{
    const char *graphics_path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    const char *dungeon_path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    CSB_V1_BootProfile p;

    if (!graphics_path || !graphics_path[0] || !dungeon_path || !dungeon_path[0]) {
        printf("  SKIP: real M564 test needs FIRESTAFF_CSB_GRAPHICS_DAT and FIRESTAFF_CSB_DUNGEON_DAT\n");
        return;
    }

    /* ReDMCSB OBJECT.C:58-61 loads M564_GRAPHIC_OBJECT_NAMES (DEFS.H:2394
     * item 694) before object UI reads icon-indexed names. The caller supplies
     * the hash-verified PC3.4 pair; no test-side GRAPHICS.DAT is substituted. */
    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
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
          "real PC3.4 pair enters the CSB boot handoff");
    CHECK(p.runtime.object_name_table_valid == 1,
          "real PC3.4 GRAPHICS.DAT item 694 loads the M564 name stream");
    CHECK(p.runtime.object_names[0][0] != '\0',
          "real M564 stream supplies a non-empty first object name");
    /* ReDMCSB MENU.C F0620:543-551 loads C699_GRAPHIC_ACTION_NAMES into
     * G0490 before the action UI reads its text. These assertions stay bound
     * to the supplied hash-verified PC3.4 corpus, not a test-side table. */
    CHECK(p.runtime.action_name_table_valid == 1,
          "real PC3.4 GRAPHICS.DAT item 699 loads the C699 action-name stream");
    CHECK(strcmp(csb_v1_runtime_action_name_c699(&p.runtime, 1u), "BLOCK") == 0,
          "real C699 action-name stream supplies BLOCK at index 1");
    CHECK(strcmp(csb_v1_runtime_action_name_c699(&p.runtime, 43u), "FUSE") == 0,
          "real C699 action-name stream supplies FUSE at index 43");
    /* G0489 is executable-initialized PC3.4 source data (MENU.C:90-136),
     * not a GRAPHICS.DAT record. The boot runtime owns the exact six-byte
     * rows independently of M11's DM1 table. */
    CHECK(p.runtime.action_set_table_valid == 1,
          "PC3.4 runtime owns the ReDMCSB G0489 action-set source table");
    {
        unsigned char action_set[3];
        CHECK(csb_v1_runtime_action_set_indices_g0489(
                  &p.runtime, 2u, action_set) == 1 &&
              action_set[0] == 6u && action_set[1] == 7u &&
              action_set[2] == 8u,
              "CSB G0489 set 2 preserves PUNCH KICK WAR CRY");
        CHECK(csb_v1_runtime_action_set_indices_g0489(
                  &p.runtime, 5u, action_set) == 1 &&
              action_set[0] == 13u && action_set[1] == 255u &&
              action_set[2] == 255u,
              "CSB G0489 set 5 preserves the one-row STAB action set");
    }
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
    CHECK(write_source_layout_dungeon(dungeon_path, 2) == 0,
          "source-layout DUNGEON.DAT written for party rotation follow-up");
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
    /* CHAMPION.C F0284 never consults the dungeon or the party: after
     * boot_cleanup the zeroed runtime is in the same state as a new PC34
     * game before party import, where the first turn command must still
     * rotate G0308_i_PartyDirection.  A no-op rotation to the current
     * (zeroed) direction and a real 90-degree turn both stay on the
     * F0284 boundary and return 0; only the argument validations above
     * may reject. */
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_NORTH) == 0,
          "rotate_party to the zeroed direction is an F0284 no-op after "
          "boot_cleanup (new-game boundary, no dungeon/party consulted)");
    CHECK(csb_v1_runtime_rotate_party(&p.runtime, CSB_V1_DIR_EAST) == 0,
          "rotate_party still turns the zeroed runtime after boot_cleanup "
          "(CHAMPION.C F0284 writes G0308_i_PartyDirection unconditionally)");
    CHECK(p.runtime.party_dir == CSB_V1_DIR_EAST,
          "post-cleanup rotation commits party_dir (F0284 G0308 write)");
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

    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects a verified profile when its dungeon cannot be materialized");
    CHECK(p.state == CSB_V1_BOOT_STATE_ASSETS_READY,
          "unreadable dungeon leaves the profile outside RUNTIME_READY");
    CHECK(p.runtime.dungeon_handle == NULL,
          "dungeon_handle stays NULL when the path cannot be opened");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "dungeon_asset.path still points at the verified path even if load fails");
    CHECK(p.engine_version_displayed == 0,
          "unreadable dungeon does not advertise a CSB runtime");

    /* Free the global dungeon singleton if the loader left anything set. */
    csb_v1_dungeon_set_current(NULL);
}

static void test_enter_game_rejects_legacy_fixture_dungeon(void)
{
    CSB_V1_BootProfile p;
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-legacy-dng";

    (void)TEST_MKDIR(tmp_dir);
    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_legacy_fixture_dungeon(p.dungeon_path) == 0,
          "legacy unit-fixture dungeon written");
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "legacy fixture cannot enter a CSB runtime");
    CHECK(p.runtime.dungeon_handle == NULL &&
              csb_v1_dungeon_get_current() == NULL,
          "boot rejects legacy fixture as a live CSB dungeon");
    csb_v1_boot_cleanup(&p);
}

static void test_enter_game_runtime_handoff_is_idempotent(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-idempotent";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_source_layout_dungeon(dungeon_path, 7) == 0,
          "source-layout DUNGEON.DAT written for idempotence test");

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
    CHECK(write_source_layout_dungeon(dungeon_path, 2) == 0,
          "source-layout DUNGEON.DAT written for CSB V2 profile fallback guard");
    CHECK(write_source_layout_dungeon(bonus_dungeon_path, 7) == 0,
          "source-layout DUNGEONB.DAT written for CSB bonus-dungeon handoff");

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
    CHECK(csb_v1_runtime_try_load_bonus_dungeon(&p.runtime) == 0,
          "runtime rejects an unregistered synthetic sibling bonus dungeon");
    CHECK(csb_v1_runtime_get_bonus_dungeon_path(&p.runtime) == NULL,
          "rejected bonus dungeon does not publish a path");
    CHECK(strcmp(p.runtime.dungeon_path, dungeon_path) == 0,
          "rejected bonus dungeon preserves the authenticated dungeon owner");
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
              !runtime_receipt.bonus_dungeon_loaded &&
              !runtime_receipt.sync_profile_state,
          "runtime startup plan fails closed for an unregistered bonus dungeon");

    snprintf(runtime_save_path,
             sizeof(runtime_save_path),
             "%s/runtime-plan-resume.fsav",
             tmp_dir);
    p.runtime.party_x = 12;
    p.runtime.party_y = 13;
    p.runtime.party_dir = 2;
    p.runtime.timeline_queue.gameTick = p.runtime.game_time;
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
    const char *utility_path = "/tmp/firestaff-csb-v1-runtime-utility.adf";
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
    set_csb_utility_disk_env(NULL);
    csb_v1_runtime_init(&runtime, NULL);
    CHECK(csb_v1_runtime_import_dm1_party_path(&runtime,
                                               path,
                                               &imported_count,
                                               &utility_state,
                                               utility_prompt,
                                               sizeof(utility_prompt)) == 0,
          "runtime DM1 import rejects missing Utility Disk media");
    csb_v1_runtime_cleanup(&runtime);
    CHECK(write_csb_utility_adf_fixture(utility_path),
          "runtime DM1 import fixture writes a valid Utility Disk identity");
    set_csb_utility_disk_env(utility_path);

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
                snprintf(launch.receipts.session_state.entrance_resume_path,
                         sizeof(launch.receipts.session_state.entrance_resume_path),
                         "%s", "/tmp/firestaff_csb_resume.sav");
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
                          runtime_receipt.bind_graphics_to_runtime_asset_loader ==
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
                          runtime_receipt.startup_asset_gate_valid &&
                          runtime_receipt.startup_asset_gate.valid &&
                          runtime_receipt.startup_asset_gate.title_assets_owned &&
                          runtime_receipt.startup_asset_gate.entrance_assets_owned &&
                          runtime_receipt.startup_asset_gate.hud_assets_owned &&
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
    set_csb_utility_disk_env(NULL);
    remove(utility_path);
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

static void test_door_opening_runtime_handoff_owns_hud_transition(void)
{
    CSB_V1_BootProfile boot;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_BootStartupDoorRuntimeReceipt_PC34 receipt;
    CSB_V1_DungeonData dummy_dungeon;
    unsigned char title;
    unsigned char presents;
    unsigned char chaos;
    unsigned char strikes;
    unsigned char left_door;
    unsigned char right_door;
    unsigned char entrance;
    unsigned char inventory[224 * 136];
    unsigned char resurrect[144 * 73];

    csb_v1_boot_profile_init(&boot);
    memset(&snapshot, 0, sizeof(snapshot));
    memset(&dummy_dungeon, 0, sizeof(dummy_dungeon));
    dummy_dungeon.level_count = 1;
    dummy_dungeon.map_difficulty[0] = -1;
    boot.runtime.dungeon_handle = &dummy_dungeon;
    boot.runtime.current_level = 0;
    boot.runtime.party_x = 2;
    boot.runtime.party_y = 0;
    boot.runtime.party_dir = CSB_V1_DIR_SOUTH;
    boot.runtime.party_state_valid = 1;
    boot.runtime.party_state.ChampionCount = 1;
    boot.runtime.party_state.LeaderIndex = 0;
    snprintf(boot.graphics_path, sizeof(boot.graphics_path),
             "/tmp/firestaff_csb_GRAPHICS.DAT");
    boot.assets_verified = 1;
    boot.graphics_verified = 1;
    boot.dungeon_verified = 1;
    snapshot.boot_profile = &boot;
    snapshot.entrance_active = 1;
    snapshot.opening_active = 1;
    snapshot.opening_step = 31;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;

    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    session.valid = 1;
    session.real_asset_matched = 1;
    session.title_assets_ready = 1;
    session.title_presents_ready = 1;
    session.title_chaos_ready = 1;
    session.title_strikes_back_ready = 1;
    session.entrance_assets_ready = 1;
    session.door_assets_ready = 1;
    session.hud_assets_bound = 1;
    session.full_startup_ready = 1;
    session.rejects_legacy_wrappers = 1;
    session.generation = 1u;
    session.surfaces.valid = 1;
    session.surfaces.real_asset_matched = 1;
    session.surfaces.title_regions_ready = 1;
    session.surfaces.opening_frame_ready = 1;
    session.surfaces.entrance_screen_ready = 1;
    session.surfaces.hud_surfaces_ready = 1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &title, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 320,
            .height = 153, .source_asset_id = 1, .transparent_color = -1,
            .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &presents, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 320,
            .height = 16, .source_asset_id = 1, .source_y = 137,
            .transparent_color = -1, .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &chaos, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 320,
            .height = 80, .source_asset_id = 1, .transparent_color = -1,
            .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &strikes, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 320,
            .height = 57, .source_asset_id = 1, .source_y = 80,
            .transparent_color = 0, .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &left_door, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 105,
            .height = 161, .source_asset_id = 2, .transparent_color = -1,
            .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &right_door, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 128,
            .height = 161, .source_asset_id = 3, .transparent_color = -1,
            .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34] =
        (CSB_V1_StartupRuntimeSurface_PC34){ .pixels = &entrance, .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34, .width = 320,
            .height = 200, .source_asset_id = 4, .transparent_color = -1,
            .valid = 1 };
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].source_kind =
        CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].pixels = inventory;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].width = 224;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].height = 136;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].source_asset_id = 17;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].transparent_color = -1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].source_kind =
        CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].pixels = resurrect;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].width = 144;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].height = 73;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].source_asset_id = 40;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].transparent_color = 6;
    session.hud_inventory_binding.verified = 1;
    session.hud_inventory_binding.source =
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34;
    session.hud_inventory_binding.graphic_index = 17u;
    snprintf(session.hud_inventory_binding.path,
             sizeof(session.hud_inventory_binding.path), "%s",
             boot.graphics_path);
    session.hud_resurrect_binding.verified = 1;
    session.hud_resurrect_binding.source =
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34;
    session.hud_resurrect_binding.graphic_index = 40u;
    snprintf(session.hud_resurrect_binding.path,
             sizeof(session.hud_resurrect_binding.path), "%s",
             boot.graphics_path);
    session.playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session.playback.title_phase_mask = 0x0f;
    session.playback.entrance_music_active = 1;
    session.playback.entrance_complete = 1;
    session.playback.no_fallback_routes = 1;

    CHECK(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) == 1 &&
              receipt.valid && receipt.door_opening_finished &&
              receipt.runtime_view_ready && receipt.hud_session_ready &&
              receipt.route == CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_HUD_READY_PC34 &&
              receipt.runtime_mirror.valid &&
              receipt.runtime_mirror.view.level_loaded &&
              receipt.runtime_mirror.view.party_x == 2 &&
              receipt.runtime_mirror.view.party_y == 0 &&
              session.playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34,
          "door completion atomically hands the live dungeon pose to the verified HUD session");

    session.playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session.playback.entrance_music_active = 1;
    boot.runtime.dungeon_handle = NULL;
    CHECK(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) == 1 && receipt.valid &&
              receipt.door_opening_finished && !receipt.runtime_view_ready &&
              !receipt.hud_session_ready &&
              receipt.route == CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34 &&
              session.playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
          "door completion blocks the HUD transition when no dungeon is live");

    csb_v1_runtime_cleanup(&boot.runtime);
}

static void test_runtime_utility_startup_receipt_facades(void)
{
    CSB_V1_BootProfile boot;
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_UtilRenderPlan plan;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupActionReceipt_PC34 boot_action_receipt;
    CSB_V1_UtilRenderPlan receipt_utility_plan;
    CSB_V1_StartupPresentationReceipt_PC34 presentation_receipt;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 route_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 view_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 poisoned_view_receipt;
    CSB_V1_BootStartupRenderViewReceipt_PC34 runtime_view_receipt;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness_receipt;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_draw_receipt;
    CSB_V1_BootStartupCaptureReceipt_PC34 capture_receipt;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 packaged_proof;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 packaged_proof_from_snapshot;
    CSB_V1_BootStartupHostViewReceipt_PC34 host_view_receipt;
    CSB_V1_BootStartupHostViewReceipt_PC34 poisoned_host_view_receipt;
    CSB_V1_BootStartupM11PresentationReceipt_PC34 m11_presentation_receipt;
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 host_view_draw_receipt;
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 host_input_dispatch;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 host_ownership;
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 visual_sequence;
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 runtime_host_gate;
    CSB_V1_StartupRuntimeAssetSession_PC34 full_session;
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    CSB_V1_StartupCompleteSupportReceipt_PC34 complete_support;
    CSB_V1_StartupCompleteSupportReceipt_PC34 partial_complete_support;
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 release_app_capture;
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 partial_release_app_capture;
    CSB_V1_StartupPresentedAppCaptureFacts_PC34 presented_app_facts;
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 presented_app_capture;
    CSB_V1_StartupRenderExecutor_PC34 hud_draw_executor;
    CSB_V1_StartupRenderExecutor_PC34 capture_render_executor;
    TestHudMenuDrawProbe hud_draw_probe;
    TestStartupRenderProbe capture_render_probe;
    CSB_V1_StartupRenderPlan_PC34 receipt_title_plan;
    CSB_V1_StartupRenderPlan_PC34 receipt_closed_door_plan;
    int visual_sequence_ok;
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
    visual_sequence_ok =
        csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
            &boot, &visual_sequence);
    CHECK(visual_sequence_ok &&
              visual_sequence.valid &&
              visual_sequence.real_asset_matched &&
              visual_sequence.sequence_capture_hash != 0u &&
              visual_sequence.title_sample_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              visual_sequence.title_unique_sample_hash_count ==
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
              visual_sequence.source_title_chaos_zoom_ticks == 20 &&
              visual_sequence.source_title_chaos_hold_ticks == 20 &&
              visual_sequence.source_title_strikes_back_ticks == 2 &&
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
    CHECK(csb_v1_boot_startup_runtime_host_capture_gate_receipt_from_profile_pc34(
              &boot,
              &capture_render_executor,
              &runtime_host_gate) == 1 &&
              runtime_host_gate.valid &&
              runtime_host_gate.runtime_visual_valid &&
              runtime_host_gate.runtime_visual.valid &&
              runtime_host_gate.runtime_visual.visual_sequence_valid &&
              runtime_host_gate.runtime_visual.real_asset_matched &&
              runtime_host_gate.runtime_visual.sequence_capture_hash ==
                  visual_sequence.sequence_capture_hash &&
              runtime_host_gate.runtime_visual.runtime_capture_hash != 0u &&
              runtime_host_gate.runtime_visual.title_runtime_consumed &&
              runtime_host_gate.runtime_visual.title_runtime_sample_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              runtime_host_gate.runtime_visual
                      .title_runtime_unique_sample_hash_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              runtime_host_gate.runtime_visual.title_runtime_all_stages_consumed &&
              runtime_host_gate.runtime_visual.title_runtime_sample_hashes[0] ==
                  visual_sequence.title_sample_hashes[0] &&
              runtime_host_gate.runtime_visual.title_runtime_sample_hashes[1] ==
                  visual_sequence.title_sample_hashes[1] &&
              runtime_host_gate.runtime_visual.title_runtime_sample_hashes[2] ==
                  visual_sequence.title_sample_hashes[2] &&
              runtime_host_gate.runtime_visual.title_runtime_sample_hashes[3] ==
                  visual_sequence.title_sample_hashes[3] &&
              runtime_host_gate.runtime_visual.closed_door_hud_runtime_consumed &&
              runtime_host_gate.runtime_visual.utility_hud_runtime_consumed &&
              runtime_host_gate.runtime_visual.door_opening_delay_runtime_consumed &&
              runtime_host_gate.runtime_visual.door_opening_frame_runtime_consumed &&
              runtime_host_gate.runtime_visual.credits_runtime_consumed &&
              runtime_host_gate.runtime_visual.title_draw_consumed &&
              runtime_host_gate.runtime_visual.closed_door_hud_draw_consumed &&
              runtime_host_gate.runtime_visual.utility_hud_draw_consumed &&
              runtime_host_gate.runtime_visual.door_opening_frame_draw_consumed &&
              runtime_host_gate.runtime_visual.credits_surface_draw_consumed &&
              runtime_host_gate.runtime_visual.no_fallback_callbacks &&
              runtime_host_gate.runtime_visual.no_wrapper_fallback_routes &&
              runtime_host_gate.runtime_visual.draw_consumes_receipt_only &&
              runtime_host_gate.runtime_visual.input_consumes_receipt_only &&
              runtime_host_gate.visual_sequence_valid &&
              runtime_host_gate.route_hardening_valid &&
              runtime_host_gate.all_runtime_routes_consumed &&
              runtime_host_gate.title_runtime_captured &&
              runtime_host_gate.title_runtime_unique_sample_hash_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              runtime_host_gate.title_presents_runtime_captured &&
              runtime_host_gate.title_chaos_zoom_runtime_captured &&
              runtime_host_gate.title_chaos_hold_runtime_captured &&
              runtime_host_gate.title_strikes_back_runtime_captured &&
              runtime_host_gate.title_runtime_phase_mask == 0x0f &&
              runtime_host_gate.title_runtime_expected_phase_mask == 0x0f &&
              runtime_host_gate.title_runtime_phase_route_complete &&
              runtime_host_gate.title_runtime_phase_hash_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              runtime_host_gate.title_runtime_phase_hashes[0] ==
                  visual_sequence.title_sample_hashes[0] &&
              runtime_host_gate.title_runtime_phase_hashes[1] ==
                  visual_sequence.title_sample_hashes[1] &&
              runtime_host_gate.title_runtime_phase_hashes[2] ==
                  visual_sequence.title_sample_hashes[2] &&
              runtime_host_gate.title_runtime_phase_hashes[3] ==
                  visual_sequence.title_sample_hashes[3] &&
              runtime_host_gate.title_runtime_phase_hash != 0u &&
              runtime_host_gate.closed_door_hud_runtime_captured &&
              runtime_host_gate.utility_hud_runtime_captured &&
              runtime_host_gate.door_opening_runtime_captured &&
              runtime_host_gate.credits_runtime_captured &&
              runtime_host_gate.title_host_ownership_valid &&
              runtime_host_gate.closed_door_host_ownership_valid &&
              runtime_host_gate.utility_host_ownership_valid &&
              runtime_host_gate.door_opening_host_ownership_valid &&
              runtime_host_gate.credits_host_ownership_valid &&
              runtime_host_gate.title_host_draw_consumes_receipt_only &&
              runtime_host_gate.closed_door_host_draw_consumes_receipt_only &&
              runtime_host_gate.utility_host_draw_consumes_receipt_only &&
              runtime_host_gate.door_opening_host_draw_consumes_receipt_only &&
              runtime_host_gate.credits_host_draw_consumes_receipt_only &&
              runtime_host_gate.title_host_input_consumes_receipt_only &&
              runtime_host_gate.closed_door_host_input_consumes_receipt_only &&
              runtime_host_gate.utility_host_input_consumes_receipt_only &&
              runtime_host_gate.door_opening_host_input_consumes_receipt_only &&
              runtime_host_gate.credits_host_input_consumes_receipt_only &&
              runtime_host_gate.title_packaged_capture_hash != 0u &&
              runtime_host_gate.closed_door_packaged_capture_hash != 0u &&
              runtime_host_gate.utility_packaged_capture_hash != 0u &&
              runtime_host_gate.door_opening_packaged_capture_hash != 0u &&
              runtime_host_gate.credits_packaged_capture_hash != 0u &&
              runtime_host_gate.draw_consumes_receipt_only &&
              runtime_host_gate.input_consumes_receipt_only &&
              runtime_host_gate.no_fallback_callbacks &&
              runtime_host_gate.no_wrapper_fallback_routes &&
              runtime_host_gate.real_startup_assets_bound &&
              runtime_host_gate.real_startup_asset_role_count == 9 &&
              runtime_host_gate.real_startup_asset_binding_hash != 0u &&
              runtime_host_gate.host_route_wrappers_retired &&
              runtime_host_gate.no_loose_render_plan_exports &&
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
              runtime_host_gate.credits_route_hardening.credits_route_covered &&
              capture_render_probe.draw_title_count >= 1 &&
              capture_render_probe.draw_full_surface_count >= 4 &&
              capture_render_probe.draw_opening_frame_count >= 1 &&
              capture_render_probe.draw_opening_frame_count >= 1,
          "boot startup runtime host gate binds full visual capture to route hardening without wrapper fallbacks");
    csb_v1_boot_startup_runtime_asset_session_init_pc34(&full_session);
    full_session.valid = 1;
    full_session.real_asset_matched = 1;
    full_session.surfaces.valid = 1;
    full_session.surfaces.hud_surfaces_ready = 1;
    full_session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    full_session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    full_session.title_presents_ready = 1;
    full_session.title_chaos_ready = 1;
    full_session.title_strikes_back_ready = 1;
    full_session.entrance_assets_ready = 1;
    full_session.hud_assets_bound = 1;
    full_session.door_assets_ready = 1;
    full_session.full_startup_ready = 1;
    full_session.rejects_legacy_wrappers = 1;
    full_session.generation = 19u;
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &full_session,
              &full_runtime) == 1 &&
              csb_v1_boot_startup_complete_support_receipt_from_runtime_and_host_pc34(
                  &full_runtime,
                  &runtime_host_gate,
                  &complete_support) == 1 &&
              complete_support.valid &&
              complete_support.full_runtime_valid &&
              complete_support.host_capture_gate_valid &&
              complete_support.real_asset_matched &&
              complete_support.title_sequence_ready &&
              complete_support.title_phase_route_complete &&
              complete_support.title_runtime_phase_mask == 0x0f &&
              complete_support.title_runtime_expected_phase_mask == 0x0f &&
              complete_support.title_runtime_phase_hash ==
                  runtime_host_gate.title_runtime_phase_hash &&
              complete_support.title_runtime_phase_hash_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              complete_support.title_runtime_phase_hashes[0] ==
                  runtime_host_gate.title_runtime_phase_hashes[0] &&
              complete_support.title_runtime_phase_hashes[1] ==
                  runtime_host_gate.title_runtime_phase_hashes[1] &&
              complete_support.title_runtime_phase_hashes[2] ==
                  runtime_host_gate.title_runtime_phase_hashes[2] &&
              complete_support.title_runtime_phase_hashes[3] ==
                  runtime_host_gate.title_runtime_phase_hashes[3] &&
              complete_support.title_presents_ready &&
              complete_support.title_chaos_ready &&
              complete_support.title_strikes_back_ready &&
              complete_support.entrance_ready &&
              complete_support.hud_ready &&
              complete_support.door_ready &&
              complete_support.playback_route_ready &&
              complete_support.title_to_hud_same_session &&
              complete_support.playback_route_hash ==
                  full_runtime.playback_route_hash &&
              complete_support.runtime_host_routes_ready &&
              complete_support.draw_consumes_receipt_only &&
              complete_support.input_consumes_receipt_only &&
              complete_support.no_legacy_wrappers &&
              complete_support.no_fallback_callbacks &&
              complete_support.no_wrapper_fallback_routes &&
              complete_support.host_route_wrappers_retired &&
              complete_support.no_loose_render_plan_exports &&
              complete_support.real_startup_assets_bound &&
              complete_support.credits_packaged_capture_hash ==
                  runtime_host_gate.credits_packaged_capture_hash &&
              complete_support.real_startup_asset_binding_hash ==
                  runtime_host_gate.real_startup_asset_binding_hash &&
              complete_support.session_generation == 19u &&
              complete_support.playback_route_hash != 0u &&
              complete_support.runtime_host_gate_hash ==
                  runtime_host_gate.runtime_host_gate_hash &&
              complete_support.complete_support_hash != 0u &&
              strstr(complete_support.source_evidence, "TITLE.C F0437") != NULL,
          "CSB complete-support receipt joins full startup runtime with PRESENTS/CHAOS/STRIKES HUD and door host capture");
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &complete_support,
              &release_app_capture) == 1 &&
              release_app_capture.valid &&
              release_app_capture.complete_support_valid &&
              release_app_capture.host_capture_gate_valid &&
              release_app_capture.release_app_capture_ready &&
              release_app_capture.title_release_app_capture_ready &&
              release_app_capture.closed_door_release_app_capture_ready &&
              release_app_capture.utility_release_app_capture_ready &&
              release_app_capture.door_opening_release_app_capture_ready &&
              release_app_capture.credits_release_app_capture_ready &&
              release_app_capture.title_sequence_capture_ready &&
              release_app_capture.title_sequence_host_consumer_ready &&
              release_app_capture.title_sequence_same_capture_route &&
              release_app_capture.title_host_consumer_ready &&
              release_app_capture.closed_door_host_consumer_ready &&
              release_app_capture.utility_host_consumer_ready &&
              release_app_capture.door_opening_host_consumer_ready &&
              release_app_capture.credits_host_consumer_ready &&
              release_app_capture.route_specific_host_consumers_ready &&
              release_app_capture.hud_door_capture_ready &&
              release_app_capture.hud_door_host_consumers_ready &&
              release_app_capture.hud_door_same_capture_route &&
              release_app_capture.title_phase_route_complete &&
              release_app_capture.title_runtime_phase_mask == 0x0f &&
              release_app_capture.title_runtime_expected_phase_mask == 0x0f &&
              release_app_capture.title_runtime_phase_hash ==
                  complete_support.title_runtime_phase_hash &&
              release_app_capture.title_runtime_phase_hash_count ==
                  CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
              release_app_capture.title_runtime_phase_hashes[0] ==
                  complete_support.title_runtime_phase_hashes[0] &&
              release_app_capture.title_runtime_phase_hashes[1] ==
                  complete_support.title_runtime_phase_hashes[1] &&
              release_app_capture.title_runtime_phase_hashes[2] ==
                  complete_support.title_runtime_phase_hashes[2] &&
              release_app_capture.title_runtime_phase_hashes[3] ==
                  complete_support.title_runtime_phase_hashes[3] &&
              release_app_capture.runtime_host_routes_ready &&
              release_app_capture.draw_consumes_receipt_only &&
              release_app_capture.input_consumes_receipt_only &&
              release_app_capture.no_fallback_callbacks &&
              release_app_capture.no_wrapper_fallback_routes &&
              release_app_capture.host_route_wrappers_retired &&
              release_app_capture.no_loose_render_plan_exports &&
              release_app_capture.release_app_real_asset_capture_ready &&
              release_app_capture.full_runtime_real_asset_matched &&
              release_app_capture.host_runtime_visual_real_asset_matched &&
              release_app_capture.release_app_real_asset_capture_hash != 0u &&
              release_app_capture.real_startup_assets_bound &&
              release_app_capture.title_packaged_capture_hash ==
                  runtime_host_gate.title_packaged_capture_hash &&
              release_app_capture.title_sequence_capture_hash != 0u &&
              release_app_capture.title_sequence_capture_hash !=
                  release_app_capture.release_app_capture_hash &&
              release_app_capture.closed_door_packaged_capture_hash ==
                  runtime_host_gate.closed_door_packaged_capture_hash &&
              release_app_capture.utility_packaged_capture_hash ==
                  runtime_host_gate.utility_packaged_capture_hash &&
              release_app_capture.door_opening_packaged_capture_hash ==
                  runtime_host_gate.door_opening_packaged_capture_hash &&
              release_app_capture.credits_packaged_capture_hash ==
                  runtime_host_gate.credits_packaged_capture_hash &&
              release_app_capture.hud_door_capture_hash != 0u &&
              release_app_capture.hud_door_capture_hash !=
                  release_app_capture.release_app_capture_hash &&
              release_app_capture.release_app_real_asset_capture_hash !=
                  release_app_capture.release_app_capture_hash &&
              release_app_capture.runtime_host_gate_hash ==
                  complete_support.runtime_host_gate_hash &&
              release_app_capture.complete_support_hash ==
                  complete_support.complete_support_hash &&
              release_app_capture.release_app_capture_hash != 0u &&
              strstr(release_app_capture.source_evidence,
                     "ENTRANCE.C F0580") != NULL,
          "CSB release/app capture gate consumes complete title HUD and door packaged hashes");
    memset(&presented_app_facts, 0, sizeof(presented_app_facts));
    presented_app_facts.running_from_macos_app_bundle = 1;
    presented_app_facts.mac_window_capture_ready = 1;
    presented_app_facts.presented_frame_captured = 1;
    presented_app_facts.presented_frame_width = 320;
    presented_app_facts.presented_frame_height = 200;
    presented_app_facts.presented_frame_indexed_pixels = 1;
    presented_app_facts.presented_frame_uses_real_csb_assets = 1;
    presented_app_facts.presented_frame_hash = 0x51b4c0deu;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 1 &&
              presented_app_capture.valid &&
              presented_app_capture.release_app_capture_valid &&
              presented_app_capture.running_from_macos_app_bundle &&
              presented_app_capture.mac_window_capture_ready &&
              presented_app_capture.presented_frame_captured &&
              presented_app_capture.presented_frame_geometry_ready &&
              presented_app_capture.presented_frame_pixels_ready &&
              presented_app_capture.presented_frame_real_asset_ready &&
              presented_app_capture.presented_title_sequence_ready &&
              presented_app_capture.presented_title_phase_mask_ready &&
              presented_app_capture.presented_hud_door_ready &&
              presented_app_capture.presented_hud_door_route_hash_ready &&
              presented_app_capture.presented_credits_ready &&
              presented_app_capture.presented_credits_route_hash_ready &&
              presented_app_capture.presented_route_aggregates_ready &&
              presented_app_capture.presented_wrapper_cleanup_ready &&
              presented_app_capture.presented_runtime_capture_boundary_ready &&
              presented_app_capture.release_app_capture_hash ==
                  release_app_capture.release_app_capture_hash &&
              presented_app_capture.title_sequence_capture_hash ==
                  release_app_capture.title_sequence_capture_hash &&
              presented_app_capture.hud_door_capture_hash ==
                  release_app_capture.hud_door_capture_hash &&
              presented_app_capture.credits_capture_hash ==
                  release_app_capture.credits_packaged_capture_hash &&
              presented_app_capture.presented_wrapper_cleanup_hash != 0u &&
              presented_app_capture.presented_wrapper_cleanup_hash !=
                  presented_app_capture.presented_app_capture_hash &&
              presented_app_capture.presented_frame_hash ==
                  presented_app_facts.presented_frame_hash &&
              presented_app_capture.presented_app_capture_hash != 0u &&
              strstr(presented_app_capture.source_evidence,
                     "CSBWin Graphics.cpp") != NULL,
          "CSB presented app capture requires Mac window real-asset frame above release route proof");
    presented_app_facts.mac_window_capture_ready = 0;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.mac_window_capture_ready &&
              presented_app_capture.presented_runtime_capture_boundary_ready,
          "CSB presented app capture rejects missing Mac window capture");
    presented_app_facts.mac_window_capture_ready = 1;
    presented_app_facts.presented_frame_uses_real_csb_assets = 0;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_frame_real_asset_ready,
          "CSB presented app capture rejects non-real presented frame");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.title_sequence_capture_ready = 0;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_title_sequence_ready &&
              !presented_app_capture.presented_route_aggregates_ready &&
              !presented_app_capture.presented_runtime_capture_boundary_ready,
          "CSB presented app capture rejects title sequence aggregate loss");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.hud_door_capture_ready = 0;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_hud_door_ready &&
              !presented_app_capture.presented_route_aggregates_ready &&
              !presented_app_capture.presented_runtime_capture_boundary_ready,
          "CSB presented app capture rejects HUD/door aggregate loss");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.host_route_wrappers_retired = 0;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_wrapper_cleanup_ready &&
              !presented_app_capture.presented_runtime_capture_boundary_ready,
          "CSB presented app capture rejects startup wrapper cleanup loss");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.title_runtime_phase_mask = 0x1u;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_title_phase_mask_ready &&
              !presented_app_capture.presented_route_aggregates_ready,
          "CSB presented app capture rejects incomplete PRESENTS/CHAOS/STRIKES phase mask");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.hud_door_capture_hash =
        partial_release_app_capture.title_sequence_capture_hash;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_hud_door_route_hash_ready &&
              !presented_app_capture.presented_route_aggregates_ready,
          "CSB presented app capture rejects HUD/door route hash collapsed into title sequence");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.credits_packaged_capture_hash = 0u;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              !presented_app_capture.presented_credits_ready &&
              !presented_app_capture.presented_credits_route_hash_ready &&
              !presented_app_capture.presented_route_aggregates_ready,
          "CSB presented app capture rejects missing credits package route");
    partial_release_app_capture = release_app_capture;
    partial_release_app_capture.credits_packaged_capture_hash =
        partial_release_app_capture.title_sequence_capture_hash;
    CHECK(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &partial_release_app_capture,
              &presented_app_facts,
              &presented_app_capture) == 0 &&
              !presented_app_capture.valid &&
              presented_app_capture.presented_credits_ready &&
              !presented_app_capture.presented_credits_route_hash_ready &&
              !presented_app_capture.presented_route_aggregates_ready,
          "CSB presented app capture rejects credits route hash collapsed into title sequence");
    partial_complete_support = complete_support;
    partial_complete_support.full_runtime.real_asset_matched = 0;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.release_app_real_asset_capture_ready &&
              !release_app_capture.full_runtime_real_asset_matched,
          "CSB release/app capture gate rejects non-real full runtime capture");
    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.runtime_visual.real_asset_matched = 0;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.release_app_real_asset_capture_ready &&
              !release_app_capture.host_runtime_visual_real_asset_matched,
          "CSB release/app capture gate rejects non-real host visual capture");
    partial_complete_support = complete_support;
    partial_complete_support.title_runtime_phase_hash_count = 3;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              release_app_capture.title_runtime_phase_hash_count == 3 &&
              !release_app_capture.title_sequence_capture_ready &&
              !release_app_capture.title_sequence_same_capture_route,
          "CSB release/app capture gate rejects collapsed title phase hashes");

    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.title_packaged_capture_hash = 0u;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.title_release_app_capture_ready &&
              !release_app_capture.title_sequence_capture_ready &&
              !release_app_capture.title_sequence_same_capture_route,
          "CSB release/app capture gate rejects missing title package");

    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.closed_door_packaged_capture_hash = 0u;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.closed_door_release_app_capture_ready &&
              !release_app_capture.hud_door_capture_ready &&
              !release_app_capture.hud_door_same_capture_route,
          "CSB release/app capture gate rejects partial closed-door HUD package");

    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.door_opening_packaged_capture_hash = 0u;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.door_opening_release_app_capture_ready &&
              !release_app_capture.hud_door_capture_ready &&
              !release_app_capture.hud_door_same_capture_route,
          "CSB release/app capture gate rejects partial door-opening package");
    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.utility_host_draw_consumes_receipt_only = 0;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.utility_host_consumer_ready &&
              !release_app_capture.route_specific_host_consumers_ready &&
              !release_app_capture.hud_door_host_consumers_ready &&
              !release_app_capture.hud_door_same_capture_route,
          "CSB release/app capture gate rejects utility HUD host wrappers");
    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.credits_packaged_capture_hash = 0u;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.credits_release_app_capture_ready,
          "CSB release/app capture gate rejects partial credits package");
    partial_complete_support = complete_support;
    partial_complete_support.host_capture_gate.credits_host_input_consumes_receipt_only = 0;
    CHECK(csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
              &partial_complete_support,
              &release_app_capture) == 0 &&
              !release_app_capture.valid &&
              !release_app_capture.credits_host_consumer_ready &&
              !release_app_capture.route_specific_host_consumers_ready,
          "CSB release/app capture gate rejects credits host wrappers");
    runtime_host_gate.title_chaos_hold_runtime_captured = 0;
    CHECK(csb_v1_boot_startup_complete_support_receipt_from_runtime_and_host_pc34(
              &full_runtime,
              &runtime_host_gate,
              &complete_support) == 0 &&
              !complete_support.valid &&
              !complete_support.title_chaos_ready,
          "CSB complete-support receipt rejects partial CHAOS title capture");
    memset(&facts, 0, sizeof(facts));
    facts.boot_profile = &boot;
    facts.utility_overlay_active = 1;
    facts.utility_selected_action_index = 0;
    facts.utility_imported_champion_count = 2;
    facts.utility_preview_active = 0;
    facts.utility_prompt = "CHAOS STRIKES BACK READY";
    facts.entrance_active = 1;
    facts.entrance_source_step = 4;
    facts.resume_available = 1;
    facts.resume_path = resume_path;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &boot;
    snapshot.entrance_active = facts.entrance_active;
    snapshot.entrance_source_step = facts.entrance_source_step;
    snapshot.utility_overlay_active = facts.utility_overlay_active;
    snapshot.utility_selected_action_index =
        facts.utility_selected_action_index;
    snapshot.utility_imported_champion_count =
        facts.utility_imported_champion_count;
    snapshot.utility_prompt = facts.utility_prompt;
    snapshot.resume_available = facts.resume_available;
    snapshot.resume_path = facts.resume_path;

    CHECK(csb_v1_runtime_util_render_plan_from_boot_profile_facts_pc34(
              facts.utility_selected_action_index,
              facts.utility_imported_champion_count,
              facts.boot_profile,
              facts.utility_prompt,
              facts.utility_preview_active,
              &plan) == 1,
          "runtime utility render receipt accepts boot profile facts");
    CHECK(plan.menu_row_count == CSB_V1_UTIL_MENU_ROW_COUNT &&
              strstr(plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL,
          "runtime utility render receipt owns boot utility facts");

    facts.utility_selected_action_index = 0;
    snapshot.utility_selected_action_index = facts.utility_selected_action_index;
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              2,
              &boot_action_receipt) == 1,
          "boot utility keyboard action receipt accepts startup snapshot");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 &&
              boot_action_receipt.utility_receipt.util_state_receipt
                      .selected_action_index == 1 &&
              boot_action_receipt.utility_receipt.util_receipt.result ==
                  CSB_V1_UTIL_APPLY_REDRAW &&
              !boot_action_receipt.utility_receipt.entrance_receipt_valid,
          "boot utility keyboard action receipt owns redraw receipt");

    CHECK(csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
              &snapshot,
              72,
              126,
              1u,
              &boot_action_receipt) == 1,
          "boot utility pointer action receipt accepts startup snapshot");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 &&
              boot_action_receipt.utility_receipt.util_receipt.result ==
                  CSB_V1_UTIL_APPLY_ENTRANCE_COMMAND &&
              boot_action_receipt.utility_receipt.entrance_receipt_valid,
          "boot utility pointer action receipt owns utility facts");

    facts.utility_selected_action_index = 1;
    snapshot.utility_selected_action_index = facts.utility_selected_action_index;
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              9,
              &boot_action_receipt) == 1,
          "boot utility keyboard action receipt handles accept");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 &&
              boot_action_receipt.utility_receipt.util_receipt.result ==
                  CSB_V1_UTIL_APPLY_ENTRANCE_COMMAND &&
              boot_action_receipt.utility_receipt.entrance_receipt_valid,
          "boot utility keyboard action receipt chains entrance receipt");

    snapshot.utility_overlay_active = 0;
    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              2,
              &boot_action_receipt) == 1,
          "boot entrance keyboard action receipt accepts startup snapshot");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 &&
              !boot_action_receipt.entrance_receipt.handled,
          "boot entrance keyboard action receipt ignores navigation input");

    CHECK(csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
              &snapshot,
              9,
              &boot_action_receipt) == 1,
          "boot entrance keyboard action receipt handles accept input");
    CHECK(boot_action_receipt.kind ==
                  CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 &&
              boot_action_receipt.entrance_receipt.handled,
          "boot entrance keyboard action receipt chains command receipt");

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
    CHECK(csb_v1_boot_startup_m11_presentation_receipt_from_snapshot_pc34(
              &snapshot,
              &m11_presentation_receipt) == 1 &&
              m11_presentation_receipt.valid &&
              m11_presentation_receipt.startup_render_plan_valid &&
              m11_presentation_receipt.startup_render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              !m11_presentation_receipt.utility_render_plan_valid &&
              !m11_presentation_receipt.hud_menu_draw_valid &&
              m11_presentation_receipt.capture_proof_valid &&
              m11_presentation_receipt.capture_proof.title_route &&
              m11_presentation_receipt.host_view_valid &&
              m11_presentation_receipt.host_draw_package_ready &&
              m11_presentation_receipt.host_draw_uses_receipt_package &&
              m11_presentation_receipt.no_legacy_render_wrapper_ready &&
              m11_presentation_receipt.legacy_plan_exports_inspection_only &&
              !m11_presentation_receipt.input_ready &&
              !m11_presentation_receipt.hud_ready &&
              !m11_presentation_receipt.runtime_ready,
          "M11 presentation receipt owns title package without wrapper-era plan chaining");
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
              route_receipt.presentation.redmcsb_chaos_zoom_ticks == 20 &&
              route_receipt.presentation.redmcsb_chaos_hold_ticks == 20 &&
              route_receipt.presentation.redmcsb_strikes_back_ticks == 2 &&
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
              strstr(host_view_receipt.render_draw.source_evidence,
                     "TITLE.C F0437") != NULL &&
              strstr(host_view_receipt.render_draw.source_evidence,
                     "ENTRANCE.C F0441") != NULL &&
              host_view_receipt.render_draw.primitive_commands_ready &&
              host_view_receipt.render_draw.title_asset_commands_ready &&
              host_view_receipt.host_draw_package_ready &&
              host_view_receipt.host_draw_uses_receipt_package &&
              host_view_receipt.no_legacy_render_wrapper_ready &&
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
              host_view_draw_receipt.render_draw_receipt_consumed &&
              host_view_draw_receipt.capture_proof_consumed &&
              host_view_draw_receipt.route_capture_proof_consumed &&
              host_view_draw_receipt.readiness_receipt_consumed &&
              host_view_draw_receipt.no_legacy_plan_fallback &&
              host_view_draw_receipt.fallback_callbacks_stripped &&
              host_view_draw_receipt.render_executed &&
              !host_view_draw_receipt.hud_menu_executed &&
              host_view_draw_receipt.real_asset_matched &&
              strstr(host_view_draw_receipt.source_evidence,
                     "ENTRANCE.C F0580") != NULL &&
              host_view_draw_receipt.primitive_commands_consumed &&
              host_view_draw_receipt.title_asset_commands_consumed &&
              !host_view_draw_receipt.closed_door_asset_commands_consumed &&
              !host_view_draw_receipt.opening_frame_command_consumed &&
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
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &runtime_view_receipt) == 1 &&
              runtime_view_receipt.title_after_swoosh_route &&
              runtime_view_receipt.render_plan.surface ==
                  view_receipt.render_plan.surface,
          "boot startup snapshot render-view receipt matches post-swoosh title route");
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
              view_receipt.title_source_x == 0 &&
              view_receipt.title_source_y == 0 &&
              view_receipt.title_source_w == 320 &&
              view_receipt.title_source_h == 80 &&
              view_receipt.title_dest_x == 152 &&
              view_receipt.title_dest_y == 78 &&
              view_receipt.title_dest_w == 16 &&
              view_receipt.title_dest_h == 4 &&
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
              receipt_title_plan.asset_commands[0].source_y == 0 &&
              receipt_title_plan.asset_commands[0].dest_h == 4 &&
              receipt_title_plan.title_source_y == 0 &&
              receipt_title_plan.title_dest_h == 4 &&
              receipt_title_plan.render_command_count == 2 &&
              receipt_title_plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 &&
              receipt_title_plan.render_commands[1].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34,
          "boot startup host-view title draw consumes render-view receipt fields");
    snapshot.title_frame = 79;
    snapshot.title_source_step = 21;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              view_receipt.title_source_step == 21 &&
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
    snapshot.title_source_step = 21;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &view_receipt) == 1 &&
              view_receipt.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              view_receipt.title_render_command_count == 2 &&
              view_receipt.title_phase_tick == 0 &&
              view_receipt.title_phase_tick_count ==
                  csb_v1_startup_title_chaos_hold_ticks_pc34() &&
              view_receipt.title_blit_kind ==
                  CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34 &&
              view_receipt.title_transparent_color == -1 &&
              view_receipt.title_source_x == 0 &&
              view_receipt.title_source_y == 0 &&
              view_receipt.title_source_w == 320 &&
              view_receipt.title_source_h == 80 &&
              view_receipt.title_dest_x == 0 &&
              view_receipt.title_dest_y == 40 &&
              view_receipt.title_dest_w == 320 &&
              view_receipt.title_dest_h == 80 &&
              !view_receipt.title_strikes_back_visible &&
              !view_receipt.title_presents_visible &&
              view_receipt.title_chaos_visible,
          "boot startup render-view receipt holds source CHAOS before STRIKES BACK");
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
              host_input_dispatch.input_render.action_valid &&
              host_input_dispatch.input_render.host_decision_valid &&
              host_input_dispatch.input_render.pre_input_readiness_valid &&
              !host_input_dispatch.input_render.post_input_readiness_valid &&
              !host_input_dispatch.input_render.hud_menu_draw_valid &&
              host_input_dispatch.input_render.input_consumed &&
              !host_input_dispatch.input_render.startup_redraw &&
              !host_input_dispatch.input_render.startup_hud_draw_ready &&
              host_input_dispatch.input_render.host_decision.blocked_by_title &&
              host_input_dispatch.input_render.host_decision.pre_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
              host_input_dispatch.input_render.host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 &&
              host_input_dispatch.input_render.pre_input_readiness
                  .post_ftl_title_active,
          "boot startup host input dispatch receipt owns title-block host decision");
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
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_runtime_host_capture_gate_receipt_from_profile_pc34(
              snapshot.boot_profile,
              &capture_render_executor,
              &runtime_host_gate) == 0 &&
              !runtime_host_gate.valid &&
              !runtime_host_gate.all_runtime_routes_consumed,
          "boot startup runtime host-capture gate rejects unverified raw title route");
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
              presentation_receipt.render_command_count == 4 &&
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
              presentation_receipt.redmcsb_closed_door_y == 30 &&
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
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.host_draw_package_ready &&
              host_view_receipt.host_draw_uses_receipt_package &&
              host_view_receipt.no_legacy_render_wrapper_ready,
          "boot startup snapshot host-view consumes utility draw package receipt");
    CHECK(csb_v1_boot_startup_m11_presentation_receipt_from_snapshot_pc34(
              &snapshot,
              &m11_presentation_receipt) == 1 &&
              m11_presentation_receipt.valid &&
              m11_presentation_receipt.startup_render_plan_valid &&
              m11_presentation_receipt.startup_render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              m11_presentation_receipt.hud_menu_draw_valid &&
              m11_presentation_receipt.hud_menu_draw.draw_utility_panel &&
              m11_presentation_receipt.utility_render_plan_valid &&
              m11_presentation_receipt.utility_render_plan.menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              m11_presentation_receipt.input_ready &&
              m11_presentation_receipt.hud_ready &&
              !m11_presentation_receipt.runtime_ready &&
              m11_presentation_receipt.host_view_valid &&
              m11_presentation_receipt.host_draw_package_ready &&
              m11_presentation_receipt.host_draw_uses_receipt_package &&
              m11_presentation_receipt.no_legacy_render_wrapper_ready &&
              m11_presentation_receipt.legacy_plan_exports_inspection_only &&
              m11_presentation_receipt.capture_proof.utility_menu_route,
          "M11 presentation receipt owns utility HUD package and readiness together");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
              &capture_receipt,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.render_plan_valid &&
              host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.hud_menu_draw_ready &&
              host_view_receipt.render_draw.primitive_commands_ready &&
              host_view_receipt.render_draw.closed_door_asset_commands_ready &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.hud_menu_draw.draw_utility_panel &&
              host_view_receipt.hud_menu_draw.utility_render_plan_valid &&
              host_view_receipt.readiness_valid &&
              host_view_receipt.readiness.hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_view_receipt.host_draw_package_ready &&
              host_view_receipt.host_draw_uses_receipt_package &&
              host_view_receipt.no_legacy_render_wrapper_ready &&
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
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_utility_selected_action_index ==
                  host_view_receipt.selected_utility_action_index &&
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
              host_view_draw_receipt.fallback_callbacks_stripped &&
              host_view_draw_receipt.primitive_commands_consumed &&
              !host_view_draw_receipt.title_asset_commands_consumed &&
              host_view_draw_receipt.closed_door_asset_commands_consumed &&
              !host_view_draw_receipt.opening_frame_command_consumed &&
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
              hud_draw_probe.last_waiting_for_input &&
              hud_draw_probe.last_utility_selected_action_index ==
                  hud_draw_receipt.selected_utility_action_index &&
              hud_draw_probe.last_surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "boot startup HUD/menu executor draws utility panel from readiness-gated receipt");
    CHECK(hud_draw_receipt.utility_render_plan_valid &&
              (receipt_utility_plan =
                   hud_draw_receipt.utility_render_plan,
               1) &&
              receipt_utility_plan.menu_row_count ==
                  CSB_V1_UTIL_MENU_ROW_COUNT &&
              receipt_utility_plan.menu_rows[0].selected &&
              !receipt_utility_plan.menu_rows[1].selected &&
              receipt_utility_plan.has_prompt_row &&
              strstr(receipt_utility_plan.prompt_row.text,
                     "CHAOS STRIKES BACK READY") != NULL,
          "boot startup utility render plan is owned by HUD/menu draw receipt");
    CHECK(csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
              &snapshot,
              &host_view_receipt) == 1 &&
              host_view_receipt.valid &&
              host_view_receipt.hud_menu_draw_valid &&
              host_view_receipt.hud_menu_draw.utility_render_plan_valid &&
              host_view_receipt.hud_menu_draw.utility_render_plan
                  .menu_rows[0]
                  .selected &&
              !host_view_receipt.hud_menu_draw.utility_render_plan
                   .preview_active &&
              !receipt_utility_plan.preview_active,
          "boot startup host-view receipt owns utility render plan without runtime-state facade");
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
              route_receipt.hud_menu_state.prompt[0] == '\0',
          "boot startup route receipt keeps closed entrance prompt no-draw");
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
                  view_receipt.closed_door_render_command_count &&
              receipt_closed_door_plan.render_commands[2].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34 &&
              receipt_closed_door_plan.asset_command_count ==
                  view_receipt.closed_door_asset_command_count &&
              receipt_closed_door_plan.menu_option_count == 4 &&
              receipt_closed_door_plan.menu_options[0].selected &&
              !receipt_closed_door_plan.menu_options[1].selected &&
              !receipt_closed_door_plan.blink_prompt_visible,
          "boot startup closed-door HUD/menu receipt rejects host text fallback");
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
              hud_draw_receipt.prompt[0] == '\0',
          "boot startup HUD/menu draw receipt keeps closed-door prompt no-draw");
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
              host_view_draw_receipt.fallback_callbacks_stripped &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_closed_doors_count == 1,
          "boot startup host-view draw receipt consumes closed-door render plus HUD without fallback text");
    poisoned_host_view_receipt = host_view_receipt;
    poisoned_host_view_receipt.render_draw.valid = 0;
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &poisoned_host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 0 &&
              !host_view_draw_receipt.valid &&
              !host_view_draw_receipt.no_legacy_plan_fallback &&
              capture_render_probe.draw_full_surface_count == 0 &&
              capture_render_probe.draw_closed_doors_count == 0,
          "boot startup host-view draw rejects HUD fallback without render-draw receipt");
    poisoned_host_view_receipt = host_view_receipt;
    poisoned_host_view_receipt.capture_proof.closed_door_menu_route = 0;
    poisoned_host_view_receipt.capture_proof.utility_menu_route = 0;
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &poisoned_host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 0 &&
              !host_view_draw_receipt.valid &&
              !host_view_draw_receipt.route_capture_proof_consumed &&
              !host_view_draw_receipt.no_legacy_plan_fallback &&
              capture_render_probe.draw_full_surface_count == 0 &&
              capture_render_probe.draw_closed_doors_count == 0,
          "boot startup host-view draw rejects mismatched HUD capture proof");
    render_probe_executor_init(&capture_render_executor,
                               &capture_render_probe);
    capture_render_probe.draw_full_surface_result = 0;
    CHECK(csb_v1_boot_startup_execute_host_view_receipt_pc34(
              &host_view_receipt,
              &capture_render_executor,
              &host_view_draw_receipt) == 1 &&
              host_view_draw_receipt.valid &&
              capture_render_probe.draw_full_surface_count == 1 &&
              host_view_draw_receipt.fallback_callbacks_stripped &&
              capture_render_probe.draw_closed_doors_count == 1,
          "boot startup closed-door host-view receipt refuses fallback when surface assets fail");
    CHECK(host_view_receipt.render_draw_valid &&
              host_view_receipt.render_draw.render_plan_valid &&
              host_view_receipt.render_draw.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              host_view_receipt.render_draw.render_plan.waiting_for_input,
          "boot startup host-view receipt returns packaged closed-door render plan");
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot,
              &runtime_view_receipt) == 1 &&
              runtime_view_receipt.closed_door_menu_route &&
              runtime_view_receipt.suppress_legacy_utility_fallback &&
              runtime_view_receipt.hud_menu_receipt_ready,
          "boot startup snapshot render-view receipt owns closed-door HUD gate");
    CHECK(runtime_view_receipt.render_plan_valid &&
              runtime_view_receipt.render_plan.surface ==
                  host_view_receipt.render_draw.render_plan.surface &&
              runtime_view_receipt.render_plan.waiting_for_input ==
                  host_view_receipt.render_draw.render_plan.waiting_for_input,
          "boot startup runtime render-view receipt matches host-view packaged plan");
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
              host_view_receipt.render_draw.primitive_commands_ready &&
              host_view_receipt.render_draw.opening_frame_command_ready &&
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
              host_view_draw_receipt.fallback_callbacks_stripped &&
              host_view_draw_receipt.primitive_commands_consumed &&
              !host_view_draw_receipt.title_asset_commands_consumed &&
              host_view_draw_receipt.closed_door_asset_commands_consumed &&
              host_view_draw_receipt.opening_frame_command_consumed &&
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
              host_view_draw_receipt.fallback_callbacks_stripped &&
              capture_render_probe.draw_full_surface_count == 1 &&
              capture_render_probe.draw_opening_frame_count == 0 &&
              capture_render_probe.draw_closed_doors_count == 0,
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
              host_view_draw_receipt.fallback_callbacks_stripped &&
              capture_render_probe.draw_full_surface_count == 1,
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
    CHECK(csb_v1_boot_startup_presentation_state_receipt_from_snapshot_pc34(
              &snapshot,
              &presentation_receipt) == 1 &&
              presentation_receipt.valid &&
              presentation_receipt.render_plan.waiting_for_input,
          "boot startup presentation receipt accepts snapshot fields");
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
              host_input_dispatch.input_render.action_valid &&
              host_input_dispatch.input_render.host_decision_valid &&
              host_input_dispatch.input_render.post_input_readiness_valid &&
              host_input_dispatch.input_render.hud_menu_draw_valid &&
              host_input_dispatch.input_render.draw_from_post_input &&
              host_input_dispatch.input_render.input_consumed &&
              host_input_dispatch.input_render.startup_redraw &&
              host_input_dispatch.input_render.host_decision.routed_to_utility &&
              !host_input_dispatch.input_render.host_decision.routed_to_entrance &&
              host_input_dispatch.input_render.host_decision.stays_on_startup &&
              !host_input_dispatch.input_render.host_decision.return_to_launcher &&
              host_input_dispatch.input_render.host_decision
                      .utility_selected_action_index == 1 &&
              host_input_dispatch.input_render.host_decision.pre_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              host_input_dispatch.input_render.host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              host_input_dispatch.input_render.startup_hud_draw_ready &&
              !host_input_dispatch.input_render.return_to_launcher &&
              host_input_dispatch.input_render.hud_menu_draw.kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 &&
              host_input_dispatch.input_render.hud_menu_draw
                      .selected_utility_action_index == 1 &&
              host_input_dispatch.input_render.hud_menu_draw
                      .utility_render_plan_valid &&
              host_input_dispatch.input_render.hud_menu_draw
                      .utility_render_plan.menu_rows[1].selected &&
              host_input_dispatch.input_render.post_input_readiness
                      .hud_menu_kind ==
                  CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34,
          "boot startup host input dispatch receipt owns utility redraw snapshot and post-input HUD draw");
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
              host_input_dispatch.input_render.host_decision_valid &&
              host_input_dispatch.input_render.post_input_readiness_valid &&
              !host_input_dispatch.input_render.hud_menu_draw_valid &&
              host_input_dispatch.input_render.input_consumed &&
              host_input_dispatch.input_render.startup_redraw &&
              !host_input_dispatch.input_render.startup_hud_draw_ready &&
              host_input_dispatch.input_render.host_decision.routed_to_entrance &&
              host_input_dispatch.input_render.host_decision
                      .entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              host_input_dispatch.input_render.post_input_readiness
                  .host_hud_blocked,
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
    CHECK(csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
              &snapshot,
              9,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.input_render_valid &&
              host_input_dispatch.input_render.host_decision_valid &&
              host_input_dispatch.input_render.input_consumed &&
              !host_input_dispatch.input_render.host_decision.routed_to_utility &&
              host_input_dispatch.input_render.host_decision.routed_to_entrance &&
              host_input_dispatch.input_render.host_decision.redraw_startup &&
              host_input_dispatch.input_render.host_decision.stays_on_startup &&
              host_input_dispatch.input_render.host_decision.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 &&
              host_input_dispatch.input_render.host_decision.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 &&
              host_input_dispatch.input_render.host_decision.post_render_route !=
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
              strcmp(host_input_dispatch.input_render.host_decision.status,
                     "CSB DOORS") == 0,
          "boot startup host input dispatch receipt owns entrance redraw host decision");
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
    CHECK(csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
              &snapshot,
              10,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.input_render_valid &&
              host_input_dispatch.input_render.host_decision_valid &&
              host_input_dispatch.input_render.pre_input_readiness_valid &&
              !host_input_dispatch.input_render.post_input_readiness_valid &&
              !host_input_dispatch.input_render.hud_menu_draw_valid &&
              host_input_dispatch.input_render.input_consumed &&
              host_input_dispatch.input_render.return_to_launcher &&
              !host_input_dispatch.input_render.startup_redraw &&
              !host_input_dispatch.input_render.startup_hud_draw_ready &&
              host_input_dispatch.input_render.host_decision.return_to_launcher &&
              !host_input_dispatch.input_render.host_decision.stays_on_startup &&
              !host_input_dispatch.input_render.host_decision.redraw_startup &&
              host_input_dispatch.input_render.host_decision.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34 &&
              host_input_dispatch.input_render.host_decision.post_render_route ==
                  CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 &&
              strcmp(host_input_dispatch.input_render.host_decision.status,
                     "BACK TO LAUNCHER") == 0,
          "boot startup host input dispatch receipt owns launcher-return host decision");
    CHECK(csb_v1_boot_startup_host_input_dispatch_pointer_from_snapshot_pc34(
              &snapshot,
              enter_menu_x,
              enter_menu_y,
              0U,
              &host_input_dispatch) == 1 &&
              host_input_dispatch.valid &&
              host_input_dispatch.startup_active &&
              !host_input_dispatch.pointer_button_relevant &&
              !host_input_dispatch.should_dispatch_input &&
              !host_input_dispatch.should_ignore_input &&
              !host_input_dispatch.input_render_valid,
          "boot startup host input dispatch ignores irrelevant buttons at CSB boundary");

    csb_v1_boot_cleanup(&boot);
}

static void test_startup_full_runtime_receipt_requires_complete_real_session(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupFullRuntimeReceipt_PC34 receipt;

    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    session.valid = 1;
    session.real_asset_matched = 1;
    session.surfaces.valid = 1;
    session.surfaces.hud_surfaces_ready = 1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    session.title_presents_ready = 1;
    session.title_chaos_ready = 1;
    session.title_strikes_back_ready = 1;
    session.entrance_assets_ready = 1;
    session.door_assets_ready = 1;
    session.hud_assets_bound = 1;
    session.generation = 7u;
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session,
              &receipt) == 0 &&
              !receipt.valid,
          "CSB full startup runtime receipt rejects legacy-wrapper route");

    session.rejects_legacy_wrappers = 1;
    session.full_startup_ready = 1;
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session,
              &receipt) == 1 &&
              receipt.valid &&
              receipt.real_asset_matched &&
              receipt.title_sequence_ready &&
              receipt.title_presents_ready &&
              receipt.title_chaos_ready &&
              receipt.title_strikes_back_ready &&
              receipt.entrance_ready &&
              receipt.hud_ready &&
              receipt.door_ready &&
              receipt.playback_route_ready &&
              receipt.playback_reaches_title &&
              receipt.playback_reaches_entrance &&
              receipt.playback_reaches_hud &&
              receipt.title_to_hud_same_session &&
              receipt.no_legacy_wrappers &&
              receipt.session_generation == 7u &&
              receipt.playback_route_hash != 0u &&
              strstr(receipt.source_evidence, "TITLE.C F0437") != NULL,
          "CSB full startup runtime receipt gates PRESENTS/CHAOS/STRIKES HUD and door together");

    session.title_chaos_ready = 0;
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session,
              &receipt) == 0 &&
              !receipt.valid &&
              !receipt.title_sequence_ready &&
              !receipt.playback_route_ready,
          "CSB full startup runtime receipt rejects partial title sequence");
}

static void test_runtime_variant_hint_identity(void)
{
    static const struct {
        const char *hint;
        CSB_V1_VariantId expected;
    } cases[] = {
        { "pc34_en", CSB_V1_VARIANT_PC34_EN },
        { "PC34_MULTI", CSB_V1_VARIANT_PC34_MULTI },
        { "st20_en", CSB_V1_VARIANT_ST20_EN },
        { "st21_en", CSB_V1_VARIANT_ST21_EN },
        { "amiga35_en", CSB_V1_VARIANT_AMIGA35_EN },
        { "amiga35_multi", CSB_V1_VARIANT_AMIGA35_MULTI },
        { "st_f20j", CSB_V1_VARIANT_ST_F20J },
        { "st_f20e", CSB_V1_VARIANT_ST_F20E }
    };
    const char *tmp_dir = "/tmp/firestaff-csb-v1-graphics-hint";
    char graphics_path[ASSET_PATH_MAX];
    CSB_V1_AssetResult result;
    FILE *file;
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        CHECK(csb_v1_runtime_variant_from_hint(cases[i].hint) == cases[i].expected,
              "CSB known launcher hint resolves to its exact media variant");
    }
    CHECK(csb_v1_runtime_variant_from_hint(NULL) == CSB_V1_VARIANT_UNKNOWN,
          "CSB absent launcher hint retains broad hash discovery");
    CHECK(csb_v1_runtime_variant_from_hint("custom_dungeon") ==
              CSB_V1_VARIANT_UNKNOWN,
          "CSB unknown launcher hint retains broad hash discovery");
    CHECK(strcmp(csb_v1_runtime_get_variant_info(CSB_V1_VARIANT_ST20_EN)->md5_gfx,
                 "ebf6a57af3f27782e358c0490bfd2f2e") == 0 &&
              strcmp(csb_v1_runtime_get_variant_info(CSB_V1_VARIANT_ST21_EN)->md5_gfx,
                 "ebf6a57af3f27782e358c0490bfd2f2e") == 0,
          "CSB ST2.x variant tokens share the canonical floppy graphics identity");

    (void)TEST_MKDIR(tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    remove(graphics_path);
    file = fopen(graphics_path, "wb");
    CHECK(file != NULL, "CSB graphics-hint fixture opens");
    if (file) {
        static const unsigned char unverified[] = { 0x43u, 0x53u, 0x42u, 0x00u };
        CHECK(fwrite(unverified, 1u, sizeof(unverified), file) == sizeof(unverified),
              "CSB graphics-hint fixture writes unverified archive bytes");
        fclose(file);
        memset(&result, 0, sizeof(result));
        CHECK(csb_v1_runtime_find_graphics(tmp_dir, "pc34_en", &result) == NULL,
              "CSB selected PC3.4 variant rejects filename-only graphics substitute");
        memset(&result, 0, sizeof(result));
        CHECK(csb_v1_runtime_find_graphics(tmp_dir, "custom_dungeon", &result) != NULL &&
                  result.kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
              "CSB unknown variant retains legacy filename fallback");
    }
    remove(graphics_path);
}

int main(void)
{
    const char *focus_dsa_save_handoff =
        getenv("FIRESTAFF_FOCUS_CSB_DSA_SAVE_HANDOFF");

    printf("=== CSB V1 Boot → Runtime Handoff Regression ===\n\n");
    test_enter_game_with_verified_profile_loads_dungeon();
    if (focus_dsa_save_handoff && focus_dsa_save_handoff[0] != '\0') {
        printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
        return failed ? 1 : 0;
    }
    test_enter_game_loads_m564_object_names_from_graphics_dat();
    test_enter_game_loads_real_m564_object_names_when_supplied();
    test_enter_game_preserves_imported_party_and_switches_leader();
    test_runtime_import_dm1_party_path_owns_utility_handoff();
    test_runtime_view_state_receipt_owns_scalar_handoff();
    test_door_opening_runtime_handoff_owns_hud_transition();
    test_runtime_utility_startup_receipt_facades();
    test_startup_real_asset_receipt_is_skip_safe_and_deterministic();
    test_startup_full_runtime_receipt_requires_complete_real_session();
    test_runtime_variant_hint_identity();
    test_enter_game_rotate_party_aligns_champion_state();
    test_enter_game_with_missing_dungeon_path_keeps_runtime_safe();
    test_enter_game_rejects_legacy_fixture_dungeon();
    test_enter_game_runtime_handoff_is_idempotent();
    test_enter_game_rejects_partial_or_misrouted_profiles();
    test_enter_game_v2_profile_labels_do_not_change_v1_handoff();
    test_utility_flow_new_game_handoff_preserves_leader_index();
    test_utility_import_confirmation_is_transactional();
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
