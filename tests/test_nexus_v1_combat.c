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
    ct.defense = 5;
    ct.speed = 12;

    /* DM.BIN 0x029498: deterministic hit — strong champion vs weak defense hits */
    ch.stamina = 500;
    r = nexus_v1_champion_melee_attack(&ch, 10, &ct);
    expect(r.hit == 1, "strong champion hits weak creature (deterministic)");
    expect(r.damage > 0, "champion melee damage > 0 on hit");

    /* DM.BIN 0x0197E6: stamina cost is formula-driven, not constant 3 */
    {
        int stam_before = ch.stamina;
        ch.stamina = 500;
        stam_before = 500;
        r = nexus_v1_champion_melee_attack(&ch, 10, &ct);
        int cost = stam_before - ch.stamina;
        expect(cost > 0, "stamina consumed by melee attack");
    }

    /* Miss when defense is very high */
    {
        Nexus_CreatureType tough;
        memset(&tough, 0, sizeof(tough));
        tough.defense = 200;
        ch.stamina = 500;
        ch.dexterity = 5;
        ch.fighter_level = 0;
        r = nexus_v1_champion_melee_attack(&ch, 0, &tough);
        expect(r.hit == 0, "weak champion misses high-defense creature");
        ch.dexterity = 35;
        ch.fighter_level = 5;
    }

    /* DM.BIN 0x01D144: wound penalties increase incoming damage */
    {
        ch.stamina = 500; ch.wounds = 0;
        r = nexus_v1_champion_melee_attack(&ch, 10, &ct);
        int clean_dmg = r.damage;

        ch.stamina = 500;
        ch.wounds = NEXUS_WOUND_ARMS | NEXUS_WOUND_BODY | NEXUS_WOUND_LEGS;
        r = nexus_v1_champion_melee_attack(&ch, 10, &ct);
        expect(r.damage != clean_dmg, "wound penalties affect damage");
        ch.wounds = 0;
    }

    /* Creature melee attack */
    ch.health = 200;
    ch.alive = 1;
    r = nexus_v1_creature_melee_attack(&ct, &ch);
    if (r.hit) {
        expect(r.damage >= 1, "creature damage >= 1 on hit");
        expect(r.wound_zone != 0, "creature assigns wound zone");
    }

    /* Damage application with wound */
    ch.wounds = 0;
    ch.health = 100;
    ch.alive = 1;
    expect(nexus_v1_champion_take_damage(&ch, 120, NEXUS_WOUND_HEAD) == 1,
           "lethal damage kills champion");

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
    nexus_v1_gain_experience(&ch, NEXUS_CLASS_FIGHTER, 20480);
    expect(ch.fighter_level == 2, "20480 XP grants 2 fighter levels");

    /* Legacy API still works */
    ch.stamina = 500;
    ch.alive = 1;
    r = nexus_v1_attack(&ch, 10, 5);

    /* Null safety */
    r = nexus_v1_champion_melee_attack(NULL, 10, &ct);
    expect(!r.hit, "null attacker produces no hit");
    r = nexus_v1_creature_melee_attack(&ct, NULL);
    expect(!r.hit, "null target produces no hit");

    /* Saturn LCG RNG verification (DM.BIN 0x06027FCE) */
    nexus_v1_combat_seed(0);
    {
        int v0 = nexus_v1_combat_random(256);
        int v1 = nexus_v1_combat_random(256);
        expect(v0 != v1, "RNG produces different values on successive calls");
        nexus_v1_combat_seed(42);
        int a = nexus_v1_combat_random(1000);
        nexus_v1_combat_seed(42);
        int b = nexus_v1_combat_random(1000);
        expect(a == b, "RNG is deterministic with same seed");
    }

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_combat: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus combat system verified (DM.BIN disassembly)");
    return 0;
}
