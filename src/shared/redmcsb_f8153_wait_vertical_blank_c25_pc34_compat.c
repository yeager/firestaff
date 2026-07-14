#include "redmcsb_f8153_wait_vertical_blank_c25_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f8153_wait_vertical_blank_c25_pc34_compat(
    RedmcsbF8153VideoStatusReaderPc34Compat read_status, void *context)
{
    if (read_status == NULL) {
        return false;
    }

    /* VIDEODRV.C:3166-3184 polls 0x3DA bit 3: inactive, then active. */
    while ((read_status(context) & 0x08U) != 0U) {
    }
    while ((read_status(context) & 0x08U) == 0U) {
    }
    return true;
}

const char *redmcsb_f8153_wait_vertical_blank_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3163-3185 C25 F8153: poll VGA status port "
           "0x3DA bit 3 until vertical sync is inactive, then poll until the "
           "next vertical sync is active.";
}
