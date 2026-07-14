#include "redmcsb_f0713_init_io_interrupt_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    unsigned int call_count;
    void *seen_context;
} redmcsb_f0713_capture_pc34_compat;

static void capture_init_io_interrupt(void *context)
{
    redmcsb_f0713_capture_pc34_compat *capture = context;

    ++capture->call_count;
    capture->seen_context = context;
}

int main(void)
{
    redmcsb_f0713_capture_pc34_compat capture = { 0U, NULL };
    redmcsb_f0713_io_driver_pc34_compat driver = {
        capture_init_io_interrupt,
        &capture
    };

    assert(redmcsb_f0713_init_io_interrupt_pc34_compat(&driver));
    assert(capture.call_count == 1U);
    assert(capture.seen_context == &capture);

    driver.init_io_interrupt = NULL;
    assert(!redmcsb_f0713_init_io_interrupt_pc34_compat(&driver));
    assert(capture.call_count == 1U);
    assert(!redmcsb_f0713_init_io_interrupt_pc34_compat(NULL));
    assert(strstr(redmcsb_f0713_init_io_interrupt_source_evidence_pc34(),
                  "ANIM.C:1632") != NULL);
    return 0;
}
