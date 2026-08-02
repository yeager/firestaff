#include "theron_v1_track02_creature_spawn.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_spawn_zone_count() == 5);

    /* AKUTUBA: category 3, 5 creatures, map 47x44 */
    const Theron_SpawnZoneDesc *z0 = theron_v1_track02_spawn_zone(0);
    assert(z0 != NULL);
    assert(z0->map_width == 47);
    assert(z0->map_height == 44);
    assert(z0->category == 3);
    assert(z0->count == 5);
    assert(z0->param1 == 14);

    /* DRATOR: category 2 */
    const Theron_SpawnZoneDesc *z1 = theron_v1_track02_spawn_zone(1);
    assert(z1->category == 2);
    assert(z1->count == 4);

    /* FORMIC and SARMON: same category and count */
    const Theron_SpawnZoneDesc *z2 = theron_v1_track02_spawn_zone(2);
    const Theron_SpawnZoneDesc *z3 = theron_v1_track02_spawn_zone(3);
    assert(z2->category == z3->category);
    assert(z2->count == z3->count);

    /* THIEF/DEMON: no spawn zone (index >= 5) */
    assert(theron_v1_track02_spawn_zone(5) == NULL);
    assert(theron_v1_track02_spawn_zone(6) == NULL);

    /* Category formulas */
    const Theron_SpawnCategoryFormula *f0 = theron_v1_track02_spawn_formula(0);
    assert(f0->dice_param == 4);
    assert(f0->multiplier == 0);

    const Theron_SpawnCategoryFormula *f1 = theron_v1_track02_spawn_formula(1);
    assert(f1->multiplier == 21);

    const Theron_SpawnCategoryFormula *f2 = theron_v1_track02_spawn_formula(2);
    assert(f2->multiplier == 25);
    assert(f2->uses_1_5x == 1);

    const Theron_SpawnCategoryFormula *f3 = theron_v1_track02_spawn_formula(3);
    assert(f3->dice_param == 5);
    assert(f3->uses_1_5x == 1);

    assert(theron_v1_track02_spawn_formula(4) == NULL);

    /* Creature pointer table */
    const Theron_CreaturePointerEntry *p0 = theron_v1_track02_creature_pointer(0);
    assert(p0 != NULL);
    assert(p0->sprite_desc_offset == 0x0172);
    assert(p0->constant_278a == 0x278A);
    assert(p0->spawn_data_offset == 0x0058);
    assert(p0->constant_016b == 0x016B);

    /* AKUTUBA, DRATOR, SARMON share sprite descriptor 0x0172 */
    assert(theron_v1_track02_creature_pointer(1)->sprite_desc_offset == 0x0172);
    assert(theron_v1_track02_creature_pointer(3)->sprite_desc_offset == 0x0172);

    /* THIEF and DEMON: spawn_data_offset == 0 (no regular spawns) */
    assert(theron_v1_track02_creature_pointer(5)->spawn_data_offset == 0x0000);
    assert(theron_v1_track02_creature_pointer(6)->spawn_data_offset == 0x0000);

    /* All entries share constants */
    for (unsigned i = 0; i < 7; i++) {
        const Theron_CreaturePointerEntry *p = theron_v1_track02_creature_pointer(i);
        assert(p->constant_278a == 0x278A);
        assert(p->constant_016b == 0x016B);
    }

    assert(theron_v1_track02_creature_pointer(7) == NULL);

    /* HP cap constant */
    assert(THERON_CREATURE_HP_CAP == 900);

    printf("PASS: theron_v1_track02_creature_spawn\n");
    return 0;
}
