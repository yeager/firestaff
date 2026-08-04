#ifndef DM2_V1_CREATURE_AI_PREPARE_PC34_COMPAT_H
#define DM2_V1_CREATURE_AI_PREPARE_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_prepare_pc34_compat.h — DM2 creature AI preparation
 * and lifecycle functions.
 *
 * Ports PREPARE/UNPREPARE_LOCAL_CREATURE_VAR, DM2_4EA8 (animation frame
 * counting), DM2_13e4_01a3 (AI state init), DM2_14cd_062e (action flags),
 * DM2_2c1d_09d9 (party power level), and the animation timing handlers
 * DM2_ai_13e4_071b / DM2_ai_13e4_0806.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* DM2_4EA8 — count animation frames (c_ai.cpp:5786-5814)            */
/* ------------------------------------------------------------------ */

/*
 * Counts animation frames from a GDAT data pointer.  The caller queries
 * GDAT category 0xf, type creature_type, arg 7, sub 0xfc and passes the
 * result.  Each entry is 4 bytes; the walk starts at start_offset*4 and
 * counts entries until byte@1 bits 4-7 are zero.  Returns frame count
 * (always >= 1).
 */
int dm2_v1_creature_count_animation_frames(
    const uint8_t *gdat_data,
    int gdat_data_len,
    int start_offset);

/* ------------------------------------------------------------------ */
/* DM2_2c1d_09d9 — party power level (c_ai.cpp:2466-2514)            */
/* ------------------------------------------------------------------ */

/*
 * Hero skill data for power level computation.  Each hero has 4 skill
 * values (fighter, ninja, priest, wizard — indexed as skill[hero][0..3]).
 */
#define DM2_V1_MAX_HEROES 4
#define DM2_V1_SKILLS_PER_HERO 4

typedef struct {
    int heroes_in_party;  /* 0..4 */
    uint16_t skill[DM2_V1_MAX_HEROES][DM2_V1_SKILLS_PER_HERO];
} DM2_V1_PartySkillData;

/*
 * Computes party power level: sum all skill values across all heroes,
 * then compute floor(log2(sum/512)) + 1.  Returns 1 when sum < 512.
 * Pure math — no external dependencies.
 *
 * c_ai.cpp:2466-2514 (DM2_2c1d_09d9)
 */
int dm2_v1_compute_party_power_level(const DM2_V1_PartySkillData *party);

/* ------------------------------------------------------------------ */
/* PREPARE/UNPREPARE context (c_ai.cpp:5817-5909)                     */
/* ------------------------------------------------------------------ */

/*
 * The s350 creature AI context fields set by PREPARE_LOCAL_CREATURE_VAR.
 * In the source, s350 is a ~0x350-byte global struct; the prepare
 * function saves it (if reentrant) and loads new values from the
 * creature record and CAII slot.  This module defines the interface
 * but implements fail-closed until wired to live creature data.
 */

typedef struct {
    int16_t creature_handle;
    int16_t tile_x;
    int16_t tile_y;
    int16_t map_level;
    int16_t timer_type;    /* 0x21 or 0x22 */
    uint32_t game_tick;
} DM2_V1_PrepareCreatureContextRequest;

typedef struct {
    int valid;
    int fail_closed;
    int reentrant_save;    /* 1 if s350 was saved (reentrant call) */
    int creature_record_resolved;
    int caii_slot_resolved;
    int ai_spec_resolved;
    int timer_type_0x22_init;  /* extra init for timer type 0x22 */
    /* Source fields documented for wiring:
     * s350.v1e07ea = 1 (active flag)
     * s350.v1e054c = creature_handle
     * s350.v1e054e = creature record pointer
     * s350.creatures = CAII slot pointer (or NULL if byte@5 == 0xff)
     * s350.v1e0552 = AI spec pointer (from QUERY_CREATURE_AI_SPEC_FROM_RECORD)
     * s350.v1e055e = query_1c9a_02c3 result
     * s350.v1e0571 = map_level
     * s350.v1e0562 = timer entry (mticks, actor, type, xy)
     * s350.v1e055a = NULL
     * s350.v1e0570 = 0
     * s350.v1e0584 = -1
     * For timer_type 0x22:
     *   s350.v1e0572 = 0, s350.v1e0574 = 0
     *   s350.v1e056e = from CAII byte@0x1a (or 0 if -1)
     *   zero CAII bytes 0x18..0x21, set byte@0x1a = -1
     */
    char source_evidence[256];
} DM2_V1_PrepareCreatureContextReceipt;

/*
 * DM2_PREPARE_LOCAL_CREATURE_VAR (c_ai.cpp:5817-5892).
 * Fail-closed: returns NULL save buffer and receipt.fail_closed = 1.
 * The return value is the save buffer pointer (opaque, for unprepare).
 */
void *dm2_v1_prepare_creature_ai_context(
    const DM2_V1_PrepareCreatureContextRequest *request,
    DM2_V1_PrepareCreatureContextReceipt *receipt);

typedef struct {
    int valid;
    int fail_closed;
    int restored;          /* 1 if save buffer was restored */
    int cleared;           /* 1 if s350.v1e07ea was cleared (no buffer) */
} DM2_V1_UnprepareCreatureContextReceipt;

/*
 * DM2_UNPREPARE_LOCAL_CREATURE_VAR (c_ai.cpp:5895-5909).
 * If save_buffer is non-NULL, restores s350 from it and frees memory.
 * If NULL, clears s350.v1e07ea to 0.
 * Fail-closed: documents what would happen.
 */
int dm2_v1_unprepare_creature_ai_context(
    void *save_buffer,
    DM2_V1_UnprepareCreatureContextReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_13e4_01a3 — init AI state (c_ai.cpp:2340-2412)                */
/* ------------------------------------------------------------------ */

typedef struct {
    int valid;
    int fail_closed;
    int already_initialized;  /* s350.v1e07eb was already 1 */
    int gdat_word_queried;    /* QUERY_GDAT_CREATURE_WORD_VALUE called */
    int ai_spec_fields_loaded;
    int timing_flag_computed; /* s350.v1e058d set */
    /* Source fields documented:
     * s350.v1e0576 = ai_spec word@0x0a (attack types)
     * s350.v1e0578 = ai_spec word@0x0e
     * s350.v1e057a = ai_spec word@0x10
     * s350.v1e057c = ai_spec word@0x0c
     * s350.v1e057e = ai_spec word@0x12
     * s350.v1e0582 = GDAT creature word value 7
     * s350.v1e058d = timing flag (gametick vs creature speed)
     * s350.v1e07d8 = allocation11 result or zeroed
     */
    char source_evidence[256];
} DM2_V1_InitCreatureAiStateReceipt;

/*
 * DM2_13e4_01a3 (c_ai.cpp:2340-2412).
 * Fail-closed: receipt documents which fields would be set.
 */
int dm2_v1_init_creature_ai_state(
    DM2_V1_InitCreatureAiStateReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_062e — get creature action flags (c_ai.cpp:2414-2443)     */
/* ------------------------------------------------------------------ */

/*
 * Action table entry: 7-byte record from table1d5f82.
 * byte@5 holds flags; bits 5-7 (& 0xe0) are the action flags.
 */
typedef struct {
    uint8_t bytes[7];
} DM2_V1_ActionTableEntry;

typedef struct {
    /* table1d5f82: array of pointers to action table rows */
    const DM2_V1_ActionTableEntry *tables[256];
    int table_count;
} DM2_V1_ActionTableSet;

typedef struct {
    int valid;
    int fail_closed;
    uint8_t action_flags;    /* byte@5 & 0xe0 from the resolved entry */
    int no_action_table;     /* creatures byte@0x12 was 0xff */
    int party_map_mismatch;  /* party map != creature map (flag cleared) */
} DM2_V1_GetCreatureActionFlagsReceipt;

/*
 * DM2_14cd_062e (c_ai.cpp:2414-2443).
 * Reads CAII slot bytes 0x12/0x13 (table index / entry index),
 * resolves into table1d5f82, returns action flags (byte@5 & 0xe0).
 * Also checks party map (ddat.v1e08d6) vs creature map (s350.v1e0571).
 */
int dm2_v1_get_creature_action_flags(
    const uint8_t *caii_slot,
    int caii_slot_len,
    const DM2_V1_ActionTableSet *action_tables,
    int16_t creature_map,
    int16_t party_map,
    DM2_V1_GetCreatureActionFlagsReceipt *receipt);

/* ------------------------------------------------------------------ */
/* Animation timing — DM2_ai_13e4_071b (c_ai.cpp:5962-5999)          */
/* ------------------------------------------------------------------ */

typedef struct {
    int valid;
    int fail_closed;
    int already_aligned;     /* flags were already 0x8001 — early return */
    int aligned_this_tick;   /* gametick aligned, set flags 0x8001 */
    int requeued;            /* not aligned, timer requeued */
    uint16_t new_flags;      /* value written to v1e055e word@2 */
} DM2_V1_CreatureAnimTimingReceipt;

/*
 * DM2_ai_13e4_071b (c_ai.cpp:5962-5999).
 * Animation timing for 0x4000 flag case.  Queries animation frame count,
 * computes gametick alignment.  If aligned, sets flags 0x8001.  Otherwise
 * sets frame count + 0xc000 and requeues timer.
 * Fail-closed: receipt documents what would happen.
 */
int dm2_v1_creature_animation_timing_4000(
    DM2_V1_CreatureAnimTimingReceipt *receipt);

/* ------------------------------------------------------------------ */
/* Animation timing — DM2_ai_13e4_0806 (c_ai.cpp:6001-6040)          */
/* ------------------------------------------------------------------ */

/*
 * DM2_ai_13e4_0806 (c_ai.cpp:6001-6040).
 * Animation timing for 0x2000 flag case.  Similar to 071b but uses
 * 0xe000 mask and 0x8000 check, with different flag composition.
 * Fail-closed: receipt documents what would happen.
 */
int dm2_v1_creature_animation_timing_2000(
    DM2_V1_CreatureAnimTimingReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_CREATURE_AI_PREPARE_PC34_COMPAT_H */
