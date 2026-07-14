#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0749_emm_release_handle_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int call_count;
    void *seen_context;
    int16_t released_handle;
} redmcsb_f0749_capture_pc34_compat;

static void capture_release_handle(void *context, int16_t ems_handle)
{
    redmcsb_f0749_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    capture->released_handle = ems_handle;
}

int main(void)
{
    redmcsb_f0749_capture_pc34_compat capture = { 0U, NULL, 0 };
    redmcsb_f0749_emm_state_pc34_compat emm_state = {
        0,
        capture_release_handle,
        &capture
    };

    redmcsb_f0749_emm_release_handle_pc34_compat(&emm_state);
    assert(capture.call_count == 0U);
    assert(emm_state.ems_handle == 0);

    emm_state.ems_handle = INT16_MAX;
    redmcsb_f0749_emm_release_handle_pc34_compat(&emm_state);
    assert(capture.call_count == 1U);
    assert(capture.seen_context == &capture);
    assert(capture.released_handle == INT16_MAX);
    assert(emm_state.ems_handle == INT16_MAX);

    emm_state.ems_handle = INT16_MIN;
    redmcsb_f0749_emm_release_handle_pc34_compat(&emm_state);
    assert(capture.call_count == 2U);
    assert(capture.released_handle == INT16_MIN);
    assert(emm_state.ems_handle == INT16_MIN);

    redmcsb_f0749_emm_release_handle_pc34_compat(&emm_state);
    assert(capture.call_count == 3U);
    assert(capture.released_handle == INT16_MIN);
    assert(emm_state.ems_handle == INT16_MIN);

    assert(strstr(redmcsb_f0749_emm_release_handle_source_evidence_pc34(),
                  "STARTUP2.C:276-286") != NULL);

    puts("ok: ReDMCSB F0749 PC 3.4 EMS handle release dispatch");
    return 0;
}
