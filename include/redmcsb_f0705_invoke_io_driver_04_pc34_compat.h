/*
 * ReDMCSB IO.C F0705_, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0705_INVOKE_IO_DRIVER_04_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0705_INVOKE_IO_DRIVER_04_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DEFS.H names this slot only as IODRV_04_.  It takes no arguments. */
typedef void (*redmcsb_f0705_io_driver_04_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0705_io_driver_04_pc34_compat io_driver_04;
    void *context;
} redmcsb_f0705_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 action in F0705_: G2161_IODriver->IODRV_04_().
 * Returns false only when no compatible I/O-driver slot was supplied.
 */
bool redmcsb_f0705_invoke_io_driver_04_pc34_compat(
    const redmcsb_f0705_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0705_invoke_io_driver_04_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
