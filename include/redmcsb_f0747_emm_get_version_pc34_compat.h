/*
 * ReDMCSB STARTUP2.C F0747_EMM_GetVersion, PC 3.4 (I34E/I34M) route.
 *
 * STARTUP2.C:148-158 invokes LIM EMS interrupt 67h function 46h, then
 * returns the high nibble of the AL version byte.
 */
#ifndef FIRESTAFF_REDMCSB_F0747_EMM_GET_VERSION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0747_EMM_GET_VERSION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns AL after LIM EMS interrupt 67h, AH=46h. */
typedef uint8_t (*redmcsb_f0747_ems_get_version_al_pc34_compat)(
    void *context);

/*
 * Executes F0747 exactly: request AL from EMS and discard its low nibble.
 * As in the source, the callback must be valid.
 */
uint8_t redmcsb_f0747_emm_get_version_pc34_compat(
    redmcsb_f0747_ems_get_version_al_pc34_compat get_version_al,
    void *context);

const char *redmcsb_f0747_emm_get_version_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
