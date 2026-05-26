
#ifndef FIRESTAFF_DATA_VALIDATOR_H
#define FIRESTAFF_DATA_VALIDATOR_H

typedef enum {
    FS_VALIDATE_OK = 0,
    FS_VALIDATE_MISSING,
    FS_VALIDATE_WRONG_SIZE,
    FS_VALIDATE_CORRUPT,
} FS_ValidateResult;

typedef struct {
    const char *file;
    FS_ValidateResult result;
    int expected_min_size;
    int actual_size;
} FS_ValidateEntry;

typedef struct {
    FS_ValidateEntry dm1[2];
    FS_ValidateEntry csb[2];
    FS_ValidateEntry dm2[2];
    int dm1_ready;
    int csb_ready;
    int dm2_ready;
    int nexus_ready;
    int theron_ready;  /* PC Engine / TurboGrafx-16; Phase 0 gate: no hash set yet */
} FS_ValidationReport;

int fs_validate_data_dir(const char *data_dir, FS_ValidationReport *report);
void fs_validate_print_report(const FS_ValidationReport *report);

#endif
