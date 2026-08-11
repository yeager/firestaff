#include "theron_v1_track02_creature_spawn.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    Theron_Track02SpawnSource source;
    size_t size;
    uint8_t *bytes = read_file(path, &size);

    if (!bytes) return;
    assert(theron_v1_track02_decode_spawn_source(bytes, size, variant,
                                                 &source) == 1);
    assert(source.authenticated == 1);
    assert(source.variant == variant);
    for (unsigned int i = 0; i < THERON_TRACK02_SPAWN_POINTER_COUNT; ++i) {
        const Theron_CreaturePointerEntry *expected =
            theron_v1_track02_creature_pointer(i);
        assert(memcmp(&source.pointers[i], expected, sizeof(*expected)) == 0);
    }
    for (unsigned int i = 0; i < THERON_TRACK02_SPAWN_ZONE_COUNT; ++i) {
        const Theron_SpawnZoneDesc *expected =
            theron_v1_track02_spawn_zone(i);
        assert(memcmp(&source.zones[i], expected, sizeof(*expected)) == 0);
    }
    bytes[0x2d157fu] ^= 1u;
    assert(theron_v1_track02_decode_spawn_source(bytes, size, variant,
                                                 &source) == 0);
    assert(source.authenticated == 0);
    free(bytes);
}

static void assert_real_bin_rejected(const char *path,
                                     Theron_V1Track02Variant variant) {
    Theron_Track02SpawnSource source;
    size_t size;
    uint8_t *bytes = read_file(path, &size);

    if (!bytes) return;
    assert(theron_v1_track02_decode_spawn_source(bytes, size, variant,
                                                 &source) == 0);
    assert(source.authenticated == 0);
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
        if (!us) us = "/Users/bosse/.firestaff/data/theron/TQUS02.bin";
        if (!jp) jp = "/Users/bosse/.firestaff/data/theron/TQJP02.bin";
        FILE *us_file = fopen(us, "rb");
        FILE *jp_file = fopen(jp, "rb");
        if (!us_file || !jp_file) {
            if (us_file) fclose(us_file);
            if (jp_file) fclose(jp_file);
            puts("SKIP: authentic Theron Track 02 BINs not present");
            return 77;
        }
        fclose(us_file);
        fclose(jp_file);
        assert_decoded_real_bin(us, THERON_V1_TRACK02_VARIANT_US_BIN);
        /* JP is a real, hash-verified asset, but its pointer table is not at
         * the US disassembly offsets.  Do not silently reinterpret it. */
        assert_real_bin_rejected(jp, THERON_V1_TRACK02_VARIANT_JP_BIN);
    }

    printf("PASS: theron_v1_track02_creature_spawn\n");
    return 0;
}
