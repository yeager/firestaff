#include <string.h>

#include "redmcsb_f8134_execute_pc34_compat.h"

int16_t redmcsb_f8134_execute_program_with_parameters_pc34_compat(
    redmcsb_f8134_execute_pc34_compat execute,
    void *context,
    const char *filename,
    const char *parameters)
{
    uint8_t parameter_tail[REDMCSB_F8134_PARAMETER_BUFFER_BYTES_PC34];
    size_t parameter_length;
    uint16_t exit_status_ax;

    if (execute == 0 || filename == 0 || parameters == 0) {
        return -1;
    }

    parameter_length = strlen(parameters);
    if (parameter_length > REDMCSB_F8134_MAX_PARAMETER_BYTES_PC34) {
        return -1;
    }

    parameter_tail[0] = (uint8_t)parameter_length;
    memcpy(&parameter_tail[1], parameters, parameter_length);
    parameter_tail[parameter_length + 1U] = (uint8_t)'\r';
    if (!execute(context, filename, parameter_tail, parameter_length + 2U,
                 &exit_status_ax) ||
        (exit_status_ax & UINT16_C(0xff00)) != 0U) {
        return -1;
    }
    return (int16_t)(exit_status_ax & UINT16_C(0x00ff));
}

const char *redmcsb_f8134_execute_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:2318-2364; MEDIA701_I34E PC route";
}
