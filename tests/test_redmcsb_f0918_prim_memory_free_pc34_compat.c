#include "redmcsb_f0918_prim_memory_free_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    void *expected_buffer;
    void *expected_context;
    unsigned int call_count;
    bool result;
} ReleaseObservation;

static __attribute__((unused)) bool observe_release(void *buffer, void *context)
{
    (void)buffer;
    ReleaseObservation *observation = context;

    assert(buffer == observation->expected_buffer);
    assert(context == observation->expected_context);
    ++observation->call_count;
    return observation->result;
}

static void null_buffer_is_a_no_op_without_a_callback(void)
{
    assert(redmcsb_f0918_prim_memory_free_pc34_compat(NULL, NULL, NULL));
}

static void non_null_buffer_is_dispatched_once_without_ownership(void)
{
    int buffer;
    ReleaseObservation observation = { &buffer, NULL, 0U, true };

    observation.expected_context = &observation;
    assert(redmcsb_f0918_prim_memory_free_pc34_compat(
        &buffer, observe_release, &observation));
    assert(observation.call_count == 1U);
}

static void unavailable_or_rejected_release_is_reported(void)
{
    int buffer;
    ReleaseObservation observation = { &buffer, NULL, 0U, false };

    observation.expected_context = &observation;
    assert(!redmcsb_f0918_prim_memory_free_pc34_compat(
        &buffer, NULL, &observation));
    assert(observation.call_count == 0U);
    assert(!redmcsb_f0918_prim_memory_free_pc34_compat(
        &buffer, observe_release, &observation));
    assert(observation.call_count == 1U);
}

int main(void)
{
    null_buffer_is_a_no_op_without_a_callback();
    non_null_buffer_is_dispatched_once_without_ownership();
    unavailable_or_rejected_release_is_reported();
    assert(strstr(redmcsb_f0918_prim_memory_free_source_evidence_pc34(),
                  "F0918_PRIM_16_Memory_Free") != NULL);
    return 0;
}
