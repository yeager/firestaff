/*
 * test_dm2_v1_move_2c1d_028c.c
 *
 * Focused C11 coverage for skproject DM2_move_2c1d_028c commit receipts.
 */

#include "dm2_v1_move_2c1d_028c.h"
#include "dm2_v1_world_model.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static size_t build_word_square_fixture(uint8_t *buf, size_t cap)
{
    const size_t header_size = 44u;
    const size_t desc_count = 28u;
    const size_t desc_size = 16u;
    const size_t tile_base = header_size + desc_count * desc_size;
    uint8_t *desc;

    if (cap < tile_base + 32u) return 0u;
    memset(buf, 0, cap);
    put16le(buf + 2, 0x4731u);
    put16le(buf + 4, (uint16_t)header_size);
    buf[6] = 1u;
    desc = buf + header_size;
    put16le(desc + 0, 0u);
    put16le(desc + 4, (uint16_t)(((4u - 1u) << 5) | (4u - 1u)));
    put16le(desc + 12, 4u);
    put16le(desc + 14, 4u);

    for (int i = 0; i < 16; ++i) {
        put16le(buf + tile_base + (size_t)i * 2u, DM2_SQUARE_FLOOR);
    }
    put16le(buf + tile_base + (size_t)(2 * 4 + 1) * 2u, DM2_SQUARE_WALL);
    put16le(buf + tile_base + (size_t)(3 * 4 + 1) * 2u,
            DM2_SQUARE_STAIRS_DOWN);
    return tile_base + 32u;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_dungeon(uint8_t **out_data, size_t *out_size,
                        char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/DUNGEON.DAT",
        "dm2/dungeon.dat",
        "dm2/data/DUNGEON.DAT",
        "dm2/data/dungeon.dat"
    };
    size_t i;

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_synthetic_commit_receipts(void)
{
    uint8_t fixture[640];
    size_t size = build_word_square_fixture(fixture, sizeof(fixture));
    DM2_V1_DungeonData dungeon;
    DM2_V1_Move075f1bc2Receipt target;
    DM2_V1_Move2c1d028cReceipt receipt;

    CHECK(size > 0u, "synthetic commit fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, fixture, (int)size) == 0,
          "synthetic commit fixture loads through DM2 dungeon loader");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 1, 1, 2, 0, &target) == 1 &&
              dm2_v1_DM2_move_2c1d_028c_commit_receipt(
                  &target, &receipt) == 1 &&
              receipt.valid && receipt.committed && !receipt.blocked &&
              receipt.outcome == DM2_V1_MOVE_2C1D_028C_OUTCOME_ADVANCE &&
              receipt.x_after == 1 && receipt.y_after == 0 &&
              receipt.dir_after == 2 && receipt.preserves_facing &&
              receipt.commit_hash != 0u,
          "DM2_move_2c1d_028c commits accepted floor movement");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 1, 1, 2, 1, &target) == 1 &&
              dm2_v1_DM2_move_2c1d_028c_commit_receipt(
                  &target, &receipt) == 1 &&
              receipt.blocked && !receipt.committed &&
              receipt.outcome == DM2_V1_MOVE_2C1D_028C_OUTCOME_BLOCKED &&
              receipt.x_after == 1 && receipt.y_after == 1 &&
              receipt.block_reason == DM2_V1_MOVE_075F_1BC2_BLOCK_WALL,
          "DM2_move_2c1d_028c keeps pose when target is blocked");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 2, 1, 3, 1, &target) == 1 &&
              dm2_v1_DM2_move_2c1d_028c_commit_receipt(
                  &target, &receipt) == 1 &&
              receipt.committed &&
              receipt.outcome == DM2_V1_MOVE_2C1D_028C_OUTCOME_STAIR_DOWN &&
              receipt.vertical_delta == 1 &&
              receipt.requires_post_step_chain,
          "DM2_move_2c1d_028c records stair post-step chain requirement");

    CHECK(dm2_v1_DM2_move_2c1d_028c_commit_receipt(NULL, &receipt) == 0,
          "DM2_move_2c1d_028c rejects missing target receipt");
    memset(&target, 0, sizeof(target));
    CHECK(dm2_v1_DM2_move_2c1d_028c_commit_receipt(
              &target, &receipt) == 0,
          "DM2_move_2c1d_028c rejects invalid target receipt");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_real_dungeon_commit_receipts(void)
{
    uint8_t *data = NULL;
    size_t size = 0u;
    char path[1024];
    DM2_V1_DungeonData dungeon;
    int evaluated = 0;
    int committed = 0;
    int blocked = 0;
    int bounded = 1;

    if (!load_dungeon(&data, &size, path, sizeof(path))) {
        printf("  SKIP: real DM2 DUNGEON.DAT not found\n");
        return;
    }

    CHECK(dm2_v1_dungeon_load(&dungeon, data, (int)size) == 0,
          "real DM2 DUNGEON.DAT loads for move commit scan");
    for (int level = 0; level < dungeon.level_count && evaluated < 128; ++level) {
        for (int y = 0; y < dungeon.level_heights[level] && evaluated < 128; ++y) {
            for (int x = 0; x < dungeon.level_widths[level] && evaluated < 128; ++x) {
                for (int dir = 0; dir < 4 && evaluated < 128; ++dir) {
                    DM2_V1_Move075f1bc2Receipt target;
                    DM2_V1_Move2c1d028cReceipt receipt;
                    if (dm2_v1_DM2_move_075f_1bc2_target_receipt(
                            &dungeon, level, x, y, dir, dir, &target) &&
                        target.source_raw_valid && target.target_raw_valid &&
                        dm2_v1_DM2_move_2c1d_028c_commit_receipt(
                            &target, &receipt)) {
                        ++evaluated;
                        if (receipt.committed) ++committed;
                        if (receipt.blocked) ++blocked;
                        if (receipt.commit_hash == 0u ||
                            receipt.level_before != receipt.level_after ||
                            receipt.dir_before != receipt.dir_after) {
                            bounded = 0;
                        }
                    }
                }
            }
        }
    }
    CHECK(evaluated > 0, "real DM2 DUNGEON.DAT exposes commit candidates");
    CHECK(bounded, "real commit receipts preserve bounded pose fields");
    CHECK(committed > 0, "real DM2 DUNGEON.DAT exposes committed moves");
    CHECK(blocked > 0, "real DM2 DUNGEON.DAT exposes blocked commits");
    dm2_v1_dungeon_free(&dungeon);
    free(data);
}

int main(void)
{
    printf("=== DM2 V1 DM2_move_2c1d_028c Test ===\n");
    test_synthetic_commit_receipts();
    test_real_dungeon_commit_receipts();
    CHECK(dm2_v1_DM2_move_2c1d_028c_source_evidence()[0] != '\0',
          "DM2_move_2c1d_028c exposes skproject source evidence");

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
