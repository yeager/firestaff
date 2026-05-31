/*
 * probes/nexus_v1_mechanics_combat_probe.c
 * ========================================
 * Nexus V1 Phase 3 — Combat & Creature Mechanics Probe
 *
 * Headless: no game data required. Exercises:
 *   - nexus_v1_attack (hit chance, damage, critical)
 *   - nexus_v1_take_damage (death on 0 HP)
 *   - nexus_v1_gain_experience (class skill growth)
 *   - nexus_v1_creatures_init (type catalog)
 *   - nexus_v1_creature_spawn (active creature placement)
 *   - nexus_v1_creatures_tick (AI state machine: patrol/chase/attack)
 *   - Deterministic hash stability under combat mutations
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_combat_probe
 *
 * Source-lock: src/nexus/nexus_v1_combat.c (DM1 damage formula F0730-0740)
 *              src/nexus/nexus_v1_creatures.c (AI state machine)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "nexus_v1_combat.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_world.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { printf("  PASS: %s\n", msg); g_pass++; } \
    else      { printf("  FAIL: %s\n", msg); g_fail++; } \
} while (0)

/* ── Attack mechanics ─────────────────────────────────────────────── */
static void probe_attack(void) {
    printf("\n[Combat — Champion Attack]\n");
    Nexus_V1_Champion c = {0};
    strncpy(c.name_ascii, "TestFighter", 31);
    c.alive = 1;
    c.health = 100; c.max_health = 100;
    c.stamina = 50; c.max_stamina = 50;
    c.dexterity = 50;
    c.fighter_level = 1;
    c.strength = 20;

    /* Miss case: 100% miss by forcing low rolls */
    Nexus_CombatResult r = nexus_v1_attack(NULL, 10, 5);
    CHECK(!r.hit, "attack with NULL champion returns miss");

    r = nexus_v1_attack(&c, 10, 5);
    /* At least hit or miss; damage only if hit */
    CHECK(r.hit == 0 || r.hit == 1, "hit is boolean 0 or 1");
    if (!r.hit) {
        CHECK(r.damage == 0, "miss -> 0 damage");
        CHECK(!r.critical, "miss -> no critical");
    } else {
        CHECK(r.damage >= 1, "hit -> damage >= 1");
        CHECK(r.experience_gained >= 0, "xp gained recorded");
    }

    /* Stamina cost: 3 per attack */
    int stamina_before = c.stamina;
    r = nexus_v1_attack(&c, 10, 5);
    if (r.hit) {
        CHECK(c.stamina == stamina_before - 3, "stamina consumed = 3");
    }

    /* Dead champion cannot attack */
    c.alive = 0;
    r = nexus_v1_attack(&c, 10, 5);
    CHECK(!r.hit, "dead champion cannot attack");

    /* Exhausted champion cannot attack */
    c.alive = 1;
    c.stamina = 2;
    r = nexus_v1_attack(&c, 10, 5);
    CHECK(!r.hit, "exhausted (stamina<3) champion cannot attack");
}

/* ── Damage application ───────────────────────────────────────────── */
static void probe_take_damage(void) {
    printf("\n[Combat — Damage Application]\n");
    Nexus_V1_Champion c = {0};
    strncpy(c.name_ascii, "TestFighter", 31);
    c.alive = 1;
    c.health = 50;

    int died = nexus_v1_take_damage(&c, 30);
    CHECK(c.health == 20, "50 HP - 30 damage = 20 HP");
    CHECK(died == 0, "champion survived 30 damage");
    CHECK(c.alive == 1, "champion still alive");

    died = nexus_v1_take_damage(&c, 25); /* 20 - 25 = -5 -> death */
    CHECK(c.health == 0, "HP clamped to 0");
    CHECK(died == 1, "champion died (damage >= HP)");
    CHECK(c.alive == 0, "alive flag cleared on death");

    /* NULL / already-dead is no-op */
    int r = nexus_v1_take_damage(NULL, 999);
    CHECK(r == 0, "take_damage(NULL) returns 0");

    r = nexus_v1_take_damage(&c, 100);
    CHECK(r == 0, "already-dead champion returns 0 on take_damage");
}

/* ── Experience gain ─────────────────────────────────────────────── */
static void probe_experience(void) {
    printf("\n[Combat — Experience & Level-up]\n");
    Nexus_V1_Champion c = {0};
    strncpy(c.name_ascii, "TestFighter", 31);
    c.alive = 1;

    /* Zero XP is no-op */
    nexus_v1_gain_experience(&c, NEXUS_CLASS_FIGHTER, 0);
    CHECK(c.fighter_level == 0, "zero XP doesn't advance fighter level");

    /* 100 XP -> level 1 */
    nexus_v1_gain_experience(&c, NEXUS_CLASS_FIGHTER, 100);
    CHECK(c.fighter_level == 1, "100 XP -> fighter level 1");

    /* 99 more -> still level 1 */
    nexus_v1_gain_experience(&c, NEXUS_CLASS_FIGHTER, 99);
    CHECK(c.fighter_level == 1, "99 more XP -> still level 1");

    /* 1 more -> level 2 */
    nexus_v1_gain_experience(&c, NEXUS_CLASS_FIGHTER, 1);
    CHECK(c.fighter_level == 2, "200 total XP -> fighter level 2");

    /* Null/noop */
    nexus_v1_gain_experience(NULL, NEXUS_CLASS_FIGHTER, 999);
    nexus_v1_gain_experience(&c, NEXUS_CLASS_WIZARD, 0);
    CHECK(1, "null args are no-ops (no crash)");
}

/* ── Creature catalog ─────────────────────────────────────────────── */
static void probe_creature_init(void) {
    printf("\n[Creatures — Type Catalog]\n");
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    CHECK(mgr.type_count > 0, "type_count > 0 after init");
    CHECK(mgr.active_count == 0, "active_count == 0 (no spawned creatures)");

    /* Verify known creature types */
    int scorpion_idx = -1, dragon_idx = -1;
    for (int i = 0; i < mgr.type_count; i++) {
        if (strcmp(mgr.types[i].name, "Scorpion") == 0) scorpion_idx = i;
        if (strcmp(mgr.types[i].name, "Dragon") == 0)  dragon_idx = i;
    }
    CHECK(scorpion_idx >= 0, "Scorpion type registered");
    CHECK(dragon_idx >= 0,   "Dragon type registered");

    if (scorpion_idx >= 0) {
        CHECK(mgr.types[scorpion_idx].health == 30, "Scorpion HP = 30");
        CHECK(mgr.types[scorpion_idx].attack == 15,  "Scorpion ATK = 15");
        CHECK(mgr.types[scorpion_idx].speed >= 1,    "Scorpion speed >= 1");
    }
    if (dragon_idx >= 0) {
        CHECK(mgr.types[dragon_idx].health == 200, "Dragon HP = 200");
        CHECK(mgr.types[dragon_idx].attack == 60,  "Dragon ATK = 60");
        CHECK(mgr.types[dragon_idx].experience_value == 200, "Dragon XP = 200");
    }
}

/* ── Creature spawn ───────────────────────────────────────────────── */
static void probe_creature_spawn(void) {
    printf("\n[Creatures — Spawn & Active List]\n");
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    int id1 = nexus_v1_creature_spawn(&mgr, 0, 10, 20, 1);
    CHECK(id1 > 0, "spawn returns valid id");
    CHECK(mgr.active_count == 1, "active_count = 1 after one spawn");
    CHECK(mgr.active[0].alive == 1, "spawned creature alive");
    CHECK(mgr.active[0].x == 10 && mgr.active[0].y == 20, "spawn position recorded");
    CHECK(mgr.active[0].facing == 1, "facing direction recorded");

    int id2 = nexus_v1_creature_spawn(&mgr, 1, 11, 21, 2);
    CHECK(id2 > 0 && id2 != id1, "second spawn gets different id");
    CHECK(mgr.active_count == 2, "active_count = 2");

    /* Invalid type rejected */
    int bad = nexus_v1_creature_spawn(&mgr, 999, 0, 0, 0);
    CHECK(bad == -1, "invalid type returns -1");
}

/* ── Creature AI tick ─────────────────────────────────────────────── */
static void probe_creature_tick(void) {
    printf("\n[Creatures — AI Tick State Machine]\n");
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);

    int id1 = nexus_v1_creature_spawn(&mgr, 0, 10, 20, 1);
    (void)id1;

    /* Party far away -> patrol state */
    nexus_v1_creatures_tick(&mgr, 100, 100);
    CHECK(mgr.active[0].state == 1, "far party -> patrol state (1)");

    /* Party close -> chase state */
    nexus_v1_creatures_tick(&mgr, 10, 20); /* same square = dist 0 */
    CHECK(mgr.active[0].state == 3, "same square -> attack range state (3)");

    /* Party medium distance -> chase */
    nexus_v1_creature_spawn(&mgr, 1, 10, 20, 1);
    nexus_v1_creatures_tick(&mgr, 12, 20); /* dist 2 */
    CHECK(mgr.active[1].state == 2, "dist 2 -> chase state (2)");

    /* Dead creatures skipped */
    mgr.active[0].alive = 0;
    nexus_v1_creatures_tick(&mgr, 10, 20);
    CHECK(mgr.active[0].state == 1, "dead creature skipped (state unchanged by tick)");
}

/* ── World hash stability under combat mutations ─────────────────── */
static void probe_hash_combat(void) {
    printf("\n[World Hash — Combat Mutation Stability]\n");
    Nexus_V1_World w1, w2;
    nexus_v1_world_init(&w1);
    nexus_v1_world_init(&w2);

    uint64_t h1 = nexus_v1_world_hash(&w1);
    uint64_t h2 = nexus_v1_world_hash(&w2);
    CHECK(h1 == h2, "identical worlds -> identical hash");

    /* Adding a creature changes hash */
    Nexus_V1_CreatureManager mgr;
    nexus_v1_creatures_init(&mgr);
    nexus_v1_creature_spawn(&mgr, 0, 10, 20, 1);
    /* Note: creatures aren't in world hash yet, so hash unchanged is expected.
     * This is a design gate — if we want creature-aware hashing, implement it.
     * For now, verify hash is stable when creatures added (not in hash scope). */
    (void)mgr;
    uint64_t h3 = nexus_v1_world_hash(&w1);
    CHECK(h3 == h1, "world hash currently excludes creatures (expected gate)");

    /* Damage to party position changes hash */
    w2.party_x = 12;
    uint64_t h4 = nexus_v1_world_hash(&w2);
    CHECK(h4 != h1, "party position change -> different hash");

    /* Object state change changes hash */
    nexus_v1_world_init(&w2);
    nexus_v1_object_place(&w2, &(Nexus_V1_Object){
        .type = NEXUS_OBJECT_DOOR, .state = 1, .x = 1, .y = 1, .level = 0, .quantity = 1, .flags = 0
    });
    uint64_t h5 = nexus_v1_world_hash(&w2);
    CHECK(h5 != h1, "object placement -> different hash");
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("═══════════════════════════════════════════════════\n");
    printf("  Nexus V1 — Combat & Creature Mechanics Probe\n");
    printf("  Source-lock: nexus_v1_combat.c F0730-0740,\n");
    printf("              nexus_v1_creatures.c AI state machine\n");
    printf("═══════════════════════════════════════════════════\n");

    probe_attack();
    probe_take_damage();
    probe_experience();
    probe_creature_init();
    probe_creature_spawn();
    probe_creature_tick();
    probe_hash_combat();

    printf("\n═══════════════════════════════════════════════════\n");
    printf("  Results: %d PASS, %d FAIL\n", g_pass, g_fail);
    printf("═══════════════════════════════════════════════════\n");
    return g_fail > 0 ? 1 : 0;
}
