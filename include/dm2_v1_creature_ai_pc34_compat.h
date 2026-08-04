#ifndef FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_pc34_compat.h — DM2 creature AI subsystem.
 *
 * Ports DM2_THINK_CREATURE, DM2_ROTATE_CREATURE, and the PROCEED_XACT
 * handler table from skproject/SKULLWIN/c_ai.cpp and c_creature.cpp.
 *
 * THINK_CREATURE (c_ai.cpp:5649): the main AI tick. Called from timer
 * proc (c_tim_proc.cpp:4088) for each creature. Resolves creature at
 * tile via GET_CREATURE_AT, prepares local creature vars via
 * PREPARE_LOCAL_CREATURE_VAR, then runs the XACT state machine.
 *
 * ROTATE_CREATURE (c_creature.cpp:58-101): rotates a creature's facing
 * direction. Updates word at record+0xe bits 14-15 (direction field).
 * If AI spec flag bit 0 set, also rotates all possession records.
 *
 * PROCEED_XACT handlers (c_ai.cpp): 30+ handlers numbered 56-88 that
 * implement creature behaviors — move, attack, cast, flee, patrol,
 * guard, follow, etc.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp, c_creature.cpp
 */

#include <stdint.h>
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* XACT action types — the behavior opcodes in the AI state machine.
 * Each maps to a PROCEED_XACT_NN handler in c_ai.cpp */
typedef enum {
    DM2_XACT_STOP              = 56,
    DM2_XACT_STOP_CLEAR        = 57,
    DM2_XACT_MOVE_FORWARD      = 59,
    DM2_XACT_ATTACK            = 62,
    DM2_XACT_CAST_SPELL        = 63,
    DM2_XACT_SHOOT_MISSILE     = 64,
    DM2_XACT_FLEE              = 65,
    DM2_XACT_MOVE_RANDOM       = 66,
    DM2_XACT_MOVE_TO_TARGET    = 67,
    DM2_XACT_GUARD_POSITION    = 68,
    DM2_XACT_SOUND             = 69,
    DM2_XACT_FACE_PARTY        = 70,
    DM2_XACT_PATROL            = 71,
    DM2_XACT_REGENERATE        = 72,
    DM2_XACT_SPAWN             = 73,
    DM2_XACT_OPEN_DOOR         = 74,
    DM2_XACT_USE_ITEM          = 75,
    DM2_XACT_MOVE_BACKWARD     = 76,
    DM2_XACT_FOLLOW            = 77,
    DM2_XACT_SET_STATE_58      = 58,
    DM2_XACT_SET_STATE_60      = 60,
    DM2_XACT_NOP_61            = 61,
    DM2_XACT_SLEEP_78          = 78,
    DM2_XACT_WAKE_79           = 79,
    DM2_XACT_MOVE_80           = 80,
    DM2_XACT_MOVE_81           = 81,
    DM2_XACT_MOVE_82           = 82,
    DM2_XACT_MOVE_83           = 83,
    DM2_XACT_MOVE_84           = 84,
    DM2_XACT_MOVE_85           = 85,
    DM2_XACT_SET_STATE_86      = 86,
    DM2_XACT_SLEEP             = 87,
    DM2_XACT_WAKE              = 88,
    DM2_XACT_89                = 89,
    DM2_XACT_RANDOM_90         = 90,
    DM2_XACT_ITEM_91           = 91
} DM2_V1_XactType;

/* Callback: query AI spec flags for a creature handle.
 * Returns the flags word (bit 0 = rotate possessions). */
typedef uint16_t (*DM2_V1_QueryCreatureAiSpecFlagsFn)(void *ctx,
                                                       uint16_t creature_handle);

/* ROTATE_CREATURE request */
typedef struct {
    int16_t creature_handle;
    int16_t new_direction;
    int rotate_relative;
    DM2_V1_RecordPoolSet *pool_set;
    DM2_V1_QueryCreatureAiSpecFlagsFn query_ai_spec_flags;
    void *ai_spec_ctx;
} DM2_V1_RotateCreatureRequest;

/* ROTATE_CREATURE receipt */
typedef struct {
    int valid;
    int fail_closed;
    int16_t old_direction;
    int16_t new_direction;
    int possessions_rotated;
} DM2_V1_RotateCreatureReceipt;

/* THINK_CREATURE request */
typedef struct {
    int16_t tile_x;
    int16_t tile_y;
    int16_t map_level;
    uint32_t game_tick;
} DM2_V1_ThinkCreatureRequest;

/* THINK_CREATURE receipt */
typedef struct {
    int valid;
    int fail_closed;
    int creature_found;
    int16_t creature_handle;
    DM2_V1_XactType xact_executed;
    int xact_succeeded;
    int creature_moved;
    int creature_attacked;
    int creature_cast;
    int creature_died;
} DM2_V1_ThinkCreatureReceipt;

int dm2_v1_rotate_creature(
    const DM2_V1_RotateCreatureRequest *request,
    DM2_V1_RotateCreatureReceipt *receipt);

int dm2_v1_think_creature(
    const DM2_V1_ThinkCreatureRequest *request,
    DM2_V1_ThinkCreatureReceipt *receipt);

/* ------------------------------------------------------------------ */
/* XACT dispatch (c_ai.cpp:2152-2338)                                 */
/* ------------------------------------------------------------------ */

/* Result of dispatching an action byte through PROCEED_XACT.
 * handler_id is the XACT number (56-91) or -1 if out of range.
 * has_return_value: some handlers return a value, others are void. */
typedef struct {
    int valid;
    int fail_closed;
    int handler_id;        /* XACT number 56-91, or -1 if invalid */
    int sets_ret;          /* 1 if handler sets return value, 0 if void */
    int inline_action;     /* 1 if action is handled inline (sets b_1a) */
    int8_t inline_b_1a;   /* value to set in b_1a for inline actions */
    int16_t inline_v1e0572;/* value to set in v1e0572 for case 76 (opt 20) */
    int16_t inline_v1e0574;/* value to set in v1e0574 for case 76 (opt 20) */
} DM2_V1_XactDispatchReceipt;

int dm2_v1_xact_dispatch_action(int8_t action_byte,
                                 DM2_V1_XactDispatchReceipt *receipt);

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_72_87_88 (c_ai.cpp:949-955)                          */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t v1e0572;       /* s350.v1e0572 */
    int16_t v1e07d8_w04;   /* s350.v1e07d8.w_04 */
} DM2_V1_Xact72_87_88Request;

typedef struct {
    int valid;
    int fail_closed;
    int8_t b_1a;           /* value to write to creatures->b_1a */
} DM2_V1_Xact72_87_88Receipt;

int dm2_v1_proceed_xact_72_87_88(const DM2_V1_Xact72_87_88Request *req,
                                  DM2_V1_Xact72_87_88Receipt *receipt);

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_86 (c_ai.cpp:2120-2126)                              */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t v1e0572;       /* s350.v1e0572 */
    int16_t v1e07d8_w04;   /* s350.v1e07d8.w_04 */
    int16_t v1e07d8_w06;   /* s350.v1e07d8.w_06 */
} DM2_V1_Xact86Request;

typedef struct {
    int valid;
    int fail_closed;
    int8_t b_20;           /* value to write to creatures->b_20 */
    int8_t b_1e;           /* value to write to creatures->b_1e */
    int8_t b_1a;           /* value to write to creatures->b_1a */
} DM2_V1_Xact86Receipt;

int dm2_v1_proceed_xact_86(const DM2_V1_Xact86Request *req,
                            DM2_V1_Xact86Receipt *receipt);

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_90 (c_ai.cpp:2136-2138)                              */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t v1e0572;       /* s350.v1e0572 — threshold */
    int16_t rand_value;    /* random value 0-99 (caller provides) */
} DM2_V1_Xact90Request;

typedef struct {
    int valid;
    int fail_closed;
    int8_t result;         /* -2 (pass) or -3 (fail) */
} DM2_V1_Xact90Receipt;

int dm2_v1_proceed_xact_90(const DM2_V1_Xact90Request *req,
                            DM2_V1_Xact90Receipt *receipt);

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_91 (c_ai.cpp:2142-2150)                              */
/* ------------------------------------------------------------------ */

/* Callback: DM2_CREATURE_CAN_HANDLE_ITEM_IN.
 * Returns -2 if cannot handle, other value if can. */
typedef int16_t (*DM2_V1_CreatureCanHandleItemFn)(void *ctx,
                                                    int16_t item_type,
                                                    uint16_t possession_w00,
                                                    int32_t mask);

typedef struct {
    int16_t v1e0572;       /* first item type */
    int16_t v1e0574;       /* second item type */
    uint16_t possession_w00; /* s350.v1e054e->possession.w_00 */
    DM2_V1_CreatureCanHandleItemFn can_handle_item;
    void *ctx;
} DM2_V1_Xact91Request;

typedef struct {
    int valid;
    int fail_closed;
    int8_t result;         /* -2 (can handle) or -3 (cannot) */
} DM2_V1_Xact91Receipt;

int dm2_v1_proceed_xact_91(const DM2_V1_Xact91Request *req,
                            DM2_V1_Xact91Receipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H */
