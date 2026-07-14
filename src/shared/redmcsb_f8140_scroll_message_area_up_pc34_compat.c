#include "redmcsb_f8140_scroll_message_area_up_pc34_compat.h"

#include <stdint.h>

void redmcsb_f8140_scroll_message_area_up_pc34_compat(
    const uint8_t *source, uint8_t *destination, size_t byte_count)
{
    size_t index;

    if (byte_count == 0U) {
        return;
    }

    /*
     * NEC816.C:468-498 compares the huge pointers.  The forward branch is
     * used only when source is above destination; all other cases decrement
     * both endpoints before copying.  Integer addresses preserve that
     * segmented-pointer ordering without using undefined relational pointer
     * comparisons across unrelated C objects.
     */
    if ((uintptr_t)source > (uintptr_t)destination) {
        for (index = 0U; index < byte_count; ++index) {
            destination[index] = source[index];
        }
        return;
    }

    for (index = byte_count; index != 0U; --index) {
        destination[index - 1U] = source[index - 1U];
    }
}

const char *redmcsb_f8140_scroll_message_area_up_source_evidence_pc34(void)
{
    return "ReDMCSB NEC816.C:463-499; VIDEODRV.C:1010-1048; "
           "F8162_VIDRV_10_ScrollMessageAreaUp calls F8140 per plane.";
}
