/*
 * test_dm2_v1_arrange_dungeon_receipt.c
 *
 * Real-data receipt coverage for skproject DM2_ARRANGE_DUNGEON.
 */

#include "dm2_v1_dungeon_loader.h"

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

int main(void)
{
    uint8_t *dungeon_data = NULL;
    uint8_t *corrupt_data = NULL;
    size_t dungeon_size = 0u;
    char path[1024];
    DM2_V1_ArrangeDungeonReceipt receipt;

    printf("=== DM2 V1 ARRANGE_DUNGEON Receipt Test ===\n");
    if (!load_dungeon(&dungeon_data, &dungeon_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 DUNGEON.DAT not present\n");
        return 0;
    }

    CHECK(dm2_v1_DM2_ARRANGE_DUNGEON_receipt(
              dungeon_data, (int)dungeon_size, &receipt) == 1 &&
              receipt.valid && receipt.committed,
          "real DUNGEON.DAT admits an arranged dungeon receipt");
    CHECK(receipt.map_count > 0 && receipt.map_count <= DM2_V1_MAX_LEVELS &&
              receipt.outdoor_map_count > 0 &&
              receipt.outdoor_map_count + receipt.indoor_map_count ==
                  receipt.map_count,
          "arranged dungeon receipt accounts for outdoor and indoor maps");
    CHECK((receipt.square_bytes == 1 || receipt.square_bytes == 2) &&
              receipt.raw_map_data_base > 0 &&
              receipt.map_dimension_hash != 0u &&
              receipt.map_graphics_style_hash != 0u &&
              receipt.arrangement_hash != 0u,
          "arranged dungeon receipt carries real layout/style hashes");
    CHECK(receipt.record_graph_complete == 1 &&
              receipt.incomplete == 0 &&
              receipt.g1_extension_size > 0,
          "real PC G1 record graph validates with extension records");
    corrupt_data = (uint8_t *)malloc(dungeon_size);
    CHECK(corrupt_data != NULL, "corrupt real-data copy allocated");
    if (corrupt_data) {
        memcpy(corrupt_data, dungeon_data, dungeon_size);
        corrupt_data[6] = 0u;
        CHECK(dm2_v1_DM2_ARRANGE_DUNGEON_receipt(
                  corrupt_data, (int)dungeon_size, &receipt) == 0,
              "corrupt real DUNGEON.DAT map count is rejected");
    }
    CHECK(dm2_v1_DM2_ARRANGE_DUNGEON_receipt(
              NULL, (int)dungeon_size, &receipt) == 0,
          "ARRANGE_DUNGEON rejects missing bytes");
    CHECK(dm2_v1_DM2_ARRANGE_DUNGEON_source_evidence()[0] != '\0',
          "ARRANGE_DUNGEON exposes skproject source evidence");

    free(corrupt_data);
    free(dungeon_data);
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
