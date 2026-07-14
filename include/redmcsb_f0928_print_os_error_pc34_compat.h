/*
 * ReDMCSB PRIM1.C F0928_PrintOSError, PC 3.4 (I34E/I34M) route.
 *
 * The routine prints an OS-error prefix and its hexadecimal code, waits
 * until a key is available, consumes that key, then returns the same code.
 */
#ifndef FIRESTAFF_REDMCSB_F0928_PRINT_OS_ERROR_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0928_PRINT_OS_ERROR_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0928_console_write_pc34_compat)(
    void *context,
    const char *text);

typedef bool (*redmcsb_f0928_key_available_pc34_compat)(void *context);

typedef void (*redmcsb_f0928_read_key_pc34_compat)(void *context);

typedef struct {
    void *context;
    redmcsb_f0928_console_write_pc34_compat console_write;
    redmcsb_f0928_key_available_pc34_compat key_available;
    redmcsb_f0928_read_key_pc34_compat read_key;
} redmcsb_f0928_print_os_error_callbacks_pc34_compat;

/*
 * The callbacks must supply the source Cconws, Cconis, and Crawcin actions.
 * Returns error_code after performing the original output and key wait.
 */
int16_t redmcsb_f0928_print_os_error_pc34_compat(
    int16_t error_code,
    const redmcsb_f0928_print_os_error_callbacks_pc34_compat *callbacks);

const char *redmcsb_f0928_print_os_error_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0928_PRINT_OS_ERROR_PC34_COMPAT_H */
