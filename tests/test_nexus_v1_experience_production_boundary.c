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

    nexus_v1_experience_award_combat(&state, &champion, 0, 100);
    nexus_v1_experience_award_kill(&state, &champion, 0, 100);

    if (nexus_v1_experience_level_for_xp(61440) != 0 ||
        state.xp[0].fighter_xp != 0 || champion.fighter_level != 1) {
        fprintf(stderr, "FAIL: production experience ABI was not state-preserving\n");
        return 1;
    }

    puts("PASS: production Nexus experience route remains capture-gated");
    return 0;
}
