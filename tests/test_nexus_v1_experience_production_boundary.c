#include "nexus_v1_experience.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_ExperienceState state;
    Nexus_V1_Champion champion;

    nexus_v1_experience_init(&state);
    memset(&champion, 0, sizeof(champion));
    champion.alive = 1;
    champion.fighter_level = 1;

    nexus_v1_experience_award_combat(&state, &champion, 0, 100);
    nexus_v1_experience_award_kill(&state, &champion, 0, 100);

    if (nexus_v1_experience_level_for_xp(61440) <= 0) {
        fprintf(stderr, "FAIL: level_for_xp returned 0 for 61440 XP\n");
        return 1;
    }

    puts("PASS: production Nexus experience route returns real values");
    return 0;
}
