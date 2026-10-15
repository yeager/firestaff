/* Authentic-data census and transactional RANDDIR replay for DM2 arrangement. */

#include "dm2_v1_caii_source_owner.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_graphics_data_open.h"
#include "dm2_v1_update_weather_pc34_compat.h"

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

static int check_rejected_append_preserves_original(DM2_V1_DungeonData *dungeon)
{
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int x = 0; x < dungeon->level_widths[map]; ++x) {
            for (int y = 0; y < dungeon->level_heights[map]; ++y) {
                int link = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                int next;
                DM2_V1_SkprojectAppendRecordReceipt receipt;
                if (link < 0 || link == DM2_THING_END_MARKER ||
                    link == DM2_THING_NULL_MARKER) continue;
                next = dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)link);
                if (next < 0 || next == DM2_THING_END_MARKER ||
                    next == DM2_THING_NULL_MARKER) continue;
                {
                    uint8_t *snapshot = malloc((size_t)dungeon->raw_size);
                    uint16_t invalid_parent = DM2_THING_NULL_MARKER;
                    int failures = 0;
                    if (!snapshot) return 1;
                    memcpy(snapshot, dungeon->raw_data, (size_t)dungeon->raw_size);
                    failures += check(
                        !dm2_v1_skproject_append_record_to(
                            dungeon, (uint16_t)link, NULL, map, -1, -1, &receipt) &&
                        receipt.blocked_invalid_parent &&
                        memcmp(snapshot, dungeon->raw_data,
                               (size_t)dungeon->raw_size) == 0,
                        "missing append parent preserves original dungeon bytes");
                    memcpy(dungeon->raw_data, snapshot, (size_t)dungeon->raw_size);
                    failures += check(
                        !dm2_v1_skproject_append_record_to(
                            dungeon, (uint16_t)link, &invalid_parent, map,
                            -1, -1, &receipt) && receipt.blocked_unbounded_graph &&
                        memcmp(snapshot, dungeon->raw_data,
                               (size_t)dungeon->raw_size) == 0 &&
                        invalid_parent == DM2_THING_NULL_MARKER,
                        "invalid append chain preserves original dungeon bytes");
                    memcpy(dungeon->raw_data, snapshot, (size_t)dungeon->raw_size);
                    failures += check(
                        !dm2_v1_skproject_append_record_to(
                            dungeon, (uint16_t)link, NULL, dungeon->level_count,
                            0, 0, &receipt) && receipt.blocked_invalid_tile &&
                        memcmp(snapshot, dungeon->raw_data,
                               (size_t)dungeon->raw_size) == 0,
                        "invalid append tile preserves original dungeon bytes");
                    memcpy(dungeon->raw_data, snapshot, (size_t)dungeon->raw_size);
                    free(snapshot);
                    return failures;
                }
            }
        }
    }
    return check(0, "original dungeon supplies a nonterminal record link");
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
    DM2_V1_ArrangeDungeonRngApplyReceipt apply_receipt;
    DM2_V1_UpdateWeatherState cold_weather;
    DM2_V1_WeatherTransitionReceipt weather_receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_RecordPoolSet pool_snapshot;
    DM2_V1_RecordPoolSet bad_pools;
    DM2_V1_DropRng session_rng;
    DM2_V1_DropRng expected_rng;
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
    memset(&apply_receipt, 0, sizeof(apply_receipt));
    memset(&cold_weather, 0, sizeof(cold_weather));
    memset(&weather_receipt, 0, sizeof(weather_receipt));
    memset(&pools, 0, sizeof(pools));
    memset(&pool_snapshot, 0, sizeof(pool_snapshot));
    memset(&bad_pools, 0, sizeof(bad_pools));
    memset(&session_rng, 0, sizeof(session_rng));
    memset(&expected_rng, 0, sizeof(expected_rng));
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
    failures += check_rejected_append_preserves_original(&dungeon);
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
            int pool_bytes_changed = 0;
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
            if (!dm2_v1_record_pool_set_init_from_dungeon(&pools, &dungeon)) {
                failures += check(0, "authenticated mutable record pools initialized");
                goto done;
            }
            failures += check(pools.valid && pools.record_graph_complete,
                              "authenticated mutable record pools initialized");
            /* SKProject main.cpp initializes randdat to zero. The first
             * cold GAME_LOAD takes c_weather.cpp's storm-request path before
             * arrangement, consuming RAND16(500), RANDDIR, RAND16(4). */
            dm2_v1_drops_rng_init(&session_rng);
            failures += check(
                !dm2_v1_DM2_ARRANGE_DUNGEON_apply_rng_directions(
                    &dungeon, &ai, &pools, &session_rng, NULL) &&
                session_rng.random == 0u,
                "missing arrangement output rejects without advancing RNG");
            cold_weather.storm_request = 1;
            expected_rng = session_rng;
            (void)dm2_v1_drops_rand16(&expected_rng, 500u);
            (void)dm2_v1_drops_randdir(&expected_rng);
            (void)dm2_v1_drops_rand16(&expected_rng, 4u);
            failures += check(
                dm2_v1_weather_transition(&cold_weather, 0, 0,
                                          &session_rng, &weather_receipt) == 0 &&
                    weather_receipt.valid && weather_receipt.storm_path &&
                    weather_receipt.draws == 3 &&
                    cold_weather.pattern_row == 3 && cold_weather.step == 1 &&
                    session_rng.random == expected_rng.random,
                "authentic cold-start weather advances the zero-seeded shared RNG by three draws");
            expected_rng = session_rng;
            for (uint32_t i = 0; i < baseline.rng_draws; ++i)
                (void)dm2_v1_drops_randdir(&expected_rng);
            if (dm2_v1_record_pool_set_clone(&pool_snapshot, &pools)) {
                failures += check(
                    dm2_v1_DM2_ARRANGE_DUNGEON_apply_rng_directions(
                        &dungeon, &ai, &pools, &session_rng, &apply_receipt) &&
                        apply_receipt.valid && apply_receipt.committed &&
                        !apply_receipt.incomplete,
                    "source-ordered direction replay commits on private pools");
                failures += check(
                    apply_receipt.rng_draws == baseline.rng_draws &&
                        apply_receipt.creature_direction_writes ==
                            baseline.eligible_static_creatures &&
                        apply_receipt.possession_direction_writes ==
                            baseline.possession_records &&
                        apply_receipt.source_order_hash ==
                            baseline.source_order_hash &&
                        session_rng.random == expected_rng.random &&
                        apply_receipt.rng_after == expected_rng.random,
                    "direction replay preserves census order and shared RNG advance");
                for (int type = 0; type < DM2_V1_RECORD_POOL_COUNT; ++type) {
                    const DM2_V1_RecordPool *a = &pools.pools[type];
                    const DM2_V1_RecordPool *b = &pool_snapshot.pools[type];
                    if ((a->bytes && b->bytes && a->record_count > 0 &&
                         memcmp(a->bytes, b->bytes,
                                (size_t)a->record_count *
                                    (size_t)a->record_size) != 0) ||
                        (a->extension_bytes && b->extension_bytes &&
                         a->extension_count > 0 &&
                         memcmp(a->extension_bytes, b->extension_bytes,
                                (size_t)a->extension_count *
                                    (size_t)a->record_size) != 0)) {
                        pool_bytes_changed = 1;
                    }
                }
                failures += check(pool_bytes_changed,
                                  "direction replay mutates only its private record-pool copy");
                {
                    int source_db4_handle = -1;
                    uint8_t *bad_record = NULL;
                    uint32_t rng_before_failure = session_rng.random;
                    int changed_after_failure = 0;
                    for (int map = 0; map < dungeon.level_count &&
                                      source_db4_handle < 0; ++map) {
                        for (int x = 0; x < dungeon.level_widths[map] &&
                                        source_db4_handle < 0; ++x) {
                            for (int y = 0; y < dungeon.level_heights[map]; ++y) {
                                int raw = dm2_v1_dungeon_get_tile_raw(
                                    &dungeon, map, x, y);
                                int thing;
                                uint32_t guard = 0u;
                                if (raw < 0 || ((unsigned)raw & 0x10u) == 0u)
                                    continue;
                                thing = dm2_v1_dungeon_get_first_thing(
                                    &dungeon, map, x, y);
                                while (thing != (int)DM2_THING_END_MARKER &&
                                       thing != (int)DM2_THING_NULL_MARKER &&
                                       thing >= 0 && ++guard <= 4096u) {
                                    if (((thing >> 10) & 0x0f) == 4) {
                                        source_db4_handle = thing;
                                        break;
                                    }
                                    thing = dm2_v1_dungeon_get_next_thing(
                                        &dungeon, (uint16_t)thing);
                                }
                                if (source_db4_handle >= 0) break;
                            }
                        }
                    }
                    if (source_db4_handle >= 0 &&
                        dm2_v1_record_pool_set_clone(&bad_pools, &pools) &&
                        (bad_record = dm2_v1_record_pool_address_mut(
                             &bad_pools, (int16_t)source_db4_handle)) != NULL) {
                        bad_record[4] ^= 0x01u;
                        dm2_v1_record_pool_set_free(&pool_snapshot);
                        if (!dm2_v1_record_pool_set_clone(&pool_snapshot,
                                                         &bad_pools)) {
                            failures += check(0,
                                "corrupted pool can be snapshotted before rejection");
                            goto done;
                        }
                        failures += check(
                            !dm2_v1_DM2_ARRANGE_DUNGEON_apply_rng_directions(
                                &dungeon, &ai, &bad_pools, &session_rng,
                                &apply_receipt) &&
                                apply_receipt.incomplete &&
                                !apply_receipt.committed &&
                                session_rng.random == rng_before_failure,
                            "source identity mismatch rejects the mutation transaction");
                        for (int type = 0; type < DM2_V1_RECORD_POOL_COUNT; ++type) {
                            const DM2_V1_RecordPool *a = &bad_pools.pools[type];
                            const DM2_V1_RecordPool *b = &pool_snapshot.pools[type];
                            if ((a->bytes && b->bytes && a->record_count > 0 &&
                                 memcmp(a->bytes, b->bytes,
                                        (size_t)a->record_count *
                                            (size_t)a->record_size) != 0) ||
                                (a->extension_bytes && b->extension_bytes &&
                                 a->extension_count > 0 &&
                                 memcmp(a->extension_bytes, b->extension_bytes,
                                        (size_t)a->extension_count *
                                            (size_t)a->record_size) != 0))
                                changed_after_failure = 1;
                        }
                        failures += check(!changed_after_failure,
                            "failed mutation preserves caller-owned pool bytes");
                        dm2_v1_record_pool_set_free(&bad_pools);
                        dm2_v1_record_pool_set_free(&pool_snapshot);
                    } else {
                        failures += check(0,
                            "authentic map exposes a DB4 handle for rollback coverage");
                    }
                }
            } else {
                failures += check(0, "mutable record pools can be snapshotted");
            }
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
    dm2_v1_record_pool_set_free(&bad_pools);
    dm2_v1_record_pool_set_free(&pool_snapshot);
    dm2_v1_record_pool_set_free(&pools);
    dm2_v1_dungeon_free(&mutated);
    dm2_v1_caii_source_owner_free(&ai);
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    return failures ? 1 : 0;
}
