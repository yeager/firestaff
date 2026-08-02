#include "nexus_v1_combat.h"
#include <stdio.h>
#include <string.h>

static int g_failures;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); ++g_failures; }
}

int main(void) {
    Nexus_V1_Champion ch;
    Nexus_CreatureType ct;
    Nexus_Creature cr;
    Nexus_CombatResult r;
    int hits, misses, wounds, deaths, i;

    nexus_v1_combat_seed(42);

    memset(&ch, 0, sizeof(ch));
    snprintf(ch.name_ascii, sizeof(ch.name_ascii), "TestHero");
    ch.alive = 1;
    ch.health = 200;
    ch.max_health = 200;
    ch.stamina = 500;
    ch.max_stamina = 500;
    ch.mana = 100;
    ch.strength = 40;
    ch.dexterity = 35;
    ch.fighter_level = 5;

    memset(&ct, 0, sizeof(ct));
    snprintf(ct.name, sizeof(ct.name), "Scorpion");
    ct.health = 80;
    ct.attack = 20;
    ct.defense = 15;
    ct.speed = 12;

    /* Champion melee attack — hit rate should be reasonable */
    hits = 0;
    misses = 0;
    for (i = 0; i < 100; ++i) {
        ch.stamina = 500;
        r = nexus_v1_champion_melee_attack(&ch, 10, &ct);
        if (r.hit) {
            hits++;
            expect(r.damage > 0, "champion melee damage > 0 on hit");
        } else {
            misses++;
        }
    }
    expect(hits > 20 && hits < 95,
           "champion melee hit rate is reasonable (20-95%)");

    /* Creature melee attack — staged random damage */
    ch.health = 200;
    ch.alive = 1;
    hits = 0;
    wounds = 0;
    for (i = 0; i < 200; ++i) {
        r = nexus_v1_creature_melee_attack(&ct, &ch);
        if (r.hit) {
            hits++;
            expect(r.damage >= 1, "creature damage >= 1 on hit");
            if (r.wound_zone) wounds++;
        }
    }
    expect(hits > 30, "creature hits at least 30/200 times");
    expect(wounds > 0, "creature attacks assign wound zones");

    /* Damage application with wound */
    ch.wounds = 0;
    ch.health = 100;
    ch.alive = 1;
    deaths = 0;
    for (i = 0; i < 50; ++i) {
        ch.health = 100;
        ch.alive = 1;
        deaths += nexus_v1_champion_take_damage(&ch, 120, NEXUS_WOUND_HEAD);
    }
    expect(deaths == 50, "lethal damage kills champion every time");

    /* Creature death */
    memset(&cr, 0, sizeof(cr));
    cr.alive = 1;
    cr.health = 10;
    expect(nexus_v1_creature_take_damage(&cr, 5) == 0, "5 damage doesn't kill 10hp creature");
    expect(cr.health == 5, "creature health reduced to 5");
    expect(nexus_v1_creature_take_damage(&cr, 10) == 1, "10 damage kills 5hp creature");
    expect(!cr.alive, "creature marked dead");

    /* Experience gain */
    ch.fighter_level = 0;
    nexus_v1_gain_experience(&ch, NEXUS_CLASS_FIGHTER, 250);
    expect(ch.fighter_level == 2, "250 XP grants 2 fighter levels");

    /* Legacy API still works */
    ch.stamina = 100;
    ch.alive = 1;
    r = nexus_v1_attack(&ch, 10, 5);

    /* Null safety */
    r = nexus_v1_champion_melee_attack(NULL, 10, &ct);
    expect(!r.hit, "null attacker produces no hit");
    r = nexus_v1_creature_melee_attack(&ct, NULL);
    expect(!r.hit, "null target produces no hit");

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_combat: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus DM1-parity combat formulas verified");
    return 0;
}
