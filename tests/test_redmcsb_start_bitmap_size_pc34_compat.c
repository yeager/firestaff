#include <stdio.h>

#include "redmcsb_start_bitmap_size_pc34_compat.h"

static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #expression, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void) {
    /* STARTUP2.C's native-scale and derived-door allocation forms. */
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(96, 95, 32) == 4560);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(64, 61, 14) == 364);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(64, 61, 21) == 840);

    /* M103 rounds the scaled pixel width up to an even packed-width. */
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(65, 63, 32) == 2079);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(3, 3, 14) == 1);

    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(0, 61, 14) == 0);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(64, 0, 14) == 0);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(-1, 61, 14) == 0);
    CHECK(F0459_START_GetScaledBitmapByteCount_PC34(64, 61, -1) == 0);

    return failures != 0;
}
