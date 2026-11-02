#ifndef FIRESTAFF_DM2_V1_SHOP_PRICE_H
#define FIRESTAFF_DM2_V1_SHOP_PRICE_H

#include <stdint.h>

struct DM2_V1_AssetLoader;

typedef enum {
    DM2_V1_SHOP_PRICE_SPX_V4 = 4,
    DM2_V1_SHOP_PRICE_SPX_V5 = 5
} DM2_V1_ShopPriceVariant;

/* Source-owned SPX implementation of SkWinCore::_48ae_05ae_CREATURE.
 * Returns zero when required original GDAT or item-mask data is unavailable. */
int dm2_v1_shop_price_spx_variant(
    const struct DM2_V1_AssetLoader *loader, uint16_t itemspec,
    uint8_t creature_type, uint16_t pool_number,
    DM2_V1_ShopPriceVariant variant, int adjust, int increase,
    uint16_t *out_price);

#endif
