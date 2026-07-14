#include "redmcsb_f0699_video_interrupt_pc34_compat.h"

#include <stdio.h>

typedef struct {
    unsigned int requested_interrupt;
    int lookup_count;
    int initialize_count;
    char **received_first;
    char **received_second;
} TestContext;

static TestContext *active_context;

static void initialize_unused_globals(char **first_unused, char **second_unused)
{
    static char first_value;
    static char second_value;

    ++active_context->initialize_count;
    active_context->received_first = first_unused;
    active_context->received_second = second_unused;
    *first_unused = &first_value;
    *second_unused = &second_value;
}

static const ReDMCSBF0699VideoDriverPc34Compat test_driver = {
    initialize_unused_globals
};

static const ReDMCSBF0699VideoDriverPc34Compat *get_vector(
    unsigned int interrupt_number,
    void *context)
{
    TestContext *test_context = context;

    test_context->requested_interrupt = interrupt_number;
    ++test_context->lookup_count;
    return &test_driver;
}

static const ReDMCSBF0699VideoDriverPc34Compat *get_no_vector(
    unsigned int interrupt_number,
    void *context)
{
    TestContext *test_context = context;

    test_context->requested_interrupt = interrupt_number;
    ++test_context->lookup_count;
    return NULL;
}

int main(void)
{
    ReDMCSBF0699VideoInterruptPc34Compat state = { NULL, NULL, NULL };
    TestContext context = { 0, 0, 0, NULL, NULL };

    active_context = &context;
    if (!F0699_InitVideoInterrupt_PC34(&state, get_vector, &context) ||
        context.lookup_count != 1 ||
        context.requested_interrupt != REDMCSB_F0699_DM_VIDEO_INTERRUPT_PC34 ||
        context.initialize_count != 1 || context.received_first != &state.first_unused ||
        context.received_second != &state.second_unused || state.video_driver != &test_driver ||
        state.first_unused == NULL || state.second_unused == NULL) {
        fprintf(stderr, "F0699 did not install vector 255 and initialize slots\n");
        return 1;
    }

    state.video_driver = NULL;
    state.first_unused = NULL;
    state.second_unused = NULL;
    if (F0699_InitVideoInterrupt_PC34(&state, get_no_vector, &context) ||
        state.video_driver != NULL || state.first_unused != NULL ||
        state.second_unused != NULL) {
        fprintf(stderr, "F0699 mutated state without a host video vector\n");
        return 1;
    }

    puts("PASS redmcsb_f0699_video_interrupt_pc34_compat");
    return 0;
}
