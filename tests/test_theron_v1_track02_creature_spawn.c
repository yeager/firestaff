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

    /* Entries 0-4 share constant_016b = 0x016B; 5-7 have 0x0000 */
    for (unsigned i = 0; i < 5; i++) {
        const Theron_CreaturePointerEntry *p = theron_v1_track02_creature_pointer(i);
        assert(p->constant_278a == 0x278A);
        assert(p->constant_016b == 0x016B);
    }
    for (unsigned i = 5; i < 8; i++) {
        const Theron_CreaturePointerEntry *p = theron_v1_track02_creature_pointer(i);
        assert(p->constant_278a == 0x278A);
        assert(p->constant_016b == 0x0000);
    }

    /* Entry 7 exists (unused slot) */
    assert(theron_v1_track02_creature_pointer(7) != NULL);
    assert(theron_v1_track02_creature_pointer(7)->sprite_desc_offset == 0x01DC);
    assert(theron_v1_track02_creature_pointer(8) == NULL);

    /* HP cap constant */
    assert(THERON_CREATURE_HP_CAP == 900);

    /* Stat computation: category 0 (dice(4)) */
    {
        Theron_SpawnStats s;
        assert(theron_v1_track02_compute_spawn_stats(0, 14, 2, 0, &s) == 1);
        assert(s.hp >= 14 && s.hp <= 17); /* 0..3 + 14 */
        assert(s.attack >= 2);
        assert(s.defense >= 1);
    }

    /* Stat computation: category 2 (rand*25*1.5 + param1) */
    {
        Theron_SpawnStats s;
        assert(theron_v1_track02_compute_spawn_stats(2, 16, 2, 100, &s) == 1);
        assert(s.hp >= 16);
        assert(s.hp <= THERON_CREATURE_HP_CAP);
        assert(s.attack >= 4);
        assert(s.defense >= 2);
    }

    /* Stat computation: category 3 (dice(5)*scaling + 1.5*adj) */
    {
        Theron_SpawnStats s;
        assert(theron_v1_track02_compute_spawn_stats(3, 14, 2, 42, &s) == 1);
        assert(s.hp >= 1);
        assert(s.hp <= THERON_CREATURE_HP_CAP);
    }

    /* HP cap enforcement */
    {
        Theron_SpawnStats s;
        assert(theron_v1_track02_compute_spawn_stats(3, 200, 200, 255, &s) == 1);
        assert(s.hp <= THERON_CREATURE_HP_CAP);
    }

    /* Unknown categories must not receive invented combat statistics. */
    {
        Theron_SpawnStats s = { 9, 9, 9 };
        assert(theron_v1_track02_compute_spawn_stats(4, 14, 2, 255, &s) == 0);
        assert(s.hp == 0 && s.attack == 0 && s.defense == 0);
    }

    printf("PASS: theron_v1_track02_creature_spawn\n");
    return 0;
}
