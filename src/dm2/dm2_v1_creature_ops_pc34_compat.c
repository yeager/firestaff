/* DM2 V1 creature operations — skproject c_creature.cpp. */

#include "dm2_v1_creature_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_END_WORD 0xFFFEu

int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_type, int16_t poison_amount,
    const DM2_V1_CreaturePoisonCallbacks *cb, void *ctx)
{
    if (!cb || poison_amount == 0)
        return 0;
    const uint8_t *ai_spec = cb->query_ai_spec(ctx, creature_type);
    if (!ai_spec)
        return 0;
    /* w24 (resistance field), bits 8..11 = poison resistance 0..15 */
    uint16_t resistance_word = (uint16_t)(ai_spec[24] | (ai_spec[25] << 8));
    uint16_t poison_resist = (resistance_word >> 8) & 0xF;
    if (poison_resist == 0xF)
        return 0;
    uint16_t rand_val = cb->rand_dir(ctx);
    uint32_t numerator = ((uint32_t)poison_amount + rand_val) << 3;
    uint32_t denominator = poison_resist + 2;
    return (int16_t)(numerator / denominator);
}

void dm2_v1_rotate_creature(
    uint16_t creature_record, int mode, int direction,
    const DM2_V1_CreatureRotateCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *rec = cb->get_record_address(ctx, creature_record);
    if (!rec)
        return;
    /* Current facing from offset+0xE bits 8-9 */
    uint16_t word_0e = (uint16_t)(rec[0x0E] | (rec[0x0F] << 8));
    uint8_t cur_dir = (uint8_t)((word_0e >> 8) & 0x3);
    uint8_t new_dir;
    int delta;
    if (mode == 0) {
        new_dir = (uint8_t)((cur_dir + direction) & 0x3);
        delta = (new_dir - cur_dir) & 0x3;
    } else {
        new_dir = (uint8_t)(direction & 0x3);
        delta = (new_dir - cur_dir) & 0x3;
    }
    /* Write new direction: clear bits 8-9, set new */
    rec[0x0F] = (uint8_t)((rec[0x0F] & 0xFC) | (new_dir & 0x3));

    /* If AI flag bit 0 set, rotate all items in possession chain */
    uint16_t flags = cb->query_ai_spec_flags(ctx, creature_record);
    if ((flags & 0x01) == 0)
        return;
    /* Walk possession chain at offset+2, rotating each item's direction
     * bits by delta (skproject c_creature.cpp:86-100) */
    uint8_t *chain_ptr = rec + 2;
    for (;;) {
        uint16_t item_word = (uint16_t)(chain_ptr[0] | (chain_ptr[1] << 8));
        if (item_word == OBJECT_END_WORD || item_word == 0xFFFF)
            break;
        uint8_t item_dir = (uint8_t)((item_word >> 14) & 0x3);
        uint8_t rotated = (uint8_t)((item_dir + delta) & 0x3);
        item_word = (item_word & 0x3FFF) | ((uint16_t)rotated << 14);
        chain_ptr[0] = (uint8_t)(item_word & 0xFF);
        chain_ptr[1] = (uint8_t)((item_word >> 8) & 0xFF);
        uint8_t *item_rec = cb->get_record_address(ctx, item_word);
        if (!item_rec)
            break;
        chain_ptr = item_rec;
    }
}

int16_t dm2_v1_creature_can_handle_item_in(
    int16_t creature_type, int16_t first_record, uint8_t direction_filter,
    const DM2_V1_CreatureHandleCallbacks *cb, void *ctx)
{
    if (!cb)
        return (int16_t)OBJECT_END_WORD;
    int16_t current = first_record;
    for (;;) {
        uint16_t w = (uint16_t)current;
        if (w == OBJECT_END_WORD || w == 0xFFFF)
            return (int16_t)OBJECT_END_WORD;
        uint16_t db_type = (w >> 10) & 0xF;
        int is_item = (db_type > 4 && db_type < 14) || (db_type == 9);
        if (is_item) {
            int dir_match = 0;
            if (direction_filter == 0xFF)
                dir_match = 1;
            else if (((w >> 14) & 0x3) == direction_filter)
                dir_match = 1;
            if (dir_match) {
                if (cb->creature_can_handle_it(ctx, w, creature_type))
                    return current;
            }
        }
        current = cb->get_next_record_link(ctx, w);
    }
}

int dm2_v1_confuse_creature(uint8_t *creature_record)
{
    if (!creature_record)
        return 0;
    creature_record[0x11] |= 0x04;
    return 1;
}
