#ifndef FIRESTAFF_DM2_V1_CREATURE_AI_DECISION_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_AI_DECISION_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_decision_pc34_compat.h — DM2 creature AI decision/selection.
 *
 * Ports the AI decision functions from skproject/SKULLWIN/c_ai.cpp:
 *   DM2_DECIDE_NEXT_XACT (4445-4492)
 *   DM2_14cd_0684 (4231-4374) — AI action table lookup
 *   DM2_14cd_08f5 (4375-4443) — Post-XACT result handler
 *   DM2_14cd_0389 (3931-3978) — Target validation
 *   DM2_14cd_0457 (3979-4079) — Target selection
 *   DM2_14cd_0067 (4495-4720) — Behavior selection
 *   DM2_SELECT_CREATURE_37FC (4723-4740) — Mode selection wrapper
 *   DM2_14cd_0550 (4080-4175) — Action handler invocation
 *   DM2_14cd_0276 (4176-4230) — Action context preparation
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* DM2_DECIDE_NEXT_XACT (c_ai.cpp:4445-4492)                         */
/* ------------------------------------------------------------------ */

/* A single row in the 7-byte action table (table1d5f82 entries). */
typedef struct {
    int8_t  opcode;    /* byte 0: action opcode (>=0 = terminal, -10 = set register) */
    int8_t  arg1;      /* byte 1: register index or branch on -2 */
    int8_t  arg2;      /* byte 2: register value or branch on non-(-2) */
    int8_t  arg3;      /* byte 3: xact_arg0 (v1e0572) */
    int8_t  arg4;      /* byte 4: xact_arg1 (v1e0574) */
    int8_t  arg5;      /* byte 5: used by action handler invocation */
    int8_t  arg6;      /* byte 6: used by action handler invocation */
} DM2_V1_ActionTableRow;

typedef struct {
    const DM2_V1_ActionTableRow *table;  /* action table rows */
    int table_row_count;                  /* max rows (safety bound) */
    int8_t table_index;                   /* creature byte 0x12 */
    int8_t row_index;                     /* creature byte 0x13 */
    /* Creature record words at offsets 0x0e and 0x10 for -10 opcode. */
    int16_t creature_w0e;
    int16_t creature_w10;
} DM2_V1_DecideNextXactRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t action_opcode;   /* byte 0 of matched row (>=0) */
    int8_t new_row_index;   /* updated row index (creature byte 0x13) */
    int16_t xact_arg0;      /* byte 3 of matched row -> s350.v1e0572 */
    int16_t xact_arg1;      /* byte 4 of matched row -> s350.v1e0574 */
    /* Register writes performed by -10 opcodes during walk. */
    int reg_writes;
    int16_t new_creature_w0e;
    int16_t new_creature_w10;
} DM2_V1_DecideNextXactReceipt;

int dm2_v1_decide_next_xact(const DM2_V1_DecideNextXactRequest *req,
                             DM2_V1_DecideNextXactReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_08f5 (c_ai.cpp:4375-4443) — Post-XACT result handler     */
/* ------------------------------------------------------------------ */

typedef struct {
    int8_t xact_return_code;   /* return from XACT dispatch (-2 or -3 or other) */
    int8_t table_index;        /* creature byte 0x12 */
    int8_t row_index;          /* creature byte 0x13 */
    const DM2_V1_ActionTableRow *table;
    int table_row_count;
} DM2_V1_PostXactResultRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t new_table_index;    /* updated byte 0x12 (or -1 = clear) */
    int8_t new_row_index;      /* updated byte 0x13 */
    int state_changed;         /* return value: 1 if state changed, 0 otherwise */
} DM2_V1_PostXactResultReceipt;

int dm2_v1_ai_post_xact_result(const DM2_V1_PostXactResultRequest *req,
                                DM2_V1_PostXactResultReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0389 (c_ai.cpp:3931-3978) — Target validation             */
/* ------------------------------------------------------------------ */

typedef struct {
    int8_t v1e07d8_b00;     /* s350.v1e07d8.b_00 */
    int8_t v1e07d8_b01;     /* s350.v1e07d8.b_01 */
    int8_t v1e07d8_b03;     /* s350.v1e07d8.b_03 */
    int8_t creature_b12;    /* creature byte 0x12 (table_index) */
    int8_t creature_b13;    /* creature byte 0x13 (row_index) */
    const DM2_V1_ActionTableRow *table;
    int table_row_count;
} DM2_V1_ValidateTargetRequest;

typedef struct {
    int valid;
    int fail_closed;
    int target_valid;       /* 1 if target is still valid */
    int8_t table_index;     /* matching table index or 0xff */
} DM2_V1_ValidateTargetReceipt;

int dm2_v1_ai_validate_target(const DM2_V1_ValidateTargetRequest *req,
                               DM2_V1_ValidateTargetReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0457 (c_ai.cpp:3979-4079) — Target selection              */
/* ------------------------------------------------------------------ */

/* Target candidate entry (s350.v1e0678, 0x16 bytes each). */
typedef struct {
    int8_t  priority;       /* byte 0: priority (negative = skip) */
    uint8_t pad[1];
    int16_t target_handle;  /* word at offset 2 */
    uint8_t rest[0x12];     /* remaining bytes */
} DM2_V1_TargetCandidate;

typedef struct {
    int8_t candidate_count;             /* s350.v1e0674 */
    DM2_V1_TargetCandidate *candidates; /* s350.v1e0678 array */
    int8_t v1e07d8_b00;                 /* s350.v1e07d8.b_00 */
} DM2_V1_SelectTargetRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t new_candidate_count;   /* updated s350.v1e0674 */
    int selected_index;           /* index into candidates, or -1 */
} DM2_V1_SelectTargetReceipt;

int dm2_v1_ai_select_target(const DM2_V1_SelectTargetRequest *req,
                              DM2_V1_SelectTargetReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0067 (c_ai.cpp:4495-4720) — Behavior selection            */
/* ------------------------------------------------------------------ */

/* Behavior table entry (table1d6190, 6 bytes each). */
typedef struct {
    uint16_t flags;         /* flag pattern word */
    uint8_t  data[4];       /* pointer/params (4 bytes) */
} DM2_V1_BehaviorEntry;

/* table1d607e entry (2 bytes). */
typedef struct {
    uint8_t uc[2];
} DM2_V1_CreatureModeFlags;

typedef struct {
    uint16_t ai_flags;             /* word at creature_spec+0xa */
    uint16_t random_seed;          /* from DM2_RAND() */
    const DM2_V1_BehaviorEntry *behavior_table; /* table1d6190[creature_mode] */
    int behavior_entry_count;      /* max entries (safety bound) */
    uint16_t creature_handle;      /* s350.v1e054c */
    int8_t creature_b12;           /* creature byte 0x12 */
    int8_t creature_b16;           /* creature byte 0x16 (old behavior index) */
    uint16_t v1e08d6;              /* ddat.v1e08d6 */
    int16_t v1e0584;               /* s350.v1e0584 */
    const DM2_V1_CreatureModeFlags *mode_flags; /* table1d607e */
    uint16_t spec_w06;             /* creature_spec word at 0x06 */
    int16_t spec_w04;              /* creature_spec word at 0x04 (max hp) */
    uint16_t spec_w16;             /* creature_spec word at 0x16 */
    uint16_t v1e0571;              /* s350.v1e0571 */
} DM2_V1_SelectBehaviorRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t behavior_index;        /* selected behavior group index */
    uint16_t updated_ai_flags;     /* modified ai_flags to write back */
    int8_t new_creature_b12;       /* updated byte 0x12 (-1 if mode changed) */
    int8_t new_creature_b13;       /* updated byte 0x13 (0 if mode changed) */
    int8_t new_v1e07d8_b01;        /* updated s350.v1e07d8.b_01 */
    int8_t new_v1e07d8_b03;        /* updated s350.v1e07d8.b_03 */
    int mode_changed;              /* 1 if behavior index changed from old b16 */
} DM2_V1_SelectBehaviorReceipt;

int dm2_v1_ai_select_behavior(const DM2_V1_SelectBehaviorRequest *req,
                               DM2_V1_SelectBehaviorReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_SELECT_CREATURE_37FC (c_ai.cpp:4723-4740) — Mode selection     */
/* ------------------------------------------------------------------ */

/* Callback: query GDAT creature word value. */
typedef int16_t (*DM2_V1_QueryGdatCreatureWordFn)(void *ctx,
                                                    uint8_t creature_type,
                                                    int16_t param);

typedef struct {
    int16_t v1e0584;               /* s350.v1e0584 (-1 = needs query) */
    uint8_t creature_spec_b04;     /* creature_spec byte at 0x04 */
    DM2_V1_QueryGdatCreatureWordFn query_gdat;
    void *gdat_ctx;
    /* Behavior selection inputs (forwarded to dm2_v1_ai_select_behavior) */
    DM2_V1_SelectBehaviorRequest behavior_req;
    /* Behavior table array indexed by v1e0584. */
    const DM2_V1_BehaviorEntry **behavior_tables;
    int behavior_table_count;
} DM2_V1_SelectCreatureModeRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t v1e0584;               /* resolved creature mode */
    int16_t v1e0586;               /* behavior index (from select_behavior) */
    DM2_V1_SelectBehaviorReceipt behavior_receipt;
} DM2_V1_SelectCreatureModeReceipt;

int dm2_v1_ai_select_creature_mode(const DM2_V1_SelectCreatureModeRequest *req,
                                    DM2_V1_SelectCreatureModeReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0550 (c_ai.cpp:4080-4175) — Action handler invocation     */
/* ------------------------------------------------------------------ */

typedef struct {
    const uint8_t *entry;          /* behavior entry (byte 0 = table_index) */
    int8_t target_table_index;     /* starting table_index override */
    int8_t target_row_index;       /* starting row_index override */
    int has_active_target;         /* from prior context (ecxl) */
    int v1e07ec;                   /* s350.v1e07ec */
    const DM2_V1_ActionTableRow **action_tables; /* table1d5f82 */
    int action_table_count;
} DM2_V1_InvokeActionHandlerRequest;

typedef struct {
    int valid;
    int fail_closed;
    int handler_invoked;           /* 1 if DM2_14cd_0f0a would be called */
    int8_t resolved_table_index;   /* table index used */
    int8_t resolved_row_index;     /* row index used */
    uint8_t handler_arg_byte5;     /* byte 5 from action table row */
    int8_t handler_arg_byte6;      /* byte 6 from action table row */
} DM2_V1_InvokeActionHandlerReceipt;

int dm2_v1_ai_invoke_action_handler(const DM2_V1_InvokeActionHandlerRequest *req,
                                     DM2_V1_InvokeActionHandlerReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0276 (c_ai.cpp:4176-4230) — Action context preparation    */
/* ------------------------------------------------------------------ */

typedef struct {
    const uint8_t *entry;   /* behavior table entry (0x1a bytes) */
    int entry_size;         /* size of entry */
} DM2_V1_PrepareActionContextRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t v1e07d8_b00;    /* max(0, byte_at(entry, 6)) */
    int8_t v1e07d8_b01;    /* same as b00 */
    uint16_t v1e07d8_w08;  /* word_at(entry, 4) */
    int8_t v1e07d8_b03;    /* byte_at(entry, 7) */
    uint16_t v1e07d8_w04;  /* word_at(entry, 8) */
    uint16_t v1e07d8_w06;  /* word_at(entry, 0xa) */
    int8_t v1e07d8_b02;    /* byte_at(entry, 0x11) */
    /* xp_0a = pointer_at(entry + 0x12) — not portable, documented */
    int needs_allocation;   /* 1 if b00 > 0 (memory alloc required) */
} DM2_V1_PrepareActionContextReceipt;

int dm2_v1_ai_prepare_action_context(const DM2_V1_PrepareActionContextRequest *req,
                                      DM2_V1_PrepareActionContextReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_0684 (c_ai.cpp:4231-4374) — AI action table lookup        */
/* ------------------------------------------------------------------ */

typedef struct {
    int8_t creature_b12;           /* creature byte 0x12 */
    int8_t creature_b13;           /* creature byte 0x13 */
    int16_t v1e0584;               /* s350.v1e0584 */
    const DM2_V1_CreatureModeFlags *mode_flags; /* table1d607e */
} DM2_V1_FindActionTableRequest;

typedef struct {
    int valid;
    int fail_closed;
    int8_t result_table_index;     /* table index or -3 on failure */
    int target_validated;          /* 1 if DM2_14cd_0389 returned valid */
} DM2_V1_FindActionTableReceipt;

int dm2_v1_ai_find_action_table(const DM2_V1_FindActionTableRequest *req,
                                 DM2_V1_FindActionTableReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_AI_DECISION_PC34_COMPAT_H */
