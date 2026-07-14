/*
 * ReDMCSB FILE.C F0778_FILE_Rename, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0778_FILE_RENAME_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0778_FILE_RENAME_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Host bridge for DOS INT 21h/AH=56h (rename file). */
typedef void (*redmcsb_f0778_dos_rename_pc34_compat)(
    void *context,
    const char *source_file_name,
    const char *destination_file_name);

/*
 * Dispatches the source's DOS rename request. F0778 is void and does not
 * inspect the DOS carry/error result on the PC 3.4 route.
 */
void redmcsb_f0778_file_rename_pc34_compat(
    redmcsb_f0778_dos_rename_pc34_compat rename_file,
    void *context,
    const char *source_file_name,
    const char *destination_file_name);

const char *redmcsb_f0778_file_rename_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
