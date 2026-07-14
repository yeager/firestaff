#include "redmcsb_f0716_call_razax_in_ibmio_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0716_call_razax_in_ibmio_pc34_compat(
    const redmcsb_f0716_io_driver_pc34_compat *io_driver,
    int16_t value)
{
    if (io_driver == NULL || io_driver->call_razax_in_ibmio == NULL) {
        return;
    }

    io_driver->call_razax_in_ibmio(io_driver->context, value);
}

const char *redmcsb_f0716_call_razax_in_ibmio_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3928-3932, MEDIA709_I34E_I34M_P31J: F0716 calls "
           "G2162_IODriver->IODRV_17 with its sole int16_t argument. The "
           "declared int16_t function body has no return statement; IODRV_17 "
           "itself is void in DEFS.H:4319.";
}
