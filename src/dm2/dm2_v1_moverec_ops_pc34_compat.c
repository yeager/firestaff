/* DM2 V1 record movement operations — skproject c_moverec.cpp. */

#include "dm2_v1_moverec_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_END 0xFFFE

void dm2_v1_set_minion_recent_open_door_location(
    uint16_t first_record, uint8_t x, uint8_t y, uint8_t map, int open_flag,
    const DM2_V1_MinionDoorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *rec = cb->get_record_address(ctx, first_record);
    if (!rec)
        return;
    uint16_t link = (uint16_t)(rec[2] | (rec[3] << 8));
    while (link != OBJECT_END && link != 0xFFFF) {
        uint16_t db_type = (link >> 10) & 0xF;
        if (db_type == 0xE) {
            uint8_t *link_rec = cb->get_record_address(ctx, link);
            if (link_rec) {
                /* word+4: bits 0-4 = x, bits 5-9 = y, bits 10-15 = map */
                uint16_t w4 = (uint16_t)(link_rec[4] | (link_rec[5] << 8));
                w4 = (w4 & 0xFFE0) | (x & 0x1F);
                w4 = (w4 & 0xFC1F) | ((uint16_t)(y & 0x1F) << 5);
                w4 = (w4 & 0x03FF) | ((uint16_t)(map & 0x3F) << 10);
                link_rec[4] = (uint8_t)(w4 & 0xFF);
                link_rec[5] = (uint8_t)(w4 >> 8);
                /* word+6 bit 0 = open flag */
                link_rec[6] = (uint8_t)((link_rec[6] & 0xFE) | (open_flag & 1));
            }
        }
        int16_t next = cb->get_next_record_link(ctx, link);
        link = (uint16_t)next;
    }
}

void dm2_v1_moverec_2fcf_01c5(
    uint16_t record, uint8_t x, uint8_t y, uint8_t map, int direction_flag,
    const DM2_V1_MoverecTimerCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t timer_type = (uint8_t)(direction_flag != 0 ? 0x3D : 0x3C);
    uint32_t fire_tick = cb->game_tick + 5;
    cb->queue_timer(ctx, timer_type, x, y, record, fire_tick);
    cb->set_minion_door(ctx, record, x, y, map, 1);
}

