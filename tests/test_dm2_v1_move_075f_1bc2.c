/*
 * test_dm2_v1_move_075f_1bc2.c
 *
 * Focused C11 coverage for skproject DM2_move_075f_1bc2 target receipts.
 */

#include "dm2_v1_move_075f_1bc2.h"
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
    put16le(buf + tile_base + (size_t)(1 * 4 + 2) * 2u, DM2_SQUARE_PIT);
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

static void test_synthetic_receipts(void)
{
    uint8_t fixture[640];
    size_t size = build_word_square_fixture(fixture, sizeof(fixture));
    DM2_V1_DungeonData dungeon;
    DM2_V1_Move075f1bc2Receipt receipt;

    CHECK(size > 0u, "synthetic move fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, fixture, (int)size) == 0,
          "synthetic move fixture loads through DM2 dungeon loader");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 1, 1, 0, 0, &receipt) == 1 &&
              receipt.valid && receipt.accepted && !receipt.blocked &&
              receipt.to_x == 1 && receipt.to_y == 0 &&
              receipt.target_square_type == DM2_SQUARE_FLOOR &&
              receipt.movement_hash != 0u,
          "DM2_move_075f_1bc2 accepts passable target floor");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 1, 1, 0, 2, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_MOVE_075F_1BC2_BLOCK_PIT,
          "DM2_move_075f_1bc2 blocks pit target");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 1, 1, 0, 1, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_MOVE_075F_1BC2_BLOCK_WALL,
          "DM2_move_075f_1bc2 blocks wall target");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 2, 1, 0, 1, &receipt) == 1 &&
              receipt.accepted &&
              receipt.vertical_kind == DM2_V1_MOVE_075F_1BC2_VERTICAL_DOWN,
          "DM2_move_075f_1bc2 records vertical stair target");

    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              &dungeon, 0, 0, 0, 0, 0, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason ==
                  DM2_V1_MOVE_075F_1BC2_BLOCK_NO_TARGET_TILE,
          "DM2_move_075f_1bc2 reports out-of-bounds target");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_real_dungeon_receipts(void)
{
    uint8_t *data = NULL;
    size_t size = 0u;
    char path[1024];
    DM2_V1_DungeonData dungeon;
    int evaluated = 0;
    int accepted = 0;
    int blocked = 0;
    int bounded = 1;

    if (!load_dungeon(&data, &size, path, sizeof(path))) {
        printf("  SKIP: real DM2 DUNGEON.DAT not found\n");
        return;
    }

    CHECK(dm2_v1_dungeon_load(&dungeon, data, (int)size) == 0,
          "real DM2 DUNGEON.DAT loads for move target scan");
    for (int level = 0; level < dungeon.level_count && evaluated < 128; ++level) {
        for (int y = 0; y < dungeon.level_heights[level] && evaluated < 128; ++y) {
            for (int x = 0; x < dungeon.level_widths[level] && evaluated < 128; ++x) {
                for (int dir = 0; dir < 4 && evaluated < 128; ++dir) {
                    DM2_V1_Move075f1bc2Receipt receipt;
                    if (dm2_v1_DM2_move_075f_1bc2_target_receipt(
                            &dungeon, level, x, y, dir, dir, &receipt) &&
                        receipt.source_raw_valid && receipt.target_raw_valid) {
                        ++evaluated;
                        if (receipt.accepted) ++accepted;
                        if (receipt.blocked) ++blocked;
                        if (receipt.target_square_type < 0 ||
                            receipt.target_square_type >= DM2_SQUARE_COUNT ||
                            receipt.movement_hash == 0u) {
                            bounded = 0;
                        }
                    }
                }
            }
        }
    }
    CHECK(evaluated > 0, "real DM2 DUNGEON.DAT exposes move target candidates");
    CHECK(bounded, "real move target receipts are bounded");
    CHECK(accepted > 0, "real DM2 DUNGEON.DAT exposes accepted move targets");
    CHECK(blocked > 0, "real DM2 DUNGEON.DAT exposes blocked move targets");
    dm2_v1_dungeon_free(&dungeon);
    free(data);
}

int main(void)
{
    DM2_V1_Move075f1bc2Receipt receipt;

    printf("=== DM2 V1 DM2_move_075f_1bc2 Test ===\n");
    test_synthetic_receipts();
    CHECK(dm2_v1_DM2_move_075f_1bc2_target_receipt(
              NULL, 0, 0, 0, 0, 0, &receipt) == 1 &&
              receipt.block_reason == DM2_V1_MOVE_075F_1BC2_BLOCK_NO_DUNGEON,
          "DM2_move_075f_1bc2 reports missing dungeon explicitly");
    test_real_dungeon_receipts();
    CHECK(dm2_v1_DM2_move_075f_1bc2_source_evidence()[0] != '\0',
          "DM2_move_075f_1bc2 exposes skproject source evidence");

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
