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
    w->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
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
        c->stamina = 50;
        c->max_stamina = 50;
        c->mana = 10;
        c->max_mana = 10;
        c->strength = 14;
        c->dexterity = 12;
        c->vitality = 12;
        c->anti_magic = 2;
        c->food = 50;
        c->water = 50;
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

    int cid = theron_v1_creature_spawn(&w, THERON_CREATURE_KOBOLD,
                                       w.current_dungeon, w.current_level,
                                       9, 8);
    int killed = 0;
    int attacks = 0;
    while (attacks < 20) {
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

static void test_source_evidence(void) {
    printf("[test:source_evidence]\n");
    const char *ev = theron_v1_combat_source_evidence();
    CHECK(ev != NULL && ev[0] != '\0', "combat source evidence is present");
    CHECK(ev != NULL && strstr(ev, "THQUEST.ASM") != NULL,
          "combat evidence cites THQUEST.ASM");
}

int main(void) {
    printf("=== Theron V1 Combat Mechanics Regression ===\n");

    test_sound_validation();
    test_creature_spawn_and_lookup();
    test_champion_attack_kills_creature();
    test_creature_attack_champion();
    test_creature_drop_loot();
    test_hp_modification_clamps();
    test_source_evidence();

    printf("\nResults: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
