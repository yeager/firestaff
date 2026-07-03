#include "csb_v1_skin_cache_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t record_id;
    const uint8_t *bytes;
    size_t size;
} FixtureRecord;

typedef struct {
    const FixtureRecord *records;
    size_t count;
    int lookup_count;
    uint32_t last_record_id;
} FixtureLookup;

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    } else {
        printf("ok %s = %d\n", label, actual);
    }
}

static void check_true(const char *label, int condition)
{
    check_int(label, condition ? 1 : 0, 1);
}

static int fixture_lookup(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user)
{
    FixtureLookup *fixture = (FixtureLookup *)user;
    size_t i;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!fixture) return 0;
    ++fixture->lookup_count;
    fixture->last_record_id = record_id;
    for (i = 0u; i < fixture->count; ++i) {
        if (fixture->records[i].record_id == record_id) {
            if (out_bytes) *out_bytes = fixture->records[i].bytes;
            if (out_size) *out_size = fixture->records[i].size;
            return 1;
        }
    }
    return 0;
}

static void test_record_ids(void)
{
    check_int("ids.edt_skins", (int)CSB_V1_SKIN_CACHE_EDT_SKINS, 4);
    check_int("ids.column_l3_x5",
              (int)csb_v1_skin_cache_column_record_id(3, 5),
              (int)((4u << 24) | (3u << 4) | 2u));
    check_int("ids.default",
              (int)csb_v1_skin_cache_default_record_id(),
              (int)((4u << 24) | 0x800000u));
}

static void test_get_skin_lazy_columns(void)
{
    static const uint8_t col2[] = {
        10u, 11u, 12u, 13u, 14u, 15u
    };
    FixtureRecord records[] = {
        { (4u << 24) | (3u << 4) | 2u, col2, sizeof(col2) }
    };
    FixtureLookup lookup = { records, 1u, 0, 0u };
    CSB_V1_SkinCache cache;

    csb_v1_skin_cache_init(&cache);
    check_int("skin.bounds_negative",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          3, 8, 8, -1, 0),
              0);
    check_int("skin.bounds_no_lookup", lookup.lookup_count, 0);
    check_int("skin.x4_y2",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          3, 8, 8, 4, 2),
              14);
    check_int("skin.lookup_once", lookup.lookup_count, 1);
    check_int("skin.record_id",
              (int)lookup.last_record_id,
              (int)((4u << 24) | (3u << 4) | 2u));
    check_int("skin.x5_y2_same_column",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          3, 8, 8, 5, 2),
              15);
    check_int("skin.still_one_lookup", lookup.lookup_count, 1);
    check_int("skin.short_column_returns_zero",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          3, 8, 8, 5, 3),
              0);
}

static void test_level_change_and_truncate(void)
{
    uint8_t big_record[80];
    FixtureRecord records[] = {
        { (4u << 24) | (4u << 4) | 0u, big_record, sizeof(big_record) },
        { (4u << 24) | (5u << 4) | 0u, big_record, sizeof(big_record) }
    };
    FixtureLookup lookup = { records, 2u, 0, 0u };
    CSB_V1_SkinCache cache;
    int i;

    for (i = 0; i < (int)sizeof(big_record); ++i) {
        big_record[i] = (uint8_t)i;
    }
    csb_v1_skin_cache_init(&cache);
    check_int("skin.truncate_last_valid",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          4, 32, 40, 1, 31),
              63);
    check_int("skin.truncate_past_64",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          4, 32, 40, 0, 32),
              0);
    check_int("skin.level4_lookup", lookup.lookup_count, 1);
    check_int("skin.level_change_reloads",
              csb_v1_skin_cache_get_skin(&cache, fixture_lookup, &lookup,
                                          5, 32, 40, 0, 1),
              2);
    check_int("skin.level5_lookup", lookup.lookup_count, 2);
}

static void test_default_skin_cache(void)
{
    uint8_t defaults[80];
    FixtureRecord records[] = {
        { (4u << 24) | 0x800000u, defaults, sizeof(defaults) }
    };
    FixtureLookup lookup = { records, 1u, 0, 0u };
    CSB_V1_SkinCache cache;
    int i;

    for (i = 0; i < (int)sizeof(defaults); ++i) {
        defaults[i] = (uint8_t)(100 + i);
    }
    csb_v1_skin_cache_init(&cache);
    check_int("default.level3",
              csb_v1_skin_cache_get_default_skin(&cache,
                                                  fixture_lookup,
                                                  &lookup,
                                                  3),
              103);
    check_int("default.lookup_once", lookup.lookup_count, 1);
    check_int("default.level63_truncated",
              csb_v1_skin_cache_get_default_skin(&cache,
                                                  fixture_lookup,
                                                  &lookup,
                                                  63),
              163);
    check_int("default.no_second_lookup", lookup.lookup_count, 1);
    check_int("default.out_of_range",
              csb_v1_skin_cache_get_default_skin(&cache,
                                                  fixture_lookup,
                                                  &lookup,
                                                  64),
              0);
}

static void test_set_skin_overlay(void)
{
    CSB_V1_SkinCache cache;

    csb_v1_skin_cache_init(&cache);
    check_int("set.apply",
              csb_v1_skin_cache_set_skin(&cache, 2, 8, 8, 3, 2, 77u),
              1);
    check_int("set.read_without_lookup",
              csb_v1_skin_cache_get_skin(&cache, NULL, NULL, 2, 8, 8, 3, 2),
              77);
    check_int("set.bounds",
              csb_v1_skin_cache_set_skin(&cache, 2, 8, 8, 8, 2, 55u),
              0);
}

int main(void)
{
    test_record_ids();
    test_get_skin_lazy_columns();
    test_level_change_and_truncate();
    test_default_skin_cache();
    test_set_skin_overlay();
    check_true("source_evidence",
               strstr(csb_v1_skin_cache_source_evidence(),
                      "data.cpp:2080-2105 GetSkin") != NULL);
    if (g_failures) {
        printf("CSB V1 skin cache failed: %d/%d assertions\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("CSB V1 skin cache passed: %d assertions\n", g_assertions);
    return 0;
}
