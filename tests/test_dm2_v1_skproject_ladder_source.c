#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_skproject_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_AssetLoader assets;
    uint8_t map[27];
    uint8_t roots[8];
    uint8_t db2[8];
    uint8_t db3[16];
    uint8_t db4[8];
    uint16_t columns[9];
    uint32_t gdat_offsets[1];
    uint32_t gdat_sizes[1];
    DM2_V1_GdatEntry entries[1];
} LadderFixture;

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("PASS: %s\n", message); } \
    else { ++failed; printf("FAIL: %s\n", message); } \
} while (0)

static void put_word(uint8_t *bytes, uint16_t value, int big_endian)
{
    if (big_endian) {
        bytes[0] = (uint8_t)(value >> 8);
        bytes[1] = (uint8_t)value;
    } else {
        bytes[0] = (uint8_t)value;
        bytes[1] = (uint8_t)(value >> 8);
    }
}

static void init_fixture(LadderFixture *fixture, int big_endian)
{
    memset(fixture, 0, sizeof(*fixture));
    fixture->dungeon.raw_data = fixture->map;
    fixture->dungeon.raw_size = (int)sizeof(fixture->map);
    fixture->dungeon.level_count = 3;
    fixture->dungeon.square_bytes = 1;
    fixture->dungeon.raw_map_data_base = 0;
    fixture->dungeon.column_index_base = 32;
    fixture->dungeon.square_first_thing_base = 64;
    fixture->dungeon.square_first_thing_count = 8;
    fixture->dungeon.words_big_endian = big_endian;
    for (int level = 0; level < 3; ++level) {
        fixture->dungeon.level_widths[level] = 3;
        fixture->dungeon.level_heights[level] = 3;
        fixture->dungeon.level_offsets[level] = level * 9;
    }
    fixture->dungeon.map_level_number[0] = 0;
    fixture->dungeon.map_level_number[1] = 1;
    fixture->dungeon.map_level_number[2] = 2;
    fixture->pools.valid = 1;
    fixture->pools.words_big_endian = big_endian;
    fixture->pools.source_words_big_endian = big_endian;
    fixture->pools.pools[2].bytes = fixture->db2;
    fixture->pools.pools[2].record_count = 1;
    fixture->pools.pools[2].record_size = 8;
    fixture->pools.pools[3].bytes = fixture->db3;
    fixture->pools.pools[3].record_count = 2;
    fixture->pools.pools[3].record_size = 8;
    fixture->pools.pools[4].bytes = fixture->db4;
    fixture->pools.pools[4].record_count = 1;
    fixture->pools.pools[4].record_size = 8;
    for (int x = 0; x < 9; ++x)
        put_word((uint8_t *)fixture->columns + x * 2, 0u, big_endian);
    /* The real loader uses column-major byte tiles; only object-bearing
       squares consume an entry from this per-column root table. */
    fixture->dungeon.column_index_base = (int)((uint8_t *)fixture->columns -
                                                fixture->map);
    fixture->dungeon.square_first_thing_base = (int)((uint8_t *)fixture->roots -
                                                      fixture->map);
    /* Keep this isolated format view contiguous for the loader's bounds. */
    fixture->dungeon.raw_data = (uint8_t *)fixture;
    fixture->dungeon.raw_size = (int)sizeof(*fixture);
    fixture->dungeon.raw_map_data_base = (int)((uint8_t *)fixture->map -
                                                (uint8_t *)fixture);
    fixture->dungeon.column_index_base = (int)((uint8_t *)fixture->columns -
                                                (uint8_t *)fixture);
    fixture->dungeon.square_first_thing_base = (int)((uint8_t *)fixture->roots -
                                                      (uint8_t *)fixture);
}

static void set_square_root(LadderFixture *fixture, int level, int x, int y,
                            unsigned root_index, uint16_t link)
{
    int column = level * 3 + x;
    int square = fixture->dungeon.level_offsets[level] + x * 3 + y;
    fixture->map[square] = (uint8_t)(fixture->map[square] | 0x10u);
    put_word((uint8_t *)fixture->columns + (size_t)column * 2u,
             (uint16_t)root_index, fixture->dungeon.words_big_endian);
    put_word((uint8_t *)fixture->roots + (size_t)root_index * 2u,
             link, fixture->dungeon.words_big_endian);
}

static uint16_t make_actuator_word(unsigned target_map)
{
    return (uint16_t)(((target_map & 0x1ffu) << 7u) | 0x1cu);
}

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    if (out) *out = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out || !out_size || !(file = fopen(path, "rb")))
        return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0) {
        fclose(file);
        return 0;
    }
    rewind(file);
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)length;
    return 1;
}

static int candidate_path(char *out, size_t capacity, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");
    if (!out || capacity == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, capacity, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, capacity, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static void test_authentic_dungeon(void)
{
    uint8_t *bytes = NULL;
    uint8_t *graphics = NULL;
    size_t size = 0u;
    size_t graphics_size = 0u;
    char path[1024] = {0};
    char graphics_path[1024] = {0};
    DM2_V1_DungeonData dungeon;
    DM2_V1_AssetLoader assets;
    DM2_V1_SkprojectFindLadderSourceReceipt receipt;
    int searches = 0;
    int found = 0;
    int malformed = 0;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&assets, 0, sizeof(assets));
    if (!candidate_path(path, sizeof(path),
                        "dm2/mac-en/Dungeon Master II/DMFiles/Dungeon.dat") ||
        !read_file(path, &bytes, &size)) {
        printf("SKIP: authentic DM2 Mac DUNGEON.DAT is unavailable\n");
        return;
    }
    if (!candidate_path(graphics_path, sizeof(graphics_path),
                        "dm2/mac-en/Dungeon Master II/DMFiles/Graphics.dat") ||
        !read_file(graphics_path, &graphics, &graphics_size)) {
        printf("SKIP: authentic DM2 Mac Graphics.dat is unavailable\n");
        free(bytes);
        return;
    }
    CHECK(dm2_v1_asset_loader_init(&assets, graphics, graphics_size) == 0 &&
              assets.loaded,
          "authentic DM2 Mac Graphics.dat loads for GDAT ladder checks");
    if (!assets.loaded) {
        dm2_v1_asset_loader_free(&assets);
        free(graphics);
        free(bytes);
        return;
    }
    CHECK(dm2_v1_dungeon_load(&dungeon, bytes, (int)size) == 0,
          "authentic DM2 DUNGEON.DAT loads for ladder scan");
    if (dungeon.raw_data) {
        for (int level = 0; level < dungeon.level_count; ++level) {
            for (int x = 0; x < dungeon.level_widths[level]; ++x) {
                for (int y = 0; y < dungeon.level_heights[level]; ++y) {
                    for (int isupper = -1; isupper <= 1; isupper += 2) {
                        int result = dm2_v1_skproject_find_ladder_around_source(
                            &dungeon, NULL, assets.loaded ? &assets : NULL,
                            level, x, y, isupper,
                            DM2_V1_SKPROJECT_LADDER_SOURCE_SPX_V4, &receipt);
                        ++searches;
                        if (result >= 0 && receipt.valid && receipt.found) {
                            ++found;
                            if (receipt.direction != result ||
                                receipt.ladder_x < 0 || receipt.ladder_y < 0)
                                ++malformed;
                        } else if (!receipt.valid ||
                                   receipt.blocked_malformed_chain) {
                            ++malformed;
                        }
                    }
                }
            }
        }
        CHECK(searches > 0,
              "authentic dungeon exposes bounded ladder search origins");
        CHECK(found > 0,
              "authentic dungeon yields at least one source ladder match");
        CHECK(malformed == 0,
              "authentic ladder scan does not reject valid empty roots or chains");
    }
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&assets);
    free(graphics);
    free(bytes);
}

static void set_db3_actuator(LadderFixture *fixture, unsigned record,
                             unsigned target_map)
{
    uint8_t *bytes = fixture->db3 + record * 8u;
    put_word(bytes, DM2_THING_END_MARKER,
             fixture->pools.source_words_big_endian);
    put_word(bytes + 2u, make_actuator_word(target_map),
             fixture->pools.source_words_big_endian);
}

static void test_four_direction_order_and_altitude(void)
{
    LadderFixture fixture;
    DM2_V1_SkprojectFindLadderSourceReceipt receipt;
    int result;

    init_fixture(&fixture, 0);
    set_db3_actuator(&fixture, 0u, 1u);
    set_db3_actuator(&fixture, 1u, 1u);
    set_square_root(&fixture, 0, 1, 0, 0u, 0x0c00u); /* north */
    set_square_root(&fixture, 0, 2, 1, 1u, 0x0c01u); /* east */
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 0, 1, 1, 1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == 0 && receipt.valid && receipt.found &&
              receipt.direction == 0 && receipt.ladder_x == 1 &&
              receipt.ladder_y == 0,
          "source scan checks north before east and stops at first match");

    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 0, 1, 1, -1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == -1 && receipt.valid && !receipt.found,
          "actuator destination with the opposite level delta is rejected");
    set_square_root(&fixture, 2, 1, 0, 2u, 0x0c00u);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 2, 1, 1, -1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == 0 && receipt.valid && receipt.found,
          "actuator destination with the matching level delta is accepted");
}

static void test_null_roots_and_higher_db_stop(void)
{
    LadderFixture fixture;
    DM2_V1_SkprojectFindLadderSourceReceipt receipt;
    int result;

    init_fixture(&fixture, 0);
    set_square_root(&fixture, 0, 1, 0, 0u, DM2_THING_NULL_MARKER);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 0, 1, 1, 1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == -1 && receipt.valid && !receipt.found &&
              !receipt.blocked_malformed_chain,
          "authenticated null roots are empty chains, not malformed links");

    init_fixture(&fixture, 0);
    put_word(fixture.db4, 0x0c00u, 0);
    set_db3_actuator(&fixture, 0u, 1u);
    set_square_root(&fixture, 0, 1, 0, 0u, 0x1000u);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 0, 1, 1, 1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == -1 && receipt.valid && !receipt.found &&
              receipt.stopped_at_higher_db,
          "tile scan stops before records above DB3");
}

static void set_text_record(LadderFixture *fixture, unsigned usage,
                            unsigned ornament)
{
    uint16_t word = (uint16_t)(0x0002u | ((ornament & 0xffu) << 3u) |
                               ((usage & 0x1fu) << 11u));
    put_word(fixture->db2, DM2_THING_END_MARKER,
             fixture->pools.source_words_big_endian);
    put_word(fixture->db2 + 2u, word,
             fixture->pools.source_words_big_endian);
    fixture->entries[0].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    fixture->entries[0].cls2 = (uint8_t)ornament;
    fixture->entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    fixture->entries[0].cls4 = 0x11u;
    fixture->entries[0].data_index = 1u;
    fixture->assets.loaded = 1;
    fixture->assets.entries = fixture->entries;
    fixture->assets.entry_count = 1u;
    fixture->assets.raw_offsets = fixture->gdat_offsets;
    fixture->assets.raw_sizes = fixture->gdat_sizes;
}

static void test_dos_and_spx_text_usage(void)
{
    LadderFixture fixture;
    DM2_V1_SkprojectFindLadderSourceReceipt receipt;
    int result;

    init_fixture(&fixture, 0);
    set_text_record(&fixture, 6u, 5u);
    fixture.entries[0].data_index = 1u; /* GDAT says this ornament is up. */
    set_square_root(&fixture, 0, 1, 0, 0u, 0x0800u);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, &fixture.assets, 0, 1, 1, -1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == -1 && receipt.valid && !receipt.found,
          "DOS rejects usage-6 wall text as a ladder marker");

    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, &fixture.assets, 0, 1, 1, -1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_SPX_V4, &receipt);
    CHECK(result == 0 && receipt.valid && receipt.found &&
              receipt.matched_text_ornament,
          "SPX v4 accepts usage-6 text when GDAT marks an upward ladder");

    set_text_record(&fixture, 7u, 5u);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, &fixture.assets, 0, 1, 1, -1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == 0 && receipt.valid && receipt.found &&
              receipt.matched_text_ornament,
          "DOS accepts usage-7 text when GDAT marks an upward ladder");

    fixture.dungeon.words_big_endian = 1;
    fixture.pools.words_big_endian = 1;
    fixture.pools.source_words_big_endian = 1;
    memset(fixture.db3, 0, sizeof(fixture.db3));
    set_db3_actuator(&fixture, 0u, 1u);
    set_square_root(&fixture, 0, 1, 0, 0u, 0x0c00u);
    result = dm2_v1_skproject_find_ladder_around_source(
        &fixture.dungeon, &fixture.pools, NULL, 0, 1, 1, 1,
        DM2_V1_SKPROJECT_LADDER_SOURCE_DOS, &receipt);
    CHECK(result == 0 && receipt.valid && receipt.found,
          "actuator lookup decodes source-format big-endian records");
}

int main(void)
{
    test_four_direction_order_and_altitude();
    test_null_roots_and_higher_db_stop();
    test_dos_and_spx_text_usage();
    test_authentic_dungeon();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
