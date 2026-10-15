/* Read-only edition census for SKProject DM2_ARRANGE_DUNGEON RNG draws. */

#include "dm2_v1_caii_source_owner.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_graphics_data_open.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_u16(const DM2_V1_DungeonData *dungeon, uint8_t *bytes,
                      uint16_t value)
{
    if (dungeon->words_big_endian) {
        bytes[0] = (uint8_t)(value >> 8);
        bytes[1] = (uint8_t)value;
    } else {
        bytes[0] = (uint8_t)value;
        bytes[1] = (uint8_t)(value >> 8);
    }
}

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;
    if (!path || !out || !out_size ||
        !(file = fopen(path, "rb"))) return 0;
    *out = NULL;
    *out_size = 0u;
    if (fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int check(int condition, const char *label)
{
    printf("%s: %s\n", condition ? "PASS" : "FAIL", label);
    return condition ? 0 : 1;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char graphics_path[1100];
    char dungeon_path[1100];
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_DungeonData mutated;
    DM2_V1_CaiiSourceOwner ai;
    DM2_V1_ArrangeDungeonRngReceipt receipt;
    uint8_t *original_bytes = NULL;
    int failures = 0;

    if (!root || !root[0]) {
        fputs("SKIP: FIRESTAFF_DM2_DATA_DIR must name canonical test data\n",
              stderr);
        return 77;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&mutated, 0, sizeof(mutated));
    memset(&ai, 0, sizeof(ai));
    memset(&receipt, 0, sizeof(receipt));
    if (!read_file(graphics_path, &graphics, &graphics_size) ||
        !read_file(dungeon_path, &dungeon_bytes, &dungeon_size) ||
        dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes,
                            (int)dungeon_size) != 0) {
        fputs("FAIL: canonical DM2 data could not be admitted\n", stderr);
        failures = 1;
        goto done;
    }
    if (dm2_v1_dungeon_load(&mutated, dungeon_bytes,
                            (int)dungeon_size) != 0) {
        fputs("FAIL: could not clone authenticated dungeon bytes\n", stderr);
        failures = 1;
        goto done;
    }
    failures += check(dungeon.record_graph_complete &&
                          !dungeon.g1_w0_chains_disabled,
                      "edition exposes complete source-linked record graph");
    if (dungeon.record_graph_complete &&
        !dungeon.g1_w0_chains_disabled &&
        dm2_v1_caii_source_owner_init(&ai, &loader, &dungeon)) {
        int complete = dm2_v1_DM2_ARRANGE_DUNGEON_rng_receipt(
            &dungeon, &ai, &receipt);
        DM2_V1_ArrangeDungeonRngReceipt baseline = receipt;
        failures += check(complete && receipt.valid && !receipt.incomplete,
                          "source-ordered arrangement RNG census is complete");
        if (complete) {
            int map;
            int found_record = 0;
            original_bytes = (uint8_t *)malloc((size_t)dungeon.raw_size);
            if (!original_bytes) {
                failures++;
                goto done;
            }
            memcpy(original_bytes, dungeon.raw_data,
                   (size_t)dungeon.raw_size);
            failures += check(
                memcmp(original_bytes, dungeon.raw_data,
                       (size_t)dungeon.raw_size) == 0,
                "read-only RNG census leaves authentic dungeon bytes untouched");
            for (map = 0; map < dungeon.level_count && !found_record; ++map) {
                int x;
                for (x = 0; x < dungeon.level_widths[map] && !found_record;
                     ++x) {
                    int y;
                    for (y = 0; y < dungeon.level_heights[map]; ++y) {
                        int raw = dm2_v1_dungeon_get_tile_raw(
                            &mutated, map, x, y);
                        int link;
                        if (raw < 0 || ((unsigned)raw & 0x10u) == 0u) continue;
                        link = dm2_v1_dungeon_get_first_thing(
                            &mutated, map, x, y);
                        if (link == (int)DM2_THING_END_MARKER ||
                            link == (int)DM2_THING_NULL_MARKER) continue;
                        {
                            uint8_t *record = (uint8_t *)
                                dm2_v1_dungeon_get_thing_record(
                                    &mutated, (uint16_t)link, NULL, NULL, NULL);
                            if (!record) continue;
                            write_u16(&mutated, record, (uint16_t)link);
                            found_record = 1;
                            break;
                        }
                    }
                }
            }
            if (found_record) {
                failures += check(
                    !dm2_v1_DM2_ARRANGE_DUNGEON_rng_receipt(
                        &mutated, &ai, &receipt) &&
                        !receipt.valid && receipt.incomplete,
                    "cyclic source record chain fails closed");
                receipt = baseline;
            } else {
                failures += check(0, "test fixture contains a source record");
            }
            failures += check(
                memcmp(original_bytes, dungeon.raw_data,
                       (size_t)dungeon.raw_size) == 0,
                "authentic dungeon bytes remain unchanged after census and mutation test");
            printf("edition maps=%d creature-records=%u eligible-static=%u "
                   "possessions=%u RANDDIR-draws=%u order-hash=%08x\n",
                   dungeon.level_count,
                   (unsigned)baseline.creature_records_seen,
                   (unsigned)baseline.eligible_static_creatures,
                   (unsigned)baseline.possession_records,
                   (unsigned)baseline.rng_draws,
                   (unsigned)baseline.source_order_hash);
        } else {
            printf("arrangement RNG census incomplete: creatures=%u draws=%u\n",
                   (unsigned)receipt.creature_records_seen,
                   (unsigned)receipt.rng_draws);
        }
    } else {
        puts("BLOCKED: edition lacks complete raw-map record and AI binding");
        if (!dungeon.g1_w0_chains_disabled) ++failures;
    }
done:
    free(original_bytes);
    dm2_v1_dungeon_free(&mutated);
    dm2_v1_caii_source_owner_free(&ai);
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    return failures ? 1 : 0;
}
