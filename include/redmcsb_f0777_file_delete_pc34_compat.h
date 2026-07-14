/*
 * ReDMCSB FILE.C F0777_FILE_Delete, PC 3.4 (C03_GAME/C06_CEDT) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0777_FILE_DELETE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0777_FILE_DELETE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Host bridge for DOS INT 21h/AH=41h (delete file). */
typedef void (*redmcsb_f0777_dos_delete_pc34_compat)(
    void *context,
    const char *file_name);

/*
 * Dispatches the source's DOS delete request. F0777 is void and does not
 * inspect the DOS carry/error result on the PC 3.4 route.
 */
void redmcsb_f0777_file_delete_pc34_compat(
    redmcsb_f0777_dos_delete_pc34_compat delete_file,
    void *context,
    const char *file_name);

const char *redmcsb_f0777_file_delete_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
