#include "redmcsb_f0805_creature_name_scroll_pc34_compat.h"

#include <stdio.h>
#include <string.h>

struct test_state {
    int16_t got_x;
    int16_t got_y;
    int group_calls;
    int name_calls;
    int substitutions;
    int16_t group_result;
};

static int16_t group_get_thing(int16_t map_x, int16_t map_y, void *context)
{
    struct test_state *state = context;
    state->got_x = map_x;
    state->got_y = map_y;
    ++state->group_calls;
    return state->group_result;
}

static const char *group_creature_name(int16_t group_thing, void *context)
{
    struct test_state *state = context;
    ++state->name_calls;
    return group_thing == 12 ? "SCREAMER" : "WRONG";
}

static void substitute(char *match, int16_t replaced_length,
                       const char *replacement, void *context)
{
    struct test_state *state = context;
    size_t replacement_length = strlen(replacement);
    memmove(match + replacement_length, match + replaced_length,
            strlen(match + replaced_length) + 1U);
    memcpy(match, replacement, replacement_length);
    ++state->substitutions;
}

int main(void)
{
    const int16_t east[4] = { 0, 1, 0, -1 };
    const int16_t north[4] = { -1, 0, 1, 0 };
    struct test_state state = { 0, 0, 0, 0, 0, 12 };
    int16_t panel_content = -1;
    char text[80] = "A CREANAME AND CREANAME B";

    redmcsb_f0805_creature_name_scroll_pc34_compat(
        text, "CREANAME", 10, 20, 1, east, north, &panel_content,
        group_get_thing, group_creature_name, substitute, &state);
    if (strcmp(text, "A SCREAMER AND SCREAMER B") != 0 ||
        panel_content != REDMCSB_F0805_PANEL_CREATURE_NAME_SCROLL ||
        state.got_x != 11 || state.got_y != 20 || state.group_calls != 1 ||
        state.name_calls != 1 || state.substitutions != 2) {
        return 1;
    }

    state = (struct test_state){ 0, 0, 0, 0, 0,
                                 REDMCSB_F0805_THING_END_OF_LIST };
    strcpy(text, "CREANAME");
    redmcsb_f0805_creature_name_scroll_pc34_compat(
        text, "CREANAME", 3, 4, 0, east, north, &panel_content,
        group_get_thing, group_creature_name, substitute, &state);
    if (strcmp(text, "NO CREATURE") != 0 || state.name_calls != 0 ||
        state.got_x != 3 || state.got_y != 3 || state.substitutions != 1) {
        return 2;
    }

    if (strstr(redmcsb_f0805_creature_name_scroll_source_evidence_pc34(),
               "PANEL.C:802-843") == NULL) {
        return 3;
    }
    return 0;
}
