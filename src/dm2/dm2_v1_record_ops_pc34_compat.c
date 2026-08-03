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

int16_t dm2_v1_get_wall_tile_anyitem_record(
    int16_t map_x, int16_t map_y,
    const DM2_V1_TileRecordWalkCallbacks *cb, void *ctx)
{
    if (!cb)
        return (int16_t)OBJECT_NULL_WORD;
    int16_t rec = cb->get_tile_record_link(ctx, map_x, map_y);
    for (;;) {
        uint16_t w = (uint16_t)rec;
        if (w == OBJECT_END_WORD || w == OBJECT_NULL_WORD)
            return rec;
        uint16_t db_type = (w >> 10) & 0xF;
        if (db_type > 3)
            return rec;
        rec = cb->get_next_record_link(ctx, w);
    }
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
    const uint8_t *decoration_table, int decoration_table_size)
{
    if (!actuator_record)
        return 0xFF;
    uint8_t deco_count = (actuator_record[4] >> 4) & 0xF;
    if (deco_count == 0)
        return 0xFF;
    int idx = deco_count - 1;
    if (!decoration_table || idx >= decoration_table_size)
        return 0xFF;
    return decoration_table[idx];
}

uint8_t dm2_v1_get_floor_decoration_of_actuator(
    const uint8_t *actuator_record,
    const uint8_t *decoration_table, int decoration_table_size)
{
    if (!actuator_record)
        return 0xFF;
    uint8_t deco_count = (actuator_record[4] >> 4) & 0xF;
    if (deco_count == 0)
        return 0xFF;
    int idx = deco_count - 1;
    if (!decoration_table || idx >= decoration_table_size)
        return 0xFF;
    return decoration_table[idx];
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

uint16_t dm2_v1_rotate_record_by_teleporter(
    uint8_t teleporter_flags, uint8_t stored_dir,
    uint16_t record_word, uint8_t *out_dir)
{
    uint8_t rec_dir = (uint8_t)((record_word >> 14) & 0x3);
    uint8_t new_dir;
    if ((teleporter_flags & 0x10) == 0) {
        new_dir = stored_dir & 0x3;
    } else {
        uint8_t delta = (stored_dir - rec_dir) & 0x3;
        new_dir = stored_dir & 0x3;
        uint8_t word_dir = (uint8_t)((record_word >> 14) & 0x3);
        word_dir = (word_dir + delta) & 0x3;
        record_word = (record_word & 0x3FFF) | ((uint16_t)word_dir << 14);
    }
    if (out_dir)
        *out_dir = new_dir;
    return record_word;
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
