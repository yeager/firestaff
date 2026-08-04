/* DM2 V1 item operations — skproject c_item.cpp. */

#include "dm2_v1_item_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_NULL_WORD 0xFFFFu

/* ---- DM2_RETRIEVE_ITEM_BONUS (c_item.cpp:22) ---- */
int16_t dm2_v1_retrieve_item_bonus(
    uint16_t record_word, uint8_t bonus_idx,
    int16_t equipped, int16_t context,
    const DM2_V1_ItemBonusCallbacks *cb, void *ctx)
{
    if (!cb) return 0;
    int16_t val = cb->query_gdat_dbspec_word(ctx, record_word, bonus_idx);
    if (val == 0) return 0;
    if ((val & 0x4000) == 0) {
        /* Non-conditional bonus */
        if (equipped == 0) {
            /* Check if upper bits match (non-equipped filter) */
            if (((val >> 7) & 0xFF80) == 0)
                return 0;
        }
    } else {
        /* Conditional bonus: only applies in context 0xFFFE, 2, or 3 */
        if (context != (int16_t)0xFFFE && context != 2 && context != 3)
            return 0;
    }
    int8_t signed_val = (int8_t)(val & 0xFF);
    int16_t result = (int16_t)signed_val;
    return (context >= 0) ? result : -result;
}

/* ---- DM2_IS_MISCITEM_DRINK_WATER (c_item.cpp:528) ---- */
int dm2_v1_is_miscitem_drink_water(
    uint16_t record_word,
    const DM2_V1_DrinkWaterCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_dbspec_word || !cb->add_item_charge)
        return 0;
    int16_t drinkable = cb->query_gdat_dbspec_word(ctx, record_word, 0);
    if ((drinkable & 1) == 0)
        return 0;
    int16_t charges = cb->add_item_charge(ctx, record_word, 0);
    if (charges == 0)
        return 0;
    cb->add_item_charge(ctx, record_word, -1);
    if (record_word == cb->item_in_hand && cb->retake_object)
        cb->retake_object(ctx, record_word);
    return 1;
}

/* ---- DM2_F958 (c_item.cpp:1034) ---- */
int16_t dm2_v1_f958(uint16_t record_word,
                     const DM2_V1_ItemValueCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_item_value) return -1;
    int16_t value = cb->query_item_value(ctx, record_word, 2);
    return (value <= -1) ? value : -1;
}

/* ---- DM2_GET_MAX_CHARGE (c_item.cpp:344) ---- */
int16_t dm2_v1_get_max_charge(uint16_t record_word)
{
    if (record_word == OBJECT_NULL_WORD) return 0;
    uint16_t db_type = (record_word >> 10) & 0xF;
    if (db_type < 6) return (db_type == 5) ? 0x0F : 0;
    if (db_type == 6) return 0x0F;
    return (db_type == 0x0A) ? 3 : 0;
}

/* ---- DM2_ADD_ITEM_CHARGE (c_item.cpp:251) ---- */
int16_t dm2_v1_add_item_charge(
    uint16_t record_word, int16_t delta,
    const DM2_V1_ChargeCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD) return 0;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec) return 0;
    uint16_t db_type = (record_word >> 10) & 0xF;
    uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
    int16_t current = 0;
    int16_t max_val = 0;

    if (db_type == 5) {
        current = (int16_t)((w2 * 4) >> 12);
        max_val = 0x0F;
    } else if (db_type == 6) {
        current = (int16_t)((w2 * 8) >> 12);
        max_val = 0x0F;
    } else if (db_type == 0x0A) {
        current = (int16_t)(w2 >> 14);
        max_val = 3;
    } else {
        return 0;
    }

    int16_t new_val = current + delta;
    if (new_val < 0) new_val = 0;
    if (new_val > max_val) new_val = max_val;
    int16_t result = new_val;
    uint16_t masked = (uint16_t)(new_val & 0x0F);

    if (db_type == 5) {
        rec[3] = (rec[3] & 0xC3) | (uint8_t)0;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw = (nw & ~0x3C00) | (masked << 10);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    } else if (db_type == 6) {
        rec[3] = (rec[3] & 0xE1) | (uint8_t)0;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw = (nw & ~0x1E00) | (masked << 9);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    } else { /* 0x0A */
        uint16_t val3 = (uint16_t)(new_val & 0x03);
        rec[3] &= 0x3F;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw |= (val3 << 14);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    }
    return result;
}

/* ---- DM2_QUERY_ITEM_WEIGHT (c_item.cpp:496) ---- */
int16_t dm2_v1_query_item_weight(
    uint16_t record_word,
    const DM2_V1_ItemValueCallbacks *cb, void *ctx)
{
    if (!cb) return 0;
    return cb->query_item_value(ctx, record_word, 1);
}

/* ---- DM2_GET_ITEM_NAME (c_item.cpp:502) ---- */
DM2_V1_ItemNameReceipt dm2_v1_get_item_name(
    uint16_t record_word,
    const DM2_V1_ItemNameCallbacks *cb, void *ctx)
{
    DM2_V1_ItemNameReceipt receipt;
    receipt.name = NULL;
    receipt.hero_index = -1;
    if (!cb) return receipt;

    uint8_t cls1 = cb->query_cls1(ctx, record_word);
    uint8_t cls2 = cb->query_cls2(ctx, record_word);

    /* Check if this is a hero bones item (cls1=0x15, cls2=0) */
    if (cls1 == 0x15 && cls2 == 0) {
        const uint8_t *rec = cb->get_record_address(ctx, record_word);
        if (rec) {
            uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
            int16_t hero_idx = (int16_t)(w2 >> 14);
            if (hero_idx >= 0 && (uint16_t)hero_idx < cb->heros_in_party)
                receipt.hero_index = hero_idx;
        }
    }
    receipt.name = cb->query_gdat_item_name(ctx, cls1, cls2);
    return receipt;
}


/* ---- DM2_TAKE_OBJECT (c_item.cpp:1185) ---- */
void dm2_v1_take_object(
    uint16_t record_word, int deferred,
    const DM2_V1_TakeObjectCallbacks *cb, void *ctx)
{
    if (!cb || record_word == 0xFFFFu)
        return;

    int16_t gdat_word = cb->query_gdat_dbspec_word
                             ? cb->query_gdat_dbspec_word(ctx, record_word, 0)
                             : 0;
    int16_t weight = cb->query_item_weight
                          ? cb->query_item_weight(ctx, record_word)
                          : 0;
    if (cb->set_hand_item)
        cb->set_hand_item(ctx, record_word, gdat_word, weight);
    if (cb->draw_item_in_hand)
        cb->draw_item_in_hand(ctx);
    if (cb->display_item_name)
        cb->display_item_name(ctx, record_word);

    if (deferred == 0) {
        if (cb->process_events)
            cb->process_events(ctx);
    } else {
        if (cb->set_deferred_flag)
            cb->set_deferred_flag(ctx);
    }

    if (cb->process_item_bonus)
        cb->process_item_bonus(ctx, record_word);
    if (cb->moverec_update)
        cb->moverec_update(ctx);
}
