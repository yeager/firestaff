#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0940_copy_copper_instructions_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    redmcsb_f0940_copper_list_pc34_compat *display_instructions;
    const void *source;
    void *destination;
    long byte_count;
    unsigned int copy_count;
} redmcsb_f0940_capture_pc34_compat;

static void capture_copy_mem(void *context,
                             const void *source,
                             void *destination,
                             long byte_count)
{
    redmcsb_f0940_capture_pc34_compat *capture = context;

    assert(capture->display_instructions->max_count == 34);
    assert(capture->display_instructions->count == 34);
    capture->source = source;
    capture->destination = destination;
    capture->byte_count = byte_count;
    ++capture->copy_count;
}

int main(void)
{
    uint8_t copper_instructions[80 * REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT] = {
        0U
    };
    redmcsb_f0940_copper_list_pc34_compat display_instructions = {
        copper_instructions, -1, 66
    };
    redmcsb_f0940_view_port_pc34_compat view_port = {
        &display_instructions
    };
    redmcsb_f0940_capture_pc34_compat capture = {
        &display_instructions, NULL, NULL, -1L, 0U
    };

    redmcsb_f0940_copy_copper_instructions_pc34_compat(
        &view_port, 32L, capture_copy_mem, &capture);

    assert(capture.copy_count == 1U);
    assert(capture.destination == copper_instructions +
                                   REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT);
    assert(capture.source == copper_instructions +
                              (33L * REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT));
    assert(capture.byte_count ==
           33L * REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT);
    assert(display_instructions.max_count == 34);
    assert(display_instructions.count == 34);
    assert(strstr(redmcsb_f0940_copy_copper_instructions_source_evidence_pc34(),
                  "EXEC.C:333-348") != NULL);
    assert(strstr(redmcsb_f0940_copy_copper_instructions_source_evidence_pc34(),
                  "No PC 3.4 route") != NULL);
    puts("ok: ReDMCSB F0940 copper-instruction compaction boundary");
    return 0;
}
