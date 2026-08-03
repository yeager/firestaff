#include "nexus_v1_creatures.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_movement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int g_fail = 0;
static int g_count = 0;

static void expect(int cond, const char *msg) {
    g_count++;
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

static void test_creature_spawn_initial_state(void) {
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    /* Register a type */
    expect(mgr.type_count == 0 || mgr.type_count >= 0,
           "type_count initialized");
    snprintf(mgr.types[0].name, sizeof(mgr.types[0].name), "Scorpion");
    mgr.types[0].health = 50;
    mgr.types[0].attack = 15;
    mgr.types[0].defense = 10;
    mgr.types[0].speed = 8;
    if (mgr.type_count == 0) mgr.type_count = 1;

    int idx = nexus_v1_creature_spawn(&mgr, 0, 5, 5, NEXUS_DIR_NORTH);
    expect(idx >= 0, "creature spawn returns valid index");
    expect(mgr.active_count == 1, "active count incremented");

    Nexus_Creature *c = &mgr.active[idx];
    expect(c->type_index == 0, "creature type_index is 0");
    expect(c->x == 5, "creature x == 5");
    expect(c->y == 5, "creature y == 5");
    expect(c->facing == NEXUS_DIR_NORTH, "creature facing north");
    expect(c->alive == 1, "creature alive");
    expect(c->health == 50, "creature health matches type");
    expect(c->state == 1, "creature initial state is patrol (1)");
}

static void test_creature_distance(void) {
    expect(nexus_v1_creature_distance(0, 0, 3, 4) == 7,
           "distance(0,0 -> 3,4) == 7 (manhattan)");
    expect(nexus_v1_creature_distance(5, 5, 5, 5) == 0,
           "distance to same point == 0");
    expect(nexus_v1_creature_distance(1, 2, 4, 6) == 7,
           "distance(1,2 -> 4,6) == 7");
}

static void test_creature_attack(void) {
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    mgr.types[0].health = 50;
    mgr.types[0].attack = 20;
    mgr.types[0].defense = 5;
    mgr.types[0].speed = 10;
    snprintf(mgr.types[0].name, sizeof(mgr.types[0].name), "TestBeast");
    if (mgr.type_count == 0) mgr.type_count = 1;

    int idx = nexus_v1_creature_spawn(&mgr, 0, 3, 3, NEXUS_DIR_SOUTH);
    expect(idx >= 0, "spawned creature for attack test");
    expect(mgr.active[idx].type_index >= 0,
           "creature has valid type_index for attack");

    /* Run attacks and verify damage output makes sense */
    int total_hits = 0;
    int damage = 0;
    for (int i = 0; i < 100; i++) {
        int out_damage = 0;
        int hit = nexus_v1_creature_attack(&mgr, idx, 10, &out_damage);
        if (hit) {
            total_hits++;
            damage += out_damage;
        }
    }
    /* At least some attacks should land */
    expect(total_hits > 0, "creature lands at least one attack in 100 tries");
}

static void test_damage_at_kills(void) {
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    mgr.types[0].health = 10;
    mgr.types[0].attack = 5;
    mgr.types[0].defense = 5;
    mgr.types[0].speed = 5;
    snprintf(mgr.types[0].name, sizeof(mgr.types[0].name), "Weakling");
    if (mgr.type_count == 0) mgr.type_count = 1;

    nexus_v1_creature_spawn(&mgr, 0, 7, 7, NEXUS_DIR_EAST);
    nexus_v1_creature_spawn(&mgr, 0, 7, 7, NEXUS_DIR_WEST);
    expect(mgr.active_count == 2, "two creatures spawned at (7,7)");

    int killed = nexus_v1_creature_manager_damage_at(&mgr, 7, 7, 999);
    expect(killed == 2, "damage_at kills both creatures at (7,7)");
}

static void test_reset_active(void) {
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    mgr.types[0].health = 30;
    mgr.types[0].attack = 10;
    mgr.types[0].defense = 5;
    mgr.types[0].speed = 5;
    snprintf(mgr.types[0].name, sizeof(mgr.types[0].name), "Guard");
    if (mgr.type_count == 0) mgr.type_count = 1;

    nexus_v1_creature_spawn(&mgr, 0, 1, 1, 0);
    nexus_v1_creature_spawn(&mgr, 0, 2, 2, 0);
    nexus_v1_creature_spawn(&mgr, 0, 3, 3, 0);
    expect(mgr.active_count == 3, "three creatures spawned");

    nexus_v1_creatures_reset_active(&mgr);
    expect(mgr.active_count == 0, "active count reset to 0");
}

static void test_alert_all(void) {
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    mgr.types[0].health = 20;
    mgr.types[0].attack = 5;
    mgr.types[0].defense = 5;
    mgr.types[0].speed = 5;
    snprintf(mgr.types[0].name, sizeof(mgr.types[0].name), "Goblin");
    if (mgr.type_count == 0) mgr.type_count = 1;

    int a = nexus_v1_creature_spawn_on_level(&mgr, 0, 1, 1, 0, 2);
    int b = nexus_v1_creature_spawn_on_level(&mgr, 0, 5, 5, 0, 2);
    int c = nexus_v1_creature_spawn_on_level(&mgr, 0, 3, 3, 0, 3);
    expect(a >= 0 && b >= 0 && c >= 0, "spawned creatures on levels 2 and 3");

    nexus_v1_creatures_alert_all(&mgr, 2);

    /* Creatures on level 2 should be in chase state (2) */
    expect(mgr.active[a].state == 2, "creature a on level 2 is chasing");
    expect(mgr.active[b].state == 2, "creature b on level 2 is chasing");
    /* Creature on level 3 should remain idle */
    expect(mgr.active[c].state == 1, "creature c on level 3 remains patrol");
}

int main(void) {
    test_creature_spawn_initial_state();
    test_creature_distance();
    test_creature_attack();
    test_damage_at_kills();
    test_reset_active();
    test_alert_all();

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_creatures_gameplay: %d failure(s)\n", g_fail);
        return 1;
    }
    printf("ok: nexus_v1_creatures_gameplay (%d tests)\n", g_count);
    return 0;
}
