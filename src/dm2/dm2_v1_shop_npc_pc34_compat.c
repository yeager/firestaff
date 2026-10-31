/*
 * dm2_v1_shop_npc_pc34_compat.c — DM2 shop and NPC merchant classifier.
 *
 * Source: skproject/SKWINSPX/src/v4/skgame.cpp, skdefine.h, KSK37FC.h
 */

#include "dm2_v1_shop_npc_pc34_compat.h"

#include <string.h>

#define DM2_SHOP_STOCK_ITEM_COUNT 512u

int dm2_v1_shop_select_masked_stock(
    const uint8_t *item_list_bytes,
    size_t item_count,
    int list_words_big_endian,
    uint16_t current_item,
    const uint8_t item_mask[64],
    int direction,
    DM2_V1_ShopStockEligibleFn eligible_fn,
    void *eligible_user,
    DM2_V1_ShopStockSelection *out_selection)
{
    size_t index = 0u;
    size_t attempts;
    size_t limit;
    int found_current = 0;

    if (out_selection) memset(out_selection, 0, sizeof(*out_selection));
    if (!out_selection || !item_mask || direction == 0 ||
        (item_count != 0u && !item_list_bytes) ||
        item_count > (size_t)UINT16_MAX + 1u) return 0;

    if (item_count != 0u) {
        for (index = 0u; index < item_count; ++index) {
            const uint8_t *word = item_list_bytes + index * 2u;
            uint16_t entry = list_words_big_endian
                ? (uint16_t)(((uint16_t)word[0] << 8) | word[1])
                : (uint16_t)(word[0] | ((uint16_t)word[1] << 8));
            if ((entry & 0x01ffu) == (current_item & 0x01ffu)) {
                found_current = 1;
                break;
            }
        }
        if (!found_current) index = item_count;
        limit = item_count;
    } else {
        index = current_item & 0x01ffu;
        limit = DM2_SHOP_STOCK_ITEM_COUNT;
    }

    for (attempts = 0u; attempts < limit; ++attempts) {
        uint16_t raw_entry;
        uint16_t item;

        if (direction > 0) {
            if (index >= limit || index + 1u >= limit) index = 0u;
            else ++index;
        } else {
            if (index == 0u || index >= limit) index = limit - 1u;
            else --index;
        }

        if (item_count != 0u) {
            const uint8_t *word = item_list_bytes + index * 2u;
            raw_entry = list_words_big_endian
                ? (uint16_t)(((uint16_t)word[0] << 8) | word[1])
                : (uint16_t)(word[0] | ((uint16_t)word[1] << 8));
        } else {
            raw_entry = (uint16_t)index;
        }
        item = (uint16_t)(raw_entry & 0x01ffu);
        if ((item_mask[item >> 3] & (uint8_t)(1u << (item & 7u))) == 0u)
            continue;
        if (eligible_fn) {
            int eligible = eligible_fn(item, raw_entry, eligible_user);
            if (eligible < 0) return 0;
            if (eligible == 0) continue;
        }

        out_selection->item = item;
        out_selection->list_index = (uint16_t)index;
        out_selection->raw_list_entry = raw_entry;
        out_selection->from_list = item_count != 0u ? 1u : 0u;
        return 1;
    }
    return 0;
}

int dm2_v1_classify_shop_element(
    int16_t actuator_type,
    int16_t creature_ai_ref,
    DM2_V1_ShopClassification *result)
{
    if (!result) return 0;
    memset(result, 0, sizeof(*result));

    if (actuator_type == DM2_ACTUATOR_TYPE_SHOP_PANEL)
        result->is_shop_panel = 1;

    if (actuator_type == DM2_ACTUATOR_FLOOR_TYPE_SHOP)
        result->is_shop_floor = 1;

    if (creature_ai_ref == DM2_AI_REF_MERCHANT)
        result->is_merchant_npc = 1;

    if (creature_ai_ref == DM2_AI_REF_MERCHANT_GUARD)
        result->is_merchant_guard = 1;

    return (result->is_shop_panel || result->is_shop_floor ||
            result->is_merchant_npc || result->is_merchant_guard) ? 1 : 0;
}
