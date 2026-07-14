#include <string.h>

#include "redmcsb_f0783_lock_mouse_pc34_compat.h"

static int lock_call_count;

static void record_lock(void)
{
    ++lock_call_count;
}

int main(void)
{
    const redmcsb_f0783_io_driver_pc34_compat io_driver = {record_lock};

    redmcsb_f0783_lock_mouse_pc34_compat(&io_driver);

    if (lock_call_count != 1) {
        return 1;
    }
    if (strcmp(redmcsb_f0783_lock_mouse_source_evidence_pc34(),
               "ReDMCSB IO.C:1675-1683; IODRV_05_LockMouse dispatch") != 0) {
        return 1;
    }

    return 0;
}
