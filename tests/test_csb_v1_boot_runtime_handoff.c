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

static void test_enter_game_with_verified_profile_loads_dungeon(void)
{
    CSB_V1_BootProfile p;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 dispatch;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-test";
    int mkdir_ok = (TEST_MKDIR(tmp_dir) == 0) || 1; /* best-effort */

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
    char dungeon_path[ASSET_PATH_MAX];
    char bonus_dungeon_path[ASSET_PATH_MAX];
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

    csb_v1_boot_cleanup(&p);
}

int main(void)
{
    printf("=== CSB V1 Boot → Runtime Handoff Regression ===\n\n");
    test_enter_game_with_verified_profile_loads_dungeon();
    test_enter_game_loads_m564_object_names_from_graphics_dat();
    test_enter_game_preserves_imported_party_and_switches_leader();
    test_enter_game_rotate_party_aligns_champion_state();
    test_enter_game_with_missing_dungeon_path_keeps_runtime_safe();
    test_enter_game_runtime_handoff_is_idempotent();
    test_enter_game_rejects_partial_or_misrouted_profiles();
    test_enter_game_v2_profile_labels_do_not_change_v1_handoff();
    test_utility_flow_new_game_handoff_preserves_leader_index();
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
