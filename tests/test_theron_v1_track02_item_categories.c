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

    /* Drop item resolver */
    {
        int idx;
        /* WEAPON drops resolve to weapon-category items */
        idx = theron_v1_track02_resolve_drop_item(9, 0); /* THERON_ITEM_WEAPON */
        assert(idx >= 0 && idx < 66);
        assert(theron_v1_track02_item_category((unsigned)idx) == THERON_ITEM_CAT_WEAPON);
        idx = theron_v1_track02_resolve_drop_item(9, 12);
        assert(theron_v1_track02_item_category((unsigned)idx) == THERON_ITEM_CAT_WEAPON);

        /* ARMOR drops resolve to armor-category items */
        idx = theron_v1_track02_resolve_drop_item(10, 0); /* THERON_ITEM_ARMOR */
        assert(theron_v1_track02_item_category((unsigned)idx) == THERON_ITEM_CAT_ARMOR);

        /* POTION drops resolve to consumable-category items */
        idx = theron_v1_track02_resolve_drop_item(1, 0); /* THERON_ITEM_POTION */
        assert(theron_v1_track02_item_category((unsigned)idx) == THERON_ITEM_CAT_CONSUMABLE);

        /* FOOD drops resolve to consumable-category items */
        idx = theron_v1_track02_resolve_drop_item(5, 0); /* THERON_ITEM_FOOD */
        assert(theron_v1_track02_item_category((unsigned)idx) == THERON_ITEM_CAT_CONSUMABLE);

        /* SCROLL resolves to item 3 */
        assert(theron_v1_track02_resolve_drop_item(4, 0) == 3);

        /* KEY resolves to item 64 (TOPAZ KEY) */
        assert(theron_v1_track02_resolve_drop_item(7, 0) == 64);

        /* Unknown returns -1 */
        assert(theron_v1_track02_resolve_drop_item(0, 0) == -1);
        assert(theron_v1_track02_resolve_drop_item(127, 0) == -1);
    }

    printf("PASS: theron_v1_track02_item_categories\n");
    return 0;
}
