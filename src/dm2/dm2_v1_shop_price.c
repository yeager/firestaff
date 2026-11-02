/* Source-owned SPX v4 merchant price query.
 * Reference: SKWINSPX/src/v4/skcore.cpp::_48ae_05ae_CREATURE. */

#include "dm2_v1_shop_price.h"

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_record_ops_pc34_compat.h"
#include "dm2_v1_skproject_core.h"

#include <stddef.h>
#include <string.h>

static int dm2_shop_price_word(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    uint16_t *value)
{
    return dm2_v1_asset_load_word_value(loader, category, index, field, value);
}

static int dm2_shop_price_apply_creature_mask(
    const DM2_V1_AssetLoader *loader, uint8_t creature_type,
    uint8_t mask_creature, uint8_t word_field, uint16_t mask_field,
    uint16_t item_index,
    uint16_t item_bit, uint16_t *value)
{
    uint16_t percent = 0u;
    uint8_t item_mask[64];

    /* QUERY_GDAT_ENTRY_DATA_INDEX yields zero for an absent optional word. */
    (void)dm2_shop_price_word(loader, DM2_GDAT_CATEGORY_CREATURES,
                              creature_type, word_field, &percent);
    if (percent == 0u) return 1;
    if (!dm2_v1_query_creatures_item_mask_bytes(
            loader, mask_creature, mask_field, 0, item_mask))
        return 1;
    if ((item_mask[item_index] & (uint8_t)(1u << item_bit)) != 0u)
        *value = (uint16_t)(((uint32_t)*value * percent) / 100u);
    return 1;
}

int dm2_v1_shop_price_spx_variant(
    const DM2_V1_AssetLoader *loader, uint16_t itemspec,
    uint8_t creature_type, uint16_t pool_number,
    DM2_V1_ShopPriceVariant variant, int adjust, int increase,
    uint16_t *out_price)
{
    uint16_t value = 0u;
    uint16_t item_word = itemspec;
    uint16_t item_index = (uint16_t)(item_word >> 3);
    uint16_t slot_word = pool_number;
    uint16_t first_mask_word;
    uint8_t first_word_index;
    uint16_t first_mask_index;
    uint8_t second_word_index;
    uint16_t second_mask_index;
    uint8_t second_mask_creature = creature_type;
    uint8_t adjust_word_index;
    uint16_t charge_bonus = 0u;
    uint16_t adjust_percent = 0u;
    int16_t item_db;
    int16_t item_type;
    uint8_t item_category = 0xffu;
    DM2_V1_SkprojectQueryCls1Receipt item_category_receipt;
    uint16_t charges;

    if (out_price) *out_price = 0u;
    if (!loader || !loader->loaded || !out_price || item_index >= 64u ||
        (variant != DM2_V1_SHOP_PRICE_SPX_V4 &&
         variant != DM2_V1_SHOP_PRICE_SPX_V5))
        return 0;

    item_type = dm2_v1_get_itemtype_of_itemspec_actuator(item_word);
    item_db = dm2_v1_get_itemdb_of_itemspec_actuator(item_word);
    if (item_db < 0 || item_db > 0x0f || item_type < 0 || item_type > 0xff)
        return 0;
    if (!dm2_v1_skproject_query_cls1_from_record(
            (uint16_t)((uint16_t)item_db << 10), &item_category,
            &item_category_receipt) || !item_category_receipt.valid ||
        item_category == 0xffu)
        return 0;
    /* Missing source word values have the same zero result as QUERY_GDAT. */
    (void)dm2_shop_price_word(loader, item_category, item_type, 0x02, &value);

    /* Exact version-specific U16/U8 narrowing from the v4/v5 sources. */
    if (variant == DM2_V1_SHOP_PRICE_SPX_V4) {
        first_mask_word = (uint16_t)(slot_word * 3u + 18u);
        first_word_index = (uint8_t)(first_mask_word * 3u + 0xa2u);
        first_mask_index = (uint8_t)first_mask_word;
        second_word_index = (uint8_t)(slot_word * 3u + 0xa1u);
        second_mask_index = (uint8_t)(slot_word * 3u + 17u);
    } else {
        first_mask_word = (uint16_t)(slot_word * 3u + 0x12u);
        first_word_index = (uint8_t)((uint8_t)slot_word * 3u + 0xa2u);
        first_mask_index = first_mask_word;
        second_word_index = (uint8_t)((uint8_t)slot_word * 3u + 0xa1u);
        second_mask_index = (uint16_t)(slot_word * 3u + 0x11u);
        /* SKProject v5 has a TODO at the second mask owner (0x64 vs pointer). */
        second_mask_creature = 0x64u;
    }
    adjust_word_index = (uint8_t)((uint8_t)(slot_word * 3u) + 0xa0u);

    if (!dm2_shop_price_apply_creature_mask(
            loader, creature_type, creature_type, first_word_index,
            first_mask_index,
            item_index, (uint16_t)(item_word & 7u), &value) ||
        !dm2_shop_price_apply_creature_mask(
            loader, creature_type, second_mask_creature, second_word_index,
            second_mask_index,
            item_index, (uint16_t)(item_word & 7u), &value))
        return 0;

    /* GET_ITEM_MAX_CHARGE(ObjectID(0, item_db, 0)); charge arithmetic is U16. */
    charges = (uint16_t)dm2_v1_get_max_charge(
        (uint16_t)((uint16_t)item_db << 10));
    (void)dm2_shop_price_word(loader, item_category, item_type, 0x35,
                              &charge_bonus);
    value = (uint16_t)(value + (uint16_t)(charge_bonus * charges));

    (void)dm2_shop_price_word(loader, DM2_GDAT_CATEGORY_CREATURES,
                              creature_type, adjust_word_index,
                              &adjust_percent);
    if (adjust && adjust_percent == 0u) adjust_percent = 10u;
    if (adjust) {
        uint16_t delta = (uint16_t)(((uint32_t)value * adjust_percent) / 100u);
        if (increase) value = (uint16_t)(value + delta);
        else if (variant == DM2_V1_SHOP_PRICE_SPX_V5) {
            if (value > 1u) {
                if (delta < 1u) delta = 1u;
                value = (uint16_t)(value - delta);
            }
        } else {
            value = (uint16_t)(value - delta);
        }
    }

    *out_price = value;
    return 1;
}
