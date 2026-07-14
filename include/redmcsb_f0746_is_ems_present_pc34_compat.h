/*
 * ReDMCSB STARTUP2.C F0746_IsEMSPresent, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0746_IS_EMS_PRESENT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0746_IS_EMS_PRESENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*redmcsb_f0746_open_read_only_pc34_compat)(
    void *context,
    const char *filename,
    int16_t *file_handle);

typedef bool (*redmcsb_f0746_ioctl_get_device_info_pc34_compat)(
    void *context,
    int16_t file_handle,
    uint16_t *device_information);

typedef bool (*redmcsb_f0746_ioctl_get_output_status_pc34_compat)(
    void *context,
    int16_t file_handle,
    uint8_t *output_status);

typedef bool (*redmcsb_f0746_close_pc34_compat)(
    void *context,
    int16_t file_handle);

typedef struct {
    redmcsb_f0746_open_read_only_pc34_compat open_read_only;
    redmcsb_f0746_ioctl_get_device_info_pc34_compat ioctl_get_device_info;
    redmcsb_f0746_ioctl_get_output_status_pc34_compat
        ioctl_get_output_status;
    redmcsb_f0746_close_pc34_compat close;
    void *context;
} redmcsb_f0746_dos_pc34_compat;

/*
 * Executes the PC 3.4 F0746 sequence. A nonzero do_not_use_ems bypasses DOS
 * entirely. The source leaves SI unchanged until all checks pass; pass its
 * observed incoming value in initial_si so that post-open failure routes stay
 * source-exact when their close succeeds.
 */
int16_t redmcsb_f0746_is_ems_present_pc34_compat(
    const redmcsb_f0746_dos_pc34_compat *dos,
    int16_t do_not_use_ems,
    int16_t initial_si);

const char *redmcsb_f0746_is_ems_present_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
