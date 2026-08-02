#include "theron_v1_track02_item_properties.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_item_property_count() == 66);

    /* COMPASS: b0=0x00 */
    const Theron_ItemPropertyRecord *p0 = theron_v1_track02_item_property(0);
    assert(p0 != NULL);
    assert(p0->b0 == 0x00);
    assert(p0->b1 == 0x01);
    assert(p0->b2 == 0x82);

    /* THE RETALIATOR: maxed stats b2=0xFF, b5=0xFF */
    const Theron_ItemPropertyRecord *p7 = theron_v1_track02_item_property(7);
    assert(p7->b0 == 0x09);
    assert(p7->b2 == 0xFF);
    assert(p7->b5 == 0xFF);

    /* DELTA: high stats */
    const Theron_ItemPropertyRecord *p5 = theron_v1_track02_item_property(5);
    assert(p5->b0 == 0x20);
    assert(p5->b4 == 0x6E);

    /* VEN POTION */
    const Theron_ItemPropertyRecord *p47 = theron_v1_track02_item_property(47);
    assert(p47->b0 == 0x02);
    assert(p47->b1 == 0x10);

    /* FUL BOMB: high bit set in b0 */
    const Theron_ItemPropertyRecord *p60 = theron_v1_track02_item_property(60);
    assert(p60->b0 == 0x82);

    /* Out of bounds */
    assert(theron_v1_track02_item_property(66) == NULL);

    printf("PASS: theron_v1_track02_item_properties\n");
    return 0;
}
