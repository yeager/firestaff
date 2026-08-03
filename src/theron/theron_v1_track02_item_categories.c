#include "theron_v1_track02_item_categories.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Item category table from UD 0x21A046, 66 bytes.
 * [0]=COMPASS(0x22), [1-17]=weapons(0x80), [18-41]=armor(0x81), [42-65]=consumables(0x82). */

static const uint8_t g_categories[THERON_TRACK02_ITEM_CATEGORY_COUNT] = {
    0x22,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
    0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
    0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
};

uint8_t theron_v1_track02_item_category(unsigned int index) {
    if (index >= THERON_TRACK02_ITEM_CATEGORY_COUNT) return 0;
    return g_categories[index];
}

size_t theron_v1_track02_item_category_count(void) {
    return THERON_TRACK02_ITEM_CATEGORY_COUNT;
}

#include "theron_v1_champions.h"

int theron_v1_track02_resolve_drop_item(uint8_t synthetic_id, unsigned int seed) {
    uint8_t target_cat;
    switch (synthetic_id) {
    case THERON_ITEM_WEAPON:
        target_cat = THERON_ITEM_CAT_WEAPON; break;
    case THERON_ITEM_ARMOR:
    case THERON_ITEM_SHIELD:
    case THERON_ITEM_HELM:
    case THERON_ITEM_BOOTS:
        target_cat = THERON_ITEM_CAT_ARMOR; break;
    case THERON_ITEM_POTION:
    case THERON_ITEM_ANTIDOTE:
    case THERON_ITEM_FOOD:
    case THERON_ITEM_WATER:
        target_cat = THERON_ITEM_CAT_CONSUMABLE; break;
    case THERON_ITEM_SCROLL:
        return 3;
    case THERON_ITEM_KEY:
        return 64;
    default:
        return -1;
    }
    unsigned int matches[THERON_TRACK02_ITEM_CATEGORY_COUNT];
    unsigned int count = 0;
    for (unsigned int i = 0; i < THERON_TRACK02_ITEM_CATEGORY_COUNT; i++) {
        if (g_categories[i] == target_cat)
            matches[count++] = i;
    }
    if (count == 0) return -1;
    return (int)matches[seed % count];
}
