#include "theron_v1_combat.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", label); \
        ++failures; \
    } \
} while (0)

int main(void) {
    Theron_V1_World world;
    Theron_V1_Creature *creature;
    memset(&world, 0, sizeof(world));

    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1,
          "source-unverified level cannot create a creature");

    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;
    world.levels[0][0].dungeon_seed = 0x0108e938u;
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1,
          "verified level header without a source monster stays blocked");
    {
        const uint16_t health[4] = { 10u, 20u, 30u, 40u };
        CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x1200u, 0x0042u, 1, 1, 0x0eu, 0u,
                  health, 1u, 0u) == 0,
              "authentic source monster ledger entry binds");
    }
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1 &&
              world.source_monster_count == 1 && world.creature_count == 0,
          "real source monster remains retained while unknown RNG blocks spawn");
    creature = theron_v1_creature_by_id(&world, 1);
    CHECK(creature == NULL && theron_v1_creature_at(&world, 0, 1, 1) == NULL &&
              theron_v1_creature_count(&world, 1, 0) == 0,
          "blocked source occurrence cannot publish a synthetic live creature");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_DEMON,
                                   1, 0, 2, 1) == -1,
          "scripted Demon remains blocked without a spawn record");
    memset(&world, 0, sizeof(world));
    world.current_dungeon = 1;
    world.current_level = 2;
    world.level_loaded[0][2] = 1;
    world.levels[0][2].source_header_verified = 1;
    world.levels[0][2].width = 32;
    world.levels[0][2].height = 27;
    memset(world.levels[0][2].squares, THERON_SQUARE_FLOOR,
           sizeof(world.levels[0][2].squares));
    world.party.leader_x = 0;
    world.party.leader_y = 0;
    theron_v1_world_init_generators(&world);
    world.world_tick = 60;
    theron_v1_world_tick_generators(&world);
    CHECK(world.generator_active_count == 0 && world.creature_count == 0,
          "unbound legacy and source generator records stay out of production");
    memset(&world, 0, sizeof(world));
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;
    world.levels[0][0].dungeon_seed = 0x0108e938u;
    {
        const uint16_t health[4] = { 10u, 20u, 30u, 40u };
        CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x1201u, 0x0043u, 1, 1, 0x0eu, 0u,
                  health, 1u, 0u) == 0,
              "second source monster ledger entry binds");
    }
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1 &&
              world.source_monster_count == 1 && world.creature_count == 0,
          "regular spawn remains blocked until the original RNG consumer is bound");
    creature = theron_v1_creature_by_id(&world, 1);
    CHECK(theron_v1_champion_attack(&world, 0, 1) == -1,
          "combat behavior stays blocked without the source consumer");
    CHECK(theron_v1_champion_cast_spell(&world, 0, 0, -1) == -1,
          "spell behavior stays blocked without the source consumer");
    CHECK(creature == NULL && theron_v1_creature_kill(&world, 1) == -1,
          "blocked source occurrence cannot be retired as a live creature");
    CHECK(theron_v1_drop_loot(&world, 1, 1, 1) == -1,
          "production drop publication stays blocked");
    CHECK(theron_v1_sound_is_valid(THERON_SOUND_SWORD_SWING) == 0,
          "unbound sound record stays invalid");
    CHECK(strstr(theron_v1_combat_source_evidence(), "regular") != NULL &&
              strstr(theron_v1_combat_source_evidence(), "blocked") != NULL,
          "production evidence names the narrow blocked regular-spawn boundary");

    if (failures) return 1;
    puts("PASS: Theron production regular-spawn bridge and combat gates are wired");
    return 0;
}
