#include "theron_v1_combat.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_track02_thing_data.h"
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
    const uint16_t health[4] = { 10u, 20u, 30u, 40u };
    memset(&world, 0, sizeof(world));

    CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x11ffu, 0x0041u, 1, 1, 0x0eu, 0u,
              health, 1u, 0u, 0x0020u, 0x1200u, 0) == -1 && world.source_monster_count == 0,
          "unloaded/unverified level cannot admit source monster record");

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
        CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x1200u, 0x0042u, 1, 1, 0x07u, 4u,
                  health, 4u, 0u, 0x0020u, 0x1200u, 0) == -1 &&
                  world.source_monster_count == 0,
              "invalid source monster type/count cannot enter the ledger");
        CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x1200u, 0x0042u, 1, 1, 0u, 0u,
                  health, 1u, 0u, 0x0020u, 0x1200u, -2) == 0,
              "authentic source monster ledger entry binds");
    }
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) > 0 &&
              world.source_monster_count == 1 && world.creature_count == 2,
          "real source monster group publishes as live creatures");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_DRATOR,
                                   1, 0, 1, 1) == -1 &&
              world.creature_count == 2,
          "a source monster cannot be retyped through the source API");
    creature = theron_v1_creature_at(&world, 0, 1, 1);
    CHECK(creature != NULL && creature->source_ref == 0x1200u &&
              creature->source_chested == -2 &&
              theron_v1_creature_count(&world, 1, 0) == 2,
          "published live creatures retain their authentic source identity");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1 &&
              world.creature_count == 2,
          "the same source group cannot be admitted twice");
    {
        Theron_V1_World sparse_world;
        const uint16_t sparse_health[4] = { 0u, 20u, 0u, 0u };
        memset(&sparse_world, 0, sizeof(sparse_world));
        sparse_world.level_loaded[0][0] = 1;
        sparse_world.levels[0][0].source_header_verified = 1;
        CHECK(theron_v1_world_bind_track02_monster(
                  &sparse_world, 1, 0, 0x1210u, 0x0044u, 2, 2, 0u, 0x09u,
                  sparse_health, 2u, 0u, 0x0020u, 0x1200u, 0) == 0 &&
                  theron_v1_creature_spawn(
                      &sparse_world, THERON_CREATURE_AKUTUBA,
                      1, 0, 2, 2) > 0 && sparse_world.creature_count == 1 &&
                  sparse_world.creatures[0].source_slot == 1u &&
                  sparse_world.creatures[0].hp == 20,
              "sparse source group admits only its real non-zero member");
    }
    world.level_loaded[1][0] = 1;
    world.levels[1][0].source_header_verified = 1;
    CHECK(theron_v1_world_bind_track02_monster(
              &world, 2, 0, 0x1201u, 0x0043u, 1, 1, 0u, 0u,
              health, 0u, 0u, 0x0021u, 0x1201u, 0) == 0,
          "same-coordinate source monster in second dungeon binds");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   2, 0, 1, 1) > 0 &&
              theron_v1_creature_count(&world, 2, 0) == 1 &&
              theron_v1_creature_at_in_dungeon(&world, 2, 0, 1, 1) != NULL,
          "dungeon-aware lookup keeps same-coordinate records separate");
    CHECK(theron_v1_creature_at_in_dungeon(&world, 1, 0, 1, 1) == creature,
          "dungeon-aware lookup returns the first dungeon record");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_DEMON,
                                   1, 0, 2, 1) == -1,
          "scripted Demon remains blocked without a spawn record");
    world.current_dungeon = 1;
    world.current_level = 0;
    world.party.champions[0].alive = 1;
    world.party.champions[0].food = 7;
    world.party.champions[0].water = 8;
    world.party.champions[0].stamina = 9;
    world.party.champions[0].max_stamina = 20;
    world.object_count = 1;
    world.objects[0].type = THERON_OBJTYPE_POOL;
    world.objects[0].x = 3;
    world.objects[0].y = 3;
    world.objects[0].level = 0;
    CHECK(theron_v1_pool_use(&world, 3, 3) == -1 &&
              world.party.champions[0].food == 7 &&
              world.party.champions[0].water == 8 &&
              world.party.champions[0].stamina == 9 &&
              world.objects[0].state != THERON_OBJ_F_USED,
          "source pool cannot apply fixture T700 recovery");
    world.objects[0].type = THERON_OBJTYPE_ALTAR_VI;
    world.party.champions[0].alive = 0;
    world.party.champions[0].health = 0;
    world.party.gold = 1000;
    CHECK(theron_v1_altar_of_vi_resurrect(&world, 3, 3) == -1 &&
              world.party.gold == 1000 && !world.party.champions[0].alive,
          "source altar cannot apply fixture T900 resurrection");
    world.party.champions[0].inventory[0] = 4;
    world.inventory_source[0][0].valid = 1;
    world.inventory_source[0][0].category = THERON_CAT_WEAPON;
    world.inventory_source[0][0].item_type = 4;
    world.inventory_source[0][0].source_ref = 0x1202u;
    world.inventory_source[0][0].property_valid = 0;
    CHECK(theron_v1_drop_inventory_source_item(&world, 0, 0, 3, 3) == -1 &&
              world.object_count == 1 &&
              world.party.champions[0].inventory[0] == 4,
          "source drop cannot recreate an item without property payload");
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
        CHECK(theron_v1_world_bind_track02_monster(
                  &world, 1, 0, 0x1201u, 0x0043u, 1, 1, 0u, 0u,
                  health, 1u, 0u, 0x0020u, 0x1200u, 0) == 0,
              "second source monster ledger entry binds");
    }
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) > 0 &&
              world.source_monster_count == 1 && world.creature_count == 2,
          "source group publishes without entering the random generator path");
    creature = theron_v1_creature_at(&world, 0, 1, 1);
    CHECK(theron_v1_champion_attack(&world, 0, 1) == -1,
          "combat behavior stays blocked without the source consumer");
    CHECK(theron_v1_champion_cast_spell(&world, 0, 0, -1) == -1,
          "spell behavior stays blocked without the source consumer");
    CHECK(creature != NULL && theron_v1_creature_kill(&world, creature->id) == 0,
          "source-backed live creature can be retired without synthetic loot");
    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1 &&
              world.creature_count == 2,
          "a killed static source group stays retired without captured respawn semantics");
    CHECK(theron_v1_drop_loot(&world, 1, 1, 1) == -1,
          "production drop publication stays blocked");
    CHECK(theron_v1_sound_is_valid(THERON_SOUND_SWORD_SWING) == 0,
          "unbound sound record stays invalid");
    CHECK(theron_v1_play_sound(THERON_SOUND_SWORD_SWING) == -1,
          "unbound sound trigger reports blocked instead of false success");
    CHECK(strstr(theron_v1_combat_source_evidence(), "regular") != NULL &&
              strstr(theron_v1_combat_source_evidence(), "blocked") != NULL,
          "production evidence names the narrow blocked regular-spawn boundary");

    {
        Theron_StartupFlow flow;
        Theron_DungeonProgression progression;
        Theron_V1_World startup_world;
        Theron_V1StartupRuntimeEntryRequest request;
        Theron_V1StartupRuntimeEntryResult entry_result;
        uint8_t fake_media = 0;
        char receipt[256];

        theron_v1_dungeon_progression_init(&progression);
        theron_v1_startup_flow_init(&flow);
        CHECK(theron_v1_startup_show_stage_select(
                  &flow, THERON_DUNGEON_1_AKUTUBA) == THERON_STARTUP_OK,
              "startup roster regression reaches stage select");
        CHECK(theron_v1_startup_choose_stage(
                  &flow, &progression, THERON_DUNGEON_1_AKUTUBA) ==
                  THERON_STARTUP_OK,
              "startup roster regression chooses Akutuba");
        CHECK(theron_v1_startup_select_mirror(&flow, 0) == THERON_STARTUP_OK,
              "startup roster regression selects Hakar mirror");

        theron_v1_world_init_runtime(&startup_world);
        memset(&request, 0, sizeof(request));
        request.hucard_rom = &fake_media;
        request.hucard_rom_size = sizeof(fake_media);
        request.md5_hex = THERON_TRACK02_MD5_US_BIN;
        receipt[0] = '\0';
        CHECK(!theron_v1_startup_runtime_enter_from_forcefield(
                  &flow, &startup_world, &request, &entry_result,
                  receipt, sizeof(receipt)),
              "capture gate still blocks fake Track 02 media");
        CHECK(startup_world.party.champion_count == 2 &&
                  startup_world.party.champions[0].health == 175 &&
                  startup_world.party.champions[1].health == 400 &&
                  startup_world.party.champions[1].strength == 60 &&
                  startup_world.party.champions[1].slots[THERON_ESLOT_WEAPON] == -1 &&
                  startup_world.party.champions[1].load == 0,
              "verified startup retains source roster stats while unbound T900 equipment stays gated");
    }

    if (failures) return 1;
    puts("PASS: Theron production regular-spawn bridge and combat gates are wired");
    return 0;
}
