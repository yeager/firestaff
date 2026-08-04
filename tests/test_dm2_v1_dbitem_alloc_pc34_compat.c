/*
 * test_dm2_v1_dbitem_alloc_pc34_compat.c — DM2-006 ALLOC_NEW_DBITEM drop
 * path over the DM2-002 record pool.
 *
 * Verifies the skproject boundary:
 *   c_record.cpp:367-401   GET_ITEMDB_OF_ITEMSPEC_ACTUATOR
 *   c_record.cpp:403-444   GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR
 *   c_record.cpp:1076-1139 ALLOC_NEW_RECORD (scan, bones 0x800A, dbMisc
 *                          reserve, zero + END init, recycle fail-closed)
 *   c_record.cpp:284-345   SET_ITEMTYPE per-DB writes
 *   c_record.cpp:1142-1165 ALLOC_NEW_DBITEM
 *   c_record.cpp:1568-1634 DROP_CREATURE_POSSESSION generated-drops loop
 *                          (interleaved RNG order, OBJECT_NULL slot break,
 *                          party-cell RANDBIT / RANDDIR direction draw,
 *                          direction folded into the record word)
 */

#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_record_ops_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

static int16_t mk_handle(int pool, int index)
{
    return (int16_t)((pool << 10) | (index & 0x3ff));
}

static int16_t rd16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

/* Reference copy of the source LCG (c_random.cpp:13-47) for cross-checking
 * the consumed draw sequence. */
static uint32_t ref_rand(uint32_t *state)
{
    *state = *state * 0xbb40e62du + 11u;
    return *state >> 8;
}

/* Free slot marker: source ALLOC_NEW_RECORD scans for w0 == OBJECT_NULL. */
static void mark_free(uint8_t *pool_bytes, int record_size, int index)
{
    wr16(pool_bytes + (size_t)index * (size_t)record_size,
         DM2_V1_RECORD_HANDLE_NULL);
}

static void add_pool(DM2_V1_RecordPoolSet *set, int db, int record_size,
                     int count, int free_slots)
{
    set->pools[db].record_size = record_size;
    set->pools[db].record_count = count;
    set->pools[db].source_base = 0;
    set->pools[db].bytes = calloc((size_t)count, (size_t)record_size);
    for (int i = 0; i < count; ++i) {
        if (i < free_slots) {
            mark_free(set->pools[db].bytes, record_size, i);
        } else {
            wr16(set->pools[db].bytes + (size_t)i * (size_t)record_size,
                 DM2_V1_RECORD_HANDLE_END); /* in use */
        }
    }
}

int main(void)
{
    /* ── itemspec mapping (c_record.cpp:367-444) ─────────────────── */
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x0005) == 5,
          "group 0 -> dbWeapon");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x0005) == 5,
          "group 0 type unchanged");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x007f) == 127,
          "group 0 max type");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x0083) == 6,
          "group 1 -> dbCloth");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x0083) == 3,
          "group 1 type minus 0x80");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x0102) == 10,
          "group 2 -> dbMisc");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x0102) == 2,
          "group 2 type minus 0x100");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x0185) == 8,
          "group 3 below 0x1b0 -> dbPotion");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x0185) == 5,
          "potion type minus 0x180");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x01b3) == 4,
          "group 3 >= 0x1b0 -> dbCreature");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x01b3) == 3,
          "creature type minus 0x1b0");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x01e5) == 9,
          "group 3 >= 0x1e0 -> dbContainer");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x01e5) == 5,
          "container type minus 0x1e0");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x01fc) == 7,
          "0x1fc -> dbScroll");
    CHECK(dm2_v1_get_itemtype_of_itemspec_actuator(0x01fc) == 0,
          "scroll type 0");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x01fd) == -1,
          "0x1fd invalid -> source 0xffff");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x01ff) == -1,
          "0x1ff invalid");
    CHECK(dm2_v1_get_itemdb_of_itemspec_actuator(0x8005) == 5,
          "itemspec masked to 9 bits, bone bit ignored for db");

    /* ── ALLOC_NEW_RECORD (c_record.cpp:1076-1139) ───────────────── */
    {
        DM2_V1_RecordPoolSet set;
        int16_t r0, r1, r2, r3;

        memset(&set, 0, sizeof(set));
        /* DB5 x3: rec0 free (garbage tail), rec1 in use, rec2 free. */
        add_pool(&set, 5, 4, 3, 0);
        mark_free(set.pools[5].bytes, 4, 0);
        set.pools[5].bytes[2] = 0xAA;
        set.pools[5].bytes[3] = 0xBB;
        mark_free(set.pools[5].bytes, 4, 2);
        /* DB10 x5: all free. */
        add_pool(&set, 10, 4, 5, 5);
        /* DB9 container x1: free. */
        add_pool(&set, 9, 8, 1, 1);
        set.valid = 1;

        r0 = dm2_v1_record_pool_alloc_new_record(&set, 5);
        CHECK(r0 == mk_handle(5, 0), "first free slot allocates index 0");
        CHECK(rd16(set.pools[5].bytes + 0) == DM2_V1_RECORD_HANDLE_END,
              "allocated slot link terminated");
        CHECK(set.pools[5].bytes[2] == 0 && set.pools[5].bytes[3] == 0,
              "allocated slot zeroed first");
        r1 = dm2_v1_record_pool_alloc_new_record(&set, 5);
        CHECK(r1 == mk_handle(5, 2), "in-use records are skipped");
        r2 = dm2_v1_record_pool_alloc_new_record(&set, 5);
        CHECK(r2 == DM2_V1_RECORD_HANDLE_NULL,
              "exhausted pool fail-closed (recycle unproven)");

        /* dbMisc reserve: 5 records - 3 = 2 plain allocations. */
        r0 = dm2_v1_record_pool_alloc_new_record(&set, 10);
        r1 = dm2_v1_record_pool_alloc_new_record(&set, 10);
        r2 = dm2_v1_record_pool_alloc_new_record(&set, 10);
        CHECK(r0 == mk_handle(10, 0) && r1 == mk_handle(10, 1) &&
                  r2 == DM2_V1_RECORD_HANDLE_NULL,
              "dbMisc reserves the last 3 records");
        /* bones 0x800A: full pool, continues at index 2. */
        r0 = dm2_v1_record_pool_alloc_new_record(&set, 0x800A);
        r1 = dm2_v1_record_pool_alloc_new_record(&set, 0x800A);
        r2 = dm2_v1_record_pool_alloc_new_record(&set, 0x800A);
        r3 = dm2_v1_record_pool_alloc_new_record(&set, 0x800A);
        CHECK(r0 == mk_handle(10, 2) && r1 == mk_handle(10, 3) &&
                  r2 == mk_handle(10, 4) &&
                  r3 == DM2_V1_RECORD_HANDLE_NULL,
              "bones 0x800A allocate the reserved records");

        /* Container init: w0 and w2 terminated (c_record.cpp:1137-1138). */
        r0 = dm2_v1_record_pool_alloc_new_record(&set, 9);
        CHECK(r0 == mk_handle(9, 0), "container record allocated");
        CHECK(rd16(set.pools[9].bytes + 0) == DM2_V1_RECORD_HANDLE_END &&
                  rd16(set.pools[9].bytes + 2) == DM2_V1_RECORD_HANDLE_END,
              "container w0 + w2 terminated");

        CHECK(dm2_v1_record_pool_alloc_new_record(&set, 11) ==
                  DM2_V1_RECORD_HANDLE_NULL,
              "zero-sized DB11 fails closed");
        CHECK(dm2_v1_record_pool_alloc_new_record(NULL, 5) ==
                  DM2_V1_RECORD_HANDLE_NULL,
              "NULL set fails closed");

        dm2_v1_record_pool_set_free(&set);
    }

    /* ── SET_ITEMTYPE (c_record.cpp:284-345) ─────────────────────── */
    {
        DM2_V1_RecordPoolSet set;
        int16_t rec;

        memset(&set, 0, sizeof(set));
        add_pool(&set, 4, 16, 1, 1);
        add_pool(&set, 5, 4, 1, 1);
        add_pool(&set, 7, 4, 1, 1);
        add_pool(&set, 8, 8, 1, 1);
        add_pool(&set, 9, 8, 1, 1);
        add_pool(&set, 2, 4, 1, 1);
        set.valid = 1;

        /* db5 weapon: low 7 bits of word@2, high bit of byte@2 kept. */
        rec = mk_handle(5, 0);
        wr16(set.pools[5].bytes + 2, (int16_t)0x00ff);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 0x45) == 1,
              "weapon type write runs");
        CHECK(rd16(set.pools[5].bytes + 2) == (int16_t)0x00c5,
              "weapon type in word@2 low 7 bits, bit 7 preserved");

        /* db8 potion: high 7 bits of word@2 (byte@3). */
        rec = mk_handle(8, 0);
        wr16(set.pools[8].bytes + 2, (int16_t)0x80ff);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 0x10) == 1,
              "potion type write runs");
        CHECK(rd16(set.pools[8].bytes + 2) == (int16_t)0x90ff,
              "potion type shifted into byte@3 low 7 bits");

        /* db9 container: charge split over word@4. */
        rec = mk_handle(9, 0);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 21) == 1,
              "container type write runs");
        CHECK(rd16(set.pools[9].bytes + 4) == (int16_t)0xA004,
              "container charge split: (21/8)<<1 | (21&7)<<13");
        CHECK(rd16(set.pools[9].bytes + 6) == 0,
              "word@6 untouched when (w&6) != 2");
        rec = mk_handle(9, 0);
        wr16(set.pools[9].bytes + 4, 0);
        wr16(set.pools[9].bytes + 6, 0);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 9) == 1,
              "container type 9 write runs");
        CHECK(rd16(set.pools[9].bytes + 4) == (int16_t)0x2002,
              "container type 9: (9/8)<<1 | (9&7)<<13");
        CHECK(rd16(set.pools[9].bytes + 6) == DM2_V1_RECORD_HANDLE_NULL,
              "(w&6)==2 marks word@6 (c_record.cpp:339-340)");

        /* db7 scroll: deliberate no-op. */
        rec = mk_handle(7, 0);
        wr16(set.pools[7].bytes + 2, (int16_t)0x1234);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 0x40) == 1,
              "scroll path runs");
        CHECK(rd16(set.pools[7].bytes + 2) == (int16_t)0x1234,
              "scroll never written");

        /* db4 creature: byte@4. */
        rec = mk_handle(4, 0);
        CHECK(dm2_v1_record_pool_set_itemtype(&set, rec, 0x0C) == 1,
              "creature type write runs");
        CHECK(set.pools[4].bytes[4] == 0x0C, "creature type at byte@4");

        /* Guards. */
        CHECK(dm2_v1_record_pool_set_itemtype(
                  &set, DM2_V1_RECORD_HANDLE_NULL, 1) == 0,
              "OBJECT_NULL guard");
        CHECK(dm2_v1_record_pool_set_itemtype(&set, (int16_t)0xff80, 1) == 0,
              "handle >= 0xff80 guard");
        CHECK(dm2_v1_record_pool_set_itemtype(&set, mk_handle(2, 0), 1) == 0,
              "db outside 4..10 guard");

        dm2_v1_record_pool_set_free(&set);
    }

    /* ── ALLOC_NEW_DBITEM (c_record.cpp:1142-1165) ───────────────── */
    {
        DM2_V1_RecordPoolSet set;
        int16_t rec;

        memset(&set, 0, sizeof(set));
        add_pool(&set, 6, 4, 1, 1);
        set.valid = 1;

        rec = dm2_v1_alloc_new_dbitem(&set, 0x0083);
        CHECK(rec == mk_handle(6, 0), "cloth item allocated");
        CHECK(rd16(set.pools[6].bytes + 2) == 3,
              "SET_ITEMTYPE applied during ALLOC_NEW_DBITEM");
        CHECK(dm2_v1_alloc_new_dbitem(&set, 0x01fd) ==
                  DM2_V1_RECORD_HANDLE_NULL,
              "invalid itemspec fails closed");
        CHECK(dm2_v1_alloc_new_dbitem(NULL, 0x0083) ==
                  DM2_V1_RECORD_HANDLE_NULL,
              "NULL set fails closed");

        dm2_v1_record_pool_set_free(&set);
    }

    /* ── generated-drops placement (c_record.cpp:1568-1634) ──────── */
    {
        DM2_V1_RecordPoolSet set;
        DM2_V1_DropRng rng;
        DM2_V1_DropPlacedItem items[8];
        uint16_t slot_words[DM2_DROP_SLOT_COUNT];
        int16_t ground;
        int iterations;
        int placed;
        uint32_t ref;
        uint32_t draw;
        int count0;

        memset(&set, 0, sizeof(set));
        add_pool(&set, 5, 4, 4, 4);
        add_pool(&set, 6, 4, 2, 2);
        set.valid = 1;
        memset(slot_words, 0, sizeof(slot_words));
        /* slot 0x0A: itemspec 5 (weapon type 5), base 2, extra range 1. */
        slot_words[0] = (uint16_t)((5u << 7) | (1u << 4) | 1u);
        /* slot 0x0B: itemspec 0x83 (cloth type 3), base 1, no extra. */
        slot_words[1] = (uint16_t)(0x83u << 7);

        ground = DM2_V1_RECORD_HANDLE_END;
        dm2_v1_drops_rng_init(&rng);
        placed = dm2_v1_drops_place_source_slots(
            &set, slot_words, &rng,
            9, 9, 2,   /* party pose */
            3, 4,      /* drop cell (away from party) */
            &ground, items, 8, &iterations);

        /* Reference RNG: extra roll, then per-item RANDDIR draws. */
        ref = 0;
        draw = ref_rand(&ref);
        count0 = 2 + (int)(draw % 2u);
        CHECK(iterations == count0 + 1,
              "per-item iterations follow the source count loop");
        CHECK(placed == count0 + 1, "all items placed");
        /* Replay the reference stream: slot 0 draws. */
        for (int i = 0; i < count0; ++i) {
            draw = ref_rand(&ref);
            CHECK(items[i].slot_field == 0x0A && items[i].item_ordinal == i,
                  "slot 0 receipts in source order");
            CHECK(items[i].itemspec == 5 && items[i].item_db == 5 &&
                      items[i].item_type == 5,
                  "weapon itemspec resolved");
            CHECK(items[i].direction_rand == (int)(draw & 3u),
                  "RANDDIR draw consumed in source order");
            CHECK(items[i].direction == items[i].direction_rand,
                  "away-from-party direction is RANDDIR");
            CHECK(!items[i].at_party_cell, "away from party flagged");
            CHECK(items[i].placed && items[i].record != DM2_V1_RECORD_HANDLE_NULL,
                  "item record placed");
            CHECK((((uint16_t)items[i].record >> 14) & 3u) ==
                      (uint16_t)items[i].direction,
                  "direction folded into record word bits 14-15");
            CHECK(dm2_v1_record_handle_pool(items[i].record) == 5,
                  "record handle decodes to dbWeapon");
        }
        draw = ref_rand(&ref);
        CHECK(rng.random == ref, "rng stream matches the source interleave");
        CHECK(items[count0].slot_field == 0x0B &&
                  items[count0].item_ordinal == 0 &&
                  items[count0].direction_rand == (int)(draw & 3u),
              "slot 1 draw follows slot 0 items (interleaved order)");
        CHECK(items[count0].item_db == 6 && items[count0].item_type == 3,
              "cloth itemspec resolved");

        /* Ground list: source-order chain, END-terminated. */
        {
            int16_t cursor = ground;
            int walked = 0;
            CHECK(dm2_v1_record_handle_pool(cursor) == 5,
                  "ground head is the first weapon record");
            while (cursor != DM2_V1_RECORD_HANDLE_END && walked < 16) {
                int16_t next;
                CHECK(dm2_v1_record_pool_next_link(&set, cursor, &next),
                      "chain link resolves");
                cursor = next;
                ++walked;
            }
            CHECK(walked == count0 + 1 && cursor == DM2_V1_RECORD_HANDLE_END,
                  "ground chain holds every placed item in order");
        }
        /* Records carry the itemtype (SET_ITEMTYPE on alloc). */
        CHECK(rd16(set.pools[5].bytes + 2) == 5,
              "weapon record itemtype written");
        CHECK(rd16(set.pools[6].bytes + 2) == 3,
              "cloth record itemtype written");

        /* Party-cell direction rule: (party_dir + RANDBIT) & 3. */
        {
            DM2_V1_RecordPoolSet set2;
            DM2_V1_DropRng rng2;
            DM2_V1_DropPlacedItem one[1];
            int16_t ground2 = DM2_V1_RECORD_HANDLE_END;
            int it2;
            memset(&set2, 0, sizeof(set2));
            add_pool(&set2, 5, 4, 2, 2);
            set2.valid = 1;
            dm2_v1_drops_rng_init(&rng2);
            ref = 0;
            (void)ref_rand(&ref); /* extra roll (slot word reused) */
            draw = ref_rand(&ref); /* RANDBIT */
            placed = dm2_v1_drops_place_source_slots(
                &set2, slot_words, &rng2,
                3, 4, 1,   /* party on the drop cell, dir 1 */
                3, 4,
                &ground2, one, 1, &it2);
            CHECK(placed >= 1 && one[0].at_party_cell,
                  "party cell flagged");
            CHECK(one[0].direction_rand == (int)(draw & 1u),
                  "party-cell draw is RANDBIT");
            CHECK(one[0].direction == (int)((1u + (draw & 1u)) & 3u),
                  "party-cell direction is (party_dir + RANDBIT) & 3");
            dm2_v1_record_pool_set_free(&set2);
        }

        /* Alloc exhaustion breaks the slot loop WITHOUT a direction
         * draw (c_record.cpp:1603-1608). */
        {
            DM2_V1_RecordPoolSet set3;
            DM2_V1_DropRng rng3;
            DM2_V1_DropPlacedItem ex[4];
            int16_t ground3 = DM2_V1_RECORD_HANDLE_END;
            int it3;
            uint32_t ref3;
            memset(&set3, 0, sizeof(set3));
            add_pool(&set3, 5, 4, 1, 1); /* a single weapon record */
            set3.valid = 1;
            dm2_v1_drops_rng_init(&rng3);
            placed = dm2_v1_drops_place_source_slots(
                &set3, slot_words, &rng3,
                9, 9, 2,
                3, 4,
                &ground3, ex, 4, &it3);
            ref3 = 0;
            (void)ref_rand(&ref3); /* extra roll */
            (void)ref_rand(&ref3); /* one RANDDIR for the placed item */
            CHECK(rng3.random == ref3,
                  "no direction draw consumed on the failed alloc");
            CHECK(ex[0].placed == 1, "first item placed");
            CHECK(ex[1].alloc_failed == 1 &&
                      ex[1].record == DM2_V1_RECORD_HANDLE_NULL &&
                      ex[1].direction_rand == -1,
                  "OBJECT_NULL breaks the slot loop before the draw");
            CHECK(placed == 1, "only the allocated item placed");
            /* Slot 0 broke, but slot 1 still runs (source continues the
             * outer slot loop). */
            CHECK(it3 == 3, "slot loop continues after the break");
            CHECK(ex[2].slot_field == 0x0B && ex[2].placed == 0,
                  "next slot cannot alloc from an empty cloth pool here");
            dm2_v1_record_pool_set_free(&set3);
        }

        dm2_v1_record_pool_set_free(&set);
    }

    CHECK(strstr(dm2_v1_dbitem_alloc_source_evidence(),
                 "c_record.cpp") != NULL,
          "source evidence cites c_record.cpp");

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_dbitem_alloc_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_dbitem_alloc_pc34_compat: all checks passed\n");
    return 0;
}
