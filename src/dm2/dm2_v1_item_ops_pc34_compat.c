/* DM2 V1 item operations — skproject c_item.cpp. */

#include "dm2_v1_item_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_NULL_WORD 0xFFFFu

int16_t dm2_v1_f958(uint16_t record_word,
                     const DM2_V1_ItemValueCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    int16_t val = cb->query_item_value(ctx, record_word, 2);
    return val <= -1 ? val : -1;
}

int dm2_v1_is_miscitem_drink_water(
    uint16_t record_word,
    const DM2_V1_DrinkWaterCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD)
        return 0;
    int16_t gdat_word = cb->query_gdat_dbspec_word(ctx, record_word, 0);
    if ((gdat_word & 0x01) == 0)
        return 0;
    int16_t charges = cb->add_item_charge(ctx, record_word, 0);
    if (charges == 0)
        return 0;
    cb->add_item_charge(ctx, record_word, -1);
    if (record_word == cb->item_in_hand && cb->retake_object)
        cb->retake_object(ctx, record_word);
    return 1;
}

void dm2_v1_take_object(
    uint16_t record_word, int deferred,
    const DM2_V1_TakeObjectCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD)
        return;
    int16_t gdat_word = cb->query_gdat_dbspec_word(ctx, record_word, 0);
    int16_t weight = cb->query_item_weight(ctx, record_word);
    cb->set_hand_item(ctx, record_word, gdat_word, weight);
    cb->draw_item_in_hand(ctx);
    cb->display_item_name(ctx, record_word);
    if (deferred == 0)
        cb->process_events(ctx);
    else
        cb->set_deferred_flag(ctx);
    cb->process_item_bonus(ctx, record_word);
    cb->moverec_update(ctx);
}
