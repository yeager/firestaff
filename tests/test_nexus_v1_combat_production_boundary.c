#include <stdio.h>
#include <string.h>

#include "nexus_v1_combat.h"

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_CreatureType creature;
    Nexus_Creature active;
    Nexus_CombatResult result;

    memset(&champion, 0, sizeof(champion));
    memset(&creature, 0, sizeof(creature));
    memset(&active, 0, sizeof(active));
    champion.alive = 1;
    champion.health = 100;
    champion.stamina = 80;
    champion.wounds = NEXUS_WOUND_BODY;
    champion.fighter_level = 3;
    creature.health = 100;
    creature.attack = 20;
    active.alive = 1;
    active.health = 100;

    nexus_v1_combat_seed(1234U);
    result = nexus_v1_champion_melee_attack(&champion, 20, &creature);
    nexus_v1_gain_experience(&champion, NEXUS_CLASS_FIGHTER, 1000);
    if (result.hit || result.damage != 0 || champion.health != 100 ||
        champion.stamina != 80 || nexus_v1_combat_random(100) != 0 ||
        nexus_v1_champion_take_damage(&champion, 20, NEXUS_WOUND_HEAD) != 0 ||
        nexus_v1_creature_take_damage(&active, 20) != 0 ||
        champion.health != 100 || champion.stamina != 80 ||
        champion.wounds != NEXUS_WOUND_BODY || active.health != 100) {
        fprintf(stderr, "FAIL: production Nexus combat route mutated inferred state\n");
        return 1;
    }
    puts("PASS: production Nexus combat route is fail-closed");
    return 0;
}
