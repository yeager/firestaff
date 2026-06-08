/*
 * theron_v1_shop.c -- fixture-driven Theron V1 shop price-table guard.
 *
 * ReDMCSB source audit: ReDMCSB_WIP20210206/Toolchains/Common/Source has
 * no Theron's Quest shop/price-table code.  This helper therefore does not
 * claim source parity; it locks a small Track 02-facing table contract until
 * the real THQUEST.ASM shop block is identified.
 *
 * Source/evidence:
 *   THQUEST.ASM T560 item table; THQUEST.ASM T800 champion persistence/gold;
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md sections 1, 5.3,
 *   and save offset notes for the gold field.
 */

#include "theron_v1_shop.h"

#include <string.h>

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int is_buyable_item(uint8_t item_id) {
    if (item_id == THERON_ITEM_NONE) return 0;
    if (item_id == THERON_ITEM_GOLD) return 0;
    if (THERON_IS_QUEST_ITEM(item_id)) return 0;
    return item_id <= THERON_ITEM_GAUNTLETS;
}

Theron_ShopStatus theron_v1_shop_parse_price_table(
    Theron_ShopPriceTable *out_table,
    const uint8_t *data,
    size_t size) {

    size_t count;
    size_t i;

    if (!out_table || !data || size == 0) {
        return THERON_SHOP_BAD_INPUT;
    }
    memset(out_table, 0, sizeof(*out_table));

    if ((size % THERON_SHOP_PRICE_ROW_SIZE) != 0u) {
        return THERON_SHOP_BAD_TABLE_SIZE;
    }
    count = size / THERON_SHOP_PRICE_ROW_SIZE;
    if (count > THERON_SHOP_MAX_ENTRIES) {
        return THERON_SHOP_TOO_MANY_ENTRIES;
    }

    for (i = 0; i < count; ++i) {
        const uint8_t *row = data + (i * THERON_SHOP_PRICE_ROW_SIZE);
        const uint8_t item_id = row[0];
        const uint16_t price = rd16le(row + 1);
        const uint8_t stock = row[3];

        if (!is_buyable_item(item_id) || price == 0) {
            memset(out_table, 0, sizeof(*out_table));
            return THERON_SHOP_BAD_ITEM;
        }
        out_table->entries[i].item_id = item_id;
        out_table->entries[i].price = price;
        out_table->entries[i].stock = stock;
    }
    out_table->count = count;
    return THERON_SHOP_OK;
}

static Theron_ShopPriceEntry *find_entry(Theron_ShopPriceTable *table,
                                         uint8_t item_id) {
    size_t i;
    if (!table) return NULL;
    for (i = 0; i < table->count; ++i) {
        if (table->entries[i].item_id == item_id) {
            return &table->entries[i];
        }
    }
    return NULL;
}

static int first_empty_inventory_slot(const Theron_V1_Champion *champion) {
    int i;
    if (!champion) return -1;
    for (i = 0; i < THERON_INVENTORY_SLOTS; ++i) {
        if (champion->inventory[i] == THERON_ITEM_NONE) {
            return i;
        }
    }
    return -1;
}

Theron_ShopStatus theron_v1_shop_purchase_item(
    Theron_V1_Party *party,
    Theron_ShopPriceTable *table,
    uint8_t item_id,
    int champion_slot) {

    Theron_ShopPriceEntry *entry;
    Theron_V1_Champion *champion;
    int inventory_slot;

    if (!party || !table) {
        return THERON_SHOP_BAD_INPUT;
    }
    champion = theron_v1_party_getChampion(party, champion_slot);
    if (!champion) {
        return THERON_SHOP_BAD_INPUT;
    }
    entry = find_entry(table, item_id);
    if (!entry) {
        return THERON_SHOP_NOT_FOUND;
    }
    if (entry->stock == 0) {
        return THERON_SHOP_OUT_OF_STOCK;
    }
    if (party->gold < entry->price) {
        return THERON_SHOP_NOT_ENOUGH_GOLD;
    }
    inventory_slot = first_empty_inventory_slot(champion);
    if (inventory_slot < 0) {
        return THERON_SHOP_INVENTORY_FULL;
    }

    party->gold -= entry->price;
    entry->stock--;
    champion->inventory[inventory_slot] = item_id;
    theron_v1_party_recalculate_loads(party);
    return THERON_SHOP_OK;
}

const char *theron_v1_shop_status_name(Theron_ShopStatus status) {
    switch (status) {
    case THERON_SHOP_OK:
        return "ok";
    case THERON_SHOP_BAD_INPUT:
        return "bad-input";
    case THERON_SHOP_BAD_TABLE_SIZE:
        return "bad-table-size";
    case THERON_SHOP_TOO_MANY_ENTRIES:
        return "too-many-entries";
    case THERON_SHOP_BAD_ITEM:
        return "bad-item";
    case THERON_SHOP_NOT_FOUND:
        return "not-found";
    case THERON_SHOP_OUT_OF_STOCK:
        return "out-of-stock";
    case THERON_SHOP_NOT_ENOUGH_GOLD:
        return "not-enough-gold";
    case THERON_SHOP_INVENTORY_FULL:
        return "inventory-full";
    default:
        return "unknown";
    }
}

const char *theron_v1_shop_source_evidence(void) {
    return "ReDMCSB_WIP20210206/Toolchains/Common/Source contains no Theron "
           "shop code; fixture guard follows THQUEST.ASM T560 item table, "
           "T800 champion persistence/gold, and "
           "docs/source-lock/tqr_v1_phase2_data_formats_H2339.md gold notes.";
}
