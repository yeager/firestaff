/*
 * dm2_v1_creature_ai_prepare_pc34_compat.c — DM2 creature AI preparation
 * and lifecycle functions.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include "dm2_v1_creature_ai_prepare_pc34_compat.h"

#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* DM2_4EA8 — count animation frames (c_ai.cpp:5786-5814)            */
/* ------------------------------------------------------------------ */

int dm2_v1_creature_count_animation_frames(
    const uint8_t *gdat_data,
    int gdat_data_len,
    int start_offset)
{
    int offset;
    int count;
    uint8_t b;

    if (!gdat_data || gdat_data_len <= 0)
        return 1;

    /* c_ai.cpp:5799 — start at start_offset * 4 */
    offset = start_offset * 4;
    count = 1;

    /* c_ai.cpp:5803-5812 — walk 4-byte entries, count until
     * byte@1 bits 4-7 are zero */
    for (;;) {
        if (offset + 2 > gdat_data_len)
            break;
        /* c_ai.cpp:5805 — byte_at(ptr, 1) & 0xf0, then >> 4 */
        b = (gdat_data[offset + 1] & 0xf0) >> 4;
        if (b == 0)
            break;
        /* c_ai.cpp:5810-5811 */
        count++;
        offset += 4;
    }

    return count;
}

/* ------------------------------------------------------------------ */
/* DM2_2c1d_09d9 — party power level (c_ai.cpp:2466-2514)            */
/* ------------------------------------------------------------------ */

int dm2_v1_compute_party_power_level(const DM2_V1_PartySkillData *party)
{
    uint32_t sum;
    int hero;
    int skill;
    int level;

    if (!party || party->heroes_in_party <= 0)
        return 1;

    /* c_ai.cpp:2475-2503 — sum all skill values across all heroes */
    sum = 0;
    for (hero = 0; hero < party->heroes_in_party && hero < DM2_V1_MAX_HEROES; hero++) {
        for (skill = 0; skill < DM2_V1_SKILLS_PER_HERO; skill++) {
            /* c_ai.cpp:2501 — party.hero[hero].skill[skill/4][skill%4]
             * In the source the inner loop runs 0..3 (RG3 > 3 breaks) */
            sum += party->skill[hero][skill];
        }
    }

    /* c_ai.cpp:2506-2513 — floor(log2(sum/512)) + 1
     * Start with level=1, keep shifting sum right while sum >= 0x200 */
    level = 1;
    while (sum >= 0x200) {
        sum >>= 1;
        level++;
    }

    return level;
}

/* ------------------------------------------------------------------ */
/* PREPARE/UNPREPARE context (c_ai.cpp:5817-5909)                     */
/* ------------------------------------------------------------------ */

void *dm2_v1_prepare_creature_ai_context(
    const DM2_V1_PrepareCreatureContextRequest *request,
    DM2_V1_PrepareCreatureContextReceipt *receipt)
{
    if (!receipt)
        return NULL;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return NULL;
    }

    receipt->valid = 1;
    receipt->fail_closed = 1;

    /* c_ai.cpp:5817-5892 — PREPARE_LOCAL_CREATURE_VAR
     * Sets up s350 context for a creature AI tick.  Requires live
     * creature data (record pool, CAII array, AI spec table) which
     * this bounded slice does not own.
     *
     * Documented field assignments:
     *   s350.v1e07ea = 1 (context active flag)
     *   s350.v1e054c = creature_handle
     *   s350.v1e054e = GET_ADDRESS_OF_RECORD(creature_handle)
     *   s350.creatures = CAII slot (34 bytes at creatures + byte@5 * 34)
     *                    or NULL if byte@5 == 0xff (no CAII slot)
     *   s350.v1e0552 = QUERY_CREATURE_AI_SPEC_FROM_RECORD(byte@4)
     *   s350.v1e055e = query_1c9a_02c3(record, ai_spec)
     *   s350.v1e0571 = map_level (low byte)
     *   s350.v1e0562 = timer entry struct:
     *     .mticks = (map_level, gametick)
     *     .actor = byte@4 of creature record
     *     .type = timer_type
     *     .xyA = (tile_y, tile_x)
     *   s350.v1e055a = NULL
     *   s350.v1e0570 = 0
     *   s350.v1e0584 = -1
     *
     * For timer_type == 0x22 (c_ai.cpp:5880-5891):
     *   s350.v1e0572 = 0
     *   s350.v1e0574 = 0
     *   s350.v1e056e = CAII byte@0x1a (or 0 if it was -1)
     *   zero CAII bytes 0x18..0x21
     *   CAII byte@0x1a = 0xff
     *
     * Reentrant: if s350.v1e07ea != 0 on entry, allocates 0x350 bytes
     * from LOBIGPOOL, copies current s350 into it, returns save buffer.
     */
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
        "DM2_PREPARE_LOCAL_CREATURE_VAR c_ai.cpp:5817-5892 "
        "handle=%d x=%d y=%d map=%d timer_type=0x%x — fail-closed, "
        "requires live creature data (record pool, CAII, AI spec)",
        request->creature_handle, request->tile_x, request->tile_y,
        request->map_level, request->timer_type);

    return NULL;
}

int dm2_v1_unprepare_creature_ai_context(
    void *save_buffer,
    DM2_V1_UnprepareCreatureContextReceipt *receipt)
{
    if (!receipt)
        return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->fail_closed = 1;

    /* c_ai.cpp:5895-5909 — UNPREPARE_LOCAL_CREATURE_VAR
     * If save_buffer != NULL:
     *   COPY_C350(&s350, save_buffer)  — restore from saved copy
     *   DEALLOC_LOBIGPOOL(0x350)       — free save buffer
     *   receipt->restored = 1
     * Else:
     *   s350.v1e07ea = 0               — clear active flag
     *   receipt->cleared = 1
     */
    if (save_buffer) {
        receipt->restored = 1;
    } else {
        receipt->cleared = 1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* DM2_13e4_01a3 — init AI state (c_ai.cpp:2340-2412)                */
/* ------------------------------------------------------------------ */

int dm2_v1_init_creature_ai_state(
    DM2_V1_InitCreatureAiStateReceipt *receipt)
{
    if (!receipt)
        return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->fail_closed = 1;

    /* c_ai.cpp:2340-2412 — DM2_13e4_01a3
     * Requires live s350 context (set by PREPARE_LOCAL_CREATURE_VAR).
     *
     * Guard: if s350.v1e07eb != 0, return (already initialized).
     * Sets s350.v1e07eb = 1.
     *
     * If s350.v1e0584 == -1:
     *   s350.v1e0584 = QUERY_GDAT_CREATURE_WORD_VALUE(byte@4, 1)
     *
     * AI spec field loads (c_ai.cpp:2358-2367):
     *   s350.v1e0576 = ai_spec word@0x0a
     *   s350.v1e0578 = ai_spec word@0x0e
     *   s350.v1e057a = ai_spec word@0x10
     *   s350.v1e057c = ai_spec word@0x0c
     *   s350.v1e057e = ai_spec word@0x12
     *
     * If query_1c9a_08bd(creature_record) != 0:
     *   s350.v1e0576 &= 0x7fff  (clear high bit of attack types)
     *
     * s350.v1e058c = 1
     * Zero s350.v1e07ee (0xa8 bytes)
     * s350.v1e0898 = NULL
     * s350.v1e0896 = 4
     * s350.v1e07ec = 0
     *
     * s350.v1e0582 = QUERY_GDAT_CREATURE_WORD_VALUE(byte@4, 7)
     *
     * Timing flag (c_ai.cpp:2380-2398):
     *   elapsed = (gametick & 0xff) - CAII byte@4
     *   speed = ai_spec word@0x16 & 0xf
     *   threshold = 2 * (15 - speed) / 4 + 1 + RAND16(threshold+1)
     *   s350.v1e058d = (threshold <= elapsed) ? 1 : 0
     *
     * Allocation11 (c_ai.cpp:2399-2411):
     *   handle = (creature_handle & 0x3ff) | 0x20000000
     *   result = ALLOCATION11(handle, 0, &vw_00)
     *   If result == 0: zero s350.v1e07d8 (14 bytes), set b_03=-1, w_08=-1
     *   Else: copy BMP(vw_00) into s350.v1e07d8 (14 bytes)
     */
    snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
        "DM2_13e4_01a3 c_ai.cpp:2340-2412 — fail-closed, requires "
        "live s350 context (creature record, CAII slot, AI spec, GDAT)");

    return 0;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_062e — get creature action flags (c_ai.cpp:2414-2443)     */
/* ------------------------------------------------------------------ */

int dm2_v1_get_creature_action_flags(
    const uint8_t *caii_slot,
    int caii_slot_len,
    const DM2_V1_ActionTableSet *action_tables,
    int16_t creature_map,
    int16_t party_map,
    DM2_V1_GetCreatureActionFlagsReceipt *receipt)
{
    uint8_t table_index;
    uint8_t entry_index;
    int resolved_offset;
    const DM2_V1_ActionTableEntry *table_row;
    uint8_t flags_byte;

    if (!receipt)
        return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!caii_slot || caii_slot_len < 0x14 || !action_tables) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:2422-2423 — read bytes 0x12 and 0x13 from CAII slot */
    table_index = caii_slot[0x12];
    entry_index = caii_slot[0x13];

    /* c_ai.cpp:2424 — if table_index == 0xff, no action table */
    if (table_index == 0xff) {
        receipt->no_action_table = 1;
        receipt->action_flags = 0;
        return 1;
    }

    if (table_index >= action_tables->table_count || !action_tables->tables[table_index]) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:2428-2431 — resolve into table1d5f82:
     * table_row = table1d5f82[table_index] + 7 * entry_index */
    table_row = &action_tables->tables[table_index][(int)entry_index];

    /* c_ai.cpp:2431 — byte@5 & 0xe0 */
    flags_byte = table_row->bytes[5] & 0xe0;

    /* c_ai.cpp:2432-2438 — if (byte@5 & 0x60) == 0x40, check party map */
    if ((table_row->bytes[5] & 0x60) == 0x40) {
        /* c_ai.cpp:2436-2438 — if creature map != party map, clear flags */
        if (creature_map != party_map) {
            flags_byte = 0;
            receipt->party_map_mismatch = 1;
        }
    }

    receipt->action_flags = flags_byte;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Animation timing — DM2_ai_13e4_071b (c_ai.cpp:5962-5999)          */
/* ------------------------------------------------------------------ */

int dm2_v1_creature_animation_timing_4000(
    DM2_V1_CreatureAnimTimingReceipt *receipt)
{
    if (!receipt)
        return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->fail_closed = 1;

    /* c_ai.cpp:5962-5999 — DM2_ai_13e4_071b
     * Requires live s350 context.
     *
     * word2 = s350.v1e055e word@2
     * flags = word2 & 0xe03f
     * If flags == 0x8001: return (already aligned)
     *
     * start_offset = s350.v1e055e word@0
     * frame_count = DM2_4EA8(creature_type, start_offset)
     * base = word2 & 0xfc0
     * tick_sum = base + gametick
     * remainder = tick_sum % frame_count
     *
     * If remainder == 0:
     *   word@2 = base | 0x8001  (aligned)
     * Else:
     *   word@2 = base | frame_count | 0xc000
     *   Delete old timer (DM2_1c9a_0db0)
     *   Requeue with delay = frame_count - remainder
     *   Store timer ticket in CAII slot word@2
     */

    return 0;
}

/* ------------------------------------------------------------------ */
/* Animation timing — DM2_ai_13e4_0806 (c_ai.cpp:6001-6040)          */
/* ------------------------------------------------------------------ */

int dm2_v1_creature_animation_timing_2000(
    DM2_V1_CreatureAnimTimingReceipt *receipt)
{
    if (!receipt)
        return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->fail_closed = 1;

    /* c_ai.cpp:6001-6040 — DM2_ai_13e4_0806
     * Requires live s350 context.
     *
     * word2 = s350.v1e055e word@2
     * hi_flags = word2 & 0xe000
     * If hi_flags == 0x8000:
     *   lo = word2 & 0x3f
     *   If lo > 1: return (already stable)
     *
     * start_offset = s350.v1e055e word@0
     * frame_count = DM2_4EA8(creature_type, start_offset)
     * base = word2 & 0xfc0
     * tick_sum = base + gametick
     * remainder = tick_sum % frame_count
     * combined = frame_count | base
     *
     * If remainder == 0:
     *   word@2 = combined | 0x8000  (high byte |= 0x80)
     * Else:
     *   word@2 = combined | 0xa000  (high byte |= 0xa0)
     *   Delete old timer (DM2_1c9a_0db0)
     *   Requeue with delay = frame_count - remainder
     *   Store timer ticket in CAII slot word@2
     */

    return 0;
}
