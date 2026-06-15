#ifndef THERON_V1_SHOP_H
#define THERON_V1_SHOP_H

#include "theron_v1_champions.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_SHOP_MAX_ENTRIES 32
#define THERON_SHOP_PRICE_ROW_SIZE 4

typedef enum {
    THERON_SHOP_OK = 0,
    THERON_SHOP_BAD_INPUT = -1,
    THERON_SHOP_BAD_TABLE_SIZE = -2,
    THERON_SHOP_TOO_MANY_ENTRIES = -3,
    THERON_SHOP_BAD_ITEM = -4,
    THERON_SHOP_NOT_FOUND = -5,
    THERON_SHOP_OUT_OF_STOCK = -6,
    THERON_SHOP_NOT_ENOUGH_GOLD = -7,
    THERON_SHOP_INVENTORY_FULL = -8
} Theron_ShopStatus;

typedef struct {
    uint8_t item_id;
    uint16_t price;
    uint8_t stock;
} Theron_ShopPriceEntry;

typedef struct {
    Theron_ShopPriceEntry entries[THERON_SHOP_MAX_ENTRIES];
    size_t count;
} Theron_ShopPriceTable;

Theron_ShopStatus theron_v1_shop_parse_price_table(
    Theron_ShopPriceTable *out_table,
    const uint8_t *data,
    size_t size);

Theron_ShopStatus theron_v1_shop_purchase_item(
    Theron_V1_Party *party,
    Theron_ShopPriceTable *table,
    uint8_t item_id,
    int champion_slot);

const char *theron_v1_shop_status_name(Theron_ShopStatus status);
const char *theron_v1_shop_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_SHOP_H */
