/*
 * ReDMCSB IO.C F0715_GetConvertedDeviceType, PC 3.4 (I34E/I34M) route.
 *
 * IO.C:3922-3926 returns G2162_IODriver->IODRV_20_(drive_ordinal) without
 * conversion, caching, or fallback policy of its own.
 */
#ifndef FIRESTAFF_REDMCSB_F0715_GET_CONVERTED_DEVICE_TYPE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0715_GET_CONVERTED_DEVICE_TYPE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t (*redmcsb_f0715_get_converted_device_type_callback_pc34_compat)(
    void *context,
    int16_t drive_ordinal);

typedef struct {
    redmcsb_f0715_get_converted_device_type_callback_pc34_compat
        get_converted_device_type;
    void *context;
} redmcsb_f0715_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 F0715 action:
 * G2162_IODriver->IODRV_20_(drive_ordinal).
 *
 * As in the source, io_driver and its IODRV_20 equivalent must be valid.
 */
int16_t redmcsb_f0715_get_converted_device_type_pc34_compat(
    const redmcsb_f0715_io_driver_pc34_compat *io_driver,
    int16_t drive_ordinal);

const char *redmcsb_f0715_get_converted_device_type_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
