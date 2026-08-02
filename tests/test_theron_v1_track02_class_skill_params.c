#include "theron_v1_track02_class_skill_params.h"
#include <assert.h>

int main(void) {
    /* Raw block size */
    assert(theron_v1_track02_class_skill_raw_size() == 110);
    const uint8_t *raw = theron_v1_track02_class_skill_raw();
    assert(raw != NULL);
    assert(raw[0] == 0x28);
    assert(raw[109] == 0x00);

    /* Skill XP cost table: diagonal = 0 (primary skill is free) */
    assert(theron_v1_track02_skill_xp_cost(0, 0) == 0); /* FIGHTER -> Fighter */
    assert(theron_v1_track02_skill_xp_cost(1, 1) == 0); /* NINJA -> Ninja */
    assert(theron_v1_track02_skill_xp_cost(2, 2) == 0); /* PRIEST -> Priest */
    assert(theron_v1_track02_skill_xp_cost(3, 3) == 0); /* WIZARD -> Wizard */

    /* Off-diagonal costs */
    assert(theron_v1_track02_skill_xp_cost(0, 1) == 3); /* FIGHTER -> Ninja */
    assert(theron_v1_track02_skill_xp_cost(1, 0) == 5); /* NINJA -> Fighter (hardest) */
    assert(theron_v1_track02_skill_xp_cost(3, 0) == 1); /* WIZARD -> Fighter */

    /* Full table via pointer */
    const uint8_t *tbl = theron_v1_track02_skill_xp_cost_table();
    assert(tbl != NULL);
    assert(tbl[0] == 0x00); /* FIGHTER Fighter */
    assert(tbl[4] == 0x05); /* NINJA Fighter */

    /* Bounds check */
    assert(theron_v1_track02_skill_xp_cost(4, 0) == 0xFF);
    assert(theron_v1_track02_skill_xp_cost(0, 4) == 0xFF);

    /* Class name offsets */
    const uint8_t *offsets = theron_v1_track02_class_name_offsets();
    assert(offsets[0] == 0x04); /* FIGHTER */
    assert(offsets[1] == 0x0C); /* NINJA */
    assert(offsets[2] == 0x12); /* PRIEST */
    assert(offsets[3] == 0x19); /* WIZARD */

    return 0;
}
