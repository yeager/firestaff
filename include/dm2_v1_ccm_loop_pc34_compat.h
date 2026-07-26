#ifndef DM2_V1_CCM_LOOP_PC34_COMPAT_H
#define DM2_V1_CCM_LOOP_PC34_COMPAT_H

/*
 * DM2 v1 PC 3.4 CCM stream owner/grammar — the message loop
 * DM2_13e4_0982 (skproject/SKULLWIN/c_ai.cpp:5341-5647), bounded slice.
 *
 * This is the loop DM2_THINK_CREATURE (c_ai.cpp:5750-5752) runs for a
 * living creature; it owns the CCM byte-stream grammar:
 *
 *   - the pre-check (c_ai.cpp:5346-5383): the body runs when
 *     ddat.savegames1.b_03 == 0, OR the AIDefinition byte@1 bit 0x10 is
 *     set, OR the slot's CCM command/pending-command is 0x13; otherwise
 *     the timer payload eats adddata(4) (c_timer.h:68: l_00 += 4) and
 *     the body is skipped ENTIRELY — the timer still reaches the
 *     m_15843 re-queue tail with its bumped payload;
 *   - the !flag branch (type != 0x22, c_ai.cpp:5386-5390): the bound
 *     standalone DM2_4FCC (creature_something module) advances the
 *     animation frame from the caller-owned sequence base;
 *   - the flag branch (type == 0x22, c_ai.cpp:5391-5479): b_1a = b_17;
 *     b_17 == -1 would call DM2_14cd_09e2 — the AI goal picker, UNBOUND
 *     (fail-closed ai_goal_unbound AFTER the source's b_1a write);
 *     otherwise b_17 = -1 and DM2_14cd_062e gates a b_1a reset: its
 *     slot byte@0x12 == 0xff early return (c_ai.cpp:2442-2445) is
 *     bound, the table1d5f82 s_seven chain (c_ai.cpp:2447-2459) stays
 *     unproven (fail-closed s_seven_unbound).  A taken reset writes
 *     b_1a = -1; a kept b_1a runs the table1d613a & 4 bitmap-state
 *     reset (receipted, part of the unbound m_157BC chain) and the
 *     mode 6/7 facing write to slot byte@0x1d
 *     (c_ai.cpp:5458-5478, (rec word@0xe >> 8) ± 1 & 3);
 *   - the dying branch (b_1a == 0x13, c_ai.cpp:5481-5537): slot
 *     word@0xe/word@0x10 = the adj pair, then the table1d607e[GDAT
 *     word@1].uc[0] & 8 gate (data-backed through
 *     dm2_v1_creature_gdat_word1; per-module source-locked table copy,
 *     mdata.c:1564-1613; out-of-span w1 fails closed) — when clear, the
 *     AIDefinition byte@0x23 selects the cloud id (1 -> 0xbe, 2 ->
 *     0xff, else 0x6e) and DM2_CREATE_CLOUD is RECEIPTED
 *     (cloud_would_create + cloud_id), never simulated — the cloud
 *     system is unproven;
 *   - the 0x32..0x34 special (c_ai.cpp:5539-5553): setmticks(v1e0571,
 *     b_1a + gametick - 50) and NO animation fetch, NO loop, NO
 *     m_15785 end — the timer still reaches the m_15843 tail;
 *   - otherwise the bound DM2_GET_CREATURE_ANIMATION_FRAME (round-14
 *     module) runs with command = b_1a and parl01 = 0;
 *   - the message loop itself (c_ai.cpp:5567-5606): RG4W starts as the
 *     GAF/4FCC return; each iteration first skips the handler when
 *     (!flag && slot byte@0x21 != 0 && row byte@2 & 0x40), otherwise
 *     runs the handler when row byte@2 & 0x80 — DM2_13e4_01a3's
 *     provable slice is bound in source order (the v1e0584 lazy GDAT
 *     word@1 set, the v1e058d RAND16 draw
 *     RAND16(2*(0xf-(aidef w16 & 0xf))+1) + base/4 vs. the
 *     gametick-minus-slot-byte@4 window, the v1e07eb once-guard) and
 *     the DM2_PROCEED_CCM dispatch is RECEIPTED through the proven
 *     DM2-005 matrix (ccm_handler + ccm_handler_group) and fails
 *     closed — the per-command handlers stay host-owned; then, when
 *     slot byte@0x21 != 0 and row byte@2 & 0x40, the bound DM2_50CB
 *     deterministic row step (c_ai.cpp:5275-5338: row byte@2 & 0x3f
 *     pre-add with the == 0 -> return 2 BREAK, the byte@1 high
 *     nibble return selector, the row pointer store) advances the
 *     stream — results 0/1 CONTINUE the loop, result 2 ends it
 *     (c_ai.cpp:5602-5604).  A NULL animation row is
 *     fail-closed (anim_row_null) — the source dereferences it
 *     unchecked;
 *   - the m_15785 end (skip00269 only): the v1e0570 suppression is
 *     honored BEFORE the mticks delta is computed (c_ai.cpp:5612-5613),
 *     the bound DM2_CREATURE_SOMETHING_1c9a_0a48 (round-14 module)
 *     supplies the delta, and the round-13 dm2_v1_caii_ccm_end_requeue
 *     owns the type rebuild + cancel/queue/store;
 *   - the m_157BC bitmap block (c_ai.cpp:5618-5640) stays unbound —
 *     structurally unreachable in this slice because its v1e07eb guard
 *     requires a completed 13e4_01a3, and the handler boundary fails
 *     closed first (receipted bitmap_would_copy, always 0);
 *   - the m_15843 tail (c_ai.cpp:5642-5647) for the NON-loop exits
 *     (payload skip, 0x32..0x34) is composed from the bound primitives:
 *     slot word@2 != -1 cancels through dm2_v1_caii_delete_timer, then
 *     dm2_v1_source_timer_enqueue_ticketed re-queues the timer AS-IS
 *     and the ticket lands in slot word@2.
 *
 * s350 stays host-owned; its fields enter as explicit parameters:
 * `timer` is v1e0562 (in/out — payload adddata(4) and the 0x32..0x34
 * setmticks mutate it), `adj` the v1e055e pair (in/out), `anim_row_io`
 * v1e055a (in/out), `v1e0584_io` s350.v1e0584 (in/out, -1 unset),
 * `mticks_map` v1e0571, `suppress_requeue` v1e0570 (CCM-handler owned,
 * normally 0), `savegame_b03` ddat.savegames1.b_03, `map_current`
 * ddat.v1d3248, `map_home` ddat.v1e08d6, `v1e0238` the ddat global, and
 * `game_tick` timdat.gametick.  The CAII slot resolves through record
 * byte@5 exactly like the source's s350.creatures.
 *
 * Returns 1 when the slice completed (receipt.valid == 1); 0 for every
 * fail-closed path.  When `receipt` is non-NULL it always receives the
 * audit record.
 */

#include <stdint.h>

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_ccm_dispatch_pc34_compat.h"
#include "dm2_v1_creature_something_pc34_compat.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timeline.h"

typedef struct {
  int valid;
  int not_creature_db;       /* handle is not a DB4 creature record */
  int no_slot;               /* record byte@5 == 0xff */
  int aidef_unknown;         /* no session AI row for the record type */
  int body_entered;          /* skip00261: the message-loop body ran */
  int payload_skip;          /* adddata(4) path — body skipped entirely */
  int type_0x22;             /* flag: the type-0x22 branch ran */
  int command;               /* slot byte@0x1a after the flag dance */
  int command_out_of_span;   /* fail-closed: b_1a beyond table1d613a */
  int rng_unbound;           /* fail-closed: a source draw had no LCG */
  int ai_goal_unbound;       /* fail-closed: b_17 == -1 (DM2_14cd_09e2) */
  int s_seven_unbound;       /* fail-closed: DM2_14cd_062e table path */
  int cfg_reset;             /* 062e != 0 and b_1a != 0x13: b_1a = -1 */
  int bitmap_state_reset;    /* table1d613a[b_1a] & 4 (receipted only) */
  int dir_written;           /* mode 6/7 facing write to byte@0x1d, -1 */
  int dying_branch;          /* b_1a == 0x13 */
  int gdat_w1_out_of_span;   /* fail-closed: w1 beyond table1d607e 0x2f */
  int cloud_would_create;    /* CREATE_CLOUD receipted, never simulated */
  int cloud_id;              /* 0xbe / 0xff / 0x6e */
  int special_mticks;        /* b_1a in 0x32..0x34: setmticks, no loop */
  int gaf_return;            /* GAF/4FCC RG1L (-1 when not called) */
  int gaf_fail_closed;       /* fail-closed inside the bound GAF/4FCC */
  int loop_entered;          /* skip00269 */
  int loop_iterations;       /* completed iterations */
  int anim_row_null;         /* fail-closed: NULL row dereference */
  int init_01a3_runs;        /* bound DM2_13e4_01a3 slice invocations */
  int v1e0584_set;           /* lazy GDAT word@1 store happened */
  int v1e058d;               /* the computed 13e4_01a3 flag */
  int query_08bd_unbound;    /* DM2_query_1c9a_08bd mask (receipted) */
  int ccm_handler;           /* slot byte@0x1a at the handler boundary */
  int ccm_handler_group;     /* DM2_V1_CcmSourceHandler at the boundary */
  int ccm_handler_unbound;   /* fail-closed: PROCEED_CCM body host-owned */
  int walk_50cb_calls;       /* bound DM2_50CB invocations */
  int walk_50cb_last;        /* last 50CB return (-1 when never called) */
  int loop_result;           /* RG4W at m_15785 */
  int mticks_delta;          /* the bound 1c9a_0a48 result */
  int something_fail_closed; /* fail-closed inside bound 1c9a_0a48 */
  int requeue_end_completed; /* round-13 end_requeue receipt.completed */
  int requeue_end_suppressed;/* round-13 end_requeue receipt.suppressed */
  int tail_cancelled;        /* m_15843: pending timer cancelled */
  int tail_enqueued;         /* m_15843: timer re-queued as-is */
  uint32_t tail_ticket;      /* m_15843: ticket stored in slot word@2 */
  int bitmap_would_copy;     /* m_157BC receipted (structurally 0) */
  char source_evidence[512];
} DM2_V1_CcmLoopReceipt;

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
    DM2_V1_CcmLoopReceipt *receipt);

#endif
