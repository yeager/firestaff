/*
 * dm2_v1_ccm_loop_pc34_compat.c — the CCM stream owner/grammar
 * DM2_13e4_0982 bounded slice.  See the header for the full contract.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_ai.cpp:5341-5647   DM2_13e4_0982 (the loop)
 *   skproject/SKULLWIN/c_ai.cpp:2340-2430   DM2_13e4_01a3 (per-loop init)
 *   skproject/SKULLWIN/c_ai.cpp:2418-2460   DM2_14cd_062e (byte@0x12 head)
 *   skproject/SKULLWIN/c_ai.cpp:5275-5338   DM2_50CB (deterministic step)
 *   skproject/SKULLWIN/c_ai.cpp:5608-5614   m_15785 end (round-13 owner)
 *   skproject/SKULLWIN/c_ai.cpp:5642-5647   m_15843 tail
 *   skproject/SKULLWIN/c_timer.h:66-68      setmticks / adddata
 *   skproject/SKULLWIN/c_random.cpp:13-47   session LCG
 *   skproject/SKULLWIN/mdata.c:1564-1613    table1d607e
 */

#include "dm2_v1_ccm_loop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature.h"

/* table1d607e, bound verbatim from skproject/SKULLWIN/mdata.c:1564-1613
 * (struct s_fourb[0x2f] — 4 bytes per entry).  Per-module source-locked
 * copy, same rationale as the CAII and creature_something modules:
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

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

/* RAND16(n) with the source's CUTX16-then-modulo semantics
 * (c_random.cpp:23-28) over the session stream — the same per-module
 * form the CAII module documents (drops.h:80-86: identical to
 * dm2_v1_drops_rand16 only when n divides 2^16). */
static uint16_t ccm_rand16(DM2_V1_DropRng *rng, uint16_t n)
{
    uint32_t draw;
    if (n == 0u) {
        return 0u;
    }
    draw = dm2_v1_drops_rand24(rng);
    return (uint16_t)((uint16_t)(draw & 0xffffu) % (uint32_t)n);
}

/* DM2_50CB (c_ai.cpp:5275-5338) — the loop's deterministic animation-row
 * step.  `io_frame_word` is the adj pair's second word (the call site's
 * RG51p = RG61p + 2), `adj_base` its first word, `out_row` the
 * s350.v1e055a store (m_5147 — written on EVERY path).  Returns the
 * source RG1L (0/1/2; 2 continues the loop), or -1 fail-closed with the
 * receipt's gaf_fail_closed set (the source reads the table unchecked).
 */
static int ccm_50cb(const DM2_V1_AssetLoader *loader,
                    int creature_type,
                    uint16_t adj_base,
                    int16_t *io_frame_word,
                    const uint8_t **out_row,
                    DM2_V1_CcmLoopReceipt *rc)
{
    const uint8_t *info;
    size_t info_size = 0u;
    uint32_t frame;
    size_t idx;
    int ret = 0;
    int skip00006 = 0;

    info = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &info_size);
    if (!info || info_size < 4u) {
        rc->gaf_fail_closed = 1; /* gdat_missing inside 50CB */
        return -1;
    }
    frame = (uint32_t)(uint16_t)*io_frame_word;
    if (frame != 0xffffu) {
        /* c_ai.cpp:5296-5312 — the byte@2 & 0x3f pre-add; 0 ends the
         * stream with RG1L = 2 (the loop's continue). */
        idx = (size_t)frame + adj_base;
        if (idx * 4u + 2u >= info_size) {
            rc->gaf_fail_closed = 1; /* table_oob */
            return -1;
        }
        {
            uint16_t add = (uint16_t)(info[idx * 4u + 2u] & 0x3fu);
            if (add != 0u) {
                *io_frame_word = (int16_t)(uint16_t)((uint16_t)*io_frame_word + add);
            } else {
                ret = 2;
                skip00006 = 1;
            }
        }
    } else {
        *io_frame_word = 0;
    }
    if (!skip00006) {
        /* c_ai.cpp:5316-5331 — the byte@1 high-nibble selector. */
        idx = (size_t)(uint16_t)*io_frame_word + adj_base;
        if (idx * 4u + 1u >= info_size) {
            rc->gaf_fail_closed = 1; /* table_oob */
            return -1;
        }
        ret = ((info[idx * 4u + 1u] & 0xf0u) >> 4) != 0u ? 1 : 0;
    }
    /* c_ai.cpp:5333-5337 — the row pointer store (every path). */
    idx = (size_t)(uint16_t)*io_frame_word + adj_base;
    if (idx * 4u + 3u >= info_size) {
        rc->gaf_fail_closed = 1; /* table_oob */
        return -1;
    }
    *out_row = info + idx * 4u;
    return ret;
}

/* The m_15843 tail (c_ai.cpp:5642-5647) composed from the bound
 * primitives: cancel a pending timer through dm2_v1_caii_delete_timer,
 * re-queue the timer AS-IS over the session queue, store the ticket in
 * slot word@2.  Used by the non-loop exits (payload skip, 0x32..0x34);
 * the loop exit hands the same tail to the round-13 end_requeue. */
static int ccm_requeue_tail(DM2_V1_RecordPoolSet *pool_set,
                            DM2_V1_CaiiArray *caii,
                            DM2_V1_SourceTimerQueue *queue,
                            int16_t record_handle,
                            const DM2_V1_SourceTimer *timer,
                            uint16_t source_index,
                            uint8_t *slot,
                            DM2_V1_CcmLoopReceipt *rc)
{
    uint32_t ticket;
    DM2_V1_SourceTimerResult enq_result;

    if (rd16(slot + 2) != 0xffffu) {
        DM2_V1_CaiiDeleteTimerReceipt del;
        memset(&del, 0, sizeof(del));
        if (dm2_v1_caii_delete_timer(pool_set, caii, queue, record_handle,
                                     &del) == 1) {
            rc->tail_cancelled = 1;
        }
    }
    ticket = dm2_v1_source_timer_enqueue_ticketed(queue, timer,
                                                  source_index,
                                                  &enq_result);
    if (ticket == 0u) {
        return 0; /* queue full — fail closed */
    }
    wr16(slot + 2, (uint16_t)ticket); /* c_ai.cpp:5646 mov16 */
    rc->tail_enqueued = 1;
    rc->tail_ticket = ticket;
    return 1;
}

int dm2_v1_ccm_message_loop(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int16_t record_handle,
    DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    int16_t *adj,
    const uint8_t **anim_row_io,
    int *v1e0584_io,
    int suppress_requeue,
    int16_t mticks_map,
    int savegame_b03,
    int map_current,
    int map_home,
    int32_t v1e0238,
    unsigned long game_tick,
    DM2_V1_CcmAiGoalCallback ai_goal_cb,
    void *ai_goal_ctx,
    DM2_V1_CcmSevenCallback seven_cb,
    void *seven_ctx,
    DM2_V1_CcmProceedCallback proceed_ccm,
    void *proceed_ctx,
    DM2_V1_CcmLoopReceipt *receipt)
{
    DM2_V1_CcmLoopReceipt rc;
    uint8_t *rec;
    uint8_t *slot;
    const DM2_AIDefinition *def;
    int creature_type;
    int flag = 0;
    int b1a;
    int v1e07eb = 0;

    memset(&rc, 0, sizeof(rc));
    rc.gaf_return = -1;
    rc.walk_50cb_last = -1;
    rc.dir_written = -1;
    rc.cloud_id = -1;
    rc.loop_result = -1;
    rc.mticks_delta = -1;
    rc.ccm_handler = -1;
    rc.ccm_handler_group = -1;
    snprintf(rc.source_evidence, sizeof(rc.source_evidence),
             "c_ai.cpp:5341-5647 DM2_13e4_0982 CCM message loop; 50CB "
             "c_ai.cpp:5275-5338; 01a3 c_ai.cpp:2340-2430; dispatch "
             "matrix DM2-005; end requeue round-13; LCG c_random.cpp");

    if (!pool_set || !pool_set->valid || !caii || !caii->valid ||
        !queue || !loader || !timer || !adj || !anim_row_io ||
        !v1e0584_io || !receipt) {
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
    if (rec[5] == 0xffu || (int)rec[5] >= caii->capacity) {
        receipt->no_slot = 1;
        return 0;
    }
    slot = caii->slots + (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;
    creature_type = rec[4];

    /* s350.v1e0552 (the AIDefinition row) data-backed. */
    if (!dm2_v1_creature_ai_spec_def(creature_type, &def) || !def) {
        receipt->aidef_unknown = 1;
        return 0;
    }

    /* ── pre-check (c_ai.cpp:5346-5383) ─────────────────────────── */
    {
        int enter;
        if (savegame_b03 == 0) {
            enter = 1;
        } else if ((def->w0AIFlags & 0x1000u) != 0u) {
            enter = 1; /* aidef byte@1 & 0x10 (jnz_test8) */
        } else if ((int)(int8_t)slot[0x1a] == 0x13 ||
                   (int)(int8_t)slot[0x17] == 0x13) {
            enter = 1;
        } else {
            enter = 0;
        }
        if (!enter) {
            receipt->payload_skip = 1;
            /* c_timer.h:68 adddata(4): the payload long eats +4; the
             * body, the loop and the m_15785 end are ALL skipped; the
             * timer still reaches the m_15843 tail. */
            timer->ticks_and_map += 4u;
            if (!ccm_requeue_tail(pool_set, caii, queue, record_handle,
                                  timer, source_index, slot, receipt)) {
                return 0;
            }
            receipt->valid = 1;
            return 1;
        }
    }
    receipt->body_entered = 1;

    /* ── the flag branches (c_ai.cpp:5385-5479) ─────────────────── */
    if (timer->type != 0x22u) {
        /* !flag (c_ai.cpp:5386-5390): the bound standalone DM2_4FCC. */
        DM2_V1_CreatureAnimFrameReceipt frc;
        int16_t frame_word = adj[1];
        memset(&frc, 0, sizeof(frc));
        receipt->gaf_return = dm2_v1_creature_anim_4fcc(
            loader, rng, creature_type, (uint16_t)adj[0], &frame_word,
            anim_row_io, &frc);
        if (!frc.valid) {
            receipt->gaf_fail_closed = 1;
            return 0;
        }
        adj[1] = frame_word;
    } else {
        flag = 1;
        receipt->type_0x22 = 1;
        /* c_ai.cpp:5397-5401: b_1a = b_17. */
        slot[0x1a] = slot[0x17];
        if ((int)(int8_t)slot[0x17] == -1) {
            if (ai_goal_cb) {
                int goal_r = ai_goal_cb(ai_goal_ctx, slot,
                                        creature_type, game_tick);
                if (!goal_r) {
                    receipt->ai_goal_unbound = 1;
                    receipt->command = (int)(int8_t)slot[0x1a];
                    return 0;
                }
            } else {
                receipt->ai_goal_unbound = 1;
                receipt->command = (int)(int8_t)slot[0x1a];
                return 0;
            }
        }
        slot[0x17] = 0xff; /* c_ai.cpp:5406 */
        /* DM2_14cd_062e (c_ai.cpp:5412): the byte@0x12 == 0xff head is
         * bound (c_ai.cpp:2442-2445) and always returns 0, so the
         * source's skip00262 path runs; the cfg reset (b_1a = -1,
         * receipted cfg_reset) needs the table1d5f82 s_seven chain
         * (c_ai.cpp:2447-2459), which stays unproven — fail closed. */
        if (slot[0x12] != 0xffu) {
            if (seven_cb) {
                int seven_r = seven_cb(seven_ctx, slot, creature_type);
                if (seven_r == 0) {
                    slot[0x1a] = 0xff; /* cfg reset: b_1a = -1 */
                    receipt->cfg_reset = 1;
                }
            } else {
                receipt->s_seven_unbound = 1;
                return 0;
            }
        }
        {
            int timing_proven = 0;
            uint8_t timing;
            b1a = (int)(int8_t)slot[0x1a];
            /* c_ai.cpp:5432: table1d613a[b_1a] & 4.  b_1a is non-negative
             * here (the -1 case went to 14cd_09e2 and the cfg reset is
             * unreachable with the bound 062e head); the source's span
             * is table1d613a's 86 bytes. */
            timing = dm2_v1_ccm_dispatch_timing_flags((uint8_t)b1a,
                                                      &timing_proven);
            if (!timing_proven) {
                receipt->command_out_of_span = 1;
                return 0;
            }
            if ((timing & 0x04u) != 0u) {
                receipt->bitmap_state_reset = 1; /* v1e07d8 receipted */
            }
            /* c_ai.cpp:5437-5478: the mode 6/7 facing write. */
            if (b1a >= 6) {
                int dir = -1;
                uint32_t facing =
                    ((uint32_t)rd16(rec + 0xe) << 6) >> 14;
                if (b1a == 6) {
                    dir = (int)((uint16_t)facing) - 1;
                } else if (b1a == 7) {
                    dir = (int)((uint16_t)facing) + 1;
                }
                if (dir >= 0) {
                    dir &= 3;
                    slot[0x1d] = (uint8_t)dir;
                    receipt->dir_written = dir;
                }
            }
        }
        b1a = (int)(int8_t)slot[0x1a];
        receipt->command = b1a;

    /* ── the dying branch (c_ai.cpp:5481-5537) ──────────────────── */
    if (b1a == 0x13) {
        uint16_t w1 = 0u;
        receipt->dying_branch = 1;
        wr16(slot + 0xe, (uint16_t)adj[0]);
        wr16(slot + 0x10, (uint16_t)adj[1]);
        /* The source queries GDAT word@1 directly (NOT via v1e0584)
         * and indexes table1d607e unchecked — fail closed outside the
         * proven 0x2f span or when the session has no word@1. */
        if (!dm2_v1_creature_gdat_word1(creature_type, &w1) ||
            w1 >= 0x2fu) {
            receipt->gdat_w1_out_of_span = 1;
            return 0;
        }
        if ((dm2_v1_table1d607e[w1][0] & 0x08u) == 0u) {
            int cloud_id = 0x6e;
            if (def->b35 == 1u) {
                cloud_id = 0xbe;
            } else if (def->b35 == 2u) {
                cloud_id = 0xff;
            }
            /* DM2_CREATE_CLOUD(0xffa8, id, xA, yA, 0xff) — the cloud
             * system is unproven: receipted, never simulated. */
            receipt->cloud_would_create = 1;
            receipt->cloud_id = cloud_id;
        }
    }

    /* ── the 0x32..0x34 special (c_ai.cpp:5539-5553) ────────────── */
    if (b1a >= 0x32 && b1a <= 0x34) {
        receipt->special_mticks = 1;
        /* setmticks(v1e0571, b_1a + gametick - 50): the low word ORs
         * unmasked exactly like c_timer.h:66.  NO animation fetch, NO
         * loop, NO m_15785 end — but the m_15843 tail still runs. */
        timer->ticks_and_map =
            ((uint32_t)(int32_t)mticks_map << 24) |
            (uint32_t)(int32_t)(b1a + (int32_t)game_tick - 50);
        if (!ccm_requeue_tail(pool_set, caii, queue, record_handle,
                              timer, source_index, slot, receipt)) {
            return 0;
        }
        receipt->valid = 1;
        return 1;
    }

    /* ── the animation fetch (c_ai.cpp:5555-5563) ───────────────── */
    {
        DM2_V1_CreatureAnimFrameReceipt grc;
        uint16_t adj_base = (uint16_t)adj[0];
        int16_t frame_word = adj[1];
        memset(&grc, 0, sizeof(grc));
        receipt->gaf_return = dm2_v1_creature_get_animation_frame(
            loader, rng, creature_type, b1a, &adj_base, &frame_word,
            anim_row_io, 0, &grc);
        if (!grc.valid) {
            receipt->gaf_fail_closed = 1;
            return 0;
        }
        adj[0] = (int16_t)adj_base;
        adj[1] = frame_word;
    }
    }
    /* c_ai.cpp:5386-5390 + 5565: BOTH branches reach the loop with
     * skip00269 set — !flag straight from 4FCC, flag through the
     * dance above.  The !flag path's command is the slot's untouched
     * byte@0x1a. */
    if (!flag) {
        receipt->command = (int)(int8_t)slot[0x1a];
    }
    receipt->loop_entered = 1;
    receipt->loop_result = receipt->gaf_return; /* m_156E8 RG4L = RG1L */

    /* ── the message loop (c_ai.cpp:5567-5606) ──────────────────── */
    for (;;) {
        const uint8_t *row = *anim_row_io;
        int rg1 = 1;

        if (!row) {
            /* The source dereferences v1e055a unchecked (the static
             * GAF path leaves it NULL) — fail closed. */
            receipt->anim_row_null = 1;
            return 0;
        }
        if (!flag && slot[0x21] != 0u && (row[2] & 0x40u) != 0u) {
            rg1 = 0; /* skip00271 */
        }
        if (rg1 != 0 && (row[2] & 0x80u) != 0u) {
            /* DM2_13e4_01a3's provable slice, bound in source order. */
            if (!v1e07eb) {
                uint8_t diff;
                int rg3w;
                int rg2w;
                int rg4w;
                v1e07eb = 1;
                ++receipt->init_01a3_runs;
                if (*v1e0584_io == -1) {
                    uint16_t w1 = 0u;
                    if (!dm2_v1_creature_gdat_word1(creature_type,
                                                    &w1)) {
                        receipt->gdat_w1_out_of_span = 1;
                        return 0;
                    }
                    *v1e0584_io = (int)w1;
                    receipt->v1e0584_set = 1;
                }
                /* The aidef word copies (v1e0576/78/7a/7c/7e), the
                 * DM2_query_1c9a_08bd mask and the v1e0582 GDAT word@7
                 * feed only the host-owned handlers — receipted. */
                receipt->query_08bd_unbound = 1;
                if (!rng) {
                    receipt->rng_unbound = 1;
                    return 0;
                }
                diff = (uint8_t)((uint8_t)game_tick - slot[4]);
                rg3w = (int)(int8_t)diff;
                if (rg3w < 0) {
                    rg3w += 0x100;
                }
                rg2w = 2 * (0xf - (int)(def->w16 & 0xfu));
                rg4w = rg2w / 4; /* 0x1837a7 != 0: signed div */
                ++rg2w;
                rg4w += (int)ccm_rand16(rng, (uint16_t)rg2w);
                receipt->v1e058d = (rg4w <= rg3w) ? 1 : 0;
            }
            /* DM2_PROCEED_CCM — receipted through the proven DM2-005
             * dispatch matrix.  When a callback is provided, dispatch
             * through it; otherwise fail closed. */
            receipt->ccm_handler = (int)(int8_t)slot[0x1a];
            receipt->ccm_handler_group =
                (int)dm2_v1_ccm_dispatch_source_group(slot[0x1a]);
            if (proceed_ccm) {
                int32_t hr = proceed_ccm(proceed_ctx, slot[0x1a],
                    creature_type, slot, (uint16_t)adj[0],
                    (uint16_t)adj[1], game_tick);
                receipt->ccm_handler_dispatched = 1;
                receipt->ccm_handler_result = (int)hr;
                /* c_creature.cpp:3186-3188: RG3L = RG1L when handler
                 * returned non-zero.  c_creature.cpp:3191-3208: when
                 * RG3W == 0, write gametick to slot[4] if table1d613a
                 * entry & 3 != 0; otherwise set v1e07d8.b_01. */
                if (hr == 0) {
                    uint8_t cmd = slot[0x1a];
                    if (cmd < 0x2fu) {
                        uint8_t tflags = dm2_v1_table1d607e[cmd][0];
                        if ((tflags & 0x03u) != 0) {
                            slot[4] = (uint8_t)(game_tick & 0xffu);
                        }
                    }
                }
                /* c_ai.cpp:5586: or8(slot[0x21], RG1Blo) — the low byte
                 * of the handler result is OR'd into slot[0x21]. */
                slot[0x21] |= (uint8_t)(hr & 0xffu);
            } else {
                receipt->ccm_handler_unbound = 1;
                return 0;
            }
        }
        if (slot[0x21] != 0u && (row[2] & 0x40u) != 0u) {
            /* c_ai.cpp:5592-5604: the bound DM2_50CB stream step.
             * Result 2 (no duration left on the row) BREAKS the loop;
             * results 0/1 CONTINUE it (c_ai.cpp:5602-5604). */
            int step = ccm_50cb(loader, creature_type, (uint16_t)adj[0],
                                &adj[1], anim_row_io, receipt);
            if (step < 0) {
                return 0;
            }
            ++receipt->walk_50cb_calls;
            receipt->walk_50cb_last = step;
            receipt->loop_result = step;
            ++receipt->loop_iterations;
            if (step != 2) {
                continue;
            }
        }
        break;
    }

    /* ── the m_15785 end (c_ai.cpp:5608-5614) ───────────────────── */
    {
        int32_t delta = 0;
        DM2_V1_CaiiCcmEndRequeueReceipt erc;

        if (suppress_requeue == 0) {
            /* The suppression return (c_ai.cpp:5612-5613) precedes the
             * delta computation in the source — the bound 1c9a_0a48
             * (and its RNG draws) only runs when NOT suppressed. */
            DM2_V1_CreatureSomethingReceipt src;
            memset(&src, 0, sizeof(src));
            delta = dm2_v1_creature_something_1c9a_0a48(
                pool_set, caii, loader, rng, record_handle, adj,
                anim_row_io, map_current, map_home, v1e0238,
                savegame_b03, *v1e0584_io,
                (int)(int8_t)(timer->value_a & 0xff),
                (int)(int8_t)(((uint16_t)timer->value_a >> 8) & 0xff),
                game_tick, &src);
            if (!src.valid) {
                receipt->something_fail_closed = 1;
                return 0;
            }
            receipt->mticks_delta = (int)delta;
        }
        /* The round-13 end_requeue owns the type rebuild
         * (c_ai.cpp:5609-5611), the suppression receipt, the setmticks
         * (c_ai.cpp:5614) and the m_15843 cancel/queue/store
         * (c_ai.cpp:5642-5647).  The m_157BC bitmap block
         * (c_ai.cpp:5618-5640) stays unbound: its v1e07eb guard needs
         * a completed 13e4_01a3, and this slice fails closed at the
         * handler boundary first — receipted, structurally 0. */
        memset(&erc, 0, sizeof(erc));
        dm2_v1_caii_ccm_end_requeue(pool_set, caii, queue, record_handle,
                                    timer, source_index,
                                    receipt->loop_result,
                                    suppress_requeue, mticks_map, delta,
                                    &erc);
        receipt->requeue_end_completed = erc.completed;
        receipt->requeue_end_suppressed = erc.suppressed;
        if (!erc.completed && !erc.suppressed) {
            return 0; /* fail-closed inside the round-13 slice */
        }
    }

    receipt->valid = 1;
    return 1;
}
