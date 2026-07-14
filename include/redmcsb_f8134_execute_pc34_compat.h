/* ReDMCSB IBMIO.C F8134 PC 3.4 DOS EXEC boundary. */
#ifndef FIRESTAFF_REDMCSB_F8134_EXECUTE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8134_EXECUTE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8134_PARAMETER_BUFFER_BYTES_PC34 = 80,
    REDMCSB_F8134_MAX_PARAMETER_BYTES_PC34 = 78
};

typedef bool (*redmcsb_f8134_execute_pc34_compat)(
    void *context,
    const char *filename,
    const uint8_t *parameter_tail,
    size_t parameter_tail_size,
    uint16_t *out_exit_status_ax);

/*
 * The callback owns DOS EXEC and supplies the AX result of DOS function 4Dh.
 * Only normal termination (AH == 0) returns its AL exit code.
 */
int16_t redmcsb_f8134_execute_program_with_parameters_pc34_compat(
    redmcsb_f8134_execute_pc34_compat execute,
    void *context,
    const char *filename,
    const char *parameters);

const char *redmcsb_f8134_execute_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
