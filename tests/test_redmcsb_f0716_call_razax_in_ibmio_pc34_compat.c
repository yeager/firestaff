#include "redmcsb_f0716_call_razax_in_ibmio_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int calls;
    void *seen_context;
    int16_t seen_value;
} redmcsb_f0716_capture_pc34_compat;

static void capture_call_razax_in_ibmio(void *context, int16_t value)
{
    redmcsb_f0716_capture_pc34_compat *capture = context;

    capture->calls++;
    capture->seen_context = context;
    capture->seen_value = value;
}

int main(void)
{
    redmcsb_f0716_capture_pc34_compat capture = { 0U, NULL, 0 };
    redmcsb_f0716_io_driver_pc34_compat driver = {
        capture_call_razax_in_ibmio,
        &capture
    };

    redmcsb_f0716_call_razax_in_ibmio_pc34_compat(&driver, INT16_C(-32768));
    assert(capture.calls == 1U);
    assert(capture.seen_context == &capture);
    assert(capture.seen_value == INT16_MIN);

    redmcsb_f0716_call_razax_in_ibmio_pc34_compat(&driver, INT16_C(32767));
    assert(capture.calls == 2U);
    assert(capture.seen_value == INT16_MAX);

    driver.call_razax_in_ibmio = NULL;
    redmcsb_f0716_call_razax_in_ibmio_pc34_compat(&driver, 0);
    redmcsb_f0716_call_razax_in_ibmio_pc34_compat(NULL, 0);
    assert(capture.calls == 2U);
    assert(strstr(redmcsb_f0716_call_razax_in_ibmio_source_evidence_pc34(),
                  "IODRV_17") != NULL);
    return 0;
}
