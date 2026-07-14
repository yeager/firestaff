#include <stdint.h>
#include <string.h>

#include "redmcsb_f0785_set_mouse_pointer_coordinates_pc34_compat.h"

static int call_count;
static int16_t received_x;
static int16_t received_y;

static void record_coordinates(int16_t x, int16_t y)
{
    ++call_count;
    received_x = x;
    received_y = y;
}

int main(void)
{
    const redmcsb_f0785_io_driver_pc34_compat io_driver = {
        record_coordinates};

    redmcsb_f0785_set_mouse_pointer_coordinates_pc34_compat(
        &io_driver, (int16_t)-123, (int16_t)456);

    if (call_count != 1 || received_x != -123 || received_y != 456) {
        return 1;
    }
    if (strcmp(redmcsb_f0785_set_mouse_pointer_coordinates_source_evidence_pc34(),
               "ReDMCSB IO.C:3739-3744; CEDT026.C:201-204; IODRV_12 dispatch") != 0) {
        return 1;
    }

    return 0;
}
