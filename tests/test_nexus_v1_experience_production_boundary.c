#include "nexus_v1_experience.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_ExperienceState state;
    Nexus_V1_ExperienceState before_state;
    Nexus_V1_Champion champion;
    Nexus_V1_Champion before_champion;

    memset(&state, 0x5a, sizeof(state));
    memset(&champion, 0x36, sizeof(champion));
    before_state = state;
    before_champion = champion;

    nexus_v1_experience_init(&state);
    nexus_v1_experience_award_combat(&state, &champion, 0, 100);
    nexus_v1_experience_award_spell(&state, &champion, 0, 100);
    nexus_v1_experience_award_kill(&state, &champion, 0, 100);

    if (memcmp(&state, &before_state, sizeof(state)) != 0 ||
        memcmp(&champion, &before_champion, sizeof(champion)) != 0 ||
        nexus_v1_experience_check_levelup(&state, &champion, 0) != 0 ||
        nexus_v1_experience_level_for_xp(61440) != 0 ||
        nexus_v1_stat_level_for_xp(32000) != 0 ||
        nexus_v1_skill_level_for_index(0, 31) != 0 ||
        nexus_v1_stat_init_value(0) != 0) {
        fprintf(stderr, "FAIL: production Nexus experience route mutated or exposed inferred state\n");
        return 1;
    }

    puts("PASS: production Nexus experience route is fail-closed");
    return 0;
}
