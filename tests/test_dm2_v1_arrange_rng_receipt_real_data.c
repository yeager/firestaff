/* Authentic-data census and transactional RANDDIR replay for DM2 arrangement. */

#include "dm2_v1_caii_source_owner.h"
#include "dm2_v1_arrange_dungeon_owner.h"
#include "dm2_v1_boot.h"
#include "asset_find_by_hash.h"
#include "dm2_v1_skproject_core.h"
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

static int read_original_bytes(const uint8_t *memory, size_t size,
    const char *path, uint8_t **out, size_t *out_size)
{
    *out = NULL; *out_size = 0;
    if (!memory || !size) return asset_read_path_alloc(path,out,out_size);
    *out = malloc(size);
    if (!*out) return 0;
    memcpy(*out,memory,size);
    *out_size = size;
    return 1;
}

static int check(int condition, const char *label)
{
    printf("%s: %s\n", condition ? "PASS" : "FAIL", label);
    return condition ? 0 : 1;
}

typedef struct {
    const DM2_V1_DungeonData *dungeon;
    const DM2_V1_AssetLoader *loader;
    int pit_queries;
    int reject_pit;
} OriginalArrangeQueries;

static int original_pit_gdat(void *context, int map, uint16_t *value)
{
    OriginalArrangeQueries *q = context;
    int style = dm2_v1_dungeon_get_map_graphics_style(q->dungeon, map);
    ++q->pit_queries;
    if (q->reject_pit) return 0;
    if (style < 0 || !q->loader->loaded || !value) return 0;
    /* QUERY_GDAT_ENTRY_DATA_INDEX returns zero for absent optional entries. */
    *value = 0;
    (void)dm2_v1_asset_load_word_value(q->loader, 8, style, 0x6a, value);
    return 1;
}

static int original_locate_below(void *context, int map, int *x, int *y,
                                  int *result_map)
{
    OriginalArrangeQueries *q = context;
    const DM2_V1_DungeonData *d = q->dungeon;
    DM2_V1_SkprojectMapDescriptor maps[DM2_V1_MAX_LEVELS];
    uint8_t candidates[DM2_V1_MAX_LEVELS];
    uint16_t count = 0;
    int16_t tx = (int16_t)*x, ty = (int16_t)*y;
    DM2_V1_SkprojectLocateOtherLevelReceipt receipt;
    if (map < 0 || map >= d->level_count) return 0;
    if (d->map_level_number[map] + 1 >= 63) {
        *result_map = -1;
        return 1;
    }
    memset(maps, 0, sizeof(maps));
    /* c_savegame.cpp:579-611 builds each level's scan list in map order. */
    for (int m = 0; m < d->level_count; ++m) {
        int lx = *x + d->map_offset_x[map] - d->map_offset_x[m];
        int ly = *y + d->map_offset_y[map] - d->map_offset_y[m];
        int raw = dm2_v1_dungeon_c_map_get_tile_value(d, m, lx, ly);
        if (raw < 0) return 0;
        maps[m].map_id = (uint8_t)m;
        maps[m].level = d->map_level_number[m];
        maps[m].world_x = d->map_offset_x[m];
        maps[m].world_y = d->map_offset_y[m];
        maps[m].width = d->level_widths[m] - 1;
        maps[m].height = d->level_heights[m] - 1;
        maps[m].tile_type_at_local = (uint8_t)(raw >> 5);
        if ((raw >> 5) == 5) {
            int link = dm2_v1_dungeon_get_first_thing(d, m, lx, ly);
            int size = 0;
            const uint8_t *r = dm2_v1_dungeon_get_thing_record(
                d, (uint16_t)link, NULL, NULL, &size);
            if (!r || size < 6) return 0;
            maps[m].teleporter_record_active = (uint8_t)(
                dm2_v1_dungeon_read_record_u16(d, r + 4) & 1);
        }
        if (d->map_level_number[m] == d->map_level_number[map] + 1)
            candidates[count++] = (uint8_t)m;
    }
    if (!count) { *result_map = -1; return 1; }
    *result_map = dm2_v1_skproject_locate_other_level(
        maps, (uint16_t)d->level_count, (int16_t)map, 1, &tx, &ty,
        candidates, count, 0, NULL, &receipt);
    *x = tx; *y = ty;
    return receipt.valid;
}

static int original_pool_bytes_equal(const DM2_V1_RecordPoolSet *a,
                                      const DM2_V1_RecordPoolSet *b)
{
    if (a->words_big_endian != b->words_big_endian ||
        a->source_words_big_endian != b->source_words_big_endian ||
        a->valid != b->valid ||
        a->record_graph_complete != b->record_graph_complete) return 0;
    for (int t = 0; t < DM2_V1_RECORD_POOL_COUNT; ++t) {
        const DM2_V1_RecordPool *p = &a->pools[t], *q = &b->pools[t];
        size_t bytes = (size_t)p->record_count * (size_t)p->record_size;
        size_t extra = (size_t)p->extension_count * (size_t)p->record_size;
        if (p->record_count != q->record_count ||
            p->record_size != q->record_size ||
            p->extension_count != q->extension_count ||
            (bytes && (!p->bytes || !q->bytes || memcmp(p->bytes,q->bytes,bytes))) ||
            (extra && (!p->extension_bytes || !q->extension_bytes ||
                memcmp(p->extension_bytes,q->extension_bytes,extra)))) return 0;
    }
    return 1;
}

static int check_complete_arranger(const DM2_V1_DungeonData *d,
    const DM2_V1_AssetLoader *loader, const DM2_V1_CaiiSourceOwner *ai,
    const DM2_V1_RecordPoolSet *pools, const DM2_V1_DropRng *rng,
    uint32_t expected_draws)
{
    OriginalArrangeQueries context = {d, loader, 0, 0};
    DM2_V1_ArrangeDungeonQueries queries = {
        &context, original_pit_gdat, original_locate_below};
    DM2_V1_ArrangeDungeonOwner owner = {0}, repeat = {0}, rejected = {0};
    DM2_V1_RecordPoolSet snapshot = {0};
    uint8_t *raw_snapshot = malloc((size_t)d->raw_size);
    uint32_t before = rng->random;
    DM2_V1_DropRng expected_rng = *rng;
    int failures = 0;
    for (uint32_t i = 0; i < expected_draws; ++i)
        (void)dm2_v1_drops_randdir(&expected_rng);
    if (!raw_snapshot || !dm2_v1_record_pool_set_clone(&snapshot,pools)) {
        free(raw_snapshot);
        return 1;
    }
    memcpy(raw_snapshot,d->raw_data,(size_t)d->raw_size);
    failures += check(original_pool_bytes_equal(pools,&snapshot),
        "pool clone preserves byte order and all source bytes");
    failures += check(dm2_v1_arrange_dungeon_owner_build(
        d,pools,ai,rng,&queries,&owner) && owner.valid,
        "full-branch arranger builds an original-data candidate");
    if (owner.valid) {
        failures += check(owner.rng_draws == expected_draws &&
            owner.rng.random == expected_rng.random,
            "original-edition arrangement preserves observed RANDDIR census");
        failures += check(!original_pool_bytes_equal(&owner.pools,pools),
            "original-data candidate changes its private record pools");
        failures += check(dm2_v1_arrange_dungeon_owner_build(
            d,pools,ai,rng,&queries,&repeat) && repeat.valid &&
            repeat.tile_count == owner.tile_count &&
            memcmp(repeat.tiles,owner.tiles,owner.tile_count) == 0 &&
            original_pool_bytes_equal(&repeat.pools,&owner.pools) &&
            repeat.rng.random == owner.rng.random &&
            repeat.rng_draws == owner.rng_draws,
            "original-data arrangement candidate is reproducible");
        {
            const DM2_V1_RecordPool *source_db4 = &pools->pools[4];
            const DM2_V1_RecordPool *changed_db4 = &owner.pools.pools[4];
            DM2_V1_RecordPoolSet bad = {0}, bad_snapshot = {0};
            DM2_V1_ArrangeDungeonOwner bad_owner = {0};
            int index;
            for (index = 0; index < source_db4->record_count; ++index) {
                size_t offset = (size_t)index * (size_t)source_db4->record_size;
                if (memcmp(source_db4->bytes + offset,
                           changed_db4->bytes + offset,
                           (size_t)source_db4->record_size) != 0) break;
            }
            if (index < source_db4->record_count &&
                dm2_v1_record_pool_set_clone(&bad,pools)) {
                bad.pools[4].bytes[(size_t)index * source_db4->record_size + 4] ^= 1;
                if (dm2_v1_record_pool_set_clone(&bad_snapshot,&bad)) {
                    failures += check(!dm2_v1_arrange_dungeon_owner_build(
                        d,&bad,ai,rng,&queries,&bad_owner) &&
                        !bad_owner.valid && !bad_owner.tiles &&
                        !bad_owner.pools.valid &&
                        original_pool_bytes_equal(&bad,&bad_snapshot) &&
                        rng->random == before,
                        "source identity mismatch rejects without changing caller state");
                } else ++failures;
            } else failures += check(0,"original candidate changes a DB4 row");
            dm2_v1_arrange_dungeon_owner_free(&bad_owner);
            dm2_v1_record_pool_set_free(&bad_snapshot);
            dm2_v1_record_pool_set_free(&bad);
        }
        printf("complete-arranger visits=%u merges=%u draws=%u tiles=%zu\n",
            owner.record_visits,owner.teleporter_merges,owner.rng_draws,
            owner.tile_count);
    }
    failures += check(!dm2_v1_arrange_dungeon_owner_build(
        d,pools,ai,rng,&queries,NULL) && rng->random == before,
        "missing arrangement output rejects without advancing RNG");
    context.pit_queries = 0;
    context.reject_pit = 1;
    failures += check(!dm2_v1_arrange_dungeon_owner_build(
        d,pools,ai,rng,&queries,&rejected) && context.pit_queries > 0 &&
        !rejected.valid && !rejected.pools.valid && !rejected.tiles,
        "unavailable authentic query discards the entire candidate");
    failures += check(rng->random == before &&
        original_pool_bytes_equal(pools,&snapshot) &&
        memcmp(raw_snapshot,d->raw_data,(size_t)d->raw_size) == 0,
        "successful and rejected arrangement preserve all caller bytes and RNG");
    dm2_v1_arrange_dungeon_owner_free(&owner);
    dm2_v1_arrange_dungeon_owner_free(&repeat);
    dm2_v1_arrange_dungeon_owner_free(&rejected);
    dm2_v1_record_pool_set_free(&snapshot);
    free(raw_snapshot);
    return failures;
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
    const char *archive = getenv("FIRESTAFF_DM2_ARRANGE_ARCHIVE");
    DM2_V1_BootProfile *profile = NULL;
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
    DM2_V1_UpdateWeatherState cold_weather;
    DM2_V1_WeatherTransitionReceipt weather_receipt;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DropRng session_rng;
    DM2_V1_DropRng expected_rng;
    uint8_t *original_bytes = NULL;
    int failures = 0;

    if ((!root || !root[0]) && (!archive || !archive[0])) {
        fputs("SKIP: original canonical data root or archive is required\n",
              stderr);
        return 77;
    }
    if (!root) root = "";
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&mutated, 0, sizeof(mutated));
    memset(&ai, 0, sizeof(ai));
    memset(&receipt, 0, sizeof(receipt));
    memset(&cold_weather, 0, sizeof(cold_weather));
    memset(&weather_receipt, 0, sizeof(weather_receipt));
    memset(&pools, 0, sizeof(pools));
    memset(&session_rng, 0, sizeof(session_rng));
    memset(&expected_rng, 0, sizeof(expected_rng));
    if (archive && archive[0]) {
        profile = calloc(1,sizeof(*profile));
        if (!profile) { failures = 1; goto done; }
        dm2_v1_boot_profile_init(profile);
        if (dm2_v1_boot_scan_assets(profile,archive) != 0 ||
            !profile->assets_verified) {
            fputs("FAIL: original archive was not hash-admitted\n",stderr);
            failures = 1; goto done;
        }
        snprintf(graphics_path,sizeof(graphics_path),"%s",profile->graphics_path);
        snprintf(dungeon_path,sizeof(dungeon_path),"%s",profile->dungeon_path);
    }
    if (!read_original_bytes(profile ? profile->graphics_mem : NULL,
            profile ? profile->graphics_mem_size : 0,
            graphics_path,&graphics,&graphics_size) ||
        !read_original_bytes(profile ? profile->dungeon_mem : NULL,
            profile ? profile->dungeon_mem_size : 0,
            dungeon_path,&dungeon_bytes,&dungeon_size) ||
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
    printf("original-edition version=%s graphics=%s dungeon=%s words-be=%d source-be=%d chains-disabled=%d\n",
        profile ? profile->version_id : "canonical",
        profile ? profile->graphics_md5 : "local",
        profile ? profile->dungeon_md5 : "local",
        dungeon.words_big_endian,dungeon.source_words_big_endian,
        dungeon.g1_w0_chains_disabled);
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
                          "unchanged-graph arrangement RNG census is complete");
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
                        !receipt.valid,
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
                "conditional zero-seeded weather advances the shared RNG by three draws");
            failures += check_complete_arranger(&dungeon, &loader, &ai,
                                                   &pools, &session_rng,
                                                   baseline.rng_draws);
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
    if (profile) { dm2_v1_boot_cleanup(profile); free(profile); }
    free(original_bytes);
    dm2_v1_record_pool_set_free(&pools);
    dm2_v1_dungeon_free(&mutated);
    dm2_v1_caii_source_owner_free(&ai);
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    return failures ? 1 : 0;
}
