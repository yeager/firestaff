#include "theron_v1_track02_creature_spawn.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* This target is also built with NDEBUG by release configurations.  Keep the
 * source-admission assertions live: a passing executable must have evaluated
 * its real Track 02 checks, not merely compiled them away. */
#undef assert
#define assert(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static uint8_t *read_file(const char *path, size_t *size_out) {
    FILE *file;
    long size;
    uint8_t *bytes;

    if (size_out) *size_out = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    size = ftell(file);
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    if (size_out) *size_out = (size_t)size;
    return bytes;
}

static void assert_decoded_real_bin(const char *path,
                                    Theron_V1Track02Variant variant) {
    static const uint8_t expected_zone_records[THERON_TRACK02_SPAWN_ZONE_COUNT][8] = {
        { 0x2f, 0x00, 0x2c, 0x00, 0x03, 0x05, 0x0e, 0x02 },
        { 0x1b, 0x00, 0x18, 0x00, 0x02, 0x04, 0x10, 0x02 },
        { 0x17, 0x00, 0x14, 0x00, 0x02, 0x04, 0x10, 0x02 },
        { 0x17, 0x00, 0x14, 0x00, 0x02, 0x04, 0x10, 0x02 },
        { 0x00, 0x00, 0x18, 0x00, 0x08, 0x0a, 0x12, 0x02 }
    };
    static const uint32_t jp_zone_offsets[THERON_TRACK02_SPAWN_ZONE_COUNT] = {
        0x273858u, 0x2738d7u, 0x273902u, 0x273929u, 0x273950u
    };
    static const uint32_t us_zone_offsets[THERON_TRACK02_SPAWN_ZONE_COUNT] = {
        0x274058u, 0x2740d7u, 0x274102u, 0x274129u, 0x274150u
    };
    Theron_Track02SpawnSource source;
    Theron_Track02SpawnConsumerSourceReceipt consumer;
    size_t size;
    uint8_t *bytes = read_file(path, &size);

    if (!bytes) return;
    assert(theron_v1_track02_decode_spawn_source(bytes, size, variant,
                                                 &source) == 1);
    assert(source.authenticated == 1);
    assert(source.variant == (int)variant);
    for (unsigned int i = 0; i < THERON_TRACK02_SPAWN_POINTER_COUNT; ++i) {
        const Theron_CreaturePointerEntry *expected =
            theron_v1_track02_creature_pointer(i);
        assert(source.pointers[i].sprite_desc_offset ==
               expected->sprite_desc_offset);
        assert(source.pointers[i].constant_278a ==
               (variant == THERON_V1_TRACK02_VARIANT_JP_BIN ?
                    0x2780u : expected->constant_278a));
        assert(source.pointers[i].spawn_data_offset ==
               expected->spawn_data_offset);
        assert(source.pointers[i].constant_016b ==
               expected->constant_016b);
    }
    for (unsigned int i = 0; i < THERON_TRACK02_SPAWN_ZONE_COUNT; ++i) {
        const uint32_t user_offset = variant == THERON_V1_TRACK02_VARIANT_JP_BIN ?
            jp_zone_offsets[i] : us_zone_offsets[i];
        const size_t raw_sector = (size_t)user_offset / 2048u;
        const size_t raw_in_sector = (size_t)user_offset % 2048u;
        const uint8_t *record = bytes +
            raw_sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
            THERON_V1_TRACK02_MODE1_HEADER_BYTES + raw_in_sector;
        const Theron_SpawnZoneDesc *zone = &source.zones[i];

        assert(zone->map_width ==
               (uint16_t)(record[0] | ((uint16_t)record[1] << 8u)));
        assert(zone->map_height ==
               (uint16_t)(record[2] | ((uint16_t)record[3] << 8u)));
        assert(zone->category == record[4]);
        assert(zone->count == record[5]);
        assert(zone->param1 == record[6]);
        assert(zone->param2 == record[7]);
        if (variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
            assert(memcmp(record, expected_zone_records[i],
                          sizeof(expected_zone_records[i])) == 0);
            assert(zone->map_width ==
                   (uint16_t)(expected_zone_records[i][0] |
                              ((uint16_t)expected_zone_records[i][1] << 8u)));
            assert(zone->map_height ==
                   (uint16_t)(expected_zone_records[i][2] |
                              ((uint16_t)expected_zone_records[i][3] << 8u)));
            assert(zone->category == expected_zone_records[i][4]);
            assert(zone->count == expected_zone_records[i][5]);
            assert(zone->param1 == expected_zone_records[i][6]);
            assert(zone->param2 == expected_zone_records[i][7]);
        } else {
            const Theron_SpawnZoneDesc *expected =
                theron_v1_track02_spawn_zone(i);
            assert(memcmp(zone, expected, sizeof(*expected)) == 0);
        }
    }
    assert(theron_v1_track02_bind_spawn_consumer_source(
               bytes, size, variant, &consumer) == 1);
    assert(consumer.valid == 1 && consumer.variant == variant);
    assert(consumer.user_data_offset ==
           (variant == THERON_V1_TRACK02_VARIANT_JP_BIN ?
                0x0868d2u : 0x0870e5u));
    assert(consumer.byte_count == 0x10du);
    assert(consumer.checksum ==
           (variant == THERON_V1_TRACK02_VARIANT_JP_BIN ?
                0x7dc1e453u : 0xeb241d19u));
    assert(consumer.regional_code_verified == 1);
    assert(consumer.runtime_execution_proven == 0);
    assert(consumer.category_semantics_proven == 0);
    bytes[0x2d157fu] ^= 1u;
    assert(theron_v1_track02_decode_spawn_source(bytes, size, variant,
                                                 &source) == 0);
    assert(source.authenticated == 0);
    assert(theron_v1_track02_bind_spawn_consumer_source(
               bytes, size, variant, &consumer) == 0);
    assert(consumer.valid == 0);
    free(bytes);
}

int main(void) {
    assert(theron_v1_track02_spawn_zone_count() == 5);

    /* AKUTUBA: category 3, 5 creatures, map 47x44 */
    const Theron_SpawnZoneDesc *z0 = theron_v1_track02_spawn_zone(0);
    assert(z0 != NULL);
    assert(z0->map_width == 47);
    assert(z0->map_height == 44);
    assert(z0->category == 3);
    assert(z0->count == 5);
    assert(z0->param1 == 14);

    /* DRATOR: category 2 */
    const Theron_SpawnZoneDesc *z1 = theron_v1_track02_spawn_zone(1);
    assert(z1->category == 2);
    assert(z1->count == 4);

    /* FORMIC and SARMON: same category and count */
    const Theron_SpawnZoneDesc *z2 = theron_v1_track02_spawn_zone(2);
    const Theron_SpawnZoneDesc *z3 = theron_v1_track02_spawn_zone(3);
    assert(z2->category == z3->category);
    assert(z2->count == z3->count);

    /* THIEF/DEMON: no spawn zone (index >= 5) */
    assert(theron_v1_track02_spawn_zone(5) == NULL);
    assert(theron_v1_track02_spawn_zone(6) == NULL);

    /* Category formulas */
    const Theron_SpawnCategoryFormula *f0 = theron_v1_track02_spawn_formula(0);
    assert(f0->dice_param == 4);
    assert(f0->multiplier == 0);

    const Theron_SpawnCategoryFormula *f1 = theron_v1_track02_spawn_formula(1);
    assert(f1->multiplier == 21);

    const Theron_SpawnCategoryFormula *f2 = theron_v1_track02_spawn_formula(2);
    assert(f2->multiplier == 25);
    assert(f2->uses_1_5x == 1);

    const Theron_SpawnCategoryFormula *f3 = theron_v1_track02_spawn_formula(3);
    assert(f3->dice_param == 5);
    assert(f3->uses_1_5x == 1);

    assert(theron_v1_track02_spawn_formula(4) == NULL);

    /* Creature pointer table */
    const Theron_CreaturePointerEntry *p0 = theron_v1_track02_creature_pointer(0);
    assert(p0 != NULL);
    assert(p0->sprite_desc_offset == 0x0172);
    assert(p0->constant_278a == 0x278A);
    assert(p0->spawn_data_offset == 0x0058);
    assert(p0->constant_016b == 0x016B);

    /* AKUTUBA, DRATOR, SARMON share sprite descriptor 0x0172 */
    assert(theron_v1_track02_creature_pointer(1)->sprite_desc_offset == 0x0172);
    assert(theron_v1_track02_creature_pointer(3)->sprite_desc_offset == 0x0172);

    /* THIEF and DEMON: spawn_data_offset == 0 (no regular spawns) */
    assert(theron_v1_track02_creature_pointer(5)->spawn_data_offset == 0x0000);
    assert(theron_v1_track02_creature_pointer(6)->spawn_data_offset == 0x0000);

    /* Entries 0-4 share constant_016b = 0x016B; 5-7 have 0x0000 */
    for (unsigned i = 0; i < 5; i++) {
        const Theron_CreaturePointerEntry *p = theron_v1_track02_creature_pointer(i);
        assert(p->constant_278a == 0x278A);
        assert(p->constant_016b == 0x016B);
    }
    for (unsigned i = 5; i < 8; i++) {
        const Theron_CreaturePointerEntry *p = theron_v1_track02_creature_pointer(i);
        assert(p->constant_278a == 0x278A);
        assert(p->constant_016b == 0x0000);
    }

    /* Entry 7 exists (unused slot) */
    assert(theron_v1_track02_creature_pointer(7) != NULL);
    assert(theron_v1_track02_creature_pointer(7)->sprite_desc_offset == 0x01DC);
    assert(theron_v1_track02_creature_pointer(8) == NULL);

    /* HP cap constant */
    assert(THERON_CREATURE_HP_CAP == 900);

    /* The disassembly branch constants are a receipt, not a complete
     * gameplay formula.  Until the original RNG consumer is captured, the
     * source-bound API must not publish synthetic stats. */
    {
        Theron_SpawnStats s = { 9, 9, 9 };
        assert(theron_v1_track02_compute_spawn_stats(0, 14, 2, 0, &s) == 0);
        assert(s.hp == 0 && s.attack == 0 && s.defense == 0);
    }

    /* Unknown categories must not receive invented combat statistics. */
    {
        Theron_SpawnStats s = { 9, 9, 9 };
        assert(theron_v1_track02_compute_spawn_stats(4, 14, 2, 255, &s) == 0);
        assert(s.hp == 0 && s.attack == 0 && s.defense == 0);
    }

    /* Instruction-level receipt vectors.  These are not gameplay fixtures:
     * all helper/RNG values are explicit witness inputs and no RNG is called.
     * The expected values are the visible $B0E5-$B1EB arithmetic only. */
    {
        Theron_SpawnConsumerWitness w;
        Theron_SpawnConsumerReceipt r;
        memset(&w, 0, sizeof(w));
        w.authenticated_execution = 1;
        w.category = 2;
        w.b6 = 4;
        w.b4b5 = 0x2000;
        w.helper_b8 = 100;
        w.rng_common_1 = 3;
        w.hp_accumulator = 200;
        w.attack_accumulator = 950;
        w.defense_accumulator = 9900;
        w.ld23a_b8 = 100;
        w.ld23a_b4 = 200;
        assert(theron_v1_track02_apply_spawn_consumer_witness(&w, &r) == 1);
        assert(r.valid == 1);
        assert(r.hp_accumulator == 303); /* 200+100+(100&3) */
        assert(r.attack_accumulator == 999);
        assert(r.defense_accumulator == 9999);
        assert(r.helper_input_b8 == 150); /* 100 + (101 >> 1) */
        assert(r.helper_input_b4 == 0x00 && r.helper_input_b5 == 0x20);

        w.authenticated_execution = 0;
        assert(theron_v1_track02_apply_spawn_consumer_witness(&w, &r) == 0);
        assert(r.valid == 0);
    }

    /* The production table must also be recoverable from the authentic raw
     * BIN.  These paths are user-supplied data and are intentionally
     * skip-safe for CI machines without the copyrighted game files. */
    {
        const char *us = getenv("THERON_TRACK02_US_BIN");
        const char *jp = getenv("THERON_TRACK02_JP_BIN");
        const char *home = getenv("HOME");
        char us_default[4096];
        char jp_default[4096];
        if (!us || !us[0]) {
            int length = home && home[0]
                ? snprintf(us_default, sizeof(us_default),
                           "%s/.firestaff/data/theron/TQUS02.bin", home)
                : -1;
            us = length >= 0 && (size_t)length < sizeof(us_default)
                ? us_default : NULL;
        }
        if (!jp || !jp[0]) {
            int length = home && home[0]
                ? snprintf(jp_default, sizeof(jp_default),
                           "%s/.firestaff/data/theron/TQJP02.bin", home)
                : -1;
            jp = length >= 0 && (size_t)length < sizeof(jp_default)
                ? jp_default : NULL;
        }
        FILE *us_file = us ? fopen(us, "rb") : NULL;
        FILE *jp_file = jp ? fopen(jp, "rb") : NULL;
        if (!us_file) {
            if (jp_file) fclose(jp_file);
            puts("SKIP: authentic US Theron Track 02 BIN not present");
            return 77;
        }
        fclose(us_file);
        assert_decoded_real_bin(us, THERON_V1_TRACK02_VARIANT_US_BIN);
        if (jp_file) {
            fclose(jp_file);
            assert_decoded_real_bin(jp, THERON_V1_TRACK02_VARIANT_JP_BIN);
        } else {
            puts("NOTE: JP Track 02 BIN not present; JP source decode skipped");
        }
    }

    printf("PASS: theron_v1_track02_creature_spawn\n");
    return 0;
}
