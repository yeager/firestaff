/*
 * ReDMCSB F0713_InitIOInterrupt, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0713_INIT_IO_INTERRUPT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0713_INIT_IO_INTERRUPT_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0713_init_io_interrupt_callback_pc34_compat)(
    void *context);

typedef struct {
    redmcsb_f0713_init_io_interrupt_callback_pc34_compat init_io_interrupt;
    void *context;
} redmcsb_f0713_io_driver_pc34_compat;

/*
 * Executes the PC 3.4 F0713 I/O-driver interrupt initialization vector.
 * Returns false only when the host cannot supply that vector.
 */
bool redmcsb_f0713_init_io_interrupt_pc34_compat(
    const redmcsb_f0713_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0713_init_io_interrupt_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0713_INIT_IO_INTERRUPT_PC34_COMPAT_H */
