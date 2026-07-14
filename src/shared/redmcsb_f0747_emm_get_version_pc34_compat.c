#include "redmcsb_f0747_emm_get_version_pc34_compat.h"

uint8_t redmcsb_f0747_emm_get_version_pc34_compat(
    redmcsb_f0747_ems_get_version_al_pc34_compat get_version_al,
    void *context)
{
    const uint8_t al = get_version_al(context);

    /* ReDMCSB STARTUP2.C:154-157: AH=46h/INT 67h, then AL >> 4. */
    return (uint8_t)(al >> 4);
}

const char *redmcsb_f0747_emm_get_version_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 STARTUP2.C:148-158 defines "
           "F0747_EMM_GetVersion: invoke LIM EMS INT 67h with AH=46h, "
           "store returned AL, and return AL >> 4.";
}
