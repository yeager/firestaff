#ifndef FIRESTAFF_DM2_V1_CREATURE_XACT_HANDLERS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_XACT_HANDLERS_PC34_COMPAT_H

/*
 * dm2_v1_creature_xact_handlers_pc34_compat.h — DM2 PROCEED_XACT handler
 * functions for creature AI behaviors.
 *
 * Ports the 25 PROCEED_XACT handlers from skproject/SKULLWIN/c_ai.cpp.
 * Each handler implements a creature behavior opcode (XACT 56-89 plus
 * helpers). Return semantics: -2 = success, -3 = fail, -4 = blocked/redirected.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================== */
/* XACT return codes                                                   */
/* ================================================================== */

#define DM2_XACT_RESULT_SUCCESS    (-2)
#define DM2_XACT_RESULT_FAIL       (-3)
#define DM2_XACT_RESULT_BLOCKED    (-4)

/* ================================================================== */
/* Callback typedefs for external functions                            */
/* ================================================================== */

/* CREATURE_GO_THERE (c_ai.cpp): move creature to target tile.
 * flags: movement flags (e.g. 0x80).
 * Returns non-zero on success, 0 if blocked. */
typedef int16_t (*DM2_V1_CreatureGoThereFn)(void *ctx,
                                             uint16_t flags,
                                             int16_t src_x, int16_t src_y,
                                             int16_t dst_x, int16_t dst_y,
                                             int16_t direction);

/* GET_CREATURE_AT (c_creature.cpp): find creature handle at tile.
 * Returns creature handle or -1/0xFFFF if none. */
typedef int16_t (*DM2_V1_GetCreatureAtFn)(void *ctx,
                                           int16_t x, int16_t y,
                                           int16_t map_level);

/* CREATURE_CAN_HANDLE_ITEM_IN (c_ai.cpp): check if creature can use item.
 * Returns -2 if cannot handle, other value if can. */
typedef int16_t (*DM2_V1_CreatureCanHandleItemInFn)(void *ctx,
                                                      int16_t item_type,
                                                      uint16_t possession_w00,
                                                      int32_t mask);

/* QUERY_CREATURE_AI_SPEC_FLAGS (c_creature.cpp): get AI spec flags word. */
typedef uint16_t (*DM2_V1_QueryCreatureAiSpecFlagsFn)(void *ctx,
                                                        uint16_t creature_handle);

/* CALC_VECTOR_DIR: compute direction from source to target.
 * Returns direction 0-3. */
typedef int16_t (*DM2_V1_CalcVectorDirFn)(void *ctx,
                                            int16_t src_x, int16_t src_y,
                                            int16_t dst_x, int16_t dst_y);

/* 19f0_0559: rotate creature facing (c_ai.cpp). */
typedef void (*DM2_V1_RotateCreatureFn)(void *ctx,
                                          int16_t creature_handle,
                                          int16_t new_direction);

/* 19f0_2165: launch projectile or effect. Returns result. */
typedef int16_t (*DM2_V1_LaunchProjectileFn)(void *ctx,
                                               int16_t x, int16_t y,
                                               int16_t direction,
                                               uint16_t item_handle);

/* 14cd_2662: evaluate creature combat state. Returns evaluation code. */
typedef int16_t (*DM2_V1_EvalCombatStateFn)(void *ctx,
                                              int16_t creature_handle,
                                              int16_t x, int16_t y);

/* 14cd_2886: compute attack parameters. */
typedef int16_t (*DM2_V1_ComputeAttackParamsFn)(void *ctx,
                                                  int16_t creature_handle,
                                                  int16_t target_handle);

/* query_48ae_0767: query map/tile properties. */
typedef int16_t (*DM2_V1_QueryMapTileFn)(void *ctx,
                                           int16_t x, int16_t y,
                                           int16_t map_level);

/* 1c9a_078b: patrol waypoint lookup. */
typedef int16_t (*DM2_V1_PatrolWaypointFn)(void *ctx,
                                             int16_t v1e07d8_w04,
                                             int16_t index);

/* 1c9a_381c: navigation path compute. */
typedef int16_t (*DM2_V1_NavigatePathFn)(void *ctx,
                                           int16_t src_x, int16_t src_y,
                                           int16_t dst_x, int16_t dst_y);

/* ai_14cd_10d2: use item AI helper. */
typedef int16_t (*DM2_V1_UseItemAiFn)(void *ctx,
                                        int16_t creature_handle,
                                        int16_t item_handle);

/* ai_14cd_0f3c: follow target AI helper. */
typedef int16_t (*DM2_V1_FollowTargetAiFn)(void *ctx,
                                             int16_t creature_handle,
                                             int16_t target_x,
                                             int16_t target_y);

/* FIND_WALK_PATH: compute walk path to target. */
typedef int16_t (*DM2_V1_FindWalkPathFn)(void *ctx,
                                           int16_t src_x, int16_t src_y,
                                           int16_t dst_x, int16_t dst_y,
                                           int16_t *out_path);

/* 19f0_2813: ranged attack helper. */
typedef int16_t (*DM2_V1_RangedAttackFn)(void *ctx,
                                           int16_t x, int16_t y,
                                           int16_t direction,
                                           int16_t attack_type);

/* 19f0_0d10: ranged special attack helper. */
typedef int16_t (*DM2_V1_RangedSpecialFn)(void *ctx,
                                            int16_t x, int16_t y,
                                            int16_t direction,
                                            int16_t attack_type);

/* 19f0_0891: item activation helper. */
typedef int16_t (*DM2_V1_ActivateItemFn)(void *ctx,
                                           int16_t creature_handle,
                                           int16_t item_handle,
                                           int16_t x, int16_t y);

/* map_0cee_04e5: map query helper. */
typedef int16_t (*DM2_V1_MapQueryFn)(void *ctx,
                                       int16_t x, int16_t y,
                                       int16_t map_level);

/* GET_ADDRESS_OF_RECORD: resolve record handle to pointer. */
typedef void *(*DM2_V1_GetAddressOfRecordFn)(void *ctx,
                                               uint16_t record_handle);

/* QUERY_GDAT_FOOD_VALUE_FROM_RECORD: get food value of item. */
typedef int16_t (*DM2_V1_QueryFoodValueFn)(void *ctx,
                                             uint16_t record_handle);

/* CUT_RECORD_FROM / DEALLOC_RECORD: record management. */
typedef void (*DM2_V1_CutRecordFn)(void *ctx, uint16_t record_handle);
typedef void (*DM2_V1_DeallocRecordFn)(void *ctx, uint16_t record_handle);

/* ALLOC_NEW_DBITEM: allocate a new database item record. */
typedef uint16_t (*DM2_V1_AllocNewDbitemFn)(void *ctx, int16_t item_type);

/* RAND16 / RANDBIT / RANDDIR: random number callbacks. */
typedef uint16_t (*DM2_V1_Rand16Fn)(void *ctx);
typedef int16_t (*DM2_V1_RandBitFn)(void *ctx);
typedef int16_t (*DM2_V1_RandDirFn)(void *ctx);

/* Coin wallet operations for commerce. */
typedef int16_t (*DM2_V1_CountByCoinTypesFn)(void *ctx,
                                               uint16_t wallet_handle,
                                               int16_t coin_type);
typedef int16_t (*DM2_V1_TakeCoinFromWalletFn)(void *ctx,
                                                 uint16_t wallet_handle,
                                                 int16_t coin_type);
typedef void (*DM2_V1_AddCoinToWalletFn)(void *ctx,
                                           uint16_t wallet_handle,
                                           uint16_t coin_handle);

/* ================================================================== */
/* Direction delta tables (table1d27fc, table1d2804 from c_ai.cpp)     */
/* ================================================================== */

/* dx deltas indexed by direction 0-3: N=0,E=1,S=2,W=3 */
static const int16_t dm2_v1_dir_delta_x[4] = { 0, 1, 0, -1 };
static const int16_t dm2_v1_dir_delta_y[4] = { -1, 0, 1, 0 };

/* ================================================================== */
/* XACT 56: PROCEED_XACT_56 — Move Forward (c_ai.cpp:78-83)           */
/* ================================================================== */

typedef struct {
    int16_t pos_x;             /* s350.v1e0562 creature x */
    int16_t pos_y;             /* s350.v1e0562 creature y */
    int16_t direction;         /* s350.creatures->w_0e direction bits 14-15 */
    DM2_V1_CreatureGoThereFn go_there;
    void *ctx;
} DM2_V1_XactMoveForwardRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -4 blocked */
} DM2_V1_XactMoveForwardReceipt;

int dm2_v1_xact_move_forward(const DM2_V1_XactMoveForwardRequest *req,
                              DM2_V1_XactMoveForwardReceipt *receipt);

/* ================================================================== */
/* XACT 57: PROCEED_XACT_57 — Random Turn (c_ai.cpp:86-103)           */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e direction */
    int16_t rand_b1e;          /* random bit: 0 or 1 */
    DM2_V1_CreatureGoThereFn go_there;
    DM2_V1_RotateCreatureFn rotate_creature;
    void *ctx;
} DM2_V1_XactRandomTurnRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -4 blocked */
    int16_t new_direction;     /* direction after turn */
    int moved;                 /* 1 if creature moved */
} DM2_V1_XactRandomTurnReceipt;

int dm2_v1_xact_random_turn(const DM2_V1_XactRandomTurnRequest *req,
                              DM2_V1_XactRandomTurnReceipt *receipt);

/* ================================================================== */
/* XACT 59/76: PROCEED_XACT_59 — Move To Target (c_ai.cpp:106-116)    */
/* ================================================================== */

typedef struct {
    int16_t v1e0572;           /* AI spec field */
    int16_t v1e0574;           /* AI spec field */
    int16_t v1e07d8_w04;       /* s350.v1e07d8.w_04 */
    uint16_t possession_w00;   /* s350.v1e054e->possession.w_00 */
    int16_t target_x;          /* s350.creatures->w_18 target x */
    int16_t target_y;          /* s350.creatures->w_18 target y */
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    DM2_V1_LaunchProjectileFn launch_projectile;
    void *ctx;
} DM2_V1_XactMoveToTargetRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -3 fail */
    int16_t v1e056f;           /* updated v1e056f value */
} DM2_V1_XactMoveToTargetReceipt;

int dm2_v1_xact_move_to_target(const DM2_V1_XactMoveToTargetRequest *req,
                                DM2_V1_XactMoveToTargetReceipt *receipt);

/* ================================================================== */
/* XACT 63: PROCEED_XACT_63 — Cast Spell (c_ai.cpp:295-330)           */
/* ================================================================== */

typedef struct {
    int16_t v1e0572;
    int16_t v1e0574;
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e */
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    void *ctx;
} DM2_V1_XactCastSpellRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -3 fail */
} DM2_V1_XactCastSpellReceipt;

int dm2_v1_xact_cast_spell(const DM2_V1_XactCastSpellRequest *req,
                             DM2_V1_XactCastSpellReceipt *receipt);

/* ================================================================== */
/* XACT 64: PROCEED_XACT_64 — Shoot Missile (c_ai.cpp:333-361)        */
/* ================================================================== */

typedef struct {
    uint16_t possession_w00;   /* s350.v1e054e->possession.w_00 */
    int16_t v1e057c;           /* missile item type */
    int16_t v1e0572;
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e */
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    DM2_V1_LaunchProjectileFn launch_projectile;
    void *ctx;
} DM2_V1_XactShootMissileRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -3 fail */
    int16_t v1e056f;
} DM2_V1_XactShootMissileReceipt;

int dm2_v1_xact_shoot_missile(const DM2_V1_XactShootMissileRequest *req,
                                DM2_V1_XactShootMissileReceipt *receipt);

/* ================================================================== */
/* XACT 65: PROCEED_XACT_65 — Flee (c_ai.cpp:364-398)                 */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e */
    int16_t party_map;         /* current party map level */
    int16_t party_x;           /* party tile x */
    int16_t party_y;           /* party tile y */
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_QueryCreatureAiSpecFlagsFn query_ai_spec_flags;
    void *ctx;
} DM2_V1_XactFleeRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success or -4 blocked */
    int8_t b_1a;               /* assignment to creatures->b_1a */
} DM2_V1_XactFleeReceipt;

int dm2_v1_xact_flee(const DM2_V1_XactFleeRequest *req,
                      DM2_V1_XactFleeReceipt *receipt);

/* ================================================================== */
/* XACT 66: PROCEED_XACT_66 — Attack Ranged (c_ai.cpp:519-568)        */
/* ================================================================== */

typedef struct {
    int16_t v1e0572;
    int16_t direction;         /* s350.creatures->w_0e */
    int8_t b_1a;               /* s350.creatures->b_1a */
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_EvalCombatStateFn eval_combat_state;
    void *ctx;
    /* Internally chains to PROCEED_XACT_63 */
} DM2_V1_XactAttackRangedRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int8_t b_1a;               /* updated b_1a */
    int state_changed;         /* 1 if creature state was modified */
} DM2_V1_XactAttackRangedReceipt;

int dm2_v1_xact_attack_ranged(const DM2_V1_XactAttackRangedRequest *req,
                                DM2_V1_XactAttackRangedReceipt *receipt);

/* ================================================================== */
/* XACT 67: PROCEED_XACT_67 — Combat Evaluation (c_ai.cpp:571-742)    */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e */
    int16_t map_level;
    int16_t v1e0572;
    int16_t v1e0574;
    int8_t b_1a;               /* s350.creatures->b_1a */
    int16_t w_0c;              /* s350.creatures->w_0c */
    int16_t w_10;              /* s350.creatures->w_10 */
    int16_t party_x;
    int16_t party_y;
    DM2_V1_EvalCombatStateFn eval_combat_state;
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_ComputeAttackParamsFn compute_attack_params;
    DM2_V1_QueryMapTileFn query_map_tile;
    DM2_V1_Rand16Fn rand16;
    DM2_V1_RandDirFn randdir;
    void *ctx;
} DM2_V1_XactCombatEvaluationRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int8_t b_1a;               /* updated b_1a */
    int16_t w_0e;              /* updated direction */
    int16_t w_0c;              /* updated w_0c */
    int16_t w_10;              /* updated w_10 */
} DM2_V1_XactCombatEvaluationReceipt;

int dm2_v1_xact_combat_evaluation(const DM2_V1_XactCombatEvaluationRequest *req,
                                    DM2_V1_XactCombatEvaluationReceipt *receipt);

/* ================================================================== */
/* XACT 68: PROCEED_XACT_68 — Guard Position (c_ai.cpp:745-825)       */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;
    int16_t map_level;
    int16_t v1e0572;
    int16_t v1e0574;
    int8_t b_1a;
    int16_t party_x;
    int16_t party_y;
    DM2_V1_EvalCombatStateFn eval_combat_state;
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_ComputeAttackParamsFn compute_attack_params;
    DM2_V1_QueryMapTileFn query_map_tile;
    void *ctx;
} DM2_V1_XactGuardPositionRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int8_t b_1a;
    int16_t w_0e;
    int state_changed;
} DM2_V1_XactGuardPositionReceipt;

int dm2_v1_xact_guard_position(const DM2_V1_XactGuardPositionRequest *req,
                                 DM2_V1_XactGuardPositionReceipt *receipt);

/* ================================================================== */
/* XACT 69: PROCEED_XACT_69 — Emit Sound (c_ai.cpp:828-840)           */
/* ================================================================== */
/* Fully implementable: computes target from direction + delta tables,
 * sets b_1d sound type and b_1a to 1. */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;         /* s350.creatures->w_0e */
    int16_t v1e0572;           /* sound parameter */
    int16_t w_18_x;            /* s350.creatures->w_18 target x (input) */
    int16_t w_18_y;            /* s350.creatures->w_18 target y (input) */
} DM2_V1_XactEmitSoundRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t target_x;          /* computed target x = pos_x + dx[dir] */
    int16_t target_y;          /* computed target y = pos_y + dy[dir] */
    int8_t b_1d;               /* sound type from v1e0572 */
    int8_t b_1a;               /* always set to 1 */
} DM2_V1_XactEmitSoundReceipt;

int dm2_v1_xact_emit_sound(const DM2_V1_XactEmitSoundRequest *req,
                             DM2_V1_XactEmitSoundReceipt *receipt);

/* ================================================================== */
/* XACT 70: PROCEED_XACT_70 — Face Party (c_ai.cpp:843-894)           */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;
    int16_t v1e0572;
    int16_t w_18_x;            /* s350.creatures->w_18 */
    int16_t w_18_y;
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    void *ctx;
} DM2_V1_XactFacePartyRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int16_t new_direction;
    int state_changed;
} DM2_V1_XactFacePartyReceipt;

int dm2_v1_xact_face_party(const DM2_V1_XactFacePartyRequest *req,
                             DM2_V1_XactFacePartyReceipt *receipt);

/* ================================================================== */
/* XACT 71: PROCEED_XACT_71 — Patrol (c_ai.cpp:897-946)               */
/* ================================================================== */

typedef struct {
    int16_t v1e0574;
    int16_t v1e0572;
    int16_t v1e07d8_w04;       /* s350.v1e07d8.w_04 */
    int16_t v1e07d8_w06;       /* s350.v1e07d8.w_06 */
    uint16_t possession_w00;
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_PatrolWaypointFn patrol_waypoint;
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    DM2_V1_LaunchProjectileFn launch_projectile;
    void *ctx;
} DM2_V1_XactPatrolRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
} DM2_V1_XactPatrolReceipt;

int dm2_v1_xact_patrol(const DM2_V1_XactPatrolRequest *req,
                         DM2_V1_XactPatrolReceipt *receipt);

/* ================================================================== */
/* XACT 73: PROCEED_XACT_73 — Manage Flags (c_ai.cpp:958-1064)        */
/* ================================================================== */

typedef struct {
    int16_t v1e0574;
    int16_t v1e0572;
    int16_t v1e054e_w0a;       /* s350.v1e054e->w_0a */
    int16_t v1e07d8_xp0a;     /* s350.v1e07d8.xp_0a */
} DM2_V1_XactManageFlagsRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int16_t w_0a;              /* updated w_0a value */
    int8_t b_1a;               /* updated b_1a */
} DM2_V1_XactManageFlagsReceipt;

int dm2_v1_xact_manage_flags(const DM2_V1_XactManageFlagsRequest *req,
                               DM2_V1_XactManageFlagsReceipt *receipt);

/* ================================================================== */
/* XACT 74: PROCEED_XACT_74 — Navigate (c_ai.cpp:1067-1174)           */
/* ================================================================== */

typedef struct {
    int16_t v1e0552;           /* s350.v1e0552 */
    int16_t v1e054e_w0a;       /* s350.v1e054e->w_0a */
    int16_t v1e054e_w0e;       /* s350.v1e054e->w_0e */
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;
    int16_t party_x;
    int16_t party_y;
    int16_t party_map;
    DM2_V1_NavigatePathFn navigate_path;
    DM2_V1_CreatureGoThereFn go_there;
    DM2_V1_CalcVectorDirFn calc_vector_dir;
    DM2_V1_RotateCreatureFn rotate_creature;
    DM2_V1_RandBitFn randbit;
    void *ctx;
} DM2_V1_XactNavigateRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int8_t b_1a;
} DM2_V1_XactNavigateReceipt;

int dm2_v1_xact_navigate(const DM2_V1_XactNavigateRequest *req,
                           DM2_V1_XactNavigateReceipt *receipt);

/* ================================================================== */
/* XACT 75: PROCEED_XACT_75 — Use Item (c_ai.cpp:1470-1495)           */
/* ================================================================== */

typedef struct {
    int16_t v1e07d8_w04;
    int16_t v1e07d8_w06;
    int16_t v1e0578;           /* item handle */
    int16_t w_18_x;            /* s350.creatures->w_18 target x */
    int16_t w_18_y;            /* s350.creatures->w_18 target y */
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_UseItemAiFn use_item_ai;
    DM2_V1_ActivateItemFn activate_item;
    void *ctx;
} DM2_V1_XactUseItemRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int16_t v1e056f;
} DM2_V1_XactUseItemReceipt;

int dm2_v1_xact_use_item(const DM2_V1_XactUseItemRequest *req,
                           DM2_V1_XactUseItemReceipt *receipt);

/* ================================================================== */
/* XACT 77: PROCEED_XACT_77 — Follow (c_ai.cpp:1569-1608)             */
/* ================================================================== */

typedef struct {
    int16_t v1e07d8_xp0a;     /* s350.v1e07d8.xp_0a */
    int16_t v1e0562_x;         /* creature x */
    int16_t v1e0562_y;         /* creature y (from v1e0562 pair) */
    int16_t v1e0674;           /* follow target reference */
    DM2_V1_FollowTargetAiFn follow_target_ai;
    DM2_V1_FindWalkPathFn find_walk_path;
    void *ctx;
} DM2_V1_XactFollowRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int16_t v1e0675;           /* updated follow state */
} DM2_V1_XactFollowReceipt;

int dm2_v1_xact_follow(const DM2_V1_XactFollowRequest *req,
                         DM2_V1_XactFollowReceipt *receipt);

/* ================================================================== */
/* XACT 78: PROCEED_XACT_78 — Face Direction (c_ai.cpp:1611-1626)     */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t party_x;
    int16_t party_y;
    int16_t party_map;
    DM2_V1_CalcVectorDirFn calc_vector_dir;
    DM2_V1_MapQueryFn map_query;
    DM2_V1_RotateCreatureFn rotate_creature;
    void *ctx;
} DM2_V1_XactFaceDirectionRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t v1e056f;           /* computed direction result */
} DM2_V1_XactFaceDirectionReceipt;

int dm2_v1_xact_face_direction(const DM2_V1_XactFaceDirectionRequest *req,
                                 DM2_V1_XactFaceDirectionReceipt *receipt);

/* ================================================================== */
/* XACT 79: PROCEED_XACT_79 — Wander Setup (c_ai.cpp:1629-1642)       */
/* ================================================================== */
/* Fully implementable: uses only random values to set creature fields. */

typedef struct {
    int16_t rand_b1e;          /* random value for b_1e (RANDBIT input) */
    int16_t rand_bit;          /* RANDBIT() result: 0 or 1 */
    int16_t rand_b1b;          /* random value for b_1b (RANDDIR input) */
    int16_t rand_dir;          /* RANDDIR() result: 0-3 */
    int16_t rand_b1c;          /* random value for b_1c */
    int16_t rand_b20;          /* random value for b_20 */
} DM2_V1_XactWanderSetupRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t b_1e;               /* wander counter */
    int8_t b_1a;               /* always set to specific value */
    int8_t b_1b;               /* wander x offset */
    int8_t b_1c;               /* wander y offset */
    int8_t b_20;               /* wander flags */
} DM2_V1_XactWanderSetupReceipt;

int dm2_v1_xact_wander_setup(const DM2_V1_XactWanderSetupRequest *req,
                               DM2_V1_XactWanderSetupReceipt *receipt);

/* ================================================================== */
/* XACT 80: PROCEED_XACT_80 — Move Flagged (c_ai.cpp:1645-1669)       */
/* ================================================================== */

typedef struct {
    int16_t v1e0572;
    int16_t v1e0576;           /* movement flags */
    int16_t direction;         /* s350.creatures->w_0e */
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_CreatureGoThereFn go_there;
    void *ctx;
} DM2_V1_XactMoveFlaggedRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int16_t v1e056f;
} DM2_V1_XactMoveFlaggedReceipt;

int dm2_v1_xact_move_flagged(const DM2_V1_XactMoveFlaggedRequest *req,
                               DM2_V1_XactMoveFlaggedReceipt *receipt);

/* ================================================================== */
/* XACT 81: PROCEED_XACT_81 — Ranged Attack (c_ai.cpp:1672-1676)      */
/* ================================================================== */

typedef struct {
    int16_t v1e07d8_w04;
    int16_t v1e07d8_w06;
    int16_t pos_x;
    int16_t pos_y;
    int16_t w_18_x;            /* s350.creatures->w_18 target x */
    int16_t w_18_y;            /* s350.creatures->w_18 target y */
    DM2_V1_RangedAttackFn ranged_attack;
    void *ctx;
} DM2_V1_XactRangedAttackRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t v1e056f;
} DM2_V1_XactRangedAttackReceipt;

int dm2_v1_xact_ranged_attack(const DM2_V1_XactRangedAttackRequest *req,
                                DM2_V1_XactRangedAttackReceipt *receipt);

/* ================================================================== */
/* XACT 82: PROCEED_XACT_82 — Commerce (c_ai.cpp:1805-1936)           */
/* ================================================================== */

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    int16_t direction;
    int16_t v1e0572;
    int16_t v1e0574;
    uint16_t possession_w00;
    int16_t party_x;
    int16_t party_y;
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_CreatureCanHandleItemInFn can_handle_item;
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    void *ctx;
} DM2_V1_XactCommerceRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int state_changed;
} DM2_V1_XactCommerceReceipt;

int dm2_v1_xact_commerce(const DM2_V1_XactCommerceRequest *req,
                           DM2_V1_XactCommerceReceipt *receipt);

/* ================================================================== */
/* XACT 83: PROCEED_XACT_83 — Sleep Check (c_ai.cpp:1939-1964)        */
/* ================================================================== */
/* Mostly implementable: checks v1e054e->w_0a flags and v1e0572. */

typedef struct {
    int16_t v1e054e_w0a;       /* s350.v1e054e->w_0a flags */
    int16_t v1e0572;           /* sleep threshold / parameter */
} DM2_V1_XactSleepCheckRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;            /* -2 success (sleep) or -3 fail (awake) */
    int8_t b_1a;               /* updated b_1a */
} DM2_V1_XactSleepCheckReceipt;

int dm2_v1_xact_sleep_check(const DM2_V1_XactSleepCheckRequest *req,
                              DM2_V1_XactSleepCheckReceipt *receipt);

/* ================================================================== */
/* XACT 84: PROCEED_XACT_84 — Consume Item (c_ai.cpp:1967-2075)       */
/* ================================================================== */

typedef struct {
    uint16_t possession_w00;
    int16_t v1e0571;           /* item category / filter */
    int16_t pos_x;
    int16_t pos_y;
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_QueryFoodValueFn query_food_value;
    DM2_V1_LaunchProjectileFn launch_projectile;
    DM2_V1_CutRecordFn cut_record;
    DM2_V1_DeallocRecordFn dealloc_record;
    DM2_V1_Rand16Fn rand16;
    void *ctx;
} DM2_V1_XactConsumeItemRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int item_consumed;         /* 1 if an item was consumed */
    int record_deallocated;    /* 1 if record was freed */
} DM2_V1_XactConsumeItemReceipt;

int dm2_v1_xact_consume_item(const DM2_V1_XactConsumeItemRequest *req,
                               DM2_V1_XactConsumeItemReceipt *receipt);

/* ================================================================== */
/* XACT 89: PROCEED_XACT_89 — Ranged Special (c_ai.cpp:2129-2133)     */
/* ================================================================== */

typedef struct {
    int16_t v1e07d8_w04;
    int16_t v1e07d8_w06;
    int16_t pos_x;
    int16_t pos_y;
    int16_t w_18_x;            /* s350.creatures->w_18 target x */
    int16_t w_18_y;            /* s350.creatures->w_18 target y */
    DM2_V1_RangedSpecialFn ranged_special;
    void *ctx;
} DM2_V1_XactRangedSpecialRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t v1e056f;
} DM2_V1_XactRangedSpecialReceipt;

int dm2_v1_xact_ranged_special(const DM2_V1_XactRangedSpecialRequest *req,
                                 DM2_V1_XactRangedSpecialReceipt *receipt);

/* ================================================================== */
/* Commerce helper: rebalance_coin_wallet (c_ai.cpp:1679-1802)         */
/* ================================================================== */

typedef struct {
    uint16_t wallet_handle;    /* creature wallet/container handle */
    int16_t target_coin_type;  /* desired coin denomination */
    int16_t target_count;      /* desired count of target denomination */
    DM2_V1_CountByCoinTypesFn count_by_coin_types;
    DM2_V1_TakeCoinFromWalletFn take_coin_from_wallet;
    DM2_V1_DeallocRecordFn dealloc_record;
    DM2_V1_AllocNewDbitemFn alloc_new_dbitem;
    DM2_V1_AddCoinToWalletFn add_coin_to_wallet;
    void *ctx;
} DM2_V1_RebalanceCoinWalletRequest;

typedef struct {
    int valid;
    int fail_closed;
    int rebalanced;            /* 1 if wallet was modified */
    int coins_removed;         /* count of coins removed */
    int coins_added;           /* count of coins added */
} DM2_V1_RebalanceCoinWalletReceipt;

int dm2_v1_rebalance_coin_wallet(const DM2_V1_RebalanceCoinWalletRequest *req,
                                   DM2_V1_RebalanceCoinWalletReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_XACT_HANDLERS_PC34_COMPAT_H */
