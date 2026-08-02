#include "theron_v1_track02_hud_strings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Flask levels */
    assert(strcmp(theron_v1_track02_us_flask_level(0), "(EMPTY)") == 0);
    assert(strcmp(theron_v1_track02_us_flask_level(3), "(FULL)") == 0);
    assert(theron_v1_track02_us_flask_level(4) == NULL);

    /* Directions */
    assert(strcmp(theron_v1_track02_us_direction_name(0), "NORTH") == 0);
    assert(strcmp(theron_v1_track02_us_direction_name(3), "WEST") == 0);
    assert(theron_v1_track02_us_direction_name(4) == NULL);

    /* Stat names */
    assert(strcmp(theron_v1_track02_us_stat_name(0), "STRENGTH") == 0);
    assert(strcmp(theron_v1_track02_us_stat_name(4), "ANTI-MAGIC") == 0);
    assert(strcmp(theron_v1_track02_us_stat_name(5), "ANTI-FIRE") == 0);
    assert(theron_v1_track02_us_stat_name(6) == NULL);

    /* Vital names */
    assert(strcmp(theron_v1_track02_us_vital_name(0), "HEALTH") == 0);
    assert(strcmp(theron_v1_track02_us_vital_name(2), "MANA") == 0);
    assert(theron_v1_track02_us_vital_name(3) == NULL);

    /* Spell messages */
    assert(strstr(theron_v1_track02_us_spell_message(0), "PRACTICE") != NULL);
    assert(strstr(theron_v1_track02_us_spell_message(1), "MEANINGLESS") != NULL);
    assert(strstr(theron_v1_track02_us_spell_message(2), "FLASK") != NULL);
    assert(strcmp(theron_v1_track02_us_spell_message(5), " LEVEL!") == 0);
    assert(theron_v1_track02_us_spell_message(6) == NULL);

    /* Labels */
    assert(strcmp(theron_v1_track02_us_weight_label(), "WEIGHS") == 0);
    assert(strcmp(theron_v1_track02_us_weight_unit(), " KG.") == 0);
    assert(strcmp(theron_v1_track02_us_burnt_out_label(), "(BURNT OUT)") == 0);
    assert(strcmp(theron_v1_track02_us_party_facing_label(), "PARTY FACING") == 0);

    printf("PASS: theron_v1_track02_hud_strings\n");
    return 0;
}
