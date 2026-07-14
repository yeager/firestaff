#include "redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t buttons;
    unsigned int mouse_calls;
    unsigned int keyboard_calls;
    bool keyboard_present;
    unsigned int sequence;
    unsigned int mouse_sequence;
    unsigned int keyboard_sequence;
} InputObservation;

static InputObservation *active_observation;

static void observe_mouse_state(int16_t *out_x, int16_t *out_y, int16_t *out_buttons)
{
    ++active_observation->mouse_calls;
    active_observation->mouse_sequence = ++active_observation->sequence;
    *out_x = active_observation->x;
    *out_y = active_observation->y;
    *out_buttons = active_observation->buttons;
}

static bool observe_keyboard_input(void *context)
{
    InputObservation *observation = context;

    ++observation->keyboard_calls;
    observation->keyboard_sequence = ++observation->sequence;
    return observation->keyboard_present;
}

static void reset(InputObservation *observation, int16_t buttons, bool keyboard_present)
{
    *observation = (InputObservation){ 101, -27, buttons, 0U, 0U,
                                        keyboard_present, 0U, 0U, 0U };
    active_observation = observation;
}

int main(void)
{
    const ReDMCSB_F0706_IODriverPc34 io_driver = { observe_mouse_state };
    RedmcsbF0712InputStatePc34Compat state;
    InputObservation observation;

    state.io_driver = &io_driver;
    state.keyboard_input_present = observe_keyboard_input;
    state.keyboard_context = &observation;

    reset(&observation, 0, false);
    assert(!redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(&state));
    assert(observation.mouse_calls == 1U);
    assert(observation.keyboard_calls == 1U);
    assert(observation.mouse_sequence < observation.keyboard_sequence);

    reset(&observation, 0, true);
    assert(redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(&state));
    assert(observation.mouse_calls == 1U);
    assert(observation.keyboard_calls == 1U);

    reset(&observation, 1, false);
    assert(redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(&state));
    assert(observation.mouse_calls == 1U);
    assert(observation.keyboard_calls == 0U);

    reset(&observation, (int16_t)0x8000, false);
    assert(redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(&state));
    assert(observation.keyboard_calls == 0U);

    state.io_driver = NULL;
    reset(&observation, 0, true);
    assert(redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(&state));
    assert(observation.mouse_calls == 0U);
    assert(observation.keyboard_calls == 1U);

    assert(!redmcsb_f0712_any_keyboard_or_mouse_input_pc34_compat(NULL));
    assert(strstr(redmcsb_f0712_any_keyboard_or_mouse_input_source_evidence_pc34(),
                  "IO2.C:262-274") != NULL);
    return 0;
}
