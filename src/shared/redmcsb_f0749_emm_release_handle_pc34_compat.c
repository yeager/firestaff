#include "redmcsb_f0749_emm_release_handle_pc34_compat.h"

void redmcsb_f0749_emm_release_handle_pc34_compat(
    const redmcsb_f0749_emm_state_pc34_compat *emm_state)
{
    /* STARTUP2.C:280-284: the stored handle is the only guard. */
    if (emm_state->ems_handle != 0) {
        emm_state->release_handle(emm_state->context, emm_state->ems_handle);
    }
}

const char *redmcsb_f0749_emm_release_handle_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 STARTUP2.C:276-286 defines "
           "F0749_EMM_ReleaseHandle: when G2133_i_EMSHandle is nonzero, "
           "load it into DX and issue LIM EMS interrupt 67h function 45h; "
           "the source neither clears the handle nor reads the status.";
}
