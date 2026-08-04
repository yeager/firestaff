#ifndef FIRESTAFF_DM2_V1_CREATURE_AI_LOOP_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_AI_LOOP_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_loop_pc34_compat.h — DM2 creature AI loop orchestrators.
 *
 * Ports four functions from skproject/SKULLWIN/c_ai.cpp:
 *
 *   DM2_14cd_09e2 (4743-5272): creature strategy selector — picks what
 *     a creature does on its turn: SELECT_CREATURE_37FC, static vs mobile
 *     path, CREATURE_GO_THERE, then the XACT loop (DECIDE_NEXT_XACT +
 *     PROCEED_XACT up to 32 iterations).
 *
 *   DM2_13e4_0982 (5341-5647): CCM dispatch loop — the outer dispatcher
 *     that handles dying creatures, type-0x22 AI turns, special 0x32-0x34
 *     states, animation frame fetch, and the message loop.  The existing
 *     dm2_v1_ccm_loop_pc34_compat module implements the inner message
 *     loop; this function is the OUTER caller.
 *
 *   DM2_THINK_CREATURE (5649-5784): the full think body — resolves
 *     creature at cell, prepares local vars, handles timing/speed,
 *     self-damage, then dispatches to 13e4_0982 or timing events.
 *
 *   DM2_50CB (5275-5338): animation frame resolver (public interface
 *     wrapping the existing ccm_50cb static function).
 *
 * All functions are fail-closed with receipt-based architecture.
 * s350 stays host-owned; its fields enter as explicit parameters.
 */

#include <stdint.h>

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_ccm_loop_pc34_compat.h"
#include "dm2_v1_creature_ai_decision_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================== */
/* DM2_14cd_09e2 — creature strategy selector (c_ai.cpp:4743-5272)    */
/* ================================================================== */

typedef struct {
    int valid;
    int select_creature_ran;     /* SELECT_CREATURE_37FC completed */
    int is_static;               /* table1d607e flag 0x40 set */
    int static_random_action;    /* random action for static creature */
    int static_set_waypoint;     /* waypoint set for static creature */
    int static_rotate;           /* rotate for static creature */
    int go_there_tried;          /* CREATURE_GO_THERE attempted */
    int go_there_result;         /* CREATURE_GO_THERE return value */
    int xact_loop_entered;       /* main XACT loop reached */
    int xact_loop_iterations;    /* iterations of the XACT loop */
    int xact_loop_overflow;      /* >32 iterations: forced b_1a=0 */
    int decide_next_calls;       /* DECIDE_NEXT_XACT invocations */
    int proceed_xact_calls;      /* PROCEED_XACT invocations */
    int action_0684_result;      /* DM2_14cd_0684 return value */
    int action_08f5_result;      /* DM2_14cd_08f5 return value */
    int final_command;           /* slot byte 0x1a at exit */
    int fail_closed_reason;      /* 0=none, 1=no_slot, 2=no_aidef,
                                    3=go_there_unbound, 4=xact_unbound */
    char source_evidence[256];
} DM2_V1_CreatureStrategyReceipt;

/* Callback for CREATURE_GO_THERE (c_ai.cpp:4810-4814).
 * mode: v1e07ed (movement mode); x,y: timer position;
 * dir: movement direction; parw00/parw01: output parameters.
 * Returns 1 if movement was resolved, 0 otherwise. */
typedef int (*DM2_V1_CreatureGoThereCallback)(
    void *ctx, uint8_t *slot, uint16_t creature_type,
    int mode, int x, int y, int dir,
    int16_t *parw00, int16_t *parw01);

/* Callback for DECIDE_NEXT_XACT + PROCEED_XACT (c_ai.cpp:5242-5268).
 * Called for each iteration.  Sets slot[0x1a] to the chosen action.
 * Returns -4 (0xfc) for "no action" or the XACT result code. */
typedef int (*DM2_V1_XactLoopCallback)(
    void *ctx, uint8_t *slot, uint16_t creature_type,
    unsigned long game_tick, int iteration);

void dm2_v1_creature_strategy_receipt_init(
    DM2_V1_CreatureStrategyReceipt *receipt);

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
    DM2_V1_CreatureStrategyReceipt *receipt);

/* ================================================================== */
/* DM2_13e4_0982 — CCM dispatch (c_ai.cpp:5341-5647)                  */
/* ================================================================== */

typedef struct {
    int valid;
    int is_dying;                /* b_1a == 0x13 or b_17 == 0x13 */
    int type_0x22;               /* timer type is 0x22 */
    int payload_skip;            /* pre-check failed, payload adddata(4) */
    int ai_turn_processed;       /* type 0x22 AI turn handled */
    int pending_command;         /* b_17 value at entry */
    int strategy_called;         /* DM2_14cd_09e2 invoked */
    int dying_branch;            /* b_1a == 0x13 death processing */
    int special_mticks;          /* b_1a in 0x32..0x34 */
    int animation_fetched;       /* GAF called */
    int message_loop_ran;        /* inner CCM message loop executed */
    int ccm_loop_valid;          /* inner loop receipt valid */
    int requeue_completed;       /* timer re-queued */
    int bitmap_save;             /* AI DB allocation save */
    int fail_closed_reason;      /* 0=none, 1=no_rec, 2=no_slot,
                                    3=no_aidef, 4=strategy_fail,
                                    5=ccm_loop_fail */
    DM2_V1_CcmLoopReceipt inner_receipt; /* from the message loop */
    char source_evidence[256];
} DM2_V1_CcmDispatchReceipt;

/* Callback for the AI DB save/load (c_ai.cpp:5618-5640).
 * The bitmap block: allocate or fetch the AI DB allocation, copy
 * the v1e07d8 struct into it.  Receipted, never simulated. */
typedef int (*DM2_V1_AiDbSaveCallback)(
    void *ctx, uint16_t creature_handle, int do_save);

void dm2_v1_ccm_dispatch_receipt_init(
    DM2_V1_CcmDispatchReceipt *receipt);

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
    DM2_V1_CcmDispatchReceipt *receipt);

/* ================================================================== */
/* DM2_THINK_CREATURE main body (c_ai.cpp:5649-5784)                  */
/* ================================================================== */

typedef struct {
    int valid;
    int creature_resolved;       /* GET_CREATURE_AT found a creature */
    int16_t creature_record;     /* resolved record handle, -1 none */
    int creature_type;           /* DB4 byte@4, -1 unknown */
    int prepare_ran;             /* PREPARE_LOCAL_CREATURE_VAR completed */
    int speed_check;             /* timing: gametick>>2 mod speed */
    int speed_skipped;           /* creature did not act this tick */
    int self_damage;             /* accumulated damage > 0 */
    int wound_result;            /* WOUND_CREATURE return (0=alive) */
    int creature_killed;         /* wound killed the creature */
    int dispatched_0982;         /* DM2_13e4_0982 animation dispatch */
    int dispatched_071b;         /* DM2_ai_13e4_071b timing event */
    int dispatched_0806;         /* DM2_ai_13e4_0806 timing event */
    int unprepare_ran;           /* UNPREPARE_LOCAL_CREATURE_VAR done */
    int fail_closed_reason;      /* 0=none, 1=no_creature, 2=prepare_fail,
                                    3=dispatch_fail */
    char source_evidence[256];
} DM2_V1_ThinkCreatureMainReceipt;

/* Callback for PREPARE_LOCAL_CREATURE_VAR.  Sets up s350 context.
 * Returns the opaque prepare token (RG7p) or NULL on failure. */
typedef void *(*DM2_V1_PrepareCreatureCallback)(
    void *ctx, uint16_t creature_handle, int x, int y, int think_type);

/* Callback for UNPREPARE_LOCAL_CREATURE_VAR.  Takes the token from
 * prepare. */
typedef void (*DM2_V1_UnprepareCreatureCallback)(
    void *ctx, void *prepare_token);

/* Callback for WOUND_CREATURE (c_ai.cpp:5744).  Returns non-zero if
 * the creature was killed. */
typedef int (*DM2_V1_WoundCreatureCallback)(
    void *ctx, uint16_t creature_handle, uint16_t damage);

/* Callback for the ccm dispatch (DM2_13e4_0982) when creature has
 * an active animation.  Returns 1 if handled. */
typedef int (*DM2_V1_ThinkDispatch0982Callback)(
    void *ctx, uint16_t creature_handle,
    const DM2_V1_SourceTimer *timer, int think_type);

/* Callback for timing events (DM2_ai_13e4_071b / 0806). */
typedef int (*DM2_V1_ThinkTimingCallback)(
    void *ctx, uint16_t creature_handle, int event_type);

void dm2_v1_think_creature_main_receipt_init(
    DM2_V1_ThinkCreatureMainReceipt *receipt);

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
    DM2_V1_ThinkCreatureMainReceipt *receipt);

/* ================================================================== */
/* DM2_50CB — animation frame resolver (c_ai.cpp:5275-5338)           */
/* ================================================================== */

typedef struct {
    int valid;
    int frame_ended;       /* row exhausted (return 2 = break) */
    int frame_advanced;    /* return 1 = continue with next row */
    int frame_unchanged;   /* return 0 = continue same row */
    int gdat_missing;      /* fail-closed: GDAT data not found */
    int table_oob;         /* fail-closed: index out of bounds */
    char source_evidence[128];
} DM2_V1_AnimFrameResolveReceipt;

void dm2_v1_anim_frame_resolve_receipt_init(
    DM2_V1_AnimFrameResolveReceipt *receipt);

int dm2_v1_creature_animation_frame_resolve(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_row,
    DM2_V1_AnimFrameResolveReceipt *receipt);

/* Source evidence strings. */
const char *dm2_v1_creature_ai_loop_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_AI_LOOP_PC34_COMPAT_H */
