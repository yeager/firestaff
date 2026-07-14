#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct capture {
    int call_order;
    int allocate_order;
    int load_order;
    int allocate_calls;
    int load_calls;
    int16_t width;
    int16_t height;
    int16_t allocation_type;
    int16_t graphic_index;
    unsigned char *load_bitmap;
    unsigned char *allocated_bitmap;
} capture;

static unsigned char *capture_allocate(
    void *context, int16_t width, int16_t height, int16_t allocation_type)
{
    capture *state = context;

    state->allocate_order = ++state->call_order;
    state->allocate_calls++;
    state->width = width;
    state->height = height;
    state->allocation_type = allocation_type;
    return state->allocated_bitmap;
}

static void capture_load(void *context, int16_t graphic_index,
                         unsigned char *bitmap)
{
    capture *state = context;

    state->load_order = ++state->call_order;
    state->load_calls++;
    state->graphic_index = graphic_index;
    state->load_bitmap = bitmap;
}

static int check(int condition, const char *message)
{
    if (condition) {
        return 1;
    }
    (void)fprintf(stderr, "failure: %s\n", message);
    return 0;
}

int main(void)
{
    unsigned char bitmap;
    const redmcsb_f0763_graphic_width_height_pc34 graphics_table[] = {
        { INT16_C(320), INT16_C(200) },
        { INT16_C(96), INT16_C(88) },
        { INT16_C(17), INT16_C(31) }
    };
    capture state = {0};
    const redmcsb_f0763_graphics_pc34_compat graphics = {
        capture_allocate, capture_load, &state
    };
    unsigned char *result;

    state.allocated_bitmap = &bitmap;
    result = redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat(
        &graphics, graphics_table, INT16_C(1));
    if (!check(result == &bitmap, "returns allocation result") ||
        !check(state.allocate_calls == 1, "allocates exactly once") ||
        !check(state.load_calls == 1, "loads exactly once") ||
        !check(state.allocate_order == 1 && state.load_order == 2,
               "allocates before loading") ||
        !check(state.width == 96 && state.height == 88,
               "forwards selected G2005 dimensions") ||
        !check(state.allocation_type ==
                   REDMCSB_F0763_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP_PC34_COMPAT,
               "uses C0 temporary-on-top allocation") ||
        !check(state.graphic_index == 1,
               "forwards unmodified graphic index to F0490") ||
        !check(state.load_bitmap == &bitmap,
               "passes allocation result to F0490") ||
        !check(strstr(
                   redmcsb_f0763_load_endgame_bitmap_expanded_source_evidence_pc34(),
                   "MEMORY.C:2764-2773") != NULL,
               "records exact source evidence")) {
        return 1;
    }

    state.allocated_bitmap = NULL;
    result = redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat(
        &graphics, graphics_table, INT16_C(2));
    if (!check(result == NULL, "returns NULL allocation unchanged") ||
        !check(state.allocate_calls == 2 && state.load_calls == 2,
               "source still calls F0490 after a NULL allocation") ||
        !check(state.width == 17 && state.height == 31,
               "second call uses its own dimensions") ||
        !check(state.graphic_index == 2 && state.load_bitmap == NULL,
               "NULL allocation is forwarded without a synthetic fallback")) {
        return 1;
    }

    puts("ok: ReDMCSB F0763 PC 3.4 endgame bitmap load");
    return 0;
}
