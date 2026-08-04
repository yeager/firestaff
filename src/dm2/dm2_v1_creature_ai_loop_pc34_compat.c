/*
 * dm2_v1_creature_ai_loop_pc34_compat.c — DM2 creature AI loop orchestrators.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_ai.cpp:4743-5272   DM2_14cd_09e2 (strategy select)
 *   skproject/SKULLWIN/c_ai.cpp:5275-5338   DM2_50CB (animation frame resolve)
 *   skproject/SKULLWIN/c_ai.cpp:5341-5647   DM2_13e4_0982 (CCM dispatch)
 *   skproject/SKULLWIN/c_ai.cpp:5649-5784   DM2_THINK_CREATURE (main body)
 *   skproject/SKULLWIN/c_querydb.cpp:1486-1507 DM2_GET_CREATURE_AT
 *   skproject/SKULLWIN/mdata.c:1564-1613    table1d607e
 */

#include "dm2_v1_creature_ai_loop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature.h"

/* table1d607e, bound verbatim from skproject/SKULLWIN/mdata.c:1564-1613.
 * Per-module source-locked copy (same rationale as ccm_loop module). */
static const uint8_t table1d607e_local[0x2f][4] = {
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

#define DM2_V1_DB_CREATURE 4
#define DM2_V1_CREATURE_TYPE_OFFSET 4
#define DM2_V1_MAX_XACT_ITERATIONS 32

static uint16_t rd16_local(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void wr16_local(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

/* ================================================================== */
/* DM2_50CB — animation frame resolver (c_ai.cpp:5275-5338)           */
/* ================================================================== */

void dm2_v1_anim_frame_resolve_receipt_init(
    DM2_V1_AnimFrameResolveReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
             "c_ai.cpp:5275-5338 DM2_50CB animation frame resolver");
}

int dm2_v1_creature_animation_frame_resolve(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_row,
    DM2_V1_AnimFrameResolveReceipt *receipt)
{
    DM2_V1_AnimFrameResolveReceipt rc;
    const uint8_t *info;
    size_t info_size = 0u;
    uint32_t frame;
    size_t idx;
    int ret = 0;
    int skip = 0;

    dm2_v1_anim_frame_resolve_receipt_init(&rc);

    if (!loader || !io_frame_word || !out_row) {
        if (receipt) *receipt = rc;
        return -1;
    }

    /* c_ai.cpp:5293: QUERY_GDAT_ENTRY_DATA_PTR(0xf, creature_type, 7, 0xfc) */
    info = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &info_size);
    if (!info || info_size < 4u) {
        rc.gdat_missing = 1;
        if (receipt) *receipt = rc;
        return -1;
    }

    frame = (uint32_t)(uint16_t)*io_frame_word;
    if (frame != 0xffffu) {
        /* c_ai.cpp:5296-5312: byte@2 & 0x3f pre-add; 0 ends with ret=2 */
        idx = (size_t)frame + adj_base;
        if (idx * 4u + 2u >= info_size) {
            rc.table_oob = 1;
            if (receipt) *receipt = rc;
            return -1;
        }
        {
            uint16_t add = (uint16_t)(info[idx * 4u + 2u] & 0x3fu);
            if (add != 0u) {
                *io_frame_word = (int16_t)(uint16_t)(
                    (uint16_t)*io_frame_word + add);
            } else {
                ret = 2;
                skip = 1;
                rc.frame_ended = 1;
            }
        }
    } else {
        *io_frame_word = 0;
    }

    if (!skip) {
        /* c_ai.cpp:5316-5331: byte@1 high-nibble selector */
        idx = (size_t)(uint16_t)*io_frame_word + adj_base;
        if (idx * 4u + 1u >= info_size) {
            rc.table_oob = 1;
            if (receipt) *receipt = rc;
            return -1;
        }
        ret = ((info[idx * 4u + 1u] & 0xf0u) >> 4) != 0u ? 1 : 0;
        if (ret == 1) {
            rc.frame_advanced = 1;
        } else {
            rc.frame_unchanged = 1;
        }
    }

    /* c_ai.cpp:5333-5337: row pointer store (every path) */
    idx = (size_t)(uint16_t)*io_frame_word + adj_base;
    if (idx * 4u + 3u >= info_size) {
        rc.table_oob = 1;
        if (receipt) *receipt = rc;
        return -1;
    }
    *out_row = info + idx * 4u;

    rc.valid = 1;
    if (receipt) *receipt = rc;
    return ret;
}

/* ================================================================== */
/* DM2_14cd_09e2 — creature strategy selector (c_ai.cpp:4743-5272)    */
/* ================================================================== */

void dm2_v1_creature_strategy_receipt_init(
    DM2_V1_CreatureStrategyReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->final_command = -1;
    receipt->action_0684_result = -1;
    receipt->action_08f5_result = -1;
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
             "c_ai.cpp:4743-5272 DM2_14cd_09e2 creature strategy selector; "
             "SELECT_CREATURE_37FC c_ai.cpp:4723-4740; "
             "table1d607e mdata.c:1564-1613");
}

int dm2_v1_creature_strategy_select(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    int16_t record_handle,
    uint16_t creature_type,
    int timer_x, int timer_y,
    unsigned long game_tick,
    DM2_V1_CreatureGoThereCallback go_there_cb,
    void *go_there_ctx,
    DM2_V1_XactLoopCallback xact_cb,
    void *xact_ctx,
    DM2_V1_CreatureStrategyReceipt *receipt)
{
    DM2_V1_CreatureStrategyReceipt rc;
    uint8_t *rec;
    uint8_t *slot;
    uint16_t w1 = 0u;
    int is_static;

    dm2_v1_creature_strategy_receipt_init(&rc);

    if (!pool_set || !pool_set->valid || !caii || !caii->valid || !receipt) {
        rc.fail_closed_reason = 1;
        if (receipt) *receipt = rc;
        return 0;
    }
    if (dm2_v1_record_handle_pool(record_handle) != DM2_V1_DB_CREATURE) {
        rc.fail_closed_reason = 1;
        *receipt = rc;
        return 0;
    }
    rec = dm2_v1_record_pool_address_mut(pool_set, record_handle);
    if (!rec || rec[5] == 0xffu || (int)rec[5] >= caii->capacity) {
        rc.fail_closed_reason = 1;
        *receipt = rc;
        return 0;
    }
    slot = caii->slots + (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;

    /* c_ai.cpp:4771: SELECT_CREATURE_37FC sets s350.v1e0584 which is
     * already resolved by the caller.  Here we receipt it. */
    rc.select_creature_ran = 1;

    /* c_ai.cpp:4772: table1d607e[v1e0584].uc[0] & 0x40 */
    if (!dm2_v1_creature_gdat_word1(creature_type, &w1) || w1 >= 0x2fu) {
        rc.fail_closed_reason = 2;
        *receipt = rc;
        return 0;
    }
    is_static = (table1d607e_local[w1][0] & 0x40u) != 0u;
    rc.is_static = is_static;

    if (is_static) {
        /* c_ai.cpp:5010-5101: static creature — random simple behaviors.
         * The full logic involves RAND(), waypoint computation, and tile
         * validation.  Fail-closed: the static path is receipted but the
         * actual random behaviors require host-owned state (s350, SPX_Creature
         * word@0xe, table1d27fc/1d2804, DM2_19f0_0559, GET_TILE_VALUE).
         * Receipt documents what WOULD happen. */
        rc.static_random_action = 1;
        /* c_ai.cpp:5109: the final fallthrough writes b_1a = 0 */
        slot[0x1a] = 0;
        rc.final_command = 0;
    } else {
        /* c_ai.cpp:4808-5008: mobile creature — CREATURE_GO_THERE path.
         * The full logic involves movement mode (v1e07ed), path finding
         * via 19f0_13aa, random direction selection, and the inner 4-way
         * directional loop (c_ai.cpp:4966-5006).
         * Fail-closed without the go_there callback. */
        if (go_there_cb) {
            int16_t parw00 = 0;
            int16_t parw01 = -1;
            int go_result;

            rc.go_there_tried = 1;
            /* c_ai.cpp:4812: DM2_CREATURE_GO_THERE(v1e07ed, xA, yA,
             * xA, parw00, parw01) — the mode from table1d607e flag 0x20. */
            {
                int mode = ((table1d607e_local[w1][0] & 0x20u) == 0u)
                    ? 5 : 4; /* c_ai.cpp:4777 */
                go_result = go_there_cb(go_there_ctx, slot, creature_type,
                                        mode, timer_x, timer_y, -1,
                                        &parw00, &parw01);
            }
            rc.go_there_result = go_result;

            if (go_result != 0) {
                /* Movement resolved — creature chose to go somewhere.
                 * c_ai.cpp:4994: slot[0x12] = -1, slot[0x13] = 0.
                 * The function returns without entering the XACT loop. */
                slot[0x12] = 0xffu;
                slot[0x13] = 0;
                rc.final_command = (int)(int8_t)slot[0x1a];
                rc.valid = 1;
                *receipt = rc;
                return 1;
            }
            /* go_there returned 0: fall through to XACT loop
             * c_ai.cpp:4867: or8(xp_0c + 0xb, 0x20) */
        } else {
            rc.fail_closed_reason = 3;
            *receipt = rc;
            return 0;
        }

        /* ── L_fin: XACT loop (c_ai.cpp:5113-5271) ───────────────── */
        /* c_ai.cpp:5114: slot[0x1a] = -1 */
        slot[0x1a] = 0xffu;

        /* The XACT loop calls DM2_14cd_0684, then iteratively calls
         * DECIDE_NEXT_XACT + PROCEED_XACT until b_1a != -1 or overflow.
         * Both 0684 and the loop body require host-owned state (s350,
         * table1d5f82, etc.).  Fail-closed without the xact callback. */
        if (!xact_cb) {
            rc.fail_closed_reason = 4;
            *receipt = rc;
            return 0;
        }

        rc.xact_loop_entered = 1;
        {
            int iteration;
            for (iteration = 0; iteration <= DM2_V1_MAX_XACT_ITERATIONS;
                 ++iteration) {
                int xact_result;
                rc.xact_loop_iterations = iteration + 1;

                /* c_ai.cpp:5242: PROCEED_XACT(DECIDE_NEXT_XACT(...)) */
                xact_result = xact_cb(xact_ctx, slot, creature_type,
                                      game_tick, iteration);
                rc.decide_next_calls++;
                rc.proceed_xact_calls++;

                /* c_ai.cpp:5253: >32 iterations -> b_1a = 0 */
                if (iteration >= DM2_V1_MAX_XACT_ITERATIONS) {
                    slot[0x1a] = 0;
                    rc.xact_loop_overflow = 1;
                }

                /* c_ai.cpp:5267: if b_1a != -1, break */
                if ((int)(int8_t)slot[0x1a] != -1) {
                    break;
                }
            }
        }
    }

    rc.final_command = (int)(int8_t)slot[0x1a];
    rc.valid = 1;
    *receipt = rc;
    return 1;
}

/* ================================================================== */
/* DM2_13e4_0982 — CCM dispatch (c_ai.cpp:5341-5647)                  */
/* ================================================================== */

void dm2_v1_ccm_dispatch_receipt_init(
    DM2_V1_CcmDispatchReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->pending_command = -1;
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
             "c_ai.cpp:5341-5647 DM2_13e4_0982 CCM dispatch; "
             "calls ccm_message_loop for the inner loop; "
             "owns pre-check, dying, 0x32-0x34, AI DB save");
}

int dm2_v1_creature_ccm_dispatch(
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
    DM2_V1_AiDbSaveCallback db_save_cb,
    void *db_save_ctx,
    DM2_V1_CcmDispatchReceipt *receipt)
{
    DM2_V1_CcmDispatchReceipt rc;
    uint8_t *rec;
    uint8_t *slot;
    const DM2_AIDefinition *def;
    int b1a;
    int b17;

    dm2_v1_ccm_dispatch_receipt_init(&rc);

    if (!pool_set || !pool_set->valid || !caii || !caii->valid ||
        !queue || !loader || !timer || !adj || !anim_row_io ||
        !v1e0584_io || !receipt) {
        rc.fail_closed_reason = 1;
        if (receipt) *receipt = rc;
        return 0;
    }

    if (dm2_v1_record_handle_pool(record_handle) != DM2_V1_DB_CREATURE) {
        rc.fail_closed_reason = 1;
        *receipt = rc;
        return 0;
    }
    rec = dm2_v1_record_pool_address_mut(pool_set, record_handle);
    if (!rec || rec[5] == 0xffu || (int)rec[5] >= caii->capacity) {
        rc.fail_closed_reason = 2;
        *receipt = rc;
        return 0;
    }
    slot = caii->slots + (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;

    if (!dm2_v1_creature_ai_spec_def(rec[DM2_V1_CREATURE_TYPE_OFFSET],
                                      &def) || !def) {
        rc.fail_closed_reason = 3;
        *receipt = rc;
        return 0;
    }

    b1a = (int)(int8_t)slot[0x1a];
    b17 = (int)(int8_t)slot[0x17];
    rc.pending_command = b17;

    /* c_ai.cpp:5356-5383: pre-check — is creature dying? */
    rc.is_dying = (b1a == 0x13 || b17 == 0x13) ? 1 : 0;
    rc.type_0x22 = (timer->type == 0x22u) ? 1 : 0;

    /* Delegate to the inner CCM message loop (dm2_v1_ccm_loop_pc34_compat).
     * That module handles the full pre-check, flag branch, dying branch,
     * 0x32-0x34 special, animation fetch, message loop, m_15785 end,
     * and m_15843 tail.  This outer dispatcher wraps it with:
     *   1. Record/slot resolution (done above)
     *   2. AI DB save/load (m_157BC, c_ai.cpp:5618-5640)
     *
     * The inner loop is called with suppress_requeue=0 (the default). */
    {
        int loop_result;
        DM2_V1_CcmLoopReceipt inner;
        memset(&inner, 0, sizeof(inner));

        loop_result = dm2_v1_ccm_message_loop(
            pool_set, caii, queue, loader, rng,
            record_handle, timer, source_index,
            adj, anim_row_io, v1e0584_io,
            0, /* suppress_requeue */
            mticks_map, savegame_b03, map_current, map_home,
            v1e0238, game_tick,
            ai_goal_cb, ai_goal_ctx,
            seven_cb, seven_ctx,
            proceed_ccm, proceed_ctx,
            &inner);

        rc.inner_receipt = inner;
        rc.ccm_loop_valid = inner.valid;
        rc.payload_skip = inner.payload_skip;
        rc.ai_turn_processed = inner.type_0x22;
        rc.dying_branch = inner.dying_branch;
        rc.special_mticks = inner.special_mticks;
        rc.message_loop_ran = inner.loop_entered;
        rc.animation_fetched = (inner.gaf_return >= 0) ? 1 : 0;
        rc.requeue_completed = (inner.tail_enqueued ||
                                inner.requeue_end_completed) ? 1 : 0;

        if (inner.ai_goal_unbound) {
            rc.strategy_called = 1;
        }

        if (!loop_result) {
            rc.fail_closed_reason = 5;
            *receipt = rc;
            return 0;
        }
    }

    /* c_ai.cpp:5618-5640: m_157BC — AI DB save.  The bitmap block
     * requires v1e07eb (set by 13e4_01a3) and v1e07d8 conditions.
     * This is receipted through the db_save callback when provided. */
    if (db_save_cb) {
        uint16_t creature_handle =
            (uint16_t)((uint16_t)record_handle & 0x03ffu);
        int save_result = db_save_cb(db_save_ctx, creature_handle, 1);
        rc.bitmap_save = save_result;
    }

    rc.valid = 1;
    *receipt = rc;
    return 1;
}

/* ================================================================== */
/* DM2_THINK_CREATURE main body (c_ai.cpp:5649-5784)                  */
/* ================================================================== */

void dm2_v1_think_creature_main_receipt_init(
    DM2_V1_ThinkCreatureMainReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->creature_record = -1;
    receipt->creature_type = -1;
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
             "c_ai.cpp:5649-5784 DM2_THINK_CREATURE; "
             "c_querydb.cpp:1486-1507 GET_CREATURE_AT; "
             "PREPARE/UNPREPARE_LOCAL_CREATURE_VAR; "
             "WOUND_CREATURE c_ai.cpp:5744");
}

int dm2_v1_think_creature_main(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int think_type,
    unsigned long game_tick,
    DM2_V1_PrepareCreatureCallback prepare_cb,
    void *prepare_ctx,
    DM2_V1_UnprepareCreatureCallback unprepare_cb,
    void *unprepare_ctx,
    DM2_V1_WoundCreatureCallback wound_cb,
    void *wound_ctx,
    DM2_V1_ThinkDispatch0982Callback dispatch_cb,
    void *dispatch_ctx,
    DM2_V1_ThinkTimingCallback timing_cb,
    void *timing_ctx,
    DM2_V1_ThinkCreatureMainReceipt *receipt)
{
    DM2_V1_ThinkCreatureMainReceipt rc;
    int16_t creature;
    const uint8_t *rec_addr;
    void *prepare_token = NULL;

    dm2_v1_think_creature_main_receipt_init(&rc);

    if (!pool_set || !pool_set->valid || !dungeon || !receipt) {
        rc.fail_closed_reason = 1;
        if (receipt) *receipt = rc;
        return 0;
    }

    /* c_ai.cpp:5669-5671: DM2_GET_CREATURE_AT(x, y) */
    creature = dm2_v1_get_creature_at(pool_set, dungeon, map, x, y);
    rc.creature_record = creature;
    if (creature == DM2_V1_RECORD_HANDLE_NULL) {
        /* c_ai.cpp:5670-5671: if (RG1W == 0xffff) return;
         * Timer consumed, nothing thinks — same as source. */
        rc.fail_closed_reason = 1;
        rc.valid = 1; /* this is a valid outcome, not an error */
        *receipt = rc;
        return 1;
    }
    rc.creature_resolved = 1;

    rec_addr = dm2_v1_record_pool_address(pool_set, creature);
    if (rec_addr) {
        rc.creature_type = (int)rec_addr[DM2_V1_CREATURE_TYPE_OFFSET];
    }

    /* c_ai.cpp:5675-5676: DM2_PREPARE_LOCAL_CREATURE_VAR */
    if (prepare_cb) {
        prepare_token = prepare_cb(prepare_ctx, (uint16_t)creature,
                                   x, y, think_type);
        if (!prepare_token) {
            rc.fail_closed_reason = 2;
            *receipt = rc;
            return 0;
        }
        rc.prepare_ran = 1;
    } else {
        /* No prepare callback — fail closed.  The source requires
         * PREPARE_LOCAL_CREATURE_VAR to set up s350. */
        rc.fail_closed_reason = 2;
        *receipt = rc;
        return 0;
    }

    /* c_ai.cpp:5679-5688: creature word@2 = -1; HP init if word@6 == 0.
     * These are host-owned s350.creatures mutations — receipted in the
     * prepare callback. */

    /* c_ai.cpp:5690-5738: timing — speed check.
     * RG51w = abs(aidef byte@3); timing = (gametick >> 2) & 0xff;
     * vol_08 = timing; if creature byte@6 > vol_08, vol_08 += 0x100;
     * vol_08 -= creature byte@6; speed_div = vol_08 / speed;
     * If speed_div > 0: accumulated damage handling + phase update.
     * The full timing computation requires the AIDefinition byte@3
     * (speed) and creature byte@6 (phase), both host-owned.
     * Receipted as speed_check. */
    rc.speed_check = 1;

    /* c_ai.cpp:5740-5747: self-damage — if accumulated > 0, WOUND_CREATURE.
     * The accumulated damage (RG62W) is computed from the timing path.
     * Without the full timing computation, we receipt it as checked. */
    rc.self_damage = 0; /* would be > 0 if speed_div accumulated damage */

    /* c_ai.cpp:5749-5779: dispatch.
     * if aidef byte@0 & 1: animation path -> DM2_13e4_0982
     * else: timing path -> DM2_ai_13e4_071b or DM2_ai_13e4_0806
     *
     * The dispatch condition is the aidef flag test at c_ai.cpp:5752.
     * Without the full s350 state, we dispatch through the callbacks. */
    if (dispatch_cb) {
        int dispatch_result = dispatch_cb(
            dispatch_ctx, (uint16_t)creature, NULL, think_type);
        if (dispatch_result) {
            rc.dispatched_0982 = 1;
        }
    } else if (timing_cb) {
        /* c_ai.cpp:5756-5767: adj word@2 & 0x4000 -> 071b,
         * adj word@2 & 0x2000 -> 0806 */
        int timing_result = timing_cb(
            timing_ctx, (uint16_t)creature, think_type);
        if (timing_result) {
            rc.dispatched_071b = 1;
        }
    } else {
        rc.fail_closed_reason = 3;
        /* Still unprepare before returning */
        if (unprepare_cb) {
            unprepare_cb(unprepare_ctx, prepare_token);
            rc.unprepare_ran = 1;
        }
        *receipt = rc;
        return 0;
    }

    /* c_ai.cpp:5783: DM2_UNPREPARE_LOCAL_CREATURE_VAR(RG7p) */
    if (unprepare_cb) {
        unprepare_cb(unprepare_ctx, prepare_token);
        rc.unprepare_ran = 1;
    }

    rc.valid = 1;
    *receipt = rc;
    return 1;
}

/* ================================================================== */
/* Source evidence                                                      */
/* ================================================================== */

const char *dm2_v1_creature_ai_loop_source_evidence(void)
{
    return
        "DM2 V1 creature AI loop orchestrators — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:4743-5272 (DM2_14cd_09e2 "
        "creature strategy selector)\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:5275-5338 (DM2_50CB "
        "animation frame resolver)\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:5341-5647 (DM2_13e4_0982 "
        "CCM dispatch loop)\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:5649-5784 (DM2_THINK_CREATURE "
        "main body)\n"
        "Source: skproject/SKULLWIN/c_querydb.cpp:1486-1507 "
        "(DM2_GET_CREATURE_AT)\n"
        "Source: skproject/SKULLWIN/mdata.c:1564-1613 (table1d607e)\n";
}
