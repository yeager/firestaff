/*
 * ReDMCSB PANEL.C F0805_CreatureNameScroll, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0805_CREATURE_NAME_SCROLL_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0805_CREATURE_NAME_SCROLL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0805_PANEL_CREATURE_NAME_SCROLL = 9,
    REDMCSB_F0805_THING_END_OF_LIST = -2
};

typedef int16_t (*redmcsb_f0805_group_get_thing_pc34_compat_fn)(
    int16_t map_x,
    int16_t map_y,
    void *context);

typedef const char *(*redmcsb_f0805_group_creature_name_pc34_compat_fn)(
    int16_t group_thing,
    void *context);

typedef void (*redmcsb_f0805_substitute_pc34_compat_fn)(
    char *match,
    int16_t replaced_length,
    const char *replacement,
    void *context);

/*
 * Mirrors PANEL.C:802-843. The caller supplies the F0175 group lookup,
 * group-type name lookup and F0817 in-place substitution boundaries.
 */
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
    void *context);

const char *redmcsb_f0805_creature_name_scroll_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
