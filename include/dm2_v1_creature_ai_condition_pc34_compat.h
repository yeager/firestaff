#ifndef FIRESTAFF_DM2_V1_CREATURE_AI_CONDITION_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_AI_CONDITION_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_condition_pc34_compat.h — DM2 AI condition evaluator
 * and action dispatch helpers.
 *
 * DM2_14cd_1316 (c_ai.cpp:2516-3132): 23-case condition evaluator.
 * DM2_14cd_18f2 (c_ai.cpp:3135-3207): hexe table walker.
 * DM2_ai_14cd_0f3c (c_ai.cpp:1498-1566): action entry creator.
 * DM2_14cd_0f0a (c_ai.cpp:3841-3930): AI action dispatch table.
 * DM2_14cd_19a4..1fa7: ~20 wrapper functions collapsed into one.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* s_hexe record — 14-byte AI behavior entry (xtypes.h:70-83)         */
/* ------------------------------------------------------------------ */

typedef struct {
    int8_t  b_00;   /* @0: action priority / type selector */
    int8_t  b_01;   /* @1: condition byte (fed to DM2_14cd_1316) */
    int16_t w_02;   /* @2: condition parameter (distance, threshold) */
    int16_t w_04;   /* @4: item type / extra param */
    int16_t w_06;   /* @6: mask / range param */
    int8_t  b_08;   /* @8: attack strength / eye distance */
    int8_t  b_09;   /* @9: attack strength secondary */
    int8_t  b_0a;   /* @a */
    int8_t  b_0b;   /* @b */
    int8_t  b_0c;   /* @c: group/category byte */
    int8_t  b_0d;   /* @d: continuation flag (0 = last entry) */
} DM2_V1_HexeEntry;

/* ------------------------------------------------------------------ */
/* DM2_14cd_1316 — AI condition evaluator (c_ai.cpp:2516-3132)        */
/* ------------------------------------------------------------------ */

/* Callback: compute vector direction from (x1,y1) to (x2,y2).
 * Returns direction 0-3. */
typedef int16_t (*DM2_V1_CalcVectorDirFn)(void *ctx,
    uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* Callback: compute square distance between two points. */
typedef int16_t (*DM2_V1_CalcSquareDistanceFn)(void *ctx,
    uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* Callback: line-of-sight check. Returns nonzero if clear. */
typedef int32_t (*DM2_V1_LineOfSightFn)(void *ctx,
    int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/* Callback: CREATURE_CAN_HANDLE_ITEM_IN(item_type, possession, mask).
 * Returns -2 if cannot handle. */
typedef int16_t (*DM2_V1_CondCreatureCanHandleItemFn)(void *ctx,
    int16_t item_type, uint16_t possession, int32_t mask);

/* Callback: CREATURE_CAN_HANDLE_IT(item_handle, category).
 * Returns nonzero if creature can handle. */
typedef int32_t (*DM2_V1_CreatureCanHandleItFn)(void *ctx,
    int32_t item_handle, int32_t category);

/* Callback: GET_PLAYER_AT_POSITION(slot). Returns player index or -1. */
typedef int16_t (*DM2_V1_GetPlayerAtPositionFn)(void *ctx, int16_t slot);

/* Callback: GET_CREATURE_AT(x, y). Returns creature handle or -1. */
typedef int16_t (*DM2_V1_CondGetCreatureAtFn)(void *ctx,
    uint16_t x, uint16_t y);

/* Callback: GET_TILE_VALUE(x, y). Returns tile byte. */
typedef int32_t (*DM2_V1_GetTileValueFn)(void *ctx, int32_t x, int32_t y);

/* Callback: sound detection check DM2_19f0_045a. */
typedef void (*DM2_V1_SoundDetectFn)(void *ctx, int32_t x, int32_t y);

/* Callback: sound threshold DM2_1c9a_1b16(threshold, creature_hearing).
 * Returns 0 if heard. */
typedef int16_t (*DM2_V1_SoundThresholdFn)(void *ctx,
    int32_t threshold, int32_t hearing);

/* Callback: DM2_19f0_0d10 door passability check. Returns nonzero if passable. */
typedef int32_t (*DM2_V1_DoorCheckFn)(void *ctx,
    int32_t mode, int32_t x, int32_t y, int32_t dir, int16_t p1, int16_t p2);

/* Party hero item data for case 8 */
typedef struct {
    int16_t item_hand0;  /* party.hero[n].item[0], -1 if empty */
    int16_t item_hand1;  /* party.hero[n].item[1], -1 if empty */
} DM2_V1_HeroItems;

/* Creature SPX record fields needed by conditions */
typedef struct {
    uint16_t w_02;    /* possession word at offset 2 */
    int16_t  w_06;    /* HP at offset 6 */
    uint16_t w_08;    /* hearing at offset 8 */
    uint16_t w_0a;    /* flags at offset 0xa */
    uint16_t w_0c;    /* home position packed: bits 0-4=x, 5-9=y, 10-15=map */
    uint16_t w_0e;    /* direction/flags at offset 0xe */
} DM2_V1_CondCreatureSPX;

/* Creature c_creature record fields needed by conditions */
typedef struct {
    int8_t   b_12;    /* creature sub-type at offset 0x12 */
    uint16_t w_0e;    /* word at offset 0x0e for waypoint lookup */
    /* For case 21: array of waypoint words at offsets 0x0e + 2*n */
    const uint16_t *waypoint_words; /* pointer to array of w_0e values, NULL if unavailable */
    int      waypoint_count;        /* number of entries */
} DM2_V1_CondCreatureRec;

/* AI spec fields for creature type (from QUERY_CREATURE_AI_SPEC) */
typedef struct {
    uint8_t flags_byte0;  /* byte at offset 0 — bit 0 = friendly */
} DM2_V1_CondAiSpec;

typedef struct {
    /* Condition input — the raw byte and param from hexe entry */
    uint8_t condition_byte;  /* full byte: bits 6=subtype check, bit 7=invert, 0-5=case */
    int16_t condition_param; /* w_02 from hexe entry (distance/threshold/bit index) */
    int8_t  creature_subtype; /* ebxl low byte — creature sub-type for bit 0x40 check */

    /* Creature position */
    uint8_t creature_x;      /* s350.v1e0562.getxA() */
    uint8_t creature_y;      /* s350.v1e0562.getyA() */
    uint8_t creature_map;    /* s350.v1e0571 */

    /* Party position */
    uint16_t party_x;        /* ddat.v1e08d8 */
    uint16_t party_y;        /* ddat.v1e08d4 */
    uint16_t party_map;      /* ddat.v1e08d6 */
    uint16_t party_facing;   /* ddat.v1e08da */

    /* Creature records */
    DM2_V1_CondCreatureSPX spx;
    DM2_V1_CondCreatureRec creature_rec;

    /* AI spec max HP for HP percentage (case 14) */
    uint16_t ai_spec_max_hp; /* word at s350.v1e0552 + 4 */

    /* Global flags */
    int16_t  v1e058d;        /* timing flag for case 4 */
    uint16_t v1e057a;        /* door-related flags for case 11 */
    uint16_t v1e0976;        /* LOS blocker flag for case 1/22 */
    uint16_t starting_map;   /* ddat.v1e0266 for case 18/19/20 */
    uint16_t sound_flags;    /* ddat.v1e08ae for case 17 */

    /* Party hero items for case 8 */
    DM2_V1_HeroItems hero_items[4]; /* up to 4 party slots */

    /* Direction delta tables for case 10/11/12 (table1d27fc, table1d2804) */
    const int16_t *dx_table;  /* table1d27fc[4], NULL to skip */
    const int16_t *dy_table;  /* table1d2804[4], NULL to skip */

    /* Callbacks — NULL means that case fails closed */
    DM2_V1_CalcVectorDirFn calc_vector_dir;
    DM2_V1_CalcSquareDistanceFn calc_square_distance;
    DM2_V1_LineOfSightFn line_of_sight;
    DM2_V1_CondCreatureCanHandleItemFn can_handle_item;
    DM2_V1_CreatureCanHandleItFn can_handle_it;
    DM2_V1_GetPlayerAtPositionFn get_player_at_position;
    DM2_V1_CondGetCreatureAtFn get_creature_at;
    DM2_V1_GetTileValueFn get_tile_value;
    DM2_V1_SoundDetectFn sound_detect;
    DM2_V1_SoundThresholdFn sound_threshold;
    DM2_V1_DoorCheckFn door_check;
    void *cb_ctx;
} DM2_V1_AiConditionRequest;

typedef struct {
    int valid;
    int fail_closed;
    int condition_case;     /* 0-22 case that was evaluated */
    int inverted;           /* 1 if bit 0x80 caused inversion */
    int subtype_matched;    /* 1 if bit 0x40 early-exit triggered */
    int result;             /* final result: 1=true, 0=false */
    int needs_external;     /* 1 if case needed unavailable callback */
    const char *external_fn;/* name of missing callback, or NULL */
} DM2_V1_AiConditionReceipt;

int dm2_v1_ai_check_condition(
    const DM2_V1_AiConditionRequest *req,
    DM2_V1_AiConditionReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_ai_14cd_0f3c — create action entry (c_ai.cpp:1498-1566)       */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t current_entry_count;   /* s350.v1e0674 — current count */
    uint8_t creature_map;          /* s350.v1e0571 */
    uint16_t party_map;            /* ddat.v1e08d6 */
    uint8_t ai_spec_byte1;        /* byte at s350.v1e0552 + 1, bit 0x40 */
    int16_t v1e0580;               /* s350.v1e0580 mask */

    /* The hexe entry and original hexe pointer */
    const DM2_V1_HexeEntry *hexe;
    const uint8_t *hexe_raw;       /* original pointer for storage */

    /* Parameters from caller */
    int8_t  priority;              /* eaxl — action priority (b_00 of hexe) */
    int8_t  group_byte;            /* ecxl — group/category */
    int8_t  strength_adjust;       /* argb0 — strength adjustment */
    uint16_t argw1;                /* argl1 — stored at offset 0x0c */
    int8_t  arg_0e;                /* argb2 — stored at offset 0x0e */
    int8_t  arg_0f;                /* argb3 — stored at offset 0x0f */
} DM2_V1_CreateActionEntryRequest;

/* Action entry — 22 bytes, stored in s350.v1e0678[] */
typedef struct {
    int8_t  strength;       /* @0: computed attack strength */
    int8_t  b_01;           /* @1: hexe b_09 */
    int8_t  b_07;           /* @7: priority */
    int16_t w_08;           /* @8: hexe w_04 */
    int16_t w_0a;           /* @a: hexe w_06 & v1e0580 */
    uint16_t w_0c;          /* @c: argw1 */
    int8_t  b_0e;           /* @e: arg_0e */
    int8_t  b_0f;           /* @f: arg_0f */
    int8_t  b_11;           /* @11: group_byte */
    const uint8_t *hexe_ptr; /* @12: original hexe pointer */
} DM2_V1_ActionEntry;

typedef struct {
    int valid;
    int fail_closed;
    int entry_created;          /* 1 if entry was stored */
    int rejected_full;          /* 1 if rejected because count >= 16 */
    int rejected_negative;      /* 1 if computed strength < 0 */
    int16_t new_entry_count;    /* updated count */
    DM2_V1_ActionEntry entry;   /* the created entry (if entry_created) */
} DM2_V1_CreateActionEntryReceipt;

int dm2_v1_ai_create_action_entry(
    const DM2_V1_CreateActionEntryRequest *req,
    DM2_V1_CreateActionEntryReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_18f2 — hexe condition walk (c_ai.cpp:3135-3207)           */
/* ------------------------------------------------------------------ */

typedef struct {
    int8_t  walk_key;        /* eaxl — hexe.b_0c match key */
    int8_t  direction;       /* edxl — creature direction */
    const DM2_V1_HexeEntry *table; /* ebxp — hexe table start */
    int     table_count;     /* max entries to walk (safety) */
    int8_t  group_byte;      /* ecxl — group/category */
    uint16_t argw0;          /* argw0 — passed to create_action_entry */

    /* For condition checking */
    DM2_V1_AiConditionRequest cond_base; /* base condition request state */
} DM2_V1_HexeWalkRequest;

typedef struct {
    int valid;
    int fail_closed;
    int entries_checked;     /* number of hexe entries examined */
    int conditions_passed;   /* number that passed the condition */
    int actions_created;     /* number of action entries created */
} DM2_V1_HexeWalkReceipt;

int dm2_v1_ai_hexe_condition_walk(
    const DM2_V1_HexeWalkRequest *req,
    DM2_V1_HexeWalkReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0f0a — AI action dispatch (c_ai.cpp:3841-3930)            */
/* ------------------------------------------------------------------ */

typedef struct {
    uint8_t dispatch_type;    /* RG3L = eaxl & 0x1f — case 0-16 */
    int8_t  direction;        /* edxl low byte */
    int8_t  creature_type;    /* eaxl high byte */
    const uint8_t *hexe_data; /* ecxp — hexe table pointer */
} DM2_V1_ActionDispatchRequest;

typedef struct {
    int valid;
    int fail_closed;
    int dispatch_case;        /* which case 0-16 was dispatched */
} DM2_V1_ActionDispatchReceipt;

int dm2_v1_ai_action_dispatch(
    const DM2_V1_ActionDispatchRequest *req,
    DM2_V1_ActionDispatchReceipt *receipt);

/* ------------------------------------------------------------------ */
/* Unified wrapper — replaces DM2_14cd_19a4..1fa7 (~20 functions)     */
/* (c_ai.cpp:3210-3839)                                               */
/*                                                                    */
/* Each original wrapper calls one of:                                */
/*   DM2_14cd_18f2 (simple walk)                                      */
/*   DM2_14cd_19c2 (walk with timing gate)                            */
/*   DM2_14cd_1a78 (walk with ai_spec check)                          */
/*   DM2_14cd_1bac (walk with timing + flag gate)                     */
/*   DM2_14cd_1d6c (walk with item check)                             */
/*   DM2_14cd_1eec (walk with SPX w_08 override)                     */
/*   DM2_14cd_18cc (direct action, no walk)                           */
/*   DM2_14cd_1c63 (walk key=5, special argw0)                       */
/*   DM2_14cd_1c8d (walk key=6, home-position gate)                  */
/*   DM2_14cd_1cec (walk key=7, missile ref gate)                    */
/*   DM2_14cd_1d42 (walk key=18, special argw0)                      */
/*   DM2_14cd_1fa7 (walk key=22, party pos encoded)                  */
/*   DM2_14cd_1e6e (special: random enchant gate)                    */
/* ------------------------------------------------------------------ */

/* Dispatch case enum matching c_ai.cpp DM2_14cd_0f0a cases 0-16 */
typedef enum {
    DM2_AI_WRAPPER_DIRECT        = 0,   /* DM2_14cd_18cc: direct action */
    DM2_AI_WRAPPER_SIMPLE        = 1,   /* DM2_14cd_19a4: simple walk */
    DM2_AI_WRAPPER_TIMED_2       = 2,   /* DM2_14cd_1a3c: via 19c2(ecx=2,arg=1) */
    DM2_AI_WRAPPER_TIMED_4       = 3,   /* DM2_14cd_1a5a: via 19c2(ecx=4,arg=3) */
    DM2_AI_WRAPPER_AISPEC_1      = 4,   /* DM2_14cd_1b74: via 1a78(ecx=1) */
    DM2_AI_WRAPPER_AISPEC_3      = 5,   /* DM2_14cd_1b90: via 1a78(ecx=3) */
    DM2_AI_WRAPPER_FLAG_2        = 6,   /* DM2_14cd_1c27: via 1bac(ecx=2,arg=1) */
    DM2_AI_WRAPPER_FLAG_4        = 7,   /* DM2_14cd_1c45: via 1bac(ecx=4,arg=3) */
    DM2_AI_WRAPPER_SPECIAL_8     = 8,   /* DM2_14cd_1c63: walk_key=5, argw0 special */
    DM2_AI_WRAPPER_HOME_GATE     = 9,   /* DM2_14cd_1c8d: walk_key=6, home gate */
    DM2_AI_WRAPPER_MISSILE_REF   = 10,  /* DM2_14cd_1cec: walk_key=7, missile ref */
    DM2_AI_WRAPPER_SPECIAL_11    = 11,  /* DM2_14cd_1d42: walk_key=18, argw0 special */
    DM2_AI_WRAPPER_ITEM_CHECK_F  = 12,  /* DM2_14cd_1e36: via 1d6c(ecx=0xf) */
    DM2_AI_WRAPPER_ITEM_CHECK_10 = 13,  /* DM2_14cd_1e52: via 1d6c(ecx=0x10) */
    DM2_AI_WRAPPER_ENCHANT       = 14,  /* DM2_14cd_1e6e: random enchant */
    DM2_AI_WRAPPER_SPX_OVERRIDE  = 15,  /* DM2_14cd_1f8b: via 1eec(ecx=0x15) */
    DM2_AI_WRAPPER_PARTY_POS     = 16   /* DM2_14cd_1fa7: walk_key=22, party pos */
} DM2_V1_AiWrapperType;

typedef struct {
    DM2_V1_AiWrapperType type;
    int8_t  creature_type;    /* eaxl low byte */
    int8_t  direction;        /* edxl low byte */
    const uint8_t *hexe_data; /* ebxp — hexe table */
} DM2_V1_AiWrapperRequest;

typedef struct {
    int valid;
    int fail_closed;
    DM2_V1_AiWrapperType type_dispatched;
} DM2_V1_AiWrapperReceipt;

/* Fail-closed stub: validates dispatch type, documents which inner
 * function would be called. Full implementation requires live AI state. */
int dm2_v1_ai_action_wrapper(
    const DM2_V1_AiWrapperRequest *req,
    DM2_V1_AiWrapperReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_AI_CONDITION_PC34_COMPAT_H */
