/* DM2 V1 record operations — skproject c_record.cpp + SkWinCore2.cpp. */

#include "dm2_v1_record_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_NULL_WORD  0xFFFFu
#define OBJECT_END_WORD   0xFFFEu
#define DB_TYPE_TEXT      0x02
#define DB_WEAPON         0x05
#define DB_MISSILE        0x0E

int16_t dm2_v1_query_gdat_dbspec_word_value(
    uint16_t record_word, uint8_t data_index,
    const DM2_V1_GdatDbspecCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD)
        return 0;
    uint8_t cls1 = cb->query_cls1_from_record(ctx, record_word);
    uint8_t cls2 = cb->query_cls2_from_record(ctx, record_word);
    return cb->query_gdat_entry_data_index(ctx, cls1, cls2, 11, data_index);
}

int16_t dm2_v1_get_wall_tile_any_takeable_item_record(
    uint16_t map_x, uint16_t map_y, uint16_t direction,
    const DM2_V1_TileRecordWalkCallbacks *cb, void *ctx)
{
    if (!cb)
        return (int16_t)OBJECT_NULL_WORD;
    int16_t rec = cb->get_tile_record_link(ctx, (int16_t)map_x, (int16_t)map_y);
    for (;;) {
        uint16_t w = (uint16_t)rec;
        if (w == OBJECT_END_WORD || w == OBJECT_NULL_WORD)
            return (int16_t)OBJECT_NULL_WORD;
        uint16_t dir = (w >> 14) & 0x3;
        uint16_t db_type = (w >> 10) & 0xF;
        if (dir == direction && db_type >= DB_WEAPON && db_type <= 0x0A)
            return rec;
        rec = cb->get_next_record_link(ctx, w);
    }
}

uint8_t dm2_v1_get_wall_decoration_of_actuator(
    const uint8_t *actuator_record,
    const DM2_V1_DecorationLookupCallbacks *cb, void *ctx)
{
    if (!actuator_record)
        return 0xFF;
    uint16_t w4 = (uint16_t)(actuator_record[4] | (actuator_record[5] << 8));
    int16_t index = (int16_t)(w4 >> 12);
    if (index == 0)
        return 0xFF;
    if (!cb || !cb->lookup_wall_decoration)
        return 0xFF;
    return cb->lookup_wall_decoration(ctx, index);
}

uint8_t dm2_v1_get_floor_decoration_of_actuator(
    const uint8_t *actuator_record,
    const DM2_V1_DecorationLookupCallbacks *cb, void *ctx)
{
    if (!actuator_record)
        return 0xFF;
    uint16_t w4 = (uint16_t)(actuator_record[4] | (actuator_record[5] << 8));
    int16_t index = (int16_t)(w4 >> 12);
    if (index == 0)
        return 0xFF;
    if (!cb || !cb->lookup_floor_decoration)
        return 0xFF;
    return cb->lookup_floor_decoration(ctx, index);
}

int dm2_v1_missile_timer_cleanup(
    uint16_t record_word,
    const DM2_V1_MissileCleanupCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint16_t db_type = (record_word >> 10) & 0xF;
    if (db_type != DB_MISSILE)
        return 0;
    const uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec)
        return 0;
    int16_t timer_idx = (int16_t)(rec[6] | (rec[7] << 8));
    cb->delete_timer(ctx, timer_idx);
    return 1;
}

uint8_t dm2_v1_is_object_visible_text(
    uint16_t record_word,
    const DM2_V1_ObjectTextCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD)
        return 0;
    int db_type = cb->get_db_type(ctx, record_word);
    if (db_type != DB_TYPE_TEXT)
        return 0;
    const uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec)
        return 0;
    return rec[2] & 0x01;
}

/* ---- DM2_GET_ITEMDB_OF_ITEMSPEC_ACTUATOR (c_record.cpp:367) ---- */
int16_t dm2_v1_get_itemdb_of_itemspec_actuator(uint16_t itemspec)
{
    uint16_t val = itemspec & 0x01FF;
    uint16_t region = val >> 7;
    if (region > 3) return (int16_t)0xFFFF;
    switch (region) {
        case 0: return 5;
        case 1: return 6;
        case 2: return 10;
        case 3:
            if (val > 0x1FC) return (int16_t)0xFFFF;
            if (val == 0x1FC) return 7;
            if (val >= 0x1E0) return 9;
            return (val < 0x1B0) ? 8 : 4;
        default: return (int16_t)0xFFFF;
    }
}

/* ---- DM2_GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR (c_record.cpp:403) ---- */
int16_t dm2_v1_get_itemtype_of_itemspec_actuator(uint16_t itemspec)
{
    uint16_t val = itemspec & 0x01FF;
    uint16_t region = val >> 7;
    if (region > 3) return (int16_t)val;
    switch (region) {
        case 0: return (int16_t)val;
        case 1: return (int16_t)(val - 0x80);
        case 2: return (int16_t)(val - 0x100);
        case 3:
            if (val >= 0x1FC) return 0;
            if (val >= 0x1E0) return (int16_t)(val - 0x1E0);
            if (val < 0x1B0) return (int16_t)(val - 0x180);
            return (int16_t)(val - 0x1B0);
        default: return (int16_t)val;
    }
}

/* ---- DM2_QUERY_CLS2_OF_TEXT_RECORD (c_record.cpp:1210) ---- */
uint8_t dm2_v1_query_cls2_of_text_record(
    uint16_t record_word,
    const DM2_V1_TextCls2Callbacks *cb, void *ctx)
{
    if (!cb) return 0xFF;
    const uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec) return 0xFF;
    uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
    if ((w2 & 0x06) != 2) return 0xFF;
    /* Extract text subtype: (w2 >> 3) >> 8 & 0x1F */
    uint16_t shifted = w2 >> 3;
    uint8_t subtype = (uint8_t)((shifted >> 8) & 0x1F);
    /* Valid subtypes: 0, 2, 3..5, 0xD */
    if (subtype == 0 || subtype == 2 || (subtype >= 3 && subtype <= 5) || subtype == 0xD) {
        return (uint8_t)(w2 >> 3);
    }
    return 0xFF;
}

/* ---- DM2_QUERY_CLS1_FROM_RECORD (c_record.cpp:454) ---- */
uint8_t dm2_v1_query_cls1_from_record(
    uint16_t record_word,
    const DM2_V1_Cls1Callbacks *cb, void *ctx)
{
    (void)ctx;
    if (!cb || record_word == (uint16_t)0xFFFF)
        return 0xFF;
    uint16_t db_type = (record_word >> 10) & 0xF;
    if (db_type > 0xF)
        return 0xFF;
    return cb->table1d3298[db_type];
}

/* ---- DM2_QUERY_ITEMDB_FROM_DISTINCTIVE_ITEMTYPE (c_record.cpp:449) ---- */
uint8_t dm2_v1_query_itemdb_from_distinctive_itemtype(
    uint16_t distinctive_type,
    const DM2_V1_Cls1Callbacks *cb, void *ctx)
{
    int16_t db = dm2_v1_get_itemdb_of_itemspec_actuator(distinctive_type & 0xFFFF);
    uint16_t shifted = ((uint16_t)db << 10) & 0xFFFF;
    return dm2_v1_query_cls1_from_record(shifted, cb, ctx);
}

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD (c_record.cpp:1351) ---- */
const uint8_t *dm2_v1_query_creature_ai_spec_from_record(
    uint8_t creature_type,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx)
{
    if (!cb) return NULL;
    int16_t spec_idx = cb->query_gdat_creature_word_value(ctx, creature_type, 5);
    return cb->get_ai_spec(ctx, (uint16_t)spec_idx);
}

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FROM_TYPE (c_record.cpp:1341) ---- */
const uint8_t *dm2_v1_query_creature_ai_spec_from_type(
    uint16_t record_word,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx)
{
    if (!cb) return NULL;
    const uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec) return NULL;
    return dm2_v1_query_creature_ai_spec_from_record(rec[4], cb, ctx);
}

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FLAGS (c_record.cpp:1346) ---- */
int16_t dm2_v1_query_creature_ai_spec_flags(
    uint16_t record_word,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx)
{
    const uint8_t *spec = dm2_v1_query_creature_ai_spec_from_type(
        record_word, cb, ctx);
    if (!spec) return 0;
    return (int16_t)(spec[0] | (spec[1] << 8));
}

#define DB_ACTUATOR 0x03

int16_t *dm2_v1_oversee_record(
    int16_t *first_link, uint8_t type_filter,
    int16_t **prev_link_out, DM2_V1_OverseeFilterFn filter_fn,
    int16_t *userdata, int recurse_containers, int recurse_chests,
    const DM2_V1_OverseeCallbacks *cb, void *ctx)
{
    if (!first_link || !cb || !filter_fn)
        return NULL;
    int16_t *link_ptr = first_link;
    for (;;) {
        uint16_t w = (uint16_t)*link_ptr;
        if (w == OBJECT_END_WORD || w == OBJECT_NULL_WORD)
            return NULL;
        uint16_t db_type = (w >> 10) & 0xF;
        int type_match = (type_filter == 0xFF) || (db_type == type_filter);
        /* Recurse into type-4 containers if requested */
        if (recurse_containers && db_type == 4) {
            uint8_t *rec = cb->get_record_address(ctx, w);
            if (rec) {
                int16_t *child = (int16_t *)(rec + 2);
                int16_t *found = dm2_v1_oversee_record(
                    child, 0xFF, prev_link_out, filter_fn, userdata,
                    recurse_containers, recurse_chests, cb, ctx);
                if (found)
                    return found;
            }
        }
        /* Recurse into chests if requested */
        if (recurse_chests && cb->is_container_chest &&
            cb->is_container_chest(ctx, w)) {
            uint8_t *rec = cb->get_record_address(ctx, w);
            if (rec) {
                int16_t *child = (int16_t *)(rec + 2);
                int16_t *found = dm2_v1_oversee_record(
                    child, 0xFF, prev_link_out, filter_fn, userdata,
                    recurse_containers, recurse_chests, cb, ctx);
                if (found)
                    return found;
            }
        }
        if (type_match) {
            if (filter_fn(link_ptr, userdata)) {
                if (prev_link_out)
                    *prev_link_out = link_ptr;
                return link_ptr;
            }
        }
        /* Advance to next record */
        int16_t next = cb->get_next_record_link(ctx, w);
        uint8_t *rec = cb->get_record_address(ctx, w);
        if (!rec)
            return NULL;
        link_ptr = (int16_t *)(rec);
    }
}

int dm2_v1_rotate_actuator_list(
    int16_t map_x, int16_t map_y, uint8_t direction,
    const DM2_V1_ActuatorRotateCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int16_t rec = cb->get_tile_record_link(ctx, map_x, map_y);
    uint16_t actuators[3] = { OBJECT_END_WORD, OBJECT_END_WORD, OBJECT_END_WORD };
    int act_count = 0;
    int16_t saved_next = (int16_t)OBJECT_END_WORD;

    /* Walk tile chain, collect actuators matching direction */
    while ((uint16_t)rec != OBJECT_END_WORD &&
           (uint16_t)rec != OBJECT_NULL_WORD) {
        uint16_t w = (uint16_t)rec;
        uint16_t db_type = (w >> 10) & 0xF;
        uint16_t dir = (w >> 14) & 0x3;
        if (db_type == DB_ACTUATOR && dir == direction) {
            if (act_count < 3) {
                actuators[act_count] = w;
                act_count++;
            }
        }
        int16_t next = cb->get_next_record_link(ctx, w);
        if (act_count > 0 && act_count <= 3) {
            saved_next = next;
        }
        rec = next;
    }

    if (act_count < 2)
        return 0;

    /* Rotate: move last actuator to front position.
     * For 2 actuators [A, B]: becomes [B, A]
     * For 3 actuators [A, B, C]: becomes [C, A, B] */
    uint16_t last = actuators[act_count - 1];
    cb->set_next_record_link(ctx, last, (int16_t)actuators[0]);
    cb->set_next_record_link(ctx, actuators[act_count - 2], saved_next);

    return 1;
}
