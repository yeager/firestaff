/*
 * test_dm2_v1_find_ladder_around.c
 *
 * Focused coverage for skproject FIND_LADDER_AROUND over DM2 dungeon data.
 */

#include "dm2_v1_find_ladder_around.h"
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

static size_t build_word_square_fixture(uint8_t *buf, size_t cap,
                                        uint16_t center,
                                        uint16_t north,
                                        uint16_t east)
{
    const size_t header_size = 44u;
    const size_t desc_count = 28u;
    const size_t desc_size = 16u;
    const size_t tile_base = header_size + desc_count * desc_size;
    uint8_t *desc;

    if (cap < tile_base + 18u) return 0u;
    memset(buf, 0, cap);
    put16le(buf + 2, 0x4731u);
    put16le(buf + 4, (uint16_t)header_size);
    buf[6] = 1u;
    desc = buf + header_size;
    put16le(desc + 0, 0u);
    put16le(desc + 4, (uint16_t)(((3u - 1u) << 5) | (3u - 1u)));
    put16le(desc + 12, 3u);
    put16le(desc + 14, 3u);

    for (int i = 0; i < 9; ++i) {
        put16le(buf + tile_base + (size_t)i * 2u, DM2_SQUARE_FLOOR);
    }
    put16le(buf + tile_base + ((size_t)(1 * 3 + 1) * 2u), center);
    put16le(buf + tile_base + ((size_t)(1 * 3 + 0) * 2u), north);
    put16le(buf + tile_base + ((size_t)(2 * 3 + 1) * 2u), east);
    return tile_base + 18u;
}

/* Unit-only square view for the ladder walker. It bypasses the retired
 * word-square loader rather than compiling that loader into this target. */
static int init_word_square_ladder_view(DM2_V1_DungeonData *out,
                                        const uint8_t *buf, size_t size)
{
    const int tile_base = 44 + 28 * 16;

    if (!out || !buf || size < (size_t)tile_base + 18u) return 0;
    memset(out, 0, sizeof(*out));
    out->raw_data = (uint8_t *)malloc(size);
    if (!out->raw_data) return 0;
    memcpy(out->raw_data, buf, size);
    out->raw_size = (int)size;
    out->level_count = 1;
    out->square_bytes = 2;
    out->raw_map_data_base = tile_base;
    out->level_widths[0] = 3;
    out->level_heights[0] = 3;
    out->level_offsets[0] = 0;
    return 1;
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

static void test_synthetic_ladder_scan(void)
{
    uint8_t dat[44u + 28u * 16u + 18u];
    DM2_V1_DungeonData dungeon;
    DM2_V1_FindLadderAroundReceipt receipt;
    size_t size = build_word_square_fixture(dat, sizeof(dat),
                                            DM2_SQUARE_FLOOR,
                                            DM2_SQUARE_STAIRS_UP,
                                            DM2_SQUARE_STAIRS_DOWN);

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(size == sizeof(dat), "unit ladder view is complete");
    CHECK(init_word_square_ladder_view(&dungeon, dat, size),
          "isolated ladder view is available without fixture loading");
    CHECK(dm2_v1_FIND_LADDER_AROUND(&dungeon, 0, 1, 1, &receipt) == 1 &&
              receipt.valid && receipt.found &&
              receipt.ladder_x == 1 && receipt.ladder_y == 0 &&
              receipt.search_slot == 1 &&
              receipt.kind == DM2_V1_LADDER_AROUND_UP &&
              receipt.vertical_delta == -1 &&
              receipt.search_hash != 0u,
          "FIND_LADDER_AROUND finds nearest source-ordered stair candidate");
    dm2_v1_dungeon_free(&dungeon);
}

static void test_synthetic_not_found_and_bounds(void)
{
    uint8_t dat[44u + 28u * 16u + 18u];
    DM2_V1_DungeonData dungeon;
    DM2_V1_FindLadderAroundReceipt receipt;
    size_t size = build_word_square_fixture(dat, sizeof(dat),
                                            DM2_SQUARE_FLOOR,
                                            DM2_SQUARE_FLOOR,
                                            DM2_SQUARE_FLOOR);

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(size == sizeof(dat), "unit no-ladder view is complete");
    CHECK(init_word_square_ladder_view(&dungeon, dat, size),
          "isolated no-ladder view is available without fixture loading");
    CHECK(dm2_v1_FIND_LADDER_AROUND(&dungeon, 0, 1, 1, &receipt) == 1 &&
              receipt.valid && !receipt.found &&
              receipt.search_slot == -1 && receipt.search_hash != 0u,
          "FIND_LADDER_AROUND returns explicit not-found receipt");
    CHECK(dm2_v1_FIND_LADDER_AROUND(&dungeon, 0, -1, 1, &receipt) == 0,
          "FIND_LADDER_AROUND rejects out-of-bounds origin");
    CHECK(dm2_v1_FIND_LADDER_AROUND(NULL, 0, 1, 1, &receipt) == 0,
          "FIND_LADDER_AROUND rejects missing dungeon data");
    dm2_v1_dungeon_free(&dungeon);
}

static void test_optional_real_dungeon_scan(void)
{
    uint8_t *data = NULL;
    size_t size = 0u;
    char path[1024];
    DM2_V1_DungeonData dungeon;
    DM2_V1_FindLadderAroundReceipt receipt;
    int evaluated = 0;
    int found = 0;

    memset(&dungeon, 0, sizeof(dungeon));
    if (!load_dungeon(&data, &size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 DUNGEON.DAT not present\n");
        return;
    }
    CHECK(dm2_v1_dungeon_load(&dungeon, data, (int)size) == 0,
          "real DM2 DUNGEON.DAT loads for ladder scan");
    if (dungeon.raw_data) {
        for (int level = 0; level < dungeon.level_count && !found; ++level) {
            for (int x = 0; x < dungeon.level_widths[level] && !found; ++x) {
                for (int y = 0; y < dungeon.level_heights[level]; ++y) {
                    if (dm2_v1_FIND_LADDER_AROUND(
                            &dungeon, level, x, y, &receipt) == 1) {
                        ++evaluated;
                        if (receipt.found) {
                            found = 1;
                            CHECK(receipt.valid && receipt.search_hash != 0u &&
                                      (receipt.square_type ==
                                           DM2_SQUARE_STAIRS_UP ||
                                       receipt.square_type ==
                                           DM2_SQUARE_STAIRS_DOWN),
                                  "real FIND_LADDER_AROUND receipt is bounded");
                            break;
                        }
                    }
                }
            }
        }
        CHECK(evaluated > 0,
              "real DM2 DUNGEON.DAT exposes evaluable ladder scan origins");
        CHECK(found > 0,
              "real DM2 DUNGEON.DAT exposes at least one ladder/stair candidate");
    }
    dm2_v1_dungeon_free(&dungeon);
    free(data);
}

int main(void)
{
    printf("=== DM2 V1 FIND_LADDER_AROUND Test ===\n");
    test_synthetic_ladder_scan();
    test_synthetic_not_found_and_bounds();
    test_optional_real_dungeon_scan();
    CHECK(dm2_v1_FIND_LADDER_AROUND_source_evidence()[0] != '\0',
          "FIND_LADDER_AROUND exposes skproject source evidence");
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
