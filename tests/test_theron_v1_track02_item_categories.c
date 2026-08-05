#include "theron_v1_track02_item_categories.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_item_category_count() == 66);

    assert(theron_v1_track02_item_category(0) == THERON_ITEM_CAT_COMPASS);
    assert(theron_v1_track02_item_category(1) == THERON_ITEM_CAT_WEAPON);
    assert(theron_v1_track02_item_category(17) == THERON_ITEM_CAT_WEAPON);
    assert(theron_v1_track02_item_category(18) == THERON_ITEM_CAT_ARMOR);
    assert(theron_v1_track02_item_category(41) == THERON_ITEM_CAT_ARMOR);
    assert(theron_v1_track02_item_category(42) == THERON_ITEM_CAT_CONSUMABLE);
    assert(theron_v1_track02_item_category(65) == THERON_ITEM_CAT_CONSUMABLE);

    assert(theron_v1_track02_item_category(66) == 0);

    /* Count per category: 1 compass + 17 weapons + 24 armor + 24 consumables = 66 */
    int w = 0, a = 0, c = 0, other = 0;
    for (unsigned i = 0; i < 66; i++) {
        uint8_t cat = theron_v1_track02_item_category(i);
        if (cat == THERON_ITEM_CAT_WEAPON) w++;
        else if (cat == THERON_ITEM_CAT_ARMOR) a++;
        else if (cat == THERON_ITEM_CAT_CONSUMABLE) c++;
        else other++;
    }
    assert(w == 17);
    assert(a == 24);
    assert(c == 24);
    assert(other == 1);

    printf("PASS: theron_v1_track02_item_categories\n");
    return 0;
}
