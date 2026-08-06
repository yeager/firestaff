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
                                   1, 0, 1, 1) == 1,
          "regular Track02 spawn uses the authenticated category formula");
    creature = theron_v1_creature_by_id(&world, 1);
    CHECK(creature != NULL && creature->hp > 0 &&
              creature->max_hp == creature->hp && creature->attack > 0 &&
              creature->defense > 0,
          "regular spawn publishes only source-derived combat stats");
    CHECK(theron_v1_creature_at(&world, 0, 1, 1) == creature &&
              theron_v1_creature_count(&world, 1, 0) == 1,
          "source-backed creature lookup and count follow the live record");
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
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == 1,
          "regular spawn remains source-admitted after generator rejection");
    creature = theron_v1_creature_by_id(&world, 1);
    CHECK(theron_v1_champion_attack(&world, 0, 1) == -1,
          "combat behavior stays blocked without the source consumer");
    CHECK(theron_v1_champion_cast_spell(&world, 0, 0, -1) == -1,
          "spell behavior stays blocked without the source consumer");
    CHECK(theron_v1_creature_kill(&world, 1) == 0 &&
              !(creature->flags & THERON_CF_ACTIVE),
          "source-backed creature can be retired without fabricating loot");
    CHECK(theron_v1_drop_loot(&world, 1, 1, 1) == -1,
          "production drop publication stays blocked");
    CHECK(theron_v1_sound_is_valid(THERON_SOUND_SWORD_SWING) == 0,
          "unbound sound record stays invalid");
    CHECK(strstr(theron_v1_combat_source_evidence(), "regular") != NULL,
          "production evidence names the narrow regular-spawn boundary");

    if (failures) return 1;
    puts("PASS: Theron production regular-spawn bridge and combat gates are wired");
    return 0;
}
