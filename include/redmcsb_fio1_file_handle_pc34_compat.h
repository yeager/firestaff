#ifndef FIRESTAFF_REDMCSB_FIO1_FILE_HANDLE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_FIO1_FILE_HANDLE_PC34_COMPAT_H

#include <stdint.h>

#include "redmcsb_f0770_file_open_pc34_compat.h"
#include "redmcsb_f0771_file_close_pc34_compat.h"
#include "redmcsb_f0772_file_read_pc34_compat.h"
#include "redmcsb_f0773_file_write_pc34_compat.h"
#include "redmcsb_f0774_file_seek_pc34_compat.h"
#include "redmcsb_f0775_file_get_size_pc34_compat.h"
#include "redmcsb_f0776_file_create_pc34_compat.h"
#include "redmcsb_f0777_file_delete_pc34_compat.h"
#include "redmcsb_f0778_file_rename_pc34_compat.h"
#include "redmcsb_f0779_file_tell_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_FIO1_FILE_NAME_CAPACITY_PC34 = 256,
    REDMCSB_FIO1_RESULT_SUCCESS_PC34 = 0,
    REDMCSB_FIO1_RESULT_INVALID_PC34 = 3,
    REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34 = 4,
    REDMCSB_FIO1_RESULT_READ_FAILED_PC34 = 7,
    REDMCSB_FIO1_RESULT_WRITE_FAILED_PC34 = 8,
    REDMCSB_FIO1_RESULT_NOT_OPEN_PC34 = 12,
    REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34 = 17,
    REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34 = 22
};

typedef struct RedmcsbFio1FileHandlePc34Compat {
    int16_t fileHandle;
    int opened;
    char fileName[REDMCSB_FIO1_FILE_NAME_CAPACITY_PC34];
} RedmcsbFio1FileHandlePc34Compat;

typedef int (*redmcsb_fio1_enumerate_matches_pc34_compat)(
    void *context, const char *path_pattern, char *out_names,
    uint16_t name_stride, uint16_t max_names, uint16_t *out_count);
typedef int (*redmcsb_fio1_delete_result_pc34_compat)(
    void *context, const char *file_name);
typedef int (*redmcsb_fio1_rename_result_pc34_compat)(
    void *context, const char *source_file_name, const char *destination_file_name);

typedef struct RedmcsbFio1HostPc34Compat {
    void *context;
    redmcsb_f0770_dos_open_pc34_compat openFile;
    redmcsb_f0771_dos_close_pc34_compat closeFile;
    RedmcsbF0772ReadCallback readFile;
    RedmcsbF0773WriteCallback writeFile;
    redmcsb_f0774_dos_seek_from_beginning_pc34_compat seekFromBeginning;
    redmcsb_f0779_dos_tell_pc34_compat tellFile;
    redmcsb_f0775_dos_seek_to_end_pc34_compat seekToEnd;
    redmcsb_f0776_dos_create_pc34_compat createFile;
    redmcsb_fio1_delete_result_pc34_compat deleteFile;
    redmcsb_fio1_rename_result_pc34_compat renameFile;
    redmcsb_fio1_enumerate_matches_pc34_compat enumerateMatches;
} RedmcsbFio1HostPc34Compat;

void RedmcsbFio1FileHandleInitPc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *file_name);
void F1321_FIO1_07_SubstituteStringInFileNamePc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *needle,
    const char *replacement);
int F1323_FIO1_25_EnumerateMatchesPc34Compat(
    const RedmcsbFio1HostPc34Compat *host,
    const RedmcsbFio1FileHandlePc34Compat *file, char *out_names,
    uint16_t name_stride, uint16_t max_names, uint16_t *out_count);
int F1328_FIO1_22_ProbeClosedPc34Compat(
    const RedmcsbFio1HostPc34Compat *host,
    RedmcsbFio1FileHandlePc34Compat *file);
int F1329_FIO1_21_OpenPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 RedmcsbFio1FileHandlePc34Compat *file);
int F1330_FIO1_20_CreatePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   RedmcsbFio1FileHandlePc34Compat *file);
int F1331_FIO1_19_ClosePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                  RedmcsbFio1FileHandlePc34Compat *file);
int F1332_FIO1_18_GetSizePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                    const RedmcsbFio1FileHandlePc34Compat *file,
                                    uint32_t *out_size);
int F1333_FIO1_17_TellPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 int32_t *out_offset);
int F1334_FIO1_16_SeekPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 int32_t offset);
int F1335_FIO1_15_ReadPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 unsigned long byte_count, unsigned char *buffer);
int F1336_FIO1_14_WritePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                  const RedmcsbFio1FileHandlePc34Compat *file,
                                  unsigned long byte_count,
                                  const unsigned char *buffer);
int F1338_FIO1_12_DeletePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   const RedmcsbFio1FileHandlePc34Compat *file);
int F1339_FIO1_11_RenamePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   RedmcsbFio1FileHandlePc34Compat *source,
                                   const RedmcsbFio1FileHandlePc34Compat *destination);
int F1341_FIO1_10_IsClosedPc34Compat(
    const RedmcsbFio1FileHandlePc34Compat *file);
void F1342_FIO1_08_SubstituteWildcardPc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *replacement);
const char *redmcsb_fio1_file_handle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
