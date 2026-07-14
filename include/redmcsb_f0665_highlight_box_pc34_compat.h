#ifndef FIRESTAFF_REDMCSB_F0665_HIGHLIGHT_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0665_HIGHLIGHT_BOX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB CLIKMENU.C F0362/F0665, PC I34E/I34M route. */
typedef int (*redmcsb_f0665_get_zone_pc34_compat)(
    void *context, int16_t zone_index, int16_t xyz[4]);
typedef void (*redmcsb_f0665_screen_update_pc34_compat)(void *context);
typedef void (*redmcsb_f0665_invert_box_pc34_compat)(
    void *context, const int16_t xyz[4]);
typedef void (*redmcsb_f0665_wait_vertical_blank_pc34_compat)(void *context);

typedef struct redmcsb_f0665_highlight_state_pc34_compat {
    int16_t highlighted_zone[4];
    int highlight_box_enabled;
} redmcsb_f0665_highlight_state_pc34_compat;

typedef struct redmcsb_f0665_highlight_runtime_pc34_compat {
    redmcsb_f0665_get_zone_pc34_compat get_zone;
    redmcsb_f0665_screen_update_pc34_compat enable_screen_update;
    redmcsb_f0665_invert_box_pc34_compat invert_box;
    redmcsb_f0665_screen_update_pc34_compat disable_screen_update;
    redmcsb_f0665_wait_vertical_blank_pc34_compat wait_vertical_blank;
    void *context;
} redmcsb_f0665_highlight_runtime_pc34_compat;

/* Executes F0362's F0638 zone gate followed by F0665's PC highlight route. */
int redmcsb_f0665_highlight_box_enable_for_zone_pc34_compat(
    redmcsb_f0665_highlight_state_pc34_compat *state,
    const redmcsb_f0665_highlight_runtime_pc34_compat *runtime,
    int16_t zone_index);

const char *redmcsb_f0665_highlight_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
