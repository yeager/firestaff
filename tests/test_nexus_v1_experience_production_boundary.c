#include "nexus_v1_experience.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_ExperienceState state;
    Nexus_V1_Champion champion;

    memset(&state, 0, sizeof(state));
    nexus_v1_experience_init(&state);
    memset(&champion, 0, sizeof(champion));
    champion.alive = 1;
    champion.fighter_level = 1;
    champion.primary_class = NEXUS_CLASS_FIGHTER;

    nexus_v1_experience_award_combat(&state, &champion, 0, 100);
    nexus_v1_experience_award_kill(&state, &champion, 0, 100);
    if (state.xp[0].fighter_xp != 0 || state.xp[0].ninja_xp != 0) {
        fprintf(stderr, "FAIL: production experience route mutated XP\n");
        return 1;
    }

    if (nexus_v1_experience_level_for_xp(61440) != 0) {
        fprintf(stderr, "FAIL: production experience route exposed a level\n");
        return 1;
    }

    state.xp[0].fighter_xp = 20480;  /* threshold for level 2 */
    if (nexus_v1_experience_check_levelup(&state, &champion, 0) ||
        champion.fighter_level != 1) {
        fprintf(stderr, "FAIL: production experience route changed champion level\n");
        return 1;
    }

    puts("PASS: production Nexus experience boundary remains fail-closed");
    return 0;
}
