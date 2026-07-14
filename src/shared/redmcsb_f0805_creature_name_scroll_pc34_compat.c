#include "redmcsb_f0805_creature_name_scroll_pc34_compat.h"

#include <string.h>

void redmcsb_f0805_creature_name_scroll_pc34_compat(
    char *scroll_text,
    const char *creature_name_scroll_text,
    int16_t party_map_x,
    int16_t party_map_y,
    uint16_t party_direction,
    const int16_t *direction_to_step_east_count,
    const int16_t *direction_to_step_north_count,
    int16_t *panel_content,
    redmcsb_f0805_group_get_thing_pc34_compat_fn group_get_thing,
    redmcsb_f0805_group_creature_name_pc34_compat_fn group_creature_name,
    redmcsb_f0805_substitute_pc34_compat_fn substitute,
    void *context)
{
    char *match;
    const char *creature_name = NULL;
    int16_t placeholder_length = (int16_t)strlen(creature_name_scroll_text);

    while ((match = strstr(scroll_text, creature_name_scroll_text)) != NULL) {
        if (creature_name == NULL) {
            int16_t map_x = (int16_t)(party_map_x +
                                      direction_to_step_east_count[party_direction]);
            int16_t map_y = (int16_t)(party_map_y +
                                      direction_to_step_north_count[party_direction]);
            int16_t group_thing;

            *panel_content = REDMCSB_F0805_PANEL_CREATURE_NAME_SCROLL;
            group_thing = group_get_thing(map_x, map_y, context);
            if (group_thing == REDMCSB_F0805_THING_END_OF_LIST) {
                creature_name = "NO CREATURE";
            } else {
                creature_name = group_creature_name(group_thing, context);
            }
        }
        substitute(match, placeholder_length, creature_name, context);
    }
}

const char *redmcsb_f0805_creature_name_scroll_source_evidence_pc34(void)
{
    return "ReDMCSB PANEL.C:802-843 (PC 3.4 A31E/A31M/A33M/A35M/F31E/F31J/X31J/P31J): "
           "F0805 resolves CREANAME once from the group one party step ahead, "
           "sets C09_PANEL_CREATURE_NAME_SCROLL, then substitutes every occurrence "
           "through F0817; an empty group uses the source literal NO CREATURE.";
}
