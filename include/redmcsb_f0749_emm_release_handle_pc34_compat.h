/*
 * ReDMCSB STARTUP2.C F0749_EMM_ReleaseHandle, PC 3.4 I34E/I34M route.
 *
 * STARTUP2.C:276-286 tests G2133_i_EMSHandle and, when it is nonzero,
 * invokes LIM EMS interrupt 67h function 45h with that signed 16-bit handle
 * in DX. It neither clears the handle nor consumes the returned status.
 */
#ifndef FIRESTAFF_REDMCSB_F0749_EMM_RELEASE_HANDLE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0749_EMM_RELEASE_HANDLE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0749_emm_release_callback_pc34_compat)(
    void *context,
    int16_t ems_handle);

typedef struct {
    int16_t ems_handle;
    redmcsb_f0749_emm_release_callback_pc34_compat release_handle;
    void *context;
} redmcsb_f0749_emm_state_pc34_compat;

/*
 * Executes F0749's sole PC 3.4 action. When ems_handle is nonzero,
 * release_handle must be valid. The state is intentionally not modified.
 */
void redmcsb_f0749_emm_release_handle_pc34_compat(
    const redmcsb_f0749_emm_state_pc34_compat *emm_state);

const char *redmcsb_f0749_emm_release_handle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
