/* DM2 V1 runtime parity ops — full algorithmic parity with skproject.
 * Each function matches the original logic from the skproject reconstructed
 * source, with external state access delegated through callbacks. */

#include "dm2_v1_runtime_parity_pc34_compat.h"
#include <stddef.h>
#include <string.h>

/* =====================================================================
 * util.cpp:147 — DM2_ROTATE_5x5_POS
 * Rotates a position in a 5x5 grid by 0/1/2/3 quarter-turns clockwise.
 * ===================================================================== */

int16_t dm2_v1_rotate_5x5_pos(int16_t pos, int16_t rotation)
{
    int16_t mod = pos % 5 - 2;
    int16_t div = pos / 5 - 2;
    int16_t tmp;

    switch (rotation & 3) {
    case 0:
        break;
    case 1:
        tmp = mod;
        mod = div;
        div = (int16_t)-tmp;
        break;
    case 2:
        mod = (int16_t)-mod;
        div = (int16_t)-div;
        break;
    case 3:
        tmp = mod;
        mod = (int16_t)-div;
        div = tmp;
        break;
    }
    return (int16_t)(5 * (div + 2) + mod + 2);
}

/* =====================================================================
 * dm2global.cpp / skgdtqdb.cpp:1928 — Glob var three-tier storage
 * ===================================================================== */

void dm2_v1_glob_var_init(DM2_V1_GlobVarState *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

int32_t dm2_v1_get_glob_var(
    const DM2_V1_GlobVarState *state, uint16_t index)
{
    if (!state) return 0;
    if (index <= 0x3F) {
        uint16_t byte_idx = index / 8;
        uint8_t bit = (uint8_t)(1 << (index & 7));
        return (state->bit_vars[byte_idx] & bit) != 0 ? 1 : 0;
    }
    if (index <= 0x7F) {
        return (int32_t)(uint8_t)state->byte_vars[index - 0x40];
    }
    if (index <= 0xBF) {
        return (int32_t)state->word_vars[index];
    }
    return 0;
}

static int16_t dm2_between_value(int16_t lo, int16_t hi, int16_t val)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

int32_t dm2_v1_update_glob_var_direct(
    DM2_V1_GlobVarState *state,
    int16_t var_idx, int16_t mode, int16_t value)
{
    if (!state) return 0;
    int16_t cur = (int16_t)dm2_v1_get_glob_var(state, (uint16_t)var_idx);
    int16_t result = cur;

    if ((uint16_t)mode <= 6) {
        switch (mode) {
        case 0: result = 1; break;
        case 1: result = 0; break;
        case 2: result = (int16_t)(cur != 0 ? 0 : 1); break;
        case 3: result = (int16_t)(cur + value); break;
        case 4: result = (int16_t)(cur - value); break;
        case 5: break;
        case 6: result = value; break;
        }
    }

    uint16_t uvar = (uint16_t)var_idx;
    if (uvar <= 0x3F) {
        uint8_t bit = (uint8_t)(1 << (uvar & 7));
        uint16_t byte_idx = uvar >> 3;
        if (result != 0)
            state->bit_vars[byte_idx] |= bit;
        else
            state->bit_vars[byte_idx] &= (uint8_t)~bit;
    } else if (uvar <= 0x7F) {
        int16_t clamped = dm2_between_value(0, 255, result);
        state->byte_vars[uvar - 0x40] = (uint8_t)clamped;
        result = clamped;
    } else if (uvar <= 0xBF) {
        state->word_vars[uvar] = result;
    }
    return (int32_t)result;
}

/* =====================================================================
 * dm2global.cpp:21 — DM2_UPDATE_GLOB_VAR (legacy callback interface)
 * ===================================================================== */

int32_t dm2_v1_update_glob_var(
    int16_t var_idx, int16_t mode, int16_t value,
    const DM2_V1_UpdateGlobVarCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int16_t cur = cb->get_glob_var(ctx, var_idx);
    int16_t result = cur;

    if ((uint16_t)mode <= 6) {
        switch (mode) {
        case 0: result = 1; break;
        case 1: result = 0; break;
        case 2: result = (int16_t)(cur != 0 ? 0 : 1); break;
        case 3: result = (int16_t)(cur + value); break;
        case 4: result = (int16_t)(cur - value); break;
        case 5: break;
        case 6: result = value; break;
        }
    }

    uint16_t uvar = (uint16_t)var_idx;
    if (uvar <= 0x3F) {
        uint8_t bit = (uint8_t)(1 << (uvar & 7));
        uint16_t byte_idx = uvar >> 3;
        if (result != 0)
            cb->bit_vars[byte_idx] |= bit;
        else
            cb->bit_vars[byte_idx] &= (uint8_t)~bit;
    } else if (uvar <= 0x7F) {
        int16_t clamped = cb->between_value(0, 255, result);
        cb->byte_vars[uvar - 0x40] = (uint8_t)clamped;
        result = clamped;
    } else if (uvar <= 0xBF) {
        cb->word_vars[uvar] = result;
    }
    return (int32_t)result;
}

/* =====================================================================
 * c_timer.cpp:50 — timer heap sift (DM2_timer_3a15_0486)
 * Binary min-heap sift for the timer priority queue.
 * ===================================================================== */

static int dm2_timer_cmp(const DM2_V1_TimerEntry *a, const DM2_V1_TimerEntry *b)
{
    if (a->ticks < b->ticks) return 1;
    if (a->ticks > b->ticks) return 0;
    if (a->type <= b->type) return 1;
    return 0;
}

void dm2_v1_timer_heap_sift(DM2_V1_TimerHeapState *s, int16_t pos)
{
    if (!s)
        return;
    s->timer_unk = -1;

    int16_t last = (int16_t)(s->num_timers - 1);
    if (last == 0)
        return;

    int16_t cur = pos;
    int16_t saved_slot = s->indices[cur];
    DM2_V1_TimerEntry *saved = &s->timers[saved_slot];
    int sifted_up = 0;

    while (cur != 0) {
        int16_t parent = (int16_t)((cur - 1) / 2);
        if (!dm2_timer_cmp(saved, &s->timers[s->indices[parent]]))
            break;
        s->indices[cur] = s->indices[parent];
        cur = parent;
        sifted_up = 1;
    }

    if (!sifted_up) {
        int16_t half = (int16_t)((last - 1) / 2);
        while (cur <= half) {
            int16_t child = (int16_t)(2 * cur + 1);
            if (child + 1 < s->num_timers &&
                dm2_timer_cmp(&s->timers[s->indices[child + 1]],
                              &s->timers[s->indices[child]]))
                child++;
            if (!dm2_timer_cmp(&s->timers[s->indices[child]], saved))
                break;
            s->indices[cur] = s->indices[child];
            cur = child;
        }
    }
    s->indices[cur] = saved_slot;
}

/* c_timer.cpp:97 — DM2_REARRANGE_TIMERLIST */

void dm2_v1_rearrange_timerlist(DM2_V1_TimerHeapState *s)
{
    if (!s)
        return;
    s->num_indices = 0;
    s->available_idx = -1;
    int16_t last_free = -1;

    for (int16_t n = 0; n < s->max_timers; n++) {
        if (s->timers[n].type != 0) {
            s->num_indices = (int16_t)(n + 1);
        } else {
            if (s->available_idx != -1)
                s->timers[last_free].data_w = n;
            else
                s->available_idx = n;
            s->timers[n].data_w = -1;
            last_free = n;
        }
    }
}

/* c_timer.cpp:202 — DM2_GET_TIMER_NEW_INDEX */

int16_t dm2_v1_get_timer_new_index(DM2_V1_TimerHeapState *s, int16_t timer_slot)
{
    if (!s)
        return -1;
    for (int16_t n = 0; n < s->num_indices; n++) {
        if (s->indices[n] == timer_slot)
            return n;
    }
    if (s->raise_syserr)
        s->raise_syserr(NULL, 0x46);
    return -1;
}

/* c_timer.cpp:216 — DM2_DELETE_TIMER */

void dm2_v1_delete_timer(DM2_V1_TimerHeapState *s, int16_t timer_slot)
{
    if (!s)
        return;
    if (s->timer_unk >= 0)
        dm2_v1_timer_heap_sift(s, s->timer_unk);

    s->timers[timer_slot].type = 0;
    s->timers[timer_slot].data_w = s->available_idx;
    s->available_idx = timer_slot;
    s->num_timers--;

    if (s->num_timers != -1) {
        int16_t idx = dm2_v1_get_timer_new_index(s, timer_slot);
        if (idx >= 0 && idx != s->num_timers) {
            s->indices[idx] = s->indices[s->num_timers];
            s->timer_unk = idx;
        }
    }
}

/* c_timer.cpp:235 — DM2_QUEUE_TIMER */

int16_t dm2_v1_queue_timer(DM2_V1_TimerHeapState *s, const DM2_V1_TimerEntry *entry)
{
    if (!s || !entry || entry->type == 0)
        return -1;
    if (s->num_timers == s->max_timers) {
        if (s->raise_syserr)
            s->raise_syserr(NULL, 0x2D);
        return -1;
    }

    int16_t ti = s->available_idx;
    DM2_V1_TimerEntry *dest = &s->timers[ti];
    s->available_idx = dest->data_w;
    *dest = *entry;

    if (ti >= s->num_indices)
        s->num_indices = (int16_t)(ti + 1);

    int16_t tw = s->timer_unk;
    if (tw < 0)
        tw = s->num_timers;
    s->timer_unk = -1;
    s->num_timers++;
    s->indices[tw] = ti;
    dm2_v1_timer_heap_sift(s, tw);
    return ti;
}

/* c_timer.cpp:261 — DM2_GET_AND_DELETE_NEXT_TIMER */

void dm2_v1_get_and_delete_next_timer(DM2_V1_TimerHeapState *s, DM2_V1_TimerEntry *out)
{
    if (!s || !out)
        return;
    int16_t idx = s->indices[0];
    *out = s->timers[idx];
    dm2_v1_delete_timer(s, idx);
}

/* c_timer.cpp:269 — DM2_IS_TIMER_TO_PROCEED */

int dm2_v1_is_timer_to_proceed(DM2_V1_TimerHeapState *s)
{
    if (!s)
        return 0;
    if (s->timer_unk >= 0)
        dm2_v1_timer_heap_sift(s, s->timer_unk);
    if (s->num_timers == 0)
        return 0;
    uint32_t ticks = s->timers[s->indices[0]].ticks;
    return ticks <= s->game_tick ? 1 : 0;
}

/* c_timer.cpp:281 — DM2_timer_3a15_05f7 */

void dm2_v1_timer_reindex(DM2_V1_TimerHeapState *s, int16_t timer_slot)
{
    if (!s)
        return;
    if (s->timer_unk >= 0)
        dm2_v1_timer_heap_sift(s, s->timer_unk);
    dm2_v1_timer_heap_sift(s, dm2_v1_get_timer_new_index(s, timer_slot));
}

/* =====================================================================
 * c_record.cpp:175 — DM2_GET_DISTINCTIVE_ITEMTYPE
 * ===================================================================== */

int16_t dm2_v1_get_distinctive_itemtype(
    int16_t record_word,
    const DM2_V1_RecordQueryCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0x1FF;
    if (record_word == -1)
        return 0x1FF;

    int8_t cls2 = dm2_v1_query_cls2_from_record((int32_t)record_word, cb, ctx);
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    int16_t base = cb->table1d3278[db_type];

    uint8_t base_hi = (uint8_t)((uint16_t)base >> 8);
    uint8_t test = base_hi ^ (uint8_t)base;
    test &= 0x80;
    if (test != 0) {
        base = (int16_t)((uint16_t)base & 0x7FFF);
        cls2 = 0;
    }
    return (int16_t)((uint8_t)cls2 + base);
}

/* c_record.cpp:203 — DM2_QUERY_CLS2_FROM_RECORD */

int8_t dm2_v1_query_cls2_from_record(
    int32_t record_word,
    const DM2_V1_RecordQueryCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    int16_t rw = (int16_t)record_word;

    for (;;) {
        if (rw == -1)
            return -1;
        if ((uint16_t)rw >= 0xFF80)
            return (int8_t)((uint8_t)rw - 0x80);

        uint8_t *rec = cb->get_record_address(ctx, (uint16_t)rw);
        int16_t db_type = (int16_t)(((uint16_t)rw & 0x3C00) >> 10);
        if (db_type > 0xF)
            return -1;

        switch (db_type) {
        case 0: case 1: case 11: case 12: case 13:
            return -1;
        case 2:
            return (int8_t)cb->query_cls2_of_text(ctx, record_word);
        case 3:
            return (int8_t)cb->get_wall_decoration(ctx, cb->get_record_address(ctx, (uint16_t)rw));
        case 4:
            return (int8_t)rec[4];
        case 5: case 6: case 10: case 15:
            return (int8_t)(rec[2] & 0x7F);
        case 7:
            return 0;
        case 8: {
            uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
            return (int8_t)((w2 * 2) >> 9);
        }
        case 9: {
            uint16_t w4 = (uint16_t)(rec[4] | (rec[5] << 8));
            uint8_t hi = (uint8_t)(((w4 << 13) >> 14) * 8);
            uint8_t lo = (uint8_t)(w4 >> 13);
            return (int8_t)(lo | hi);
        }
        case 14: {
            uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
            rw = (int16_t)w2;
            continue;
        }
        default:
            return -1;
        }
    }
}

/* c_record.cpp:454 — DM2_QUERY_CLS1_FROM_RECORD */

int16_t dm2_v1_query_cls1_from_record(
    int32_t record_word,
    const DM2_V1_RecordQueryCallbacks *cb, void *ctx __attribute__((unused)))
{
    if (!cb || record_word == -1)
        return -1;
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    if (db_type > 0xF)
        return -1;
    /* CLS1 lookup table — maps DB type to GDAT category */
    static const int16_t cls1_table[16] = {
        -1, -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1, -1, -1, -1, 15
    };
    return cls1_table[db_type];
}

/* c_record.cpp:284 — DM2_SET_ITEMTYPE */

void dm2_v1_set_itemtype(
    int32_t record_word, int32_t new_type,
    const DM2_V1_SetItemtypeCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int16_t rw = (int16_t)record_word;
    if (rw == -1 || (uint16_t)rw >= 0xFF80)
        return;

    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)rw);
    int16_t db_type = (int16_t)(((uint16_t)rw & 0x3C00) >> 10);
    int16_t adj = (int16_t)(db_type - 4);
    if ((uint16_t)adj > 6)
        return;

    uint8_t type_lo = (uint8_t)new_type;
    switch (adj) {
    case 0: /* DB4 — byte at +4 */
        rec[4] = type_lo;
        /* fallthrough to case 3 (return) */
        return;
    case 1: /* DB5 */
    case 2: /* DB6 */
    case 6: /* DB10 */
        rec[2] = (uint8_t)((rec[2] & 0x80) | (type_lo & 0x7F));
        break;
    case 3: /* DB7 — no-op */
        return;
    case 4: /* DB8 */
        rec[3] = (uint8_t)((rec[3] & 0x80) | (type_lo & 0x7F));
        break;
    case 5: /* DB9 — complex encoding */ {
        uint8_t div8 = (uint8_t)(type_lo / 8);
        rec[4] = (uint8_t)((rec[4] & 0xF9) | ((div8 & 0x3) << 1));
        uint8_t mod8 = (uint8_t)(type_lo & 0x7);
        uint16_t w4 = (uint16_t)(rec[4] | (rec[5] << 8));
        rec[5] = (uint8_t)((rec[5] & 0x1F) | (mod8 << 5));
        w4 = (uint16_t)(rec[4] | (rec[5] << 8));
        if ((w4 & 0x6) == 2) {
            rec[6] = 0xFF;
            rec[7] = 0xFF;
        }
        return;
    }
    default:
        return;
    }
}

/* c_record.cpp:477 — DM2_GET_WALL_TILE_ANYITEM_RECORD */

int16_t dm2_v1_get_wall_tile_anyitem_record(
    int16_t first_record,
    const DM2_V1_WallTileRecordCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    int16_t rw = first_record;
    while (rw != -1 && rw != (int16_t)0xFFFE) {
        int16_t db_type = (int16_t)(((uint16_t)rw & 0x3C00) >> 10);
        if (db_type != 3) /* not a wall decoration record */
            return rw;
        uint8_t *rec = cb->get_record_address(ctx, (uint16_t)rw);
        if (!rec)
            return -1;
        rw = (int16_t)(rec[0] | (rec[1] << 8));
    }
    return -1;
}

/* c_record.cpp:498 — DM2_SET_ITEM_IMPORTANCE */

void dm2_v1_set_item_importance(
    int16_t record_word, int16_t importance,
    const DM2_V1_SetImportanceCallbacks *cb, void *ctx)
{
    if (!cb || record_word == -1)
        return;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)record_word);
    if (!rec)
        return;
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    if (db_type < 4 || db_type > 10)
        return;
    /* Importance stored in bits 8-15 of word+0 (high byte of first word) */
    rec[1] = (uint8_t)(importance & 0xFF);
}

/* c_record.cpp:1076 — DM2_ALLOC_NEW_RECORD */

int16_t dm2_v1_alloc_new_record(
    int16_t db_type,
    const DM2_V1_AllocRecordCallbacks *cb, void *ctx)
{
    if (!cb || db_type < 0 || db_type > 15)
        return -1;
    if (cb->db_counts[db_type] >= cb->db_max[db_type]) {
        if (cb->raise_syserr)
            cb->raise_syserr(ctx, 0x2E);
        return -1;
    }
    int16_t slot = cb->free_list[db_type];
    if (slot < 0)
        return -1;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)((db_type << 10) | slot));
    if (!rec)
        return -1;
    /* Next free is stored in word+0 */
    cb->free_list[db_type] = (int16_t)(rec[0] | (rec[1] << 8));
    memset(rec, 0, 8); /* clear the record */
    cb->db_counts[db_type]++;
    return (int16_t)((db_type << 10) | slot);
}

/* c_record.cpp:1205 — DM2_DEALLOC_RECORD */

void dm2_v1_dealloc_record(
    int16_t record_word,
    const DM2_V1_AllocRecordCallbacks *cb, void *ctx)
{
    if (!cb || record_word == -1)
        return;
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    int16_t slot = (int16_t)((uint16_t)record_word & 0x03FF);
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)record_word);
    if (!rec)
        return;
    rec[0] = (uint8_t)(cb->free_list[db_type] & 0xFF);
    rec[1] = (uint8_t)(cb->free_list[db_type] >> 8);
    cb->free_list[db_type] = slot;
    cb->db_counts[db_type]--;
}

/* c_record.cpp:1142 — DM2_ALLOC_NEW_DBITEM
 * Superseded by dm2_v1_dbitem_alloc_pc34_compat.c */

/* c_record.cpp:1261/1288 — decoration functions moved to
 * dm2_v1_record_ops_pc34_compat.c with proper map-based lookup */

/* c_record.cpp:1356 — DM2_DELETE_CREATURE_RECORD */

void dm2_v1_delete_creature_record(
    int16_t record_word, int16_t x, int16_t y,
    const DM2_V1_DeleteCreatureRecordCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->drop_possessions(ctx, record_word);
    cb->unlink_from_tile(ctx, record_word, x, y);
    cb->dealloc_record(ctx, record_word);
}

/* c_record.cpp:1537 — DM2_DROP_CREATURE_POSSESSION
 * Superseded by dm2_v1_drop_possession_pc34_compat.c */

/* c_record.cpp:1839 — DM2_ROTATE_RECORD_BY_TELEPORTER */

void dm2_v1_rotate_record_by_teleporter(
    int16_t record_word, int16_t rotation,
    const DM2_V1_RotateRecordCallbacks *cb, void *ctx)
{
    if (!cb || record_word == -1 || rotation == 0)
        return;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)record_word);
    if (!rec)
        return;
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    if (db_type == 4) {
        /* Creature: direction in bits 14-15 of word+14 */
        uint16_t w14 = (uint16_t)(rec[14] | (rec[15] << 8));
        uint16_t dir = (w14 >> 14) & 3;
        dir = (uint16_t)((dir + rotation) & 3);
        w14 = (uint16_t)((w14 & 0x3FFF) | (dir << 14));
        rec[14] = (uint8_t)(w14 & 0xFF);
        rec[15] = (uint8_t)(w14 >> 8);
    }
}

/* c_record.cpp:1870 — DM2_075f_056c */

int dm2_v1_075f_056c(
    int16_t record, int16_t x, int16_t y,
    const DM2_V1_075f056cCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return 0;
    return cb->dispatch(ctx, record, x, y);
}

/* c_record.cpp:33 — init_global_records */

void dm2_v1_init_global_records(
    const DM2_V1_InitGlobalRecordsCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->db_type_count; i++)
        cb->init_db_pool(ctx, i, cb->db_sizes[i]);
}

/* =====================================================================
 * c_creature.cpp:318 — DM2_ATTACK_CREATURE
 * ===================================================================== */

int32_t dm2_v1_attack_creature(
    int16_t creature_record, int16_t x, int16_t y,
    int16_t attack_type __attribute__((unused)), int16_t sound_id, int32_t damage,
    const DM2_V1_AttackCreatureCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)creature_record);
    if (!rec)
        return 0;
    int16_t armor = cb->query_ai_spec_armor(ctx, creature_record);
    int16_t actual = (int16_t)(damage - armor);
    if (actual < 0)
        actual = 0;
    /* Random variance: ±25% */
    int16_t variance = cb->rand16(ctx, (int16_t)(actual / 4 + 1));
    int16_t sign = (int16_t)(cb->rand(ctx) & 1);
    if (sign)
        actual = (int16_t)(actual + variance);
    else
        actual = (int16_t)(actual - variance);
    if (actual < 1)
        actual = 1;
    cb->wound_creature(ctx, creature_record, actual);
    cb->play_sound(ctx, x, y, sound_id);
    return actual;
}

/* c_creature.cpp:3299 — DM2_4FCC */

int32_t dm2_v1_4fcc(
    int16_t creature_idx, int16_t x, int16_t y,
    const DM2_V1_4FCCCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return 0;
    return cb->dispatch(ctx, creature_idx, x, y);
}

/* =====================================================================
 * c_item.cpp:528 — DM2_IS_MISCITEM_DRINK_WATER
 * ===================================================================== */

int dm2_v1_is_miscitem_drink_water(
    int16_t record_word,
    const DM2_V1_MiscItemCallbacks *cb, void *ctx)
{
    if (!cb || record_word == -1)
        return 0;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)record_word);
    if (!rec)
        return 0;
    int16_t db_type = (int16_t)(((uint16_t)record_word & 0x3C00) >> 10);
    if (db_type != 10) /* must be miscellaneous item */
        return 0;
    uint8_t subtype = rec[2] & 0x7F;
    int16_t gdat_val = cb->query_gdat(ctx, 10, subtype, 11, 0x53);
    return (gdat_val != 0) ? 1 : 0;
}

/* c_item.cpp:1034 — DM2_F958 */

int16_t dm2_v1_f958(int16_t value, int16_t threshold)
{
    int16_t result = (int16_t)-value;
    if (result < threshold)
        result = threshold;
    return result;
}

/* c_item.cpp:1185 — DM2_TAKE_OBJECT */

void dm2_v1_take_object(
    int16_t item, int16_t x, int16_t y,
    const DM2_V1_TakeObjectCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->unlink_item(ctx, item, x, y);
    cb->set_item_importance(ctx, item, 0);
}

/* =====================================================================
 * c_move.cpp:1972 — DM2_move_075f_06bd
 * ===================================================================== */

void dm2_v1_move_075f_06bd(
    int16_t x, int16_t y, int16_t dir,
    const DM2_V1_Move075f06bdCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return;
    (void)cb->dispatch(ctx, x, y, dir);
}

/* =====================================================================
 * c_tim_proc.cpp — Tile actuator stubs
 * ===================================================================== */

void dm2_v1_step_door(
    int16_t x, int16_t y, int16_t record_word, int16_t direction,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *tile = cb->get_tile_byte(ctx, x, y);
    if (!tile)
        return;
    uint8_t tile_type = *tile & 0x07;
    if (tile_type != 5)
        return;
    uint8_t *rec = cb->get_record_address(ctx, (uint16_t)record_word);
    if (!rec)
        return;
    /* Door step: advance animation frame in record byte+3 bits 0-2 */
    uint8_t frame = rec[3] & 0x07;
    if (direction > 0) {
        frame++;
        if (frame > 6) frame = 6;
    } else {
        if (frame > 0) frame--;
    }
    rec[3] = (uint8_t)((rec[3] & 0xF8) | (frame & 0x07));
    if (frame != 0 && frame != 6)
        cb->requeue_timer(ctx, 1);
    if (cb->current_map == cb->party_map && cb->redraw_flags)
        *cb->redraw_flags = 3;
}

void dm2_v1_actuate_pitfall(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *tile = cb->get_tile_byte(ctx, x, y);
    if (!tile)
        return;
    if (param != 0)
        *tile = (uint8_t)((*tile & 0xF8) | 0x03); /* open pit */
    else
        *tile = (uint8_t)((*tile & 0xF8) | 0x03); /* toggle */
    if (cb->current_map == cb->party_map && cb->redraw_flags)
        *cb->redraw_flags = 3;
}

void dm2_v1_actuate_door(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    (void)param;
    cb->invoke_actuator(ctx, x, y, 1, 0);
}

void dm2_v1_actuate_teleporter(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *tile = cb->get_tile_byte(ctx, x, y);
    if (!tile)
        return;
    if (param != 0)
        *tile |= 0x08;
    else
        *tile &= 0xF7;
    if (cb->current_map == cb->party_map && cb->redraw_flags)
        *cb->redraw_flags = 3;
}

void dm2_v1_actuate_trickwall(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *tile = cb->get_tile_byte(ctx, x, y);
    if (!tile)
        return;
    if (param != 0)
        *tile |= 0x04;
    else
        *tile &= (uint8_t)~0x04;
    if (cb->current_map == cb->party_map && cb->redraw_flags)
        *cb->redraw_flags = 3;
}

/* =====================================================================
 * c_savegame.cpp:229 — DM2_READ_DUNGEON_STRUCTURE
 * ===================================================================== */

int dm2_v1_read_dungeon_structure(
    const uint8_t *data, int32_t length,
    const DM2_V1_ReadDungeonStructureCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch || !data)
        return 0;
    return cb->dispatch(ctx, data, length);
}

/* =====================================================================
 * SkWinCore2.cpp:475 — PROCESS_PLAGUE
 * ===================================================================== */

void dm2_v1_process_plague(
    const DM2_V1_ProcessPlagueCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->hero_count; i++) {
        int16_t plague = cb->get_hero_plague(ctx, i);
        if (plague <= 0)
            continue;
        /* Plague damage = 1 per tick, decrement counter */
        cb->wound_player(ctx, i, 1);
        cb->set_hero_plague(ctx, i, (int16_t)(plague - 1));
    }
}
