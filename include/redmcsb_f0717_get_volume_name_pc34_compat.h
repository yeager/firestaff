/* ReDMCSB IO.C F0717_GetVolumeName, PC 3.4 I34E/I34M route. */
#ifndef FIRESTAFF_REDMCSB_F0717_GET_VOLUME_NAME_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0717_GET_VOLUME_NAME_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0717_get_volume_name_callback_pc34_compat)(
    void *context,
    int16_t drive_ordinal,
    char *volume_name);

typedef struct {
    redmcsb_f0717_get_volume_name_callback_pc34_compat get_volume_name;
    void *context;
} redmcsb_f0717_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 F0717 action:
 * G2162_IODriver->IODRV_19_GetVolumeName(drive_ordinal, volume_name).
 *
 * The output buffer is owned and sized by the caller, just as in IO.C.
 */
void redmcsb_f0717_get_volume_name_pc34_compat(
    const redmcsb_f0717_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal,
    char *volume_name);

const char *redmcsb_f0717_get_volume_name_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
