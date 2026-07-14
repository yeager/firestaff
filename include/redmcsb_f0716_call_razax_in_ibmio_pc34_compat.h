#ifndef FIRESTAFF_REDMCSB_F0716_CALL_RAZAX_IN_IBMIO_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0716_CALL_RAZAX_IN_IBMIO_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IO.C F0716_CallRazAXInIBMIO, PC 3.4 I34E/I34M route. */
typedef void (*redmcsb_f0716_call_razax_in_ibmio_callback_pc34_compat)(
    void *context,
    int16_t value);

typedef struct {
    redmcsb_f0716_call_razax_in_ibmio_callback_pc34_compat call_razax_in_ibmio;
    void *context;
} redmcsb_f0716_io_driver_pc34_compat;

/*
 * Executes IO.C F0716's sole action: IODRV_17(value).  ReDMCSB declares
 * F0716 as int16_t but its PC 3.4 body returns no value after calling the
 * void IODRV_17 vector; this host boundary exposes that actual void behavior.
 */
void redmcsb_f0716_call_razax_in_ibmio_pc34_compat(
    const redmcsb_f0716_io_driver_pc34_compat *io_driver,
    int16_t value);

const char *redmcsb_f0716_call_razax_in_ibmio_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
