#include "redmcsb_f0539_input_cconis_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    bool input_present;
    unsigned int call_count;
} KeyboardDriverObservation;

static __attribute__((unused)) bool observe_keyboard_input_present(void *context)
{
    KeyboardDriverObservation *observation = context;

    ++observation->call_count;
    return observation->input_present;
}

static void empty_keyboard_is_reported_without_consuming_input(void)
{
    KeyboardDriverObservation observation = { false, 0U };
    (void)observation;

    assert(!redmcsb_f0539_input_cconis_pc34_compat(
        observe_keyboard_input_present, &observation));
    assert(observation.call_count == 1U);
}

static void pending_keyboard_input_is_reported_once(void)
{
    KeyboardDriverObservation observation = { true, 0U };
    (void)observation;

    assert(redmcsb_f0539_input_cconis_pc34_compat(
        observe_keyboard_input_present, &observation));
    assert(observation.call_count == 1U);
}

static void unavailable_host_binding_reports_no_input(void)
{
    assert(!redmcsb_f0539_input_cconis_pc34_compat(NULL, NULL));
}

int main(void)
{
    empty_keyboard_is_reported_without_consuming_input();
    pending_keyboard_input_is_reported_once();
    unavailable_host_binding_reports_no_input();
    assert(strstr(redmcsb_f0539_input_cconis_source_evidence_pc34(),
                  "IO2.C:179-185") != NULL);
    return 0;
}
