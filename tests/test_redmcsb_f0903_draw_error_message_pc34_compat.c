#include <stdio.h>
#include <string.h>

#include "redmcsb_f0903_draw_error_message_pc34_compat.h"

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    uint8_t source[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34]
                  [REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANE_BYTES_PC34];
    uint8_t destination[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34]
                       [REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANE_BYTES_PC34];
    const uint8_t *source_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34];
    uint8_t *destination_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34];
    size_t plane_index;

    for (plane_index = 0;
         plane_index < REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34;
         ++plane_index) {
        memset(source[plane_index], (int)(0x20u + plane_index),
               sizeof(source[plane_index]));
        memset(destination[plane_index], 0, sizeof(destination[plane_index]));
        source_planes[plane_index] = source[plane_index];
        destination_planes[plane_index] = destination[plane_index];
    }

    check(redmcsb_f0903_draw_error_message_pc34_compat(source_planes,
                                                        destination_planes) == 1,
          "four valid original bitplanes copy");
    for (plane_index = 0;
         plane_index < REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34;
         ++plane_index) {
        check(memcmp(source[plane_index], destination[plane_index],
                     sizeof(source[plane_index])) == 0,
              "each original bitplane reaches its matching screen plane");
    }

    memset(destination[0], 0x7e, sizeof(destination[0]));
    source_planes[2] = NULL;
    check(redmcsb_f0903_draw_error_message_pc34_compat(source_planes,
                                                        destination_planes) == 0,
          "missing source plane fails closed");
    check(destination[0][0] == 0x7e,
          "a failed source validation writes no earlier plane");

    check(redmcsb_f0903_draw_error_message_pc34_compat(NULL,
                                                        destination_planes) == 0,
          "missing plane table fails closed");
    check(strstr(redmcsb_f0903_draw_error_message_source_evidence_pc34(),
                 "SWSH.C:2208-2215") != NULL,
          "source evidence identifies the original implementation");

    return failures != 0;
}
