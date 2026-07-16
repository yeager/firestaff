#include "redmcsb_f1020_initialize_x68000_pc34_compat.h"
#include "redmcsb_f1025_get_floppy_drive_status_pc34_compat.h"
#include "redmcsb_f1026_identify_disk_in_drive_pc34_compat.h"
#include "redmcsb_f1031_is_operation_successful.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_x68000_and_floppy_boundaries_fail_closed(void)
{
    CHECK(redmcsb_f1020_initialize_x68000_pc34_compat() == false);
    CHECK(F1020_InitializeX68000() == false);
    CHECK(redmcsb_f1025_get_floppy_drive_status_pc34_compat(0) == false);
    CHECK(F1025_GetFloppyDriveStatus(1) == false);
    CHECK(redmcsb_f1026_identify_disk_in_drive_pc34_compat(0) == false);
    CHECK(F1026_IdentifyDiskInDrive_CPSX(1) == false);
    return 0;
}

static int test_f1031_reports_success_only_for_zero_error_counter(void)
{
    int16_t error_count = 0;

    CHECK(F1031_IsOperationSuccessful(&error_count) == true);
    CHECK(error_count == 0);
    return 0;
}

static int test_f1031_consumes_nonzero_error_counter(void)
{
    int16_t wrapper_error_count = 3;
    int16_t compat_error_count = 3;

    CHECK(F1031_IsOperationSuccessful(&wrapper_error_count) == false);
    CHECK(wrapper_error_count == 0);
    CHECK(redmcsb_f1031_is_operation_successful(&compat_error_count) == false);
    CHECK(compat_error_count == 0);
    return 0;
}

static int test_f1031_rejects_null_counter(void)
{
    CHECK(F1031_IsOperationSuccessful(0) == false);
    CHECK(redmcsb_f1031_is_operation_successful(0) == false);
    return 0;
}

static int test_source_evidence_names_bundle(void)
{
    const char *f1020 =
        redmcsb_f1020_initialize_x68000_source_evidence_pc34();
    const char *f1025 =
        redmcsb_f1025_get_floppy_drive_status_source_evidence_pc34();
    const char *f1026 =
        redmcsb_f1026_identify_disk_in_drive_source_evidence_pc34();
    const char *f1031 =
        redmcsb_f1031_is_operation_successful_source_evidence();

    CHECK(f1020 != 0);
    CHECK(strstr(f1020, "F1020_InitializeX68000") != 0);
    CHECK(strstr(f1020, "STARTUP2.C") != 0);
    CHECK(f1025 != 0);
    CHECK(strstr(f1025, "F1025_GetFloppyDriveStatus") != 0);
    CHECK(strstr(f1025, "FILE.C:1128") != 0);
    CHECK(f1026 != 0);
    CHECK(strstr(f1026, "F1026_IdentifyDiskInDrive_CPSX") != 0);
    CHECK(strstr(f1026, "FLOPPY.C:461") != 0);
    CHECK(f1031 != 0);
    CHECK(strstr(f1031, "F1031_IsOperationSuccessful") != 0);
    CHECK(strstr(f1031, "CEDT023.C:1295") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_x68000_and_floppy_boundaries_fail_closed() == 0);
    CHECK(test_f1031_reports_success_only_for_zero_error_counter() == 0);
    CHECK(test_f1031_consumes_nonzero_error_counter() == 0);
    CHECK(test_f1031_rejects_null_counter() == 0);
    CHECK(test_source_evidence_names_bundle() == 0);
    return 0;
}
