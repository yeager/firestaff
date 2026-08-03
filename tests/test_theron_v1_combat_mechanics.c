/*
 * test_theron_v1_combat_mechanics.c
 *
 * Theron's Quest V1 — source-locked combat + creature mechanics regression.
 *
 * Headless, no real Track 02 data required.  Exercises the creature spawn,
 * champion attack, creature attack, drop, sound validation, and source-evidence
 * APIs added/implemented for Lane E cycle 12.
 *
 * Source: THQUEST.ASM T500/T600/T700/T900
 *         ReDMCSB GROUP.C / COMMAND.C / CLIKMENU.C / GAMELOOP.C analogues
 */

#include "theron_v1_champions.h"
#include "theron_v1_combat.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, label) do { \
    if (cond) { \
        g_pass++; \
    } else { \
        printf("  FAIL: %s\n", label); \
        g_fail++; \
    } \
} while (0)

#define CHECK_INT(label, got, want) do { \
    int g_ = (got); \
    int w_ = (want); \
    if (g_ == w_) { \
        g_pass++; \
    } else { \
        printf("  FAIL: %s — got %d want %d\n", label, g_, w_); \
        g_fail++; \
    } \
} while (0)

static void make_world(Theron_V1_World *w) {
    memset(w, 0, sizeof(*w));
    w->current_dungeon = THERON_DUNGEON_1_AKUTUBA;
    w->current_level = 0;

    Theron_V1_Level *lvl = &w->levels[0][0];
    lvl->width = 16;
    lvl->height = 16;
    lvl->level_index = 0;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            lvl->squares[y][x] = THERON_SQUARE_FLOOR;
        }
    }
    w->level_loaded[0][0] = 1;

    w->party.leader_x = 8;
    w->party.leader_y = 8;
    w->party.leader_dir = THERON_DIR_EAST;
    w->party.active_slot = 0;
    w->party.gold = 100;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &w->party.champions[i];
        c->alive = 1;
        c->health = 50;
        c->max_health = 50;
        c->stamina = 500;
        c->max_stamina = 500;
        c->mana = 10;
        c->max_mana = 10;
        c->strength = 30;
        c->dexterity = 12;
        c->vitality = 12;
        c->anti_magic = 2;
        c->food = 50;
        c->water = 50;
        for (int s = 0; s < THERON_EQUIP_SLOT_COUNT; s++)
            c->slots[s] = -1;
    }
}

static void test_sound_validation(void) {
    printf("[test:sound_validation]\n");
    CHECK_INT("BOOT_MUSIC valid",
              theron_v1_sound_is_valid(THERON_SOUND_BOOT_MUSIC), 1);
    CHECK_INT("negative id invalid",
              theron_v1_sound_is_valid((Theron_SoundID)-1), 0);
    CHECK_INT("COUNT id invalid",
              theron_v1_sound_is_valid(THERON_SOUND_COUNT), 0);
}

static void test_creature_spawn_and_lookup(void) {
    printf("[test:creature_spawn_and_lookup]\n");
    Theron_V1_World w;
    make_world(&w);

    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_GOBLIN,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    CHECK_INT("spawn returns positive id", cid > 0, 1);
    CHECK_INT("creature count is 1",
              theron_v1_creature_count(&w, w.current_dungeon, w.current_level), 1);

    Theron_V1_Creature *c = theron_v1_creature_at(&w, w.current_level, 9, 8);
    CHECK(c != NULL, "creature_at finds spawned creature");
    if (c) {
        CHECK_INT("creature type is goblin", c->type, THERON_CREATURE_GOBLIN);
        CHECK_INT("creature is active", (c->flags & THERON_CF_ACTIVE) != 0, 1);
    }

    Theron_V1_Creature *missing = theron_v1_creature_at(&w, w.current_level, 7, 7);
    CHECK(missing == NULL, "creature_at returns NULL for empty square");
}

static void test_champion_attack_kills_creature(void) {
    printf("[test:champion_attack_kills_creature]\n");
    Theron_V1_World w;
    make_world(&w);

    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_AKUTUBA,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    int killed = 0;
    int attacks = 0;
    while (attacks < 100) {
        int rc = theron_v1_champion_attack(&w, 0, cid);
        if (rc < 0) break;
        attacks++;
        Theron_V1_Creature *c = theron_v1_creature_by_id(&w, cid);
        if (!c || !(c->flags & THERON_CF_ACTIVE)) {
            killed = 1;
            break;
        }
    }
    CHECK_INT("creature eventually killed", killed, 1);
    CHECK_INT("object count increased from drops", w.object_count > 0, 1);
}

static void test_creature_attack_champion(void) {
    printf("[test:creature_attack_champion]\n");
    Theron_V1_World w;
    make_world(&w);

    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_ORC,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    int hp_before = w.party.champions[0].health;
    Theron_CombatResult r = theron_v1_creature_attack_champion(&w, cid, 0);
    CHECK_INT("creature attack returns HIT or KILL",
              r == THERON_COMBAT_HIT || r == THERON_COMBAT_KILL, 1);
    CHECK_INT("champion health decreased", w.party.champions[0].health < hp_before, 1);
}

static void test_creature_drop_loot(void) {
    printf("[test:creature_drop_loot]\n");
    Theron_V1_World w;
    make_world(&w);

    int before = w.object_count;
    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_GOBLIN,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    theron_v1_drop_loot(&w, cid, 9, 8);
    CHECK_INT("drop_loot increases object count", w.object_count > before, 1);
}

static void test_hp_modification_clamps(void) {
    printf("[test:hp_modification_clamps]\n");
    Theron_V1_Champion c;
    memset(&c, 0, sizeof(c));
    c.health = 20;
    c.max_health = 50;

    theron_v1_modify_champion_hp(&c, 100);
    CHECK_INT("HP clamps to max", c.health, 50);

    theron_v1_modify_champion_hp(&c, -200);
    CHECK_INT("HP clamps to zero", c.health, 0);
}

static void test_spell_casting(void) {
    printf("[test:spell_casting]\n");
    Theron_V1_World w;
    make_world(&w);
    w.party.champions[0].mana = 100;
    w.party.champions[0].max_mana = 100;
    w.party.champions[0].anti_magic = 20;

    /* HEAL (index 35, cost 2) — self-heal */
    w.party.champions[0].health = 20;
    int rc = theron_v1_champion_cast_spell(&w, 0, 35, -1);
    CHECK_INT("HEAL succeeds", rc, 0);
    CHECK_INT("HEAL restores HP", w.party.champions[0].health > 20, 1);
    CHECK_INT("HEAL deducts mana", w.party.champions[0].mana < 100, 1);

    /* Invalid spell index */
    rc = theron_v1_champion_cast_spell(&w, 0, 5, -1);
    CHECK_INT("non-spell index rejected", rc, -1);

    /* FIREBALL (index 20, cost 42) — offensive */
    w.party.champions[0].mana = 100;
    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_AKUTUBA,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    rc = theron_v1_champion_cast_spell(&w, 0, 20, cid);
    CHECK_INT("FIREBALL returns 0 or 1", rc >= 0, 1);
    CHECK_INT("FIREBALL costs mana", w.party.champions[0].mana < 100, 1);

    /* Insufficient mana */
    w.party.champions[0].mana = 1;
    rc = theron_v1_champion_cast_spell(&w, 0, 20, cid);
    CHECK_INT("insufficient mana rejected", rc, -1);
}

static void test_source_evidence(void) {
    printf("[test:source_evidence]\n");
    const char *ev = theron_v1_combat_source_evidence();
    CHECK(ev != NULL && ev[0] != '\0', "combat source evidence is present");
    CHECK(ev != NULL && strstr(ev, "THQUEST.ASM") != NULL,
          "combat evidence cites THQUEST.ASM");
}

static void test_object_table_apply_to_world(void) {
    printf("[test:object_table_apply_to_world]\n");
    Theron_V1_World w;
    Theron_Track02ObjectTable table = {0};
    Theron_V1_Object *o;

    make_world(&w);

    table.declared_record_count = 3u;
    table.record_count = 3u;

    /* Door at (9,8), locked. */
    table.records[0].object_id = 1u;
    table.records[0].kind = THERON_OBJTYPE_DOOR;
    table.records[0].x = 9u;
    table.records[0].y = 8u;
    table.records[0].level_index = 0u;
    table.records[0].flags = THERON_DOOR_F_LOCKED;

    /* Teleporter at (7,8) with link argument 5. */
    table.records[1].object_id = 2u;
    table.records[1].kind = THERON_OBJTYPE_TELEPORTER;
    table.records[1].x = 7u;
    table.records[1].y = 8u;
    table.records[1].level_index = 0u;
    table.records[1].argument = 5u;

    /* Key item at (8,9). */
    table.records[2].object_id = 3u;
    table.records[2].kind = THERON_OBJTYPE_KEY;
    table.records[2].x = 8u;
    table.records[2].y = 9u;
    table.records[2].level_index = 0u;

    CHECK_INT("apply_track02_object_table succeeds",
              theron_v1_world_apply_track02_object_table(
                  &w, w.current_dungeon, w.current_level, &table), 0);
    CHECK_INT("three objects placed", w.object_count, 3);
    CHECK_INT("door grid tile set",
              w.levels[0][0].squares[8][9], THERON_SQUARE_DOOR);
    CHECK_INT("teleporter grid tile set",
              w.levels[0][0].squares[8][7], THERON_SQUARE_TELEPORTER);

    o = theron_v1_object_at(&w, 0, 9, 8);
    CHECK(o != NULL && o->type == THERON_OBJTYPE_DOOR,
          "door object placed at (9,8)");
    if (o) {
        CHECK_INT("door locked flag preserved",
                  (o->flags & THERON_DOOR_F_LOCKED) != 0, 1);
    }

    o = theron_v1_object_at(&w, 0, 7, 8);
    CHECK(o != NULL && o->type == THERON_OBJTYPE_TELEPORTER,
          "teleporter object placed at (7,8)");
    if (o) {
        CHECK_INT("teleporter linked_id from argument", o->linked_id, 5);
    }

    o = theron_v1_object_at(&w, 0, 8, 9);
    CHECK(o != NULL && o->type == THERON_OBJTYPE_KEY,
          "key object placed at (8,9)");
}

static void test_door_mechanics(void) {
    printf("[test:door_mechanics]\n");
    Theron_V1_World w;
    Theron_V1_Object door = {0};
    int moved;

    make_world(&w);
    w.levels[0][0].squares[8][9] = THERON_SQUARE_DOOR;
    door.type = THERON_OBJTYPE_DOOR;
    door.x = 9;
    door.y = 8;
    door.level = 0;
    door.dungeon_id = w.current_dungeon;
    door.state = THERON_DOOR_STATE_CLOSED;
    door.flags = THERON_DOOR_F_LOCKED;
    theron_v1_object_place(&w, &door);

    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;
    /* Locked doors require the active champion's source-owned key item. */
    w.party.champions[w.party.active_slot].inventory[0] = THERON_ITEM_KEY;

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("closed door blocks movement", moved, THERON_MOVE_BLOCKED);
    CHECK_INT("position unchanged on blocked door", w.party.leader_x, 8);

    CHECK_INT("door_open succeeds", theron_v1_door_open(&w, 9, 8), 0);

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("open door allows movement", moved, THERON_MOVE_OK);
    CHECK_INT("position after open door", w.party.leader_x, 9);
}

static void test_pit_mechanics(void) {
    printf("[test:pit_mechanics]\n");
    Theron_V1_World w;
    int hp_before, moved;

    make_world(&w);
    w.levels[0][0].squares[8][9] = THERON_SQUARE_PIT;
    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;

    hp_before = w.party.champions[0].health;
    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("pit fall returns PIT_FALL", moved, THERON_MOVE_PIT_FALL);
    CHECK_INT("pit fall lands on pit square", w.party.leader_x, 9);
    CHECK_INT("pit fall damages health",
              w.party.champions[0].health < hp_before, 1);
}

static void test_teleporter_mechanics(void) {
    printf("[test:teleporter_mechanics]\n");
    Theron_V1_World w;
    Theron_V1_Object teleporter = {0};
    Theron_V1_Object target = {0};
    int moved;

    make_world(&w);
    w.levels[0][0].squares[8][9] = THERON_SQUARE_TELEPORTER;

    teleporter.type = THERON_OBJTYPE_TELEPORTER;
    teleporter.x = 9;
    teleporter.y = 8;
    teleporter.level = 0;
    teleporter.dungeon_id = w.current_dungeon;
    teleporter.linked_id = 2;
    theron_v1_object_place(&w, &teleporter);

    /* Non-teleporter target object ends the chain at (12,12). */
    target.type = THERON_OBJTYPE_KEY;
    target.x = 12;
    target.y = 12;
    target.level = 0;
    target.dungeon_id = w.current_dungeon;
    theron_v1_object_place(&w, &target);

    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("teleporter move returns TELEPORT", moved, THERON_MOVE_TELEPORT);
    CHECK_INT("teleporter lands at target x", w.party.leader_x, 12);
    CHECK_INT("teleporter lands at target y", w.party.leader_y, 12);
}

static void test_altar_resurrect_mechanics(void) {
    printf("[test:altar_resurrect_mechanics]\n");
    Theron_V1_World w;
    Theron_V1_Object altar = {0};
    int moved;

    make_world(&w);
    w.party.gold = 1000u;
    w.party.champions[1].alive = 0;
    w.party.champions[1].health = 0;

    altar.type = THERON_OBJTYPE_ALTAR_VI;
    altar.x = 9;
    altar.y = 8;
    altar.level = 0;
    altar.dungeon_id = w.current_dungeon;
    theron_v1_object_place(&w, &altar);

    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("altar move returns OK", moved, THERON_MOVE_OK);
    CHECK_INT("dead champion revived", w.party.champions[1].alive, 1);
    CHECK_INT("resurrection gold cost spent", w.party.gold, 500u);
}

static void test_mechanics_sound_ids_valid(void) {
    printf("[test:mechanics_sound_ids_valid]\n");
    CHECK_INT("door open sound valid",
              theron_v1_sound_is_valid(THERON_SOUND_DOOR_OPEN), 1);
    CHECK_INT("door close sound valid",
              theron_v1_sound_is_valid(THERON_SOUND_DOOR_CLOSE), 1);
    CHECK_INT("pit fall sound valid",
              theron_v1_sound_is_valid(THERON_SOUND_PIT_FALL), 1);
    CHECK_INT("teleport sound valid",
              theron_v1_sound_is_valid(THERON_SOUND_TELEPORT), 1);
    CHECK_INT("altar use sound valid",
              theron_v1_sound_is_valid(THERON_SOUND_ALTAR_USE), 1);
}

static void load_level_1(Theron_V1_World *w) {
    Theron_V1_Level *lvl = &w->levels[0][1];
    lvl->width = 16;
    lvl->height = 16;
    lvl->level_index = 1;
    lvl->start_x = 2;
    lvl->start_y = 2;
    lvl->start_dir = THERON_DIR_NORTH;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            lvl->squares[y][x] = THERON_SQUARE_FLOOR;
        }
    }
    w->level_loaded[0][1] = 1;
}

static void test_multi_level_object_table_apply(void) {
    printf("[test:multi_level_object_table_apply]\n");
    Theron_V1_World w;
    Theron_Track02ObjectTable table = {0};
    Theron_V1_Object *o;

    make_world(&w);
    load_level_1(&w);

    table.declared_record_count = 3u;
    table.record_count = 3u;

    /* Door on level 0. */
    table.records[0].object_id = 1u;
    table.records[0].kind = THERON_OBJTYPE_DOOR;
    table.records[0].x = 9u;
    table.records[0].y = 8u;
    table.records[0].level_index = 0u;

    /* Pit on level 1. */
    table.records[1].object_id = 2u;
    table.records[1].kind = THERON_OBJTYPE_PIT;
    table.records[1].x = 5u;
    table.records[1].y = 5u;
    table.records[1].level_index = 1u;

    /* Sound trigger on level 1. */
    table.records[2].object_id = 3u;
    table.records[2].kind = THERON_OBJTYPE_SOUND;
    table.records[2].x = 6u;
    table.records[2].y = 6u;
    table.records[2].level_index = 1u;
    table.records[2].argument = THERON_SOUND_STAIRS;

    CHECK_INT("apply table across dungeon succeeds",
              theron_v1_world_apply_track02_object_table_for_dungeon(
                  &w, w.current_dungeon, &table), 0);

    CHECK_INT("door placed on level 0",
              w.levels[0][0].squares[8][9], THERON_SQUARE_DOOR);
    CHECK_INT("pit placed on level 1",
              w.levels[0][1].squares[5][5], THERON_SQUARE_PIT);

    o = theron_v1_object_at(&w, 1, 6, 6);
    CHECK(o != NULL && o->type == THERON_OBJTYPE_SOUND,
          "sound object placed on level 1");
    if (o) {
        CHECK_INT("sound object stores sound id",
                  o->quantity, THERON_SOUND_STAIRS);
    }
}

static void test_level_transition_stairs(void) {
    printf("[test:level_transition_stairs]\n");
    Theron_V1_World w;
    int moved;

    make_world(&w);
    load_level_1(&w);
    w.levels[0][0].squares[8][9] = THERON_SQUARE_STAIRS_DOWN;

    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("stairs move returns STAIRS", moved, THERON_MOVE_STAIRS);
    CHECK_INT("current level changed to 1", w.current_level, 1);
    CHECK_INT("party x on new level", w.party.leader_x, 9);
    CHECK_INT("party y on new level", w.party.leader_y, 8);
}

static void test_between_dungeon_exit(void) {
    printf("[test:between_dungeon_exit]\n");
    Theron_V1_World w;
    int moved;

    make_world(&w);
    w.dungeon_complete = 1;
    w.levels[0][0].squares[8][9] = THERON_SQUARE_EXIT;

    w.party.leader_x = 8;
    w.party.leader_y = 8;
    w.party.leader_dir = THERON_DIR_EAST;

    moved = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK_INT("exit move returns EXIT", moved, THERON_MOVE_EXIT);
    CHECK_INT("dungeon advanced to Crypt of Shadows",
              w.current_dungeon, THERON_DUNGEON_2_DRATOR);
    CHECK_INT("new dungeon starts at level 0", w.current_level, 0);
    CHECK_INT("dungeon_complete reset", w.dungeon_complete, 0);
}

static void test_decode_dungeon_level_object_table(void) {
    printf("[test:decode_dungeon_level_object_table]\n");
    Theron_Track02DungeonRoute route = {0};
    Theron_Track02ObjectTable table = {0};
    Theron_Track02SignalStatus status;

    route.valid = 1;
    route.status = THERON_TRACK02_DUNGEON_ROUTE_OK;
    route.dungeon_id = THERON_DUNGEON_1_AKUTUBA;
    route.level_index = 1;
    route.objects.record_count = 2u;

    route.objects.records[0].object_id = 1u;
    route.objects.records[0].kind = THERON_OBJTYPE_DOOR;
    route.objects.records[0].x = 3u;
    route.objects.records[0].y = 3u;
    route.objects.records[0].level_index = 0u;

    route.objects.records[1].object_id = 2u;
    route.objects.records[1].kind = THERON_OBJTYPE_PIT;
    route.objects.records[1].x = 4u;
    route.objects.records[1].y = 4u;
    route.objects.records[1].level_index = 1u;

    status = theron_v1_track02_decode_dungeon_level_object_table(
        &route, 1, &table);
    CHECK_INT("decode level 1 returns OK",
              status, THERON_TRACK02_SIGNAL_OK);
    CHECK_INT("level 1 table has one record", (int)table.record_count, 1);
    if (table.record_count > 0u) {
        CHECK_INT("level 1 record is pit",
                  table.records[0].kind, THERON_OBJTYPE_PIT);
    }

    memset(&table, 0, sizeof(table));
    status = theron_v1_track02_decode_dungeon_level_object_table(
        &route, 0, &table);
    CHECK_INT("decode level 0 returns OK",
              status, THERON_TRACK02_SIGNAL_OK);
    CHECK_INT("level 0 table has one record", (int)table.record_count, 1);
    if (table.record_count > 0u) {
        CHECK_INT("level 0 record is door",
                  table.records[0].kind, THERON_OBJTYPE_DOOR);
    }

    route.valid = 0;
    memset(&table, 0, sizeof(table));
    status = theron_v1_track02_decode_dungeon_level_object_table(
        &route, 1, &table);
    CHECK_INT("invalid route returns NOT_FOUND",
              status, THERON_TRACK02_SIGNAL_NOT_FOUND);
}

int main(void) {
    printf("=== Theron V1 Combat Mechanics Regression ===\n");

    test_sound_validation();
    test_creature_spawn_and_lookup();
    test_champion_attack_kills_creature();
    test_creature_attack_champion();
    test_creature_drop_loot();
    test_hp_modification_clamps();
    test_object_table_apply_to_world();
    test_door_mechanics();
    test_pit_mechanics();
    test_teleporter_mechanics();
    test_altar_resurrect_mechanics();
    test_mechanics_sound_ids_valid();
    test_multi_level_object_table_apply();
    test_level_transition_stairs();
    test_between_dungeon_exit();
    test_decode_dungeon_level_object_table();
    test_spell_casting();
    test_source_evidence();

    printf("\nResults: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
