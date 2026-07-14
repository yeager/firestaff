/*
 * ReDMCSB IO.C F0718_GetTime, PC 3.4 (I34E/I34M) route.
 *
 * IO.C:3941-3945 returns G2161_IODriver->IODRV_21_GetTime() directly.
 * The signed 16-bit result is the driver ABI; F0718 applies no conversion,
 * caching, or fallback policy.
 */
#ifndef FIRESTAFF_REDMCSB_F0718_GET_TIME_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0718_GET_TIME_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t (*redmcsb_f0718_get_time_callback_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0718_get_time_callback_pc34_compat get_time;
    void *context;
} redmcsb_f0718_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 F0718 action:
 * G2161_IODriver->IODRV_21_GetTime().
 *
 * As in the source, io_driver and its IODRV_21 equivalent must be valid.
 */
int16_t redmcsb_f0718_get_time_pc34_compat(
    const redmcsb_f0718_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0718_get_time_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
