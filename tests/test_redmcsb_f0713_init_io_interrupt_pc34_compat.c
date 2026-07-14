#include "redmcsb_f0713_init_io_interrupt_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    unsigned int sequence;
    unsigned int vector_sequence;
    unsigned int vblank_sequence;
    unsigned int segment_sequence;
    uint8_t interrupt_number;
    void *seen_driver;
    redmcsb_f0713_vertical_blank_routine_pc34_compat seen_routine;
    void *seen_routine_context;
} redmcsb_f0713_capture_pc34_compat;

static void *get_vector(void *context, uint8_t interrupt_number)
{
    redmcsb_f0713_capture_pc34_compat *capture = context;
    capture->vector_sequence = ++capture->sequence;
    capture->interrupt_number = interrupt_number;
    return (void *)0x1234;
}

static void *set_vertical_blank(void *context, void *driver,
                                redmcsb_f0713_vertical_blank_routine_pc34_compat routine,
                                void *routine_context)
{
    redmcsb_f0713_capture_pc34_compat *capture = context;
    capture->vblank_sequence = ++capture->sequence;
    capture->seen_driver = driver;
    capture->seen_routine = routine;
    capture->seen_routine_context = routine_context;
    return (void *)0x5678;
}

static uint16_t get_data_segment(void *context)
{
    redmcsb_f0713_capture_pc34_compat *capture = context;
    capture->segment_sequence = ++capture->sequence;
    return 0xbeefU;
}

static void vertical_blank(void *context)
{
    (void)context;
}

int main(void)
{
    redmcsb_f0713_capture_pc34_compat capture = { 0 };
    redmcsb_f0713_state_pc34_compat state = {
        get_vector, set_vertical_blank, get_data_segment, vertical_blank,
        &capture, &state, NULL, NULL, NULL, 0
    };

    assert(redmcsb_f0713_init_io_interrupt_pc34_compat(&state));
    assert(capture.interrupt_number == 254U);
    assert(state.io_driver_primary == (void *)0x1234);
    assert(state.io_driver_secondary == state.io_driver_primary);
    assert(capture.seen_driver == state.io_driver_primary);
    assert(capture.seen_routine == vertical_blank);
    assert(capture.seen_routine_context == &state);
    assert(state.previous_vertical_blank_routine == (void *)0x5678);
    assert(state.data_segment_backup == 0xbeefU);
    assert(capture.vector_sequence < capture.vblank_sequence);
    assert(capture.vblank_sequence < capture.segment_sequence);

    state.get_vector = NULL;
    assert(!redmcsb_f0713_init_io_interrupt_pc34_compat(&state));
    assert(strstr(redmcsb_f0713_init_io_interrupt_source_evidence_pc34(),
                  "IO.C:3883-3903") != NULL);
    return 0;
}
