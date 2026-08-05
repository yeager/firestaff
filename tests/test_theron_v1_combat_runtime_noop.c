#include "theron_v1_combat.h"
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
    memset(&world, 0, sizeof(world));

    CHECK(theron_v1_creature_spawn(&world, THERON_CREATURE_AKUTUBA,
                                   1, 0, 1, 1) == -1,
          "production spawn stays blocked without T900 records");
    CHECK(theron_v1_creature_by_id(&world, 1) == NULL,
          "production creature lookup has no inferred state");
    CHECK(theron_v1_champion_attack(&world, 0, 1) == -1,
          "production champion attack stays blocked");
    CHECK(theron_v1_champion_cast_spell(&world, 0, 0, -1) == -1,
          "production combat spell stays blocked");
    CHECK(theron_v1_drop_loot(&world, 1, 1, 1) == -1,
          "production drop publication stays blocked");
    CHECK(theron_v1_sound_is_valid(THERON_SOUND_SWORD_SWING) == 0,
          "unbound sound record stays invalid");
    CHECK(strstr(theron_v1_combat_source_evidence(), "not decoded") != NULL,
          "production evidence names the undecoded source boundary");

    if (failures) return 1;
    puts("PASS: Theron production combat/drop route is fail-closed");
    return 0;
}
