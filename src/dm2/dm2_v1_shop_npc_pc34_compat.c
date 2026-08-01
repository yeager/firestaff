/*
 * dm2_v1_shop_npc_pc34_compat.c — DM2 shop and NPC merchant classifier.
 *
 * Source: skproject/SKWINSPX/src/v4/skgame.cpp, skdefine.h, KSK37FC.h
 */

#include "dm2_v1_shop_npc_pc34_compat.h"

#include <string.h>

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
