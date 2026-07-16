/*
 * test_dm2_v1_move_2fcf_0434.c
 *
 * Focused C11 coverage for skproject DM2_move_2fcf_0434 teleporter gate.
 */

#include "dm2_v1_move_2fcf_0434.h"

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

static void test_synthetic_teleporter_gate(void)
{
    DM2_V1_Move2fcf0434Receipt receipt;

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)((5u << 5) | 0x08u), 5,
              1, 3, 3, 1, 7, 1, 0x02, 0, 2, 2,
              &receipt) == 1 &&
              receipt.valid && receipt.admitted && !receipt.blocked &&
              receipt.source_square_type == 5 &&
              receipt.source_tile_enabled &&
              receipt.party_scope_allowed &&
              receipt.destination_bounded &&
              receipt.transition_hash != 0u,
          "DM2_move_2fcf_0434 admits complete enabled party teleporter");

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)((5u << 5) | 0x08u), 5,
              1, 3, 3, 2, 7, 1, 0x02, 0, 2, 2,
              &receipt) == 1 &&
              receipt.block_reason ==
                  DM2_V1_MOVE_2FCF_0434_BLOCK_NOT_DB1_TELEPORTER,
          "DM2_move_2fcf_0434 rejects non-DB1 records before transition");

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)((5u << 5) | 0x08u), 5,
              1, 3, 3, 1, 7, 0, 0x02, 0, 2, 2,
              &receipt) == 1 &&
              receipt.block_reason ==
                  DM2_V1_MOVE_2FCF_0434_BLOCK_INCOMPLETE_RECORD_GRAPH,
          "DM2_move_2fcf_0434 blocks incomplete record graphs");

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)((5u << 5) | 0x08u), 5,
              1, 3, 3, 1, 7, 1, 0x00, 0, 2, 2,
              &receipt) == 1 &&
              receipt.block_reason ==
                  DM2_V1_MOVE_2FCF_0434_BLOCK_PARTY_SCOPE,
          "DM2_move_2fcf_0434 blocks teleporter scope without party bit");

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)((5u << 5) | 0x08u), 5,
              1, 3, 3, 1, 7, 1, 0x02, 0xff, 2, 2,
              &receipt) == 1 &&
              receipt.block_reason ==
                  DM2_V1_MOVE_2FCF_0434_BLOCK_DESTINATION_BOUNDS,
          "DM2_move_2fcf_0434 blocks unbounded destination map");

    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
              0, 1, 1, 1, (int)(5u << 5), 5,
              1, 3, 3, 1, 7, 1, 0x02, 0, 2, 2,
              &receipt) == 1 &&
              receipt.block_reason ==
                  DM2_V1_MOVE_2FCF_0434_BLOCK_SOURCE_TILE_NOT_ENABLED_TELEPORTER,
          "DM2_move_2fcf_0434 requires source enable bit");
}

static void test_real_dungeon_scan(void)
{
    uint8_t *data = NULL;
    size_t size = 0u;
    char path[1024];
    DM2_V1_DungeonData dungeon;
    int evaluated = 0;
    int enabled = 0;
    int blocked_as_incomplete = 0;

    if (!load_dungeon(&data, &size, path, sizeof(path))) {
        printf("  SKIP: real DM2 DUNGEON.DAT not found\n");
        return;
    }
    CHECK(dm2_v1_dungeon_load(&dungeon, data, (int)size) == 0,
          "real DM2 DUNGEON.DAT loads for 2fcf scan");
    for (int level = 0; level < dungeon.level_count; ++level) {
        for (int y = 0; y < dungeon.level_heights[level]; ++y) {
            for (int x = 0; x < dungeon.level_widths[level]; ++x) {
                int type = dm2_v1_dungeon_get_square_type(&dungeon, level, x, y);
                int raw = dm2_v1_dungeon_get_tile_raw(&dungeon, level, x, y);
                if (type == 5 && raw >= 0 && (raw & 0x08) != 0) {
                    DM2_V1_Move2fcf0434Receipt receipt;
                    ++enabled;
                    if (dm2_v1_DM2_move_2fcf_0434_teleporter_gate(
                            &dungeon, level, x, y, 1, enabled, 0, 0x02,
                            level, x, y, &receipt)) {
                        ++evaluated;
                        if (receipt.block_reason ==
                            DM2_V1_MOVE_2FCF_0434_BLOCK_INCOMPLETE_RECORD_GRAPH) {
                            ++blocked_as_incomplete;
                        }
                    }
                }
            }
        }
    }
    CHECK(evaluated == enabled, "real enabled teleporter candidates are bounded");
    CHECK(blocked_as_incomplete == enabled,
          "real 2fcf scan blocks candidates without complete graph");
    dm2_v1_dungeon_free(&dungeon);
    free(data);
}

int main(void)
{
    DM2_V1_Move2fcf0434Receipt receipt;

    printf("=== DM2 V1 DM2_move_2fcf_0434 Test ===\n");
    test_synthetic_teleporter_gate();
    CHECK(dm2_v1_DM2_move_2fcf_0434_teleporter_gate(
              NULL, 0, 0, 0, 1, 0, 1, 0x02, 0, 0, 0,
              &receipt) == 1 &&
              receipt.block_reason == DM2_V1_MOVE_2FCF_0434_BLOCK_NO_DUNGEON,
          "DM2_move_2fcf_0434 reports missing dungeon explicitly");
    test_real_dungeon_scan();
    CHECK(dm2_v1_DM2_move_2fcf_0434_source_evidence()[0] != '\0',
          "DM2_move_2fcf_0434 exposes skproject source evidence");

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
