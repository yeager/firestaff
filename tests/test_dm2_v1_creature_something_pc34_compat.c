/*
 * test_dm2_v1_creature_something_pc34_compat.c —
 * DM2_GET_CREATURE_ANIMATION_FRAME (+ DM2_4FCC) and
 * DM2_CREATURE_SOMETHING_1c9a_0a48 bounded slices.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp:3217-3278  GAF
 *   skproject/SKULLWIN/c_creature.cpp:3285-3378  DM2_4FCC
 *   skproject/SKULLWIN/c_1c9a.cpp:5434-5672      1c9a_0a48
 *   skproject/SKULLWIN/c_random.cpp:13-47        session LCG
 *   skproject/SKULLWIN/mdata.c:1564-1613         table1d607e
 *
 * Synthetic dtWordValue/RAW GDAT fixture (no game data); the canonical
 * GRAPHICS.DAT proof lives in
 * test_dm2_v1_creature_something_real_data.c.
 *
 * Fixture map:
 *   type 12 -> AI row 5 -> flags 0x0000 (dynamic), BaseHP 40, jitter 0x05
 *     attribution: [0x05->2] [0x23->2] [0x13->2] [0x07->3] [0xffff->0]
 *     info rows:   2 = {0x05,0x1f,0,0x33} (stop row, noise 5, jitter on)
 *                  3 = {0x7f,0x1f,0,0x00} (stop row, noise none)
 *   type  7 -> AI row 9 -> flags 0x0001 (static), BaseHP 24
 *     attribution: [0x05->1] [0xffff->0]; info row 1 = terminator
 *   type 31: attribution only (info missing -> gdat_missing)
 *   type 32: unterminated attribution (-> table_oob)
 *   type 33: attribution + info, no AI row (-> aidef_unknown)
 */

#include "dm2_v1_creature_something_pc34_compat.h"

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

#define TYPE_DYN 12
#define TYPE_STA 7

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static int16_t rec_handle(int index)
{
    return (int16_t)((4 << 10) | (index & 0x3ff));
}

static void set_word_entry(DM2_V1_GdatEntry *e, int category, int index,
                           int field, uint16_t value)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    e->cls4 = (uint8_t)field;
    e->data_index = value;
}

static void set_raw_entry(DM2_V1_GdatEntry *e, int category, int index,
                          int type, int field, uint16_t raw_index)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)type;
    e->cls4 = (uint8_t)field;
    e->data_index = raw_index;
}

/* raw layout inside data[]:
 *   raw 0 @0    20B attribution type 12
 *   raw 1 @20   20B info type 12
 *   raw 2 @40    8B attribution type 7
 *   raw 3 @48    8B info type 7
 *   raw 4 @56    8B attribution type 31
 *   raw 5 @64    8B attribution type 32 (no terminator)
 *   raw 6 @72    8B attribution type 33
 *   raw 7 @80    8B info type 33
 */
#define RAW_COUNT 8
#define DATA_SIZE 96
#define ENTRY_COUNT 16

static void build_loader(DM2_V1_AssetLoader *loader,
                         DM2_V1_GdatEntry *entries,
                         uint32_t *raw_offsets, uint32_t *raw_sizes,
                         uint8_t *data)
{
    int n = 0;

    memset(data, 0, DATA_SIZE);

    /* type 12 attribution @0 */
    wr16(data + 0, 0x05); wr16(data + 2, 2);
    wr16(data + 4, 0x23); wr16(data + 6, 2);
    wr16(data + 8, 0x13); wr16(data + 10, 2);
    wr16(data + 12, 0x07); wr16(data + 14, 3);
    wr16(data + 16, 0xffff); wr16(data + 18, 0);
    /* type 12 info @20: rows 2 and 3 are the stop rows */
    data[20 + 8] = 0x05; data[20 + 9] = 0x1f; data[20 + 11] = 0x33;
    data[20 + 12] = 0x7f; data[20 + 13] = 0x1f; data[20 + 15] = 0x00;
    /* type 7 attribution @40 / info @48 */
    wr16(data + 40, 0x05); wr16(data + 42, 1);
    wr16(data + 44, 0xffff); wr16(data + 46, 0);
    /* info rows all zero: row 1 byte@1 == 0 terminates the static walk */
    /* type 31 attribution @56 */
    wr16(data + 56, 0xffff); wr16(data + 58, 0);
    /* type 32 attribution @64: neither match nor terminator */
    wr16(data + 64, 0x55); wr16(data + 66, 0);
    wr16(data + 68, 0x66); wr16(data + 70, 0);
    /* type 33 attribution @72 / info @80 */
    wr16(data + 72, 0xffff); wr16(data + 74, 0);

    raw_offsets[0] = 0;  raw_sizes[0] = 20;
    raw_offsets[1] = 20; raw_sizes[1] = 20;
    raw_offsets[2] = 40; raw_sizes[2] = 8;
    raw_offsets[3] = 48; raw_sizes[3] = 8;
    raw_offsets[4] = 56; raw_sizes[4] = 8;
    raw_offsets[5] = 64; raw_sizes[5] = 8;
    raw_offsets[6] = 72; raw_sizes[6] = 8;
    raw_offsets[7] = 80; raw_sizes[7] = 8;

    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                   0x05, 5);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                   0x05, 9);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x00);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 9, 0x05);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 0, 0x01);
    set_word_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 0);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 1);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 2);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 3);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, 31,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 4);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, 32,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 5);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, 33,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 6);
    set_raw_entry(&entries[n++], DM2_GDAT_CATEGORY_CREATURES, 33,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 7);

    memset(loader, 0, sizeof(*loader));
    loader->data = data;
    loader->data_size = DATA_SIZE;
    loader->loaded = 1;
    loader->raw_data_count = RAW_COUNT;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = n;
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    int i;

    memset(set, 0, sizeof(*set));
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 3;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(3, 16);
    for (i = 0; i < 3; i++) {
        wr16(set->pools[4].bytes + i * 16, DM2_V1_RECORD_HANDLE_END);
        set->pools[4].bytes[i * 16 + 5] = 0xFF;
    }
    /* rec 0: dynamic, owns slot 0; rec 1: static, owns slot 1, packed
     * word@0xc = 1155; rec 2: dynamic without a CAII slot */
    set->pools[4].bytes[0 * 16 + 4] = TYPE_DYN;
    set->pools[4].bytes[0 * 16 + 5] = 0;
    set->pools[4].bytes[1 * 16 + 4] = TYPE_STA;
    set->pools[4].bytes[1 * 16 + 5] = 1;
    wr16(set->pools[4].bytes + 1 * 16 + 0xc, 1155);
    set->pools[4].bytes[2 * 16 + 4] = TYPE_DYN;
    set->valid = 1;
}

static uint8_t *rec_bytes(DM2_V1_RecordPoolSet *set, int index)
{
    return set->pools[4].bytes + index * 16;
}

int main(void)
{
    static uint8_t data[DATA_SIZE];
    DM2_V1_GdatEntry entries[ENTRY_COUNT];
    uint32_t raw_offsets[RAW_COUNT];
    uint32_t raw_sizes[RAW_COUNT];
    DM2_V1_AssetLoader loader;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_DropRng rng;
    DM2_V1_CreatureAnimFrameReceipt grc;
    DM2_V1_CreatureSomethingReceipt rc;
    const uint8_t *anim;
    uint16_t adj_base;
    int16_t frame_word;
    int16_t adj[2];
    int32_t result;

    build_loader(&loader, entries, raw_offsets, raw_sizes, data);
    build_pools(&set);
    dm2_v1_caii_array_init(&caii, 3);
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 2,
          "AI table loaded from the fixture");

    /* ── (a) GAF dynamic path, no skip draw (low nibble 0xf) ──────── */
    rng.random = 0;
    adj_base = 0;
    frame_word = -1;
    anim = NULL;
    memset(&grc, 0, sizeof(grc));
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, TYPE_DYN, 0x05, &adj_base, &frame_word, &anim,
              0, &grc) == 1,
          "GAF dynamic returns 1");
    CHECK(grc.valid && grc.attribution_found && !grc.static_path,
          "GAF dynamic receipt");
    CHECK(adj_base == 2 && frame_word == 0 && anim != NULL &&
              grc.anim_row_set && grc.rand_draws == 0,
          "GAF dynamic frame 0 at base 2 without draws");
    CHECK(anim[0] == 0x05 && anim[3] == 0x33, "GAF row pointer identity");

    /* ── (b) GAF static path, argl1 0 and packed coordinates ──────── */
    adj_base = 0;
    frame_word = -1;
    anim = NULL;
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, TYPE_STA, 0x05, &adj_base, &frame_word, &anim,
              0, &grc) == 1,
          "GAF static returns 1");
    CHECK(grc.static_path && adj_base == 1 &&
              (uint16_t)frame_word == 0x9001u && anim == NULL &&
              !grc.anim_row_set,
          "GAF static argl1 0 encodes 0x9000|count");
    frame_word = -1;
    anim = NULL;
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, TYPE_STA, 0x05, &adj_base, &frame_word, &anim,
              1155, &grc) == 1 &&
              (uint16_t)frame_word == (0x8000u | 192u | 1u),
          "GAF static argl1 encodes coordinates");

    /* ── (c) GAF fail-closed paths ────────────────────────────────── */
    frame_word = -1;
    anim = NULL;
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, 30, 0x05, &adj_base, &frame_word, &anim, 0,
              &grc) == 0 &&
              grc.valid && !grc.attribution_found,
          "GAF missing attribution returns 0 (source SPX guard)");
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, 31, 0x05, &adj_base, &frame_word, &anim, 0,
              &grc) == 0 &&
              !grc.valid && grc.gdat_missing,
          "GAF missing info table fails closed");
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, 32, 0x05, &adj_base, &frame_word, &anim, 0,
              &grc) == 0 &&
              !grc.valid && grc.table_oob,
          "GAF unterminated attribution fails closed");
    CHECK(dm2_v1_creature_get_animation_frame(
              &loader, &rng, 33, 0x05, &adj_base, &frame_word, &anim, 0,
              &grc) == 0 &&
              !grc.valid && grc.aidef_unknown,
          "GAF unknown aidef fails closed");

    /* ── (d) 1c9a_0a48 dynamic: jitter + bit6 + noise + base delta ── */
    {
        uint8_t *slot = caii.slots;
        int32_t result2;
        uint8_t after1;
        int draws1;

        slot[7] = 0xff;
        slot[0x1a] = 0x05;
        wr16(rec_bytes(&set, 0) + 0xa, 0x0000);
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 0, -1, 9, 9, 1000ul, &rc);
        CHECK(rc.valid && result == 1003 && rc.result == 1003 &&
                  rc.delta == 3 &&
                  rc.delta_band == DM2_V1_ANIM_DELTA_BAND_PLAIN,
              "0a48 dynamic base delta = row hi nibble");
        CHECK(rc.anim_fetched && rc.gaf_return == 1 && !rc.anim_fallback,
              "0a48 fetched the GDAT row");
        CHECK(rc.jitter_applied && !rc.mode_guard &&
                  rc.rand_bit6_applied,
              "0a48 jitter + bit6 draws");
        CHECK(rc.noise_would_queue && rc.noise_index == 5,
              "0a48 noise request receipted (never simulated)");
        CHECK(rc.frame_byte_before == 0xff &&
                  (rc.frame_byte_after & 0x80) != 0 &&
                  slot[7] == rc.frame_byte_after,
              "0a48 slot byte@7 masked to 0xc0 and rewritten");
        CHECK(adj[0] == 2 && adj[1] == 0 && rc.adj_base_after == 2 &&
                  rc.frame_word_after == 0,
              "0a48 adj pair written back");
        CHECK(anim != NULL && anim[3] == 0x33,
              "0a48 anim row stays bound");
        after1 = slot[7];
        draws1 = rc.rand_draws;
        CHECK(draws1 == 5, "0a48 draw count: 2x(RAND16+RANDBIT) for "
                           "jitter nibbles 1+1, plus the bit6 draw");

        /* determinism: identical seed reproduces the identical state */
        slot[7] = 0xff;
        anim = NULL;
        adj[0] = 0;
        adj[1] = 0;
        rng.random = 0;
        result2 = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 0, -1, 9, 9, 1000ul, &rc);
        CHECK(result2 == result && slot[7] == after1 &&
                  rc.rand_draws == draws1,
              "0a48 deterministic over the session LCG");
    }

    /* ── (e) 1c9a_0a48 static creature: fallback zero row ─────────── */
    {
        uint8_t *slot = caii.slots + DM2_V1_CAII_SLOT_SIZE;

        slot[7] = 0x21;
        slot[0x1a] = 0x05;
        wr16(rec_bytes(&set, 1) + 0xa, 0x0000);
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(1), adj, &anim,
            4, 4, 0, 0, -1, 0, 0, 500ul, &rc);
        CHECK(rc.valid && result == 500 && rc.delta == 0,
              "0a48 static zero-row delta");
        CHECK(rc.anim_fetched && rc.anim_fallback && anim == NULL,
              "0a48 fallback row resets v1e055a to NULL");
        CHECK((uint16_t)rc.frame_word_after == (0x8000u | 192u | 1u) &&
                  adj[1] == (int16_t)(0x8000u | 192u | 1u),
              "0a48 static frame word encodes packed word@0xc");
        CHECK(!rc.jitter_applied && slot[7] == 0x21,
              "0a48 fallback row applies no dance");
        CHECK(rc.noise_would_queue && rc.noise_index == 0,
              "0a48 fallback row noise index 0 (source-faithful)");
    }

    /* ── (f) mode guard 0x23 skips the jitter, keeps bit6 ─────────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x23;
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 0, -1, 0, 0, 100ul, &rc);
        CHECK(rc.valid && rc.mode_guard && !rc.jitter_applied &&
                  rc.rand_bit6_applied,
              "0a48 command 0x23 guard");
        CHECK((slot[7] & 0xbf) == 0x00,
              "0a48 guard leaves all but bit6 untouched");
        CHECK(rc.delta == 3 && result == 103,
              "0a48 guard keeps the base delta");
    }

    /* ── (g) dying mode 0x13 triples the delta ────────────────────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x13;
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 1, -1, 0, 0, 100ul, &rc);
        CHECK(rc.valid && rc.delta == 9 &&
                  rc.delta_band == DM2_V1_ANIM_DELTA_BAND_DYING_X3 &&
                  result == 109,
              "0a48 dying band triples (b03 set, flags hi 0x10 clear)");
    }

    /* ── (h) big-creature word@0xa bit 0x40: min(1, hi) ───────────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x05;
        wr16(rec_bytes(&set, 0) + 0xa, 0x0040);
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 0, -1, 0, 0, 100ul, &rc);
        CHECK(rc.valid && rc.delta == 1 &&
                  rc.delta_band == DM2_V1_ANIM_DELTA_BAND_BIG_MIN &&
                  result == 101,
              "0a48 big-creature min band");
        wr16(rec_bytes(&set, 0) + 0xa, 0x0000);
    }

    /* ── (i) flee band: map != home, table probe clear, 0x8000 ────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x05;
        wr16(rec_bytes(&set, 0) + 0xa, 0x8000);
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 2, 0, 0, 3, 0, 0, 100ul, &rc);
        CHECK(rc.valid &&
                  rc.delta_band == DM2_V1_ANIM_DELTA_BAND_FLEE_X4 &&
                  (rc.delta == 12 || rc.delta == 13) &&
                  result == 100 + rc.delta,
              "0a48 flee band *4 + RANDBIT");
        wr16(rec_bytes(&set, 0) + 0xa, 0x0000);
    }

    /* ── (j) v1e0584 beyond table1d607e fails closed ──────────────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x05;
        wr16(rec_bytes(&set, 0) + 0xa, 0x8000);
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 2, 0, 0, 0x30, 0, 0, 100ul, &rc);
        CHECK(result == 0 && !rc.valid && rc.gdat_w1_out_of_span,
              "0a48 table1d607e span guard (source reads OOB)");
        wr16(rec_bytes(&set, 0) + 0xa, 0x0000);
    }

    /* ── (k) noise suppressed at index 0x7f ────────────────────────── */
    {
        uint8_t *slot = caii.slots;

        slot[7] = 0x00;
        slot[0x1a] = 0x07;
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        rng.random = 0;
        result = dm2_v1_creature_something_1c9a_0a48(
            &set, &caii, &loader, &rng, rec_handle(0), adj, &anim,
            4, 4, 0, 0, -1, 0, 0, 100ul, &rc);
        CHECK(rc.valid && !rc.noise_would_queue && rc.noise_index == -1,
              "0a48 noise index 0x7f queues nothing");
        CHECK(rc.delta == 0 && result == 100,
              "0a48 zero row byte@3 gives zero delta");
    }

    /* ── (l) fail-closed entry paths ───────────────────────────────── */
    {
        adj[0] = 0;
        adj[1] = 0;
        anim = NULL;
        CHECK(dm2_v1_creature_something_1c9a_0a48(
                  &set, &caii, &loader, NULL, rec_handle(0), adj, &anim,
                  4, 4, 0, 0, -1, 0, 0, 100ul, &rc) == 0 &&
                  !rc.valid && rc.rng_unbound,
              "0a48 NULL rng fails closed before any mutation");
        CHECK(dm2_v1_creature_something_1c9a_0a48(
                  &set, &caii, &loader, &rng, rec_handle(2), adj, &anim,
                  4, 4, 0, 0, -1, 0, 0, 100ul, &rc) == 0 &&
                  !rc.valid && rc.no_slot,
              "0a48 record without a CAII slot fails closed");
        CHECK(dm2_v1_creature_something_1c9a_0a48(
                  &set, &caii, &loader, &rng, (int16_t)((2 << 10) | 1),
                  adj, &anim, 4, 4, 0, 0, -1, 0, 0, 100ul, &rc) == 0 &&
                  !rc.valid && rc.not_creature_db,
              "0a48 non-DB4 handle fails closed");
    }

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);
    dm2_v1_creature_reset_ai_table();

    if (g_failures != 0) {
        fprintf(stderr, "FAIL: %d assertion(s) failed\n", g_failures);
        return 1;
    }
    puts("PASS: DM2_GET_CREATURE_ANIMATION_FRAME + "
         "DM2_CREATURE_SOMETHING_1c9a_0a48 bounded slices");
    return 0;
}
