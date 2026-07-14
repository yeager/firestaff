#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int allocate_calls;
    int set_calls;
    int16_t allocated_width;
    int16_t allocated_height;
    int16_t allocated_type;
    int16_t set_index;
    void *allocated_bitmap;
    void *set_bitmap;
} capture;

static void *capture_allocate(
    void *context,
    int16_t width,
    int16_t height,
    int16_t allocation_type)
{
    capture *state = context;

    state->allocate_calls++;
    state->allocated_width = width;
    state->allocated_height = height;
    state->allocated_type = allocation_type;
    return state->allocated_bitmap;
}

static void capture_set(void *context, int16_t index, void *bitmap)
{
    capture *state = context;

    state->set_calls++;
    state->set_index = index;
    state->set_bitmap = bitmap;
}

static int check(int condition, const char *message)
{
    if (!condition) {
        (void)fprintf(stderr, "failure: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned char first_bitmap;
    capture state = {0, 0, 0, 0, 0, 0, NULL, NULL};
    redmcsb_f0752_graphics_pc34_compat graphics = {
        capture_allocate, capture_set, &state
    };

    state.allocated_bitmap = &first_bitmap;
    redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat(
        &graphics, INT16_C(-17), INT16_C(96), INT16_C(88));
    if (!check(state.allocate_calls == 1, "one graphic allocation") ||
        !check(state.allocated_width == 96, "source width forwarded") ||
        !check(state.allocated_height == 88, "source height forwarded") ||
        !check(state.allocated_type ==
                   REDMCSB_F0752_ALLOCATION_PERMANENT_PC34_COMPAT,
               "permanent allocation forwarded") ||
        !check(state.set_calls == 1, "one negative-pointer update") ||
        !check(state.set_index == -17, "negative bitmap index forwarded") ||
        !check(state.set_bitmap == &first_bitmap,
               "allocated bitmap forwarded")) {
        return 1;
    }

    state.allocated_bitmap = NULL;
    redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat(
        &graphics, INT16_C(4), INT16_C(0), INT16_C(-1));
    if (!check(state.allocate_calls == 2, "NULL allocation still attempted") ||
        !check(state.set_calls == 2, "NULL allocation still assigned") ||
        !check(state.set_index == 4, "second index forwarded") ||
        !check(state.set_bitmap == NULL, "NULL allocation forwarded") ||
        !check(strstr(
                   redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_source_evidence_pc34(),
                   "STARTUP2.C:477-490") != NULL,
               "source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0752 PC 3.4 negative bitmap allocation");
    return 0;
}
