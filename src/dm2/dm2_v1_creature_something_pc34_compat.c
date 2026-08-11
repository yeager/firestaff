/*
 * dm2_v1_creature_something_pc34_compat.c — DM2_GET_CREATURE_ANIMATION_FRAME
 * (+ DM2_4FCC) and DM2_CREATURE_SOMETHING_1c9a_0a48 bounded slices.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp:3217-3278  GET_CREATURE_ANIMATION_FRAME
 *   skproject/SKULLWIN/c_creature.cpp:3285-3378  DM2_4FCC frame walk
 *   skproject/SKULLWIN/c_1c9a.cpp:5434-5672      CREATURE_SOMETHING_1c9a_0a48
 *   skproject/SKULLWIN/c_ai.cpp:5867-5869        s350.v1e0552/v1e055e owners
 *   skproject/SKULLWIN/c_ai.cpp:5614             the mticks-delta consumer
 *   skproject/SKULLWIN/c_random.cpp:13-47        session LCG draw semantics
 *   skproject/SKULLWIN/mdata.c:1564-1613         table1d607e
 */

#include "dm2_v1_creature_something_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* table1d607e, bound verbatim from skproject/SKULLWIN/mdata.c:1564-1613
 * (struct s_fourb[0x2f] — 4 bytes per entry).  Per-module source-locked
 * copy: the CAII module binds the same table for its own consumers, and
 * keeping each table with its consumer preserves the bounded-slice link
 * boundary. */
static const uint8_t dm2_v1_table1d607e[0x2f][4] = {
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x01, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x80, 0x01, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x20, 0x00, 0x00, 0x00 },
  { 0x20, 0x40, 0x00, 0x00 }, { 0x8c, 0x00, 0x01, 0x00 },
  { 0x84, 0x20, 0x01, 0x00 }, { 0x8c, 0x00, 0x01, 0x00 },
  { 0x8c, 0x00, 0x01, 0x00 }, { 0xa4, 0x00, 0x00, 0x00 },
  { 0x84, 0x00, 0x01, 0x00 }, { 0x01, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x01, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x02, 0x00, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
  { 0x80, 0x00, 0x01, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x00, 0x00 },
  { 0x00, 0x40, 0x00, 0x00 }, { 0xa0, 0x00, 0x00, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x02, 0x00, 0x00, 0x00 },
  { 0x20, 0x01, 0x00, 0x00 }, { 0x00, 0x11, 0x00, 0x00 },
  { 0x01, 0x40, 0x00, 0x00 }, { 0x60, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00 }, { 0x1b, 0x8a, 0x00, 0x00 },
  { 0x01, 0x42, 0x00, 0x00 }, { 0x02, 0x42, 0x00, 0x00 },
  { 0x00, 0x42, 0x00, 0x00 }, { 0x80, 0x40, 0x01, 0x00 },
  { 0x80, 0x00, 0x00, 0x00 }, { 0xe8, 0x00, 0x00, 0x00 },
  { 0x0a, 0x04, 0x00, 0x00 }, { 0x84, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x00, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x20, 0x00, 0x00, 0x00 },
  { 0x80, 0x40, 0x01, 0x00 }
};

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t rd16_record(const DM2_V1_RecordPoolSet *pool_set,
                            const uint8_t *p)
{
    if (pool_set && pool_set->words_big_endian)
        return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
    return rd16(p);
}

/* DM2_4FCC frame walk (skproject/SKULLWIN/c_creature.cpp:3285-3378) over
 * an already loaded dtRaw7/0xfc info table — shared by GAF's dynamic
 * tail (c_creature.cpp:3278) and the exported standalone
 * dm2_v1_creature_anim_4fcc (the c_ai.cpp:5388/5599 call-site shape).
 * Returns the source RG1L (0/1), or -1 fail-closed (table_oob set, or
 * an unbound RNG draw with rc left invalid). */
static int dm2_v1_anim_4fcc_walk(
    const uint8_t *info,
    size_t info_size,
    uint16_t base,
    DM2_V1_DropRng *rng,
    uint32_t frame0,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    DM2_V1_CreatureAnimFrameReceipt *rc,
    int *draws_io)
{
    uint32_t frame = frame0;
    int ret = 0;
    int skip00004 = 0;

    if (frame != 0xffffu) {
        /* c_creature.cpp:3299-3310 — the pre-advance. */
        size_t idx = (size_t)(frame & 0xffffu) + base;
        uint8_t hi;
        if (idx * 4u + 1u >= info_size) {
            rc->table_oob = 1;
            return -1;
        }
        hi = (uint8_t)((info[idx * 4u + 1u] & 0xf0u) >> 4);
        if (hi == 0u) {
            ret = 0;
            skip00004 = 1;
            rc->sequence_end = 1;
        } else {
            frame += hi;
        }
    } else {
        frame = 0u;
    }
    if (!skip00004) {
        /* c_creature.cpp:3325-3368 — the skip-probability walk. */
        for (;;) {
            size_t idx = (size_t)(frame & 0xffffu) + base;
            uint8_t hi;
            uint8_t lo;
            if (idx * 4u + 1u >= info_size) {
                rc->table_oob = 1;
                return -1;
            }
            hi = (uint8_t)((info[idx * 4u + 1u] & 0xf0u) >> 4);
            if (hi == 0u) {
                ret = 0;
                rc->sequence_end = 1;
                break;
            }
            lo = (uint8_t)(info[idx * 4u + 1u] & 0x0fu);
            if (lo != 0x0fu) {
                uint32_t draw;
                if (!rng) {
                    return -1; /* unbound draw: fail closed */
                }
                draw = dm2_v1_drops_rand24(rng) & 0x0fu;
                ++(*draws_io);
                if ((uint32_t)lo < draw) {
                    ++frame;
                    continue;
                }
            }
            {
                uint8_t b3 = info[idx * 4u + 3u];
                uint32_t sum =
                    (uint32_t)((b3 & 0xf0u) >> 4) +
                    (uint32_t)((b3 & 0x0cu) >> 2);
                ret = (sum != 0u) ? 1 : 0;
            }
            break;
        }
    }
    /* c_creature.cpp:3370-3373 — frame word + row pointer store. */
    {
        size_t idx = (size_t)(frame & 0xffffu) + base;
        if (idx * 4u + 3u >= info_size) {
            rc->table_oob = 1;
            return -1;
        }
        *io_frame_word = (int16_t)(frame & 0xffffu);
        *out_anim_row = info + idx * 4u;
        rc->anim_row_set = 1;
    }
    return ret;
}

int dm2_v1_creature_anim_4fcc(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int creature_type,
    uint16_t adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    DM2_V1_CreatureAnimFrameReceipt *receipt)
{
    const uint8_t *info;
    size_t info_size = 0u;
    int draws = 0;
    int walk;
    DM2_V1_CreatureAnimFrameReceipt rc;

    memset(&rc, 0, sizeof(rc));
    rc.creature_type = creature_type;
    rc.command = -1; /* 4FCC takes no command — the base is the caller's */
    rc.frame_word = -1;
    snprintf(rc.source_evidence, sizeof(rc.source_evidence),
             "c_creature.cpp:3285-3378 DM2_4FCC standalone; dtRaw7/0xfc "
             "info; call sites c_ai.cpp:5388 + c_ai.cpp:5599 (50CB twin)");

    if (!loader || !io_frame_word || !out_anim_row ||
        creature_type < 0 || creature_type > 0xff) {
        if (receipt) *receipt = rc;
        return 0;
    }
    info = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &info_size);
    if (!info || info_size < 4u) {
        rc.gdat_missing = 1;
        if (receipt) *receipt = rc;
        return 0;
    }
    walk = dm2_v1_anim_4fcc_walk(info, info_size, adj_base, rng,
                                 (uint32_t)(uint16_t)*io_frame_word,
                                 io_frame_word, out_anim_row, &rc, &draws);
    if (walk < 0) {
        if (receipt) *receipt = rc;
        return 0;
    }
    rc.frame_word = *io_frame_word;
    rc.rand_draws = draws;
    rc.valid = 1;
    rc.return_value = walk;
    if (receipt) *receipt = rc;
    return walk;
}

int dm2_v1_creature_get_animation_frame_with_ai_spec(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    const DM2_AIDefinition *ai_spec,
    int creature_type,
    int command,
    uint16_t *io_adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    int32_t argl1,
    DM2_V1_CreatureAnimFrameReceipt *receipt)
{
    const uint8_t *attribution;
    const uint8_t *info;
    size_t attribution_size = 0u;
    size_t info_size = 0u;
    size_t row;
    uint16_t base;
    int draws = 0;
    int ret = 0;
    DM2_V1_CreatureAnimFrameReceipt rc;

    memset(&rc, 0, sizeof(rc));
    rc.creature_type = creature_type;
    rc.command = command;
    rc.frame_word = -1;
    snprintf(rc.source_evidence, sizeof(rc.source_evidence),
             "c_creature.cpp:3217-3278 GET_CREATURE_ANIMATION_FRAME + "
             "3285-3378 DM2_4FCC; dtRaw8/0xfb attribution, dtRaw7/0xfc "
             "info; aidef gate via dm2_v1_creature_ai_spec_def");

    if (!loader || !io_adj_base || !io_frame_word || !out_anim_row ||
        creature_type < 0 || creature_type > 0xff) {
        if (receipt) *receipt = rc;
        return 0;
    }

    /* c_creature.cpp:3234-3239 — the SPX -1 guard returns 0. */
    attribution = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW8, DM2_GDAT_CREATURE_ANIM_ATTRIBUTION,
        &attribution_size);
    if (!attribution || attribution_size < 4u) {
        rc.valid = 1;
        rc.return_value = 0;
        if (receipt) *receipt = rc;
        return 0;
    }

    /* c_creature.cpp:3242-3248 — scan for the command row / terminator. */
    for (row = 0u; row * 4u + 1u < attribution_size; ++row) {
        uint16_t row_command = rd16(attribution + row * 4u);
        if (row_command == 0xffffu || row_command == (uint16_t)command) {
            break;
        }
    }
    if (row * 4u + 1u >= attribution_size) {
        rc.table_oob = 1; /* no terminator inside the loaded span */
        if (receipt) *receipt = rc;
        return 0;
    }
    rc.attribution_found = 1;
    base = rd16(attribution + row * 4u + 2u);
    rc.attribution_base = base;
    *io_adj_base = base; /* c_creature.cpp:3252 mov16(RG7p, word@2) */

    /* c_creature.cpp:3254-3255 — the info sequence + aidef gate.  The
     * source dereferences both unchecked; the bounded slice fails
     * closed instead. */
    info = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &info_size);
    if (!info || info_size < 4u) {
        rc.gdat_missing = 1;
        if (receipt) *receipt = rc;
        return 0;
    }
    if (!ai_spec && !dm2_v1_creature_ai_spec_def(creature_type, &ai_spec)) {
        rc.aidef_unknown = 1;
        if (receipt) *receipt = rc;
        return 0;
    }
    if (!ai_spec) {
        rc.aidef_unknown = 1;
        if (receipt) *receipt = rc;
        return 0;
    }

    if (ai_spec->w0AIFlags & 0x1u) {
        /* c_creature.cpp:3257-3277 — static path: count rows up to and
         * including the first byte@1-high-nibble-0 row, then encode. */
        uint32_t count = 0u;
        rc.static_path = 1;
        for (;;) {
            size_t idx = (size_t)base + count;
            uint8_t hi;
            if (idx * 4u + 1u >= info_size) {
                rc.table_oob = 1;
                if (receipt) *receipt = rc;
                return 0;
            }
            hi = (uint8_t)((info[idx * 4u + 1u] & 0xf0u) >> 4);
            ++count;
            if (hi == 0u) {
                uint32_t encoded = count;
                if (argl1 == 0) {
                    encoded |= 0x9000u; /* c_creature.cpp:3266 RG4Bhi |= 0x90 */
                } else {
                    /* c_creature.cpp:3269-3273 */
                    encoded |= (((uint32_t)argl1 & 0x3fu) << 6) | 0x8000u;
                }
                *io_frame_word = (int16_t)(encoded & 0xffffu);
                rc.frame_word = *io_frame_word;
                ret = 1;
                break;
            }
        }
        rc.valid = 1;
        rc.return_value = ret;
        if (receipt) *receipt = rc;
        return ret;
    }

    /* c_creature.cpp:3278 + DM2_4FCC (c_creature.cpp:3285-3378). */
    *io_frame_word = -1;
    {
        /* *ecxpw as the source just wrote it: the walk restarts at 0.
         * (The source's 3299-3310 pre-advance is unreachable in this
         * caller flow; it lives verbatim in dm2_v1_anim_4fcc_walk.) */
        int walk = dm2_v1_anim_4fcc_walk(info, info_size, base, rng,
                                         0xffffu, io_frame_word,
                                         out_anim_row, &rc, &draws);
        if (walk < 0) {
            if (receipt) *receipt = rc;
            return 0;
        }
        ret = walk;
    }
    rc.frame_word = *io_frame_word;
    rc.rand_draws = draws;
    rc.valid = 1;
    rc.return_value = ret;
    if (receipt) *receipt = rc;
    return ret;
}

int dm2_v1_creature_get_animation_frame(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int creature_type,
    int command,
    uint16_t *io_adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    int32_t argl1,
    DM2_V1_CreatureAnimFrameReceipt *receipt)
{
    return dm2_v1_creature_get_animation_frame_with_ai_spec(
        loader, rng, NULL, creature_type, command, io_adj_base,
        io_frame_word, out_anim_row, argl1, receipt);
}

int32_t dm2_v1_creature_something_1c9a_0a48_with_ai_spec(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    const DM2_V1_AssetLoader *loader,
    const DM2_AIDefinition *ai_spec,
    DM2_V1_DropRng *rng,
    int16_t record_handle,
    int16_t *adj,
    const uint8_t **anim_row_io,
    int map_current,
    int map_home,
    int32_t v1e0238,
    int savegame_b03,
    int v1e0584,
    int timer_x,
    int timer_y,
    unsigned long game_tick,
    DM2_V1_CreatureSomethingReceipt *receipt)
{
    static const uint8_t zero_row[4] = { 0, 0, 0, 0 };
    uint8_t *rec;
    uint8_t *slot;
    const uint8_t *anim;
    uint16_t vw_04;
    uint16_t vw_08;
    uint32_t rg2;
    int draws = 0;
    int32_t delta;
    int band = DM2_V1_ANIM_DELTA_BAND_PLAIN;
    DM2_V1_CreatureSomethingReceipt rc;

    memset(&rc, 0, sizeof(rc));
    rc.gaf_return = -1;
    rc.frame_byte_before = -1;
    rc.frame_byte_after = -1;
    rc.noise_index = -1;
    snprintf(rc.source_evidence, sizeof(rc.source_evidence),
             "c_1c9a.cpp:5434-5672 CREATURE_SOMETHING_1c9a_0a48; s350 "
             "context per c_ai.cpp:5855-5882; table1d607e mdata.c:1564-1613; "
             "LCG c_random.cpp:13-47; consumer c_ai.cpp:5614");

    if (!pool_set || !pool_set->valid || !caii || !caii->valid || !loader ||
        !adj || !anim_row_io || !receipt) {
        if (receipt) *receipt = rc;
        return 0;
    }
    *receipt = rc;

    if (dm2_v1_record_handle_pool(record_handle) != 4) {
        receipt->not_creature_db = 1;
        return 0;
    }
    rec = dm2_v1_record_pool_address_mut(pool_set, record_handle);
    if (!rec) {
        receipt->not_creature_db = 1;
        return 0;
    }
    receipt->creature_type = rec[4];

    /* record byte@5 == 0xff: s350.creatures is NULL (c_ai.cpp:5857-5866)
     * and the source would dereference it — fail closed. */
    if (rec[5] == 0xffu || (int)rec[5] >= caii->capacity) {
        receipt->no_slot = 1;
        return 0;
    }
    slot = caii->slots + (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;
    receipt->command = (int)(int8_t)slot[0x1a];

    /* s350.v1e0552 (the AIDefinition row, c_ai.cpp:5867) data-backed. */
    if (!ai_spec && !dm2_v1_creature_ai_spec_def(rec[4], &ai_spec)) {
        receipt->aidef_unknown = 1;
        return 0;
    }
    if (!ai_spec) {
        receipt->aidef_unknown = 1;
        return 0;
    }

    /* All remaining source paths draw from the session LCG; require the
     * stream up front so no mutation can precede an unbound draw. */
    if (!rng) {
        receipt->rng_unbound = 1;
        return 0;
    }

    {
        uint8_t vb_10 = ((const uint8_t *)ai_spec)[9]; /* aidef byte@9 */
        vw_04 = (uint16_t)adj[0];
        vw_08 = (uint16_t)adj[1];
        anim = *anim_row_io;

        if (!anim) {
            /* c_1c9a.cpp:5462-5477 — fetch through GAF. */
            int32_t parl01 =
                (ai_spec->w0AIFlags & 0x1u) ?
                    (int32_t)rd16_record(pool_set, rec + 0xc) : 0;
            uint16_t adj_base = vw_04;
            int16_t frame_word = (int16_t)vw_08;
            DM2_V1_CreatureAnimFrameReceipt grc;
            receipt->gaf_return = dm2_v1_creature_get_animation_frame_with_ai_spec(
                loader, rng, ai_spec, rec[4], receipt->command, &adj_base,
                &frame_word, &anim, parl01, &grc);
            receipt->anim_fetched = 1;
            draws += grc.rand_draws;
            if (!grc.valid) {
                receipt->gdat_missing = grc.gdat_missing;
                receipt->table_oob = grc.table_oob;
                receipt->aidef_unknown = grc.aidef_unknown;
                return 0;
            }
            vw_04 = adj_base;
            vw_08 = (uint16_t)frame_word;
            if (!anim) {
                /* c_1c9a.cpp:5478-5481 — the source's own zeroed local. */
                anim = zero_row;
                receipt->anim_fallback = 1;
            }
        }
        receipt->adj_base_after = vw_04;
        receipt->frame_word_after = (int16_t)vw_08;

        /* c_1c9a.cpp:5484-5537 — the frame/direction byte dance.  The
         * source's 32-bit ORs only ever surface RG2Blo, so the dance is
         * computed on the byte domain. */
        rg2 = slot[7];
        receipt->frame_byte_before = slot[7];
        if (anim[3] & 0x1u) {
            int cmd = receipt->command;
            if (cmd != 0x24 && cmd != 0x23 && cmd != 0x25) {
                int n;
                receipt->jitter_applied = 1;
                rg2 &= 0xc0u;
                n = vb_10 & 3;
                if (n != 0) {
                    int r = (int)dm2_v1_drops_rand16(rng, (uint16_t)n);
                    ++draws;
                    if (dm2_v1_drops_randbit(rng)) {
                        ++draws;
                        r = (-r) & 7;
                    } else {
                        ++draws;
                    }
                    rg2 |= (uint32_t)r;
                }
                n = ((int)vb_10 / 4) & 3;
                if (n != 0) {
                    int r = (int)dm2_v1_drops_rand16(rng, (uint16_t)n);
                    ++draws;
                    if (dm2_v1_drops_randbit(rng)) {
                        ++draws;
                        r = (-r) & 7;
                    } else {
                        ++draws;
                    }
                    r *= 8;
                    rg2 |= (uint32_t)r;
                }
            } else {
                receipt->mode_guard = 1;
            }
        }
        if (anim[3] & 0x2u) {
            receipt->rand_bit6_applied = 1;
            ++draws;
            if (dm2_v1_drops_randbit(rng)) {
                rg2 |= 0x40u;
            } else {
                rg2 &= ~0x40u;
            }
        }
        slot[7] = (uint8_t)(rg2 & 0xffu); /* c_1c9a.cpp:5548 */
        receipt->frame_byte_after = slot[7];
        adj[0] = (int16_t)vw_04;          /* c_1c9a.cpp:5549-5550 */
        adj[1] = (int16_t)vw_08;

        /* c_1c9a.cpp:5539-5545 — DM2_QUEUE_NOISE_GEN1 is unproven;
         * receipted, never simulated. */
        if ((anim[0] & 0x7fu) != 0x7fu) {
            receipt->noise_would_queue = 1;
            receipt->noise_index = anim[0] & 0x7fu;
        }
        (void)timer_x;
        (void)timer_y;
    }

    /* c_1c9a.cpp:5547-5667 — the delta arithmetic, verbatim branches. */
    {
        uint16_t flags = rd16_record(pool_set, rec + 0xa);
        if ((flags & 0x40u) == 0u) {
            int32_t d = (int32_t)((anim[3] & 0x0cu) >> 2);
            int skip00533 = 1;
            if (d != 0) {
                d = (int32_t)dm2_v1_drops_rand16(rng, (uint16_t)d);
                ++draws;
            }
            d += (int32_t)((anim[3] & 0xf0u) >> 4);

            if (receipt->command == 0x13 && savegame_b03 != 0) {
                if (((const uint8_t *)ai_spec)[1] & 0x10u) {
                    skip00533 = 1;
                } else {
                    d *= 3;
                    skip00533 = 0;
                    band = DM2_V1_ANIM_DELTA_BAND_DYING_X3;
                }
            }

            if (skip00533) {
                int skip00534 = 1;
                if (map_current == map_home) {
                    skip00534 = 1;
                } else if (v1e0584 < 0 || v1e0584 >= 0x2f) {
                    /* the source indexes table1d607e unchecked */
                    receipt->gdat_w1_out_of_span = 1;
                    return 0;
                } else if (dm2_v1_table1d607e[v1e0584][2] & 0x1u) {
                    skip00534 = 1;
                } else if ((flags & 0x8000u) == 0u) {
                    skip00534 = 1;
                } else if (flags & 0x2u) {
                    skip00534 = 1;
                } else {
                    ++draws;
                    d = d * 4 + (int32_t)dm2_v1_drops_randbit(rng);
                    skip00534 = 0;
                    band = DM2_V1_ANIM_DELTA_BAND_FLEE_X4;
                }

                if (skip00534) {
                    if (v1e0238 == 0) {
                        if (flags & 0x8u) {
                            if ((flags & 0x4000u) != 0u && d < 3) {
                                /* skip00536: RG1 still holds RG4 */
                                d = d > 1 ? d : 1;
                                band = DM2_V1_ANIM_DELTA_BAND_SMALL;
                            } else {
                                /* c_1c9a.cpp:5629-5637 — 75x/100; the
                                 * source's modulo result is dead, the
                                 * quotient is kept. */
                                d = (75 * d) / 100;
                                d = d > 1 ? d : 1;
                                band = DM2_V1_ANIM_DELTA_BAND_75PCT;
                            }
                        }
                    } else {
                        d = 2 * d;
                        if (map_current != map_home) {
                            d = 2 * d;
                        }
                        band = DM2_V1_ANIM_DELTA_BAND_MAP_X2;
                    }
                }
            }
            delta = (int16_t)d; /* c_1c9a.cpp:5666 signedlong(RG4W) */
        } else {
            int32_t hi = (int32_t)((anim[3] & 0xf0u) >> 4);
            delta = hi < 1 ? hi : 1; /* DM2_MIN(1, RG4W) */
            band = DM2_V1_ANIM_DELTA_BAND_BIG_MIN;
        }
    }

    /* c_1c9a.cpp:5668-5670 — the fallback row resets the pointer. */
    *anim_row_io = (anim == zero_row) ? NULL : anim;

    receipt->delta = delta;
    receipt->delta_band = band;
    receipt->rand_draws = draws;
    receipt->result = delta + (int32_t)game_tick;
    receipt->valid = 1;
    return receipt->result;
}

int32_t dm2_v1_creature_something_1c9a_0a48(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int16_t record_handle,
    int16_t *adj,
    const uint8_t **anim_row_io,
    int map_current,
    int map_home,
    int32_t v1e0238,
    int savegame_b03,
    int v1e0584,
    int timer_x,
    int timer_y,
    unsigned long game_tick,
    DM2_V1_CreatureSomethingReceipt *receipt)
{
    return dm2_v1_creature_something_1c9a_0a48_with_ai_spec(
        pool_set, caii, loader, NULL, rng, record_handle, adj,
        anim_row_io, map_current, map_home, v1e0238, savegame_b03,
        v1e0584, timer_x, timer_y, game_tick, receipt);
}
