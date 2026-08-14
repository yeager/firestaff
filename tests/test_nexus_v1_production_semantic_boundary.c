#include "nexus_v1_action_timer.h"
#include "nexus_v1_combat.h"
#include "nexus_v1_doors.h"
#include "nexus_v1_experience.h"
#include "nexus_v1_light.h"
#include "nexus_v1_magic.h"
#include "nexus_v1_projectiles.h"
#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"
#include "nexus_v1_traps.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    Nexus_V1_Champion champion;
    Nexus_LightState light;
    Nexus_StatusEffects status;
    Nexus_RestState rest;
    Nexus_ActionTimers timers;

    memset(&champion, 0x5a, sizeof(champion));
    memset(&light, 0x5a, sizeof(light));
    memset(&status, 0x5a, sizeof(status));
    memset(&rest, 0x5a, sizeof(rest));
    memset(&timers, 0x5a, sizeof(timers));

    nexus_v1_combat_seed(1U);
    expect(nexus_v1_combat_random(100) == 0,
           "production combat RNG remains unavailable");
    expect(nexus_v1_champion_take_damage(&champion, 9, 0) == 0,
           "production combat must not mutate a champion");
    expect(nexus_v1_spell_mana_cost(1, 1) == -1,
           "production magic has no inferred mana route");
    expect(nexus_v1_cast_spell(&champion, 1, 1, 1, 1) == -1,
           "production magic has no inferred caster route");
    expect(nexus_v1_experience_level_for_xp(100000) == 0,
           "production experience has no inferred level curve");

    nexus_v1_status_init(&status);
    expect(nexus_v1_status_is_active(&status, 0) == 0,
           "production status stays unavailable");
    nexus_v1_rest_start(&rest);
    expect(nexus_v1_rest_is_resting(&rest) == 0,
           "production rest stays unavailable");
    expect(nexus_v1_light_get(&light) == 0,
           "production light has no inferred timeline");

    expect(nexus_v1_action_is_ready(&timers, 0) == 0,
           "production action timer stays unavailable");
    expect(nexus_v1_door_try_open(NULL, 0, 0, 0) == NEXUS_DOOR_RESULT_NO_KEY,
           "production door route stays fail-closed");
    expect(nexus_v1_trap_add(NULL, NULL) == 0,
           "production trap route stays inert");
    expect(nexus_v1_projectile_spawn(NULL, NEXUS_PROJ_FIREBALL,
                                     0, 0, 0, 0, 0, 0) == -1,
           "production projectile route stays unavailable");

    if (failures != 0) {
        fprintf(stderr, "%d Nexus production-boundary checks failed\n", failures);
        return 1;
    }
    puts("PASS: Nexus inferred mechanics are excluded from production");
    return 0;
}
