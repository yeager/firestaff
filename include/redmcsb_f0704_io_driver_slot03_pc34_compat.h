/*
 * ReDMCSB IO.C F0704_, PC 3.4 (I34E/I34M).
 *
 * F0704 is deliberately an unnamed I/O-driver dispatch in the original.
 * Its PC 3.4 body has no arguments and performs no local work; it invokes
 * G2161_IODriver->IODRV_03_ once.
 */
#ifndef FIRESTAFF_REDMCSB_F0704_IO_DRIVER_SLOT03_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0704_IO_DRIVER_SLOT03_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0704_io_driver_slot03_callback_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0704_io_driver_slot03_callback_pc34_compat slot03;
    void *context;
} redmcsb_f0704_io_driver_pc34_compat;

/*
 * Returns false only when the installed PC 3.4 I/O-driver slot is absent.
 * On success this is the exact one-call F0704 body.
 */
bool redmcsb_f0704_call_io_driver_slot03_pc34_compat(
    const redmcsb_f0704_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0704_io_driver_slot03_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
