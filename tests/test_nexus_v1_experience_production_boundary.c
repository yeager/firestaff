#include "nexus_v1_experience.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_ExperienceState state;
    Nexus_V1_Champion champion;
    int leveled;

    memset(&state, 0, sizeof(state));
    nexus_v1_experience_init(&state);
    memset(&champion, 0, sizeof(champion));
    champion.alive = 1;
    champion.fighter_level = 1;
    champion.primary_class = NEXUS_CLASS_FIGHTER;

    /* Award combat XP — should accumulate in fighter_xp */
    nexus_v1_experience_award_combat(&state, &champion, 0, 100);
    if (state.xp[0].fighter_xp != 100) {
        fprintf(stderr, "FAIL: fighter_xp should be 100 after combat award, got %d\n",
                state.xp[0].fighter_xp);
        return 1;
    }

    /* Award kill XP — adds to fighter_xp and ninja_xp/2 */
    nexus_v1_experience_award_kill(&state, &champion, 0, 100);
    if (state.xp[0].fighter_xp != 200) {
        fprintf(stderr, "FAIL: fighter_xp should be 200 after kill award, got %d\n",
                state.xp[0].fighter_xp);
        return 1;
    }
    if (state.xp[0].ninja_xp != 50) {
        fprintf(stderr, "FAIL: ninja_xp should be 50 after kill award, got %d\n",
                state.xp[0].ninja_xp);
        return 1;
    }

    /* level_for_xp: 61440 should give level 6 (threshold[6] = 61440) */
    if (nexus_v1_experience_level_for_xp(61440) != 6) {
        fprintf(stderr, "FAIL: level for 61440 xp should be 6, got %d\n",
                nexus_v1_experience_level_for_xp(61440));
        return 1;
    }

    /* Push XP high enough for level-up and verify */
    state.xp[0].fighter_xp = 20480;  /* threshold for level 2 */
    leveled = nexus_v1_experience_check_levelup(&state, &champion, 0);
    if (!leveled) {
        fprintf(stderr, "FAIL: check_levelup should detect level-up\n");
        return 1;
    }
    if (champion.fighter_level != 2) {
        fprintf(stderr, "FAIL: fighter_level should be 2, got %d\n",
                champion.fighter_level);
        return 1;
    }

    puts("PASS: production Nexus experience gameplay verification");
    return 0;
}
