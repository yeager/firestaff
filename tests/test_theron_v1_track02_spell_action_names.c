#include "theron_v1_track02_spell_action_names.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(theron_v1_track02_us_action_spell_count() == 41);
    assert(theron_v1_track02_us_action_spell_name(41) == NULL);

    assert(strcmp(theron_v1_track02_us_action_spell_name(0), "N") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(1), "BLOCK") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(19), "BERZERK") == 0);

    assert(strcmp(theron_v1_track02_us_action_spell_name(20), "FIREBALL") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(23), "LIGHTNING") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(35), "HEAL") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(40), "THROW") == 0);

    /* Placeholders */
    assert(strcmp(theron_v1_track02_us_action_spell_name(3), "X") == 0);
    assert(strcmp(theron_v1_track02_us_action_spell_name(26), "X") == 0);

    /* Skill levels */
    assert(theron_v1_track02_us_skill_level_count() == 15);
    assert(theron_v1_track02_us_skill_level_name(15) == NULL);

    assert(strcmp(theron_v1_track02_us_skill_level_name(0), "NEOPHYTE") == 0);
    assert(strcmp(theron_v1_track02_us_skill_level_name(7), "EXPERT") == 0);
    assert(strcmp(theron_v1_track02_us_skill_level_name(14), "ARCHMASTER") == 0);

    for (unsigned i = 0; i < 41; i++) {
        const char *s = theron_v1_track02_us_action_spell_name(i);
        assert(s != NULL && s[0] != '\0');
    }

    for (unsigned i = 0; i < 15; i++) {
        const char *s = theron_v1_track02_us_skill_level_name(i);
        assert(s != NULL && s[0] != '\0');
    }

    printf("PASS: theron_v1_track02_spell_action_names\n");
    return 0;
}
