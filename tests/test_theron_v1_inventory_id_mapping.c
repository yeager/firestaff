#include "theron_v1_champions.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint8_t mapped[66];

    assert(theron_v1_inventory_id_from_source_type(0u) ==
           THERON_ITEM_SOURCE_TYPE_ZERO);
    assert(!theron_v1_inventory_id_matches_source_type(
        THERON_ITEM_NONE, 0u));
    assert(theron_v1_inventory_id_matches_source_type(
        THERON_ITEM_SOURCE_TYPE_ZERO, 0u));

    for (uint8_t raw_type = 1u; raw_type < 66u; ++raw_type) {
        assert(theron_v1_inventory_id_from_source_type(raw_type) == raw_type);
    }

    for (uint8_t raw_type = 0u; raw_type < 66u; ++raw_type) {
        mapped[raw_type] =
            theron_v1_inventory_id_from_source_type(raw_type);
        assert(mapped[raw_type] != THERON_ITEM_NONE);
        assert(mapped[raw_type] != THERON_ITEM_GOLD);
        assert(mapped[raw_type] < THERON_ITEM_QUEST_BASE);
        for (uint8_t earlier = 0u; earlier < raw_type; ++earlier) {
            assert(mapped[raw_type] != mapped[earlier]);
        }
        assert(theron_v1_inventory_id_matches_source_type(
            mapped[raw_type], raw_type));
    }

    puts("Theron inventory source ID mapping: PASS");
    return 0;
}
