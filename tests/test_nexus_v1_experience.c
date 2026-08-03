
#include <stdio.h>
#include <string.h>
#include "nexus_v1_experience.h"

int main(void) {
    int fail = 0;

    /* Test 1: init zeroes state */
    {
        Nexus_V1_ExperienceState state;
        memset(&state, 0xff, sizeof(state));
        nexus_v1_experience_init(&state);
        if (state.xp[0].fighter_xp != 0 || state.xp[0].wizard_xp != 0) {
            fprintf(stderr, "FAIL: init\n"); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: level_for_xp thresholds */
    {
        if (nexus_v1_experience_level_for_xp(0) != 0 ||
            nexus_v1_experience_level_for_xp(499) != 0 ||
            nexus_v1_experience_level_for_xp(500) != 1 ||
            nexus_v1_experience_level_for_xp(1000) != 2 ||
            nexus_v1_experience_level_for_xp(99999) != NEXUS_MAX_CLASS_LEVEL) {
            fprintf(stderr, "FAIL: level_for_xp\n"); fail++;
        } else {
            printf("  Level thresholds OK\n");
        }
    }

    /* Test 3: combat XP goes to fighter for fighter class */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_FIGHTER;
        nexus_v1_experience_award_combat(&state, &ch, 0, 100);
        if (state.xp[0].fighter_xp != 100 || state.xp[0].ninja_xp != 0) {
            fprintf(stderr, "FAIL: fighter combat xp=%d\n",
                    state.xp[0].fighter_xp); fail++;
        } else {
            printf("  Fighter combat XP OK\n");
        }
    }

    /* Test 4: combat XP goes to ninja for ninja class */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_NINJA;
        nexus_v1_experience_award_combat(&state, &ch, 0, 200);
        if (state.xp[0].ninja_xp != 200) {
            fprintf(stderr, "FAIL: ninja combat xp=%d\n",
                    state.xp[0].ninja_xp); fail++;
        } else {
            printf("  Ninja combat XP OK\n");
        }
    }

    /* Test 5: spell XP goes to wizard */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_WIZARD;
        nexus_v1_experience_award_spell(&state, &ch, 0, 150);
        if (state.xp[0].wizard_xp != 150) {
            fprintf(stderr, "FAIL: wizard spell xp=%d\n",
                    state.xp[0].wizard_xp); fail++;
        } else {
            printf("  Wizard spell XP OK\n");
        }
    }

    /* Test 6: level-up triggers stat gains */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        int old_str, result;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_FIGHTER;
        ch.strength = 10;
        ch.max_health = 50;
        ch.fighter_level = 0;
        state.xp[0].fighter_xp = 500;
        old_str = ch.strength;
        result = nexus_v1_experience_check_levelup(&state, &ch, 0);
        if (!result || ch.fighter_level != 1 ||
            ch.strength != old_str + NEXUS_STAT_GAIN_PER_LEVEL ||
            ch.max_health != 55) {
            fprintf(stderr, "FAIL: levelup str=%d lvl=%d hp=%d\n",
                    ch.strength, ch.fighter_level, ch.max_health); fail++;
        } else {
            printf("  Fighter level-up OK\n");
        }
    }

    /* Test 7: no level-up when XP insufficient */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_FIGHTER;
        ch.strength = 10;
        ch.fighter_level = 0;
        state.xp[0].fighter_xp = 499;
        if (nexus_v1_experience_check_levelup(&state, &ch, 0)) {
            fprintf(stderr, "FAIL: premature levelup\n"); fail++;
        } else {
            printf("  No premature level-up OK\n");
        }
    }

    /* Test 8: multi-level jump */
    {
        Nexus_V1_ExperienceState state;
        Nexus_V1_Champion ch;
        nexus_v1_experience_init(&state);
        memset(&ch, 0, sizeof(ch));
        ch.primary_class = NEXUS_CLASS_WIZARD;
        ch.wisdom = 10;
        ch.max_mana = 20;
        ch.wizard_level = 0;
        state.xp[0].wizard_xp = 1500;
        nexus_v1_experience_check_levelup(&state, &ch, 0);
        if (ch.wizard_level != 3 ||
            ch.wisdom != 10 + 3 * NEXUS_STAT_GAIN_PER_LEVEL ||
            ch.max_mana != 20 + 15) {
            fprintf(stderr, "FAIL: multi-level wiz_lvl=%d wis=%d mana=%d\n",
                    ch.wizard_level, ch.wisdom, ch.max_mana); fail++;
        } else {
            printf("  Multi-level jump OK\n");
        }
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_experience_init(NULL);
        nexus_v1_experience_award_combat(NULL, NULL, 0, 10);
        nexus_v1_experience_award_spell(NULL, NULL, 0, 10);
        if (nexus_v1_experience_check_levelup(NULL, NULL, 0) != 0 ||
            nexus_v1_experience_level_for_xp(-1) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus experience system verified\n");
    return 0;
}
