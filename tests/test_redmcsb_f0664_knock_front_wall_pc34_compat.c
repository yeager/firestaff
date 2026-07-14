#include "redmcsb_f0664_knock_front_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct capture {
    uint16_t sampled_buttons;
    int mouse_samples;
    int pointer_hides;
    int sound_requests;
    uint16_t sound_x;
    uint16_t sound_y;
    int16_t sound_index;
    int16_t sound_mode;
} capture;

static void get_mouse_state(void *context, uint16_t *buttons)
{
    capture *state = (capture *)context;

    state->mouse_samples += 1;
    *buttons = state->sampled_buttons;
}

static void hide_pointer(void *context)
{
    ((capture *)context)->pointer_hides += 1;
}

static void request_sound(void *context, uint16_t x, uint16_t y,
                          int16_t sound_index, int16_t mode)
{
    capture *state = (capture *)context;

    state->sound_requests += 1;
    state->sound_x = x;
    state->sound_y = y;
    state->sound_index = sound_index;
    state->sound_mode = mode;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    capture capture_state;
    redmcsb_f0664_wall_click_state_pc34_compat state;
    redmcsb_f0664_wall_click_runtime_pc34_compat runtime;
    int ok = 1;

    memset(&capture_state, 0, sizeof(capture_state));
    memset(&state, 0, sizeof(state));
    runtime.get_mouse_state = get_mouse_state;
    runtime.hide_pointer = hide_pointer;
    runtime.request_sound = request_sound;
    runtime.context = &capture_state;

    ok &= expect(redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
                     &state, &runtime, 0, 12, 34) == 0,
                 "F0664 rejects a party with no champions");
    state.party_champion_count = 1;
    ok &= expect(redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
                     &state, &runtime, 0, 12, 34) == 1,
                 "F0664 dispatches ordinary wall knock");
    ok &= expect(capture_state.sound_requests == 1 && capture_state.sound_x == 12 &&
                     capture_state.sound_y == 34 && capture_state.sound_index == 4 &&
                     capture_state.sound_mode == 0 && state.stop_waiting_for_player_input,
                 "ordinary knock requests source sound and stops wait");

    memset(&state, 0, sizeof(state));
    state.party_champion_count = 1;
    capture_state.sampled_buttons = REDMCSB_F0664_PC34_MOUSE_LEFT_BUTTON;
    ok &= expect(redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
                     &state, &runtime, 1, 1, 2) == 1,
                 "F0664 handles held closed imaginary fake wall");
    ok &= expect(capture_state.mouse_samples == 1 && capture_state.pointer_hides == 1 &&
                     state.ignore_mouse_movements && state.pressing_closed_imaginary_fake_wall &&
                     state.stop_waiting_for_player_input,
                 "held fake wall preserves source press state");
    ok &= expect(redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
                     &state, &runtime, 1, 1, 2) == 0,
                 "F0664 rejects repeated held fake-wall press");

    memset(&state, 0, sizeof(state));
    state.party_champion_count = 1;
    capture_state.sampled_buttons = 0;
    ok &= expect(redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
                     &state, &runtime, 1, 1, 2) == 1 &&
                     !state.ignore_mouse_movements &&
                     !state.pressing_closed_imaginary_fake_wall &&
                     state.stop_waiting_for_player_input,
                 "released fake wall clears source press state");
    ok &= expect(strstr(redmcsb_f0664_knock_front_wall_source_evidence_pc34(),
                        "CLIKVIEW.C") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
