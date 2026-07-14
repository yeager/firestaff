#ifndef FIRESTAFF_REDMCSB_F0664_KNOCK_FRONT_WALL_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0664_KNOCK_FRONT_WALL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB CLIKVIEW.C F0664, IBM-PC I34E/I34M branch. */
enum {
    REDMCSB_F0664_PC34_MOUSE_LEFT_BUTTON = 0x0002,
    REDMCSB_F0664_PC34_SOUND_WOODEN_THUD = 4,
    REDMCSB_F0664_PC34_SOUND_PLAY_IMMEDIATELY = 0
};

typedef void (*redmcsb_f0664_get_mouse_state_pc34_compat)(
    void *context, uint16_t *mouse_buttons_status);
typedef void (*redmcsb_f0664_hide_pointer_pc34_compat)(void *context);
typedef void (*redmcsb_f0664_request_sound_pc34_compat)(
    void *context,
    uint16_t map_x,
    uint16_t map_y,
    int16_t sound_index,
    int16_t mode);

typedef struct redmcsb_f0664_wall_click_state_pc34_compat {
    uint16_t party_champion_count;
    uint16_t mouse_buttons_status;
    int ignore_mouse_movements;
    int pressing_closed_imaginary_fake_wall;
    int stop_waiting_for_player_input;
} redmcsb_f0664_wall_click_state_pc34_compat;

typedef struct redmcsb_f0664_wall_click_runtime_pc34_compat {
    redmcsb_f0664_get_mouse_state_pc34_compat get_mouse_state;
    redmcsb_f0664_hide_pointer_pc34_compat hide_pointer;
    redmcsb_f0664_request_sound_pc34_compat request_sound;
    void *context;
} redmcsb_f0664_wall_click_runtime_pc34_compat;

/* Executes CLIKVIEW.C F0664 over caller-owned live input/state callbacks. */
int redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
    redmcsb_f0664_wall_click_state_pc34_compat *state,
    const redmcsb_f0664_wall_click_runtime_pc34_compat *runtime,
    int closed_imaginary_fake_wall,
    uint16_t map_x,
    uint16_t map_y);

const char *redmcsb_f0664_knock_front_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
