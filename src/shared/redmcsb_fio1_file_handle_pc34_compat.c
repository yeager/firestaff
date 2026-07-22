#include "redmcsb_fio1_file_handle_pc34_compat.h"

#include <string.h>

static int valid_file(const RedmcsbFio1FileHandlePc34Compat *file)
{
    return file != 0 && file->fileName[0] != '\0';
}

static int active_file(const RedmcsbFio1FileHandlePc34Compat *file)
{
    return valid_file(file) && file->opened && file->fileHandle >= 0;
}

typedef struct FioSizeContext {
    const RedmcsbFio1HostPc34Compat *host;
    void *callerContext;
} FioSizeContext;

static int32_t size_tell(void *context, int16_t handle)
{
    FioSizeContext *size_context = context;
    return size_context->host->tellFile(size_context->callerContext, handle);
}

static uint32_t size_seek_end(void *context, int16_t handle)
{
    FioSizeContext *size_context = context;
    return size_context->host->seekToEnd(size_context->callerContext, handle);
}

static void size_restore_beginning(void *context, int16_t handle, int32_t offset)
{
    FioSizeContext *size_context = context;
    (void)size_context->host->seekFromBeginning(size_context->callerContext,
                                                handle, offset);
}

static int replace_first(char *destination, size_t capacity,
                         const char *source, const char *needle,
                         const char *replacement)
{
    const char *match;
    size_t prefix;
    size_t suffix;
    size_t replacement_len;

    if (!destination || !source || !needle || !replacement || !needle[0]) {
        return 0;
    }
    match = strstr(source, needle);
    if (!match) return 0;
    prefix = (size_t)(match - source);
    suffix = strlen(match + strlen(needle));
    replacement_len = strlen(replacement);
    if (prefix + replacement_len + suffix >= capacity) return 0;
    memcpy(destination, source, prefix);
    memcpy(destination + prefix, replacement, replacement_len);
    memcpy(destination + prefix + replacement_len,
           match + strlen(needle), suffix + 1u);
    return 1;
}

void RedmcsbFio1FileHandleInitPc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *file_name)
{
    size_t length;
    if (!file) return;
    memset(file, 0, sizeof(*file));
    file->fileHandle = -1;
    if (!file_name) return;
    length = strlen(file_name);
    if (length >= sizeof(file->fileName)) return;
    memcpy(file->fileName, file_name, length + 1u);
}

void F1321_FIO1_07_SubstituteStringInFileNamePc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *needle,
    const char *replacement)
{
    char replaced[REDMCSB_FIO1_FILE_NAME_CAPACITY_PC34];
    if (!valid_file(file) ||
        !replace_first(replaced, sizeof(replaced), file->fileName, needle,
                       replacement)) return;
    memcpy(file->fileName, replaced, sizeof(file->fileName));
}

int F1323_FIO1_25_EnumerateMatchesPc34Compat(
    const RedmcsbFio1HostPc34Compat *host,
    const RedmcsbFio1FileHandlePc34Compat *file, char *out_names,
    uint16_t name_stride, uint16_t max_names, uint16_t *out_count)
{
    if (out_count) *out_count = 0;
    if (!host || !host->enumerateMatches || !valid_file(file) || !out_names ||
        !out_count || name_stride == 0 || max_names == 0) {
        return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
    }
    if (!host->enumerateMatches(host->context, file->fileName, out_names,
                                name_stride, max_names, out_count)) {
        *out_count = 0;
    }
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1329_FIO1_21_OpenPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 RedmcsbFio1FileHandlePc34Compat *file)
{
    int16_t handle;
    if (!host || !host->openFile || !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (file->opened) return REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34;
    handle = redmcsb_f0770_file_open_pc34_compat(host->openFile, host->context,
                                                  file->fileName);
    if (handle < 0) return REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34;
    file->fileHandle = handle;
    file->opened = 1;
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1330_FIO1_20_CreatePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   RedmcsbFio1FileHandlePc34Compat *file)
{
    int16_t handle;
    if (!host || !host->createFile || !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (file->opened) return REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34;
    handle = redmcsb_f0776_file_create_pc34_compat(host->createFile,
                                                    host->context, file->fileName);
    if (handle < 0) return REDMCSB_FIO1_RESULT_WRITE_FAILED_PC34;
    file->fileHandle = handle;
    file->opened = 1;
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1331_FIO1_19_ClosePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                  RedmcsbFio1FileHandlePc34Compat *file)
{
    if (!host || !host->closeFile || !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    redmcsb_f0771_file_close_pc34_compat(host->closeFile, host->context,
                                          file->fileHandle);
    file->fileHandle = -1;
    file->opened = 0;
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1328_FIO1_22_ProbeClosedPc34Compat(
    const RedmcsbFio1HostPc34Compat *host,
    RedmcsbFio1FileHandlePc34Compat *file)
{
    int result;
    if (!valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    result = F1329_FIO1_21_OpenPc34Compat(host, file);
    if (result == REDMCSB_FIO1_RESULT_SUCCESS_PC34) {
        return F1331_FIO1_19_ClosePc34Compat(host, file);
    }
    return REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34;
}

int F1332_FIO1_18_GetSizePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                    const RedmcsbFio1FileHandlePc34Compat *file,
                                    uint32_t *out_size)
{
    FioSizeContext size_context;
    if (!host || !host->tellFile || !host->seekToEnd || !host->seekFromBeginning || !out_size ||
        !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    size_context.host = host;
    size_context.callerContext = host->context;
    *out_size = redmcsb_f0775_file_get_size_pc34_compat(
        size_tell, size_seek_end, size_restore_beginning, &size_context,
        file->fileHandle);
    return *out_size == UINT32_MAX ? REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34
                                   : REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1333_FIO1_17_TellPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 int32_t *out_offset)
{
    if (!host || !host->tellFile || !out_offset || !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    *out_offset = redmcsb_f0779_file_tell_pc34_compat(
        host->tellFile, host->context, file->fileHandle);
    return *out_offset < 0 ? REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34
                           : REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1334_FIO1_16_SeekPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 int32_t offset)
{
    if (!host || !host->seekFromBeginning || !valid_file(file) || offset < 0)
        return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    return redmcsb_f0774_file_seek_pc34_compat(host->seekFromBeginning,
                                                host->context, file->fileHandle,
                                                offset)
               ? REDMCSB_FIO1_RESULT_SUCCESS_PC34
               : REDMCSB_FIO1_RESULT_INVALID_PC34;
}

int F1335_FIO1_15_ReadPc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 unsigned long byte_count, unsigned char *buffer)
{
    if (!host || !host->readFile || !valid_file(file) || (byte_count && !buffer) ||
        byte_count > 0x7fffffffUL) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    if (byte_count == 0) return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
    return RedmcsbF0772FileReadPc34Compat(host->context, host->readFile,
                                           file->fileHandle, byte_count, buffer)
               ? REDMCSB_FIO1_RESULT_SUCCESS_PC34
               : REDMCSB_FIO1_RESULT_READ_FAILED_PC34;
}

int F1336_FIO1_14_WritePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                  const RedmcsbFio1FileHandlePc34Compat *file,
                                  unsigned long byte_count,
                                  const unsigned char *buffer)
{
    if (!host || !host->writeFile || !valid_file(file) || (byte_count && !buffer) ||
        byte_count > 0x7fffffffUL) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (!active_file(file)) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    if (byte_count == 0) return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
    return RedmcsbF0773FileWritePc34Compat(host->context, host->writeFile,
                                            file->fileHandle, byte_count, buffer)
               ? REDMCSB_FIO1_RESULT_SUCCESS_PC34
               : REDMCSB_FIO1_RESULT_WRITE_FAILED_PC34;
}

int F1338_FIO1_12_DeletePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   const RedmcsbFio1FileHandlePc34Compat *file)
{
    if (!host || !host->deleteFile || !valid_file(file)) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (file->opened) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    return host->deleteFile(host->context, file->fileName)
               ? REDMCSB_FIO1_RESULT_SUCCESS_PC34
               : REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34;
}

int F1339_FIO1_11_RenamePc34Compat(const RedmcsbFio1HostPc34Compat *host,
                                   RedmcsbFio1FileHandlePc34Compat *source,
                                   const RedmcsbFio1FileHandlePc34Compat *destination)
{
    if (!host || !host->renameFile || !valid_file(source) || !valid_file(destination))
        return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (source->opened || destination->opened) return REDMCSB_FIO1_RESULT_NOT_OPEN_PC34;
    if (!host->renameFile(host->context, source->fileName, destination->fileName))
        return REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34;
    memcpy(source->fileName, destination->fileName, sizeof(source->fileName));
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
}

int F1341_FIO1_10_IsClosedPc34Compat(
    const RedmcsbFio1FileHandlePc34Compat *file)
{
    return !active_file(file);
}

void F1342_FIO1_08_SubstituteWildcardPc34Compat(
    RedmcsbFio1FileHandlePc34Compat *file, const char *replacement)
{
    F1321_FIO1_07_SubstituteStringInFileNamePc34Compat(file, "*", replacement);
}

const char *redmcsb_fio1_file_handle_source_evidence_pc34(void)
{
    return "ReDMCSB FIO1.C:125-840 F1321/F1323/F1328-F1336/F1338-F1339/"
           "F1341-F1342; FILE.C PC34 F0770-F0779 callback primitives. "
           "Floppy, format, drive lock, and disk-change routes are excluded.";
}
