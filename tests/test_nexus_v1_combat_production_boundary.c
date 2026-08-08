#include <stdio.h>
#include <string.h>

#include "nexus_v1_combat.h"

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_CreatureType creature;
    Nexus_Creature active;

    memset(&champion, 0, sizeof(champion));
    memset(&creature, 0, sizeof(creature));
    memset(&active, 0, sizeof(active));
    champion.alive = 1;
    champion.health = 100;
    champion.stamina = 80;
    champion.fighter_level = 3;
    creature.health = 100;
    creature.attack = 20;
    active.alive = 1;
    active.health = 100;

    nexus_v1_combat_seed(1234U);
    if (nexus_v1_combat_random(100) == 0 && nexus_v1_combat_random(100) == 0 &&
        nexus_v1_combat_random(100) == 0) {
        fprintf(stderr, "FAIL: combat RNG always returns 0\n");
        return 1;
    }
    nexus_v1_combat_seed(1234U);
    nexus_v1_champion_take_damage(&champion, 20, NEXUS_WOUND_HEAD);
    if (champion.health >= 100) {
        fprintf(stderr, "FAIL: take_damage did not reduce health\n");
        return 1;
    }
    nexus_v1_creature_take_damage(&active, 20);
    if (active.health >= 100) {
        fprintf(stderr, "FAIL: creature take_damage did not reduce health\n");
        return 1;
    }
    puts("PASS: production Nexus combat route returns real values");
    return 0;
}
