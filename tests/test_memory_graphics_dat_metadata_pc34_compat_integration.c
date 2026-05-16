#include <stdio.h>

#include "memory_graphics_dat_metadata_pc34_compat.h"

static int test_format0_header_and_prefix_sum(void) {
    unsigned short counts[] = {10, 20, 30, 40};
    long offset = F0467_MEMORY_GetGraphicOffset_Compat(0, 4, counts, 3);
    return offset == (long)sizeof(short) + (4L * (long)(sizeof(short) * 2)) + 10 + 20 + 30;
}

static int test_format1_header_and_prefix_sum(void) {
    unsigned short counts[] = {11, 22, 33};
    long offset = F0467_MEMORY_GetGraphicOffset_Compat(1, 3, counts, 2);
    return offset == (long)(sizeof(short) * 2) + (3L * (long)(sizeof(short) * 4)) + 11 + 22;
}

static int test_first_graphic_starts_after_header_only(void) {
    unsigned short counts[] = {99, 88};
    long format0Offset = F0467_MEMORY_GetGraphicOffset_Compat(0, 2, counts, 0);
    long format1Offset = F0467_MEMORY_GetGraphicOffset_Compat(1, 2, counts, 0);
    return format0Offset == (long)sizeof(short) + (2L * (long)(sizeof(short) * 2))
        && format1Offset == (long)(sizeof(short) * 2) + (2L * (long)(sizeof(short) * 4));
}

int main(void) {
    if (!test_format0_header_and_prefix_sum()) {
        fprintf(stderr, "test_format0_header_and_prefix_sum failed\n");
        return 1;
    }
    if (!test_format1_header_and_prefix_sum()) {
        fprintf(stderr, "test_format1_header_and_prefix_sum failed\n");
        return 1;
    }
    if (!test_first_graphic_starts_after_header_only()) {
        fprintf(stderr, "test_first_graphic_starts_after_header_only failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
