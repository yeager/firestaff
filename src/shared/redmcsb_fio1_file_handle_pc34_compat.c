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

/* ReDMCSB Toolchains/Common/Source/FIO1.C F1320_FIO1_27_ (line 113). */
int32_t F1320_FIO1_27_Pc34Compat(void)
{
    return INT32_C(234);
}

/* ReDMCSB Toolchains/Common/Source/FIO1.C F1324_FIO1_24_ (line 247). */
int16_t F1324_FIO1_24_Pc34Compat(int32_t arg0, int32_t arg1, int32_t arg2,
                                  int16_t *out_value)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    if (!out_value) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    *out_value = 0;
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
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

int F1325_FIO1_04_IsDiskWriteableHostCompat(
    const RedmcsbFio1HostPc34Compat *host, int use_default_drive_name,
    int16_t drive_index)
{
    if (use_default_drive_name != 1 || !host ||
        !host->isDiskWriteProtected || drive_index < 0 || drive_index >= 4)
        return 0;
    /* FIO1.C F1325 returns writable only for the configured default floppy
     * and when F1349_IsDiskWriteProtected reports false. Missing host media
     * probes never grant write permission. */
    return host->isDiskWriteProtected(host->context, drive_index) == 0;
}

int F1322_FIO1_26_DiskChangedHostCompat(
    const RedmcsbFio1HostPc34Compat *host, int use_default_drive_name,
    int16_t drive_index, int32_t source_disk_change_counters[4])
{
    int32_t current_counter;

    if (use_default_drive_name != 1 || drive_index < 0 || drive_index >= 4 ||
        !source_disk_change_counters || !host || !host->getDiskChangeCounter ||
        !host->isFloppyDriveEmpty)
        return 0;

    /* FIO1.C F1322 first compares F1107 with G3322. Only on a mismatch does
     * it query F1362; either condition refreshes G3322 with a second F1107. */
    current_counter = host->getDiskChangeCounter(host->context, drive_index);
    if (current_counter == source_disk_change_counters[drive_index] &&
        !host->isFloppyDriveEmpty(host->context, drive_index))
        return 0;
    source_disk_change_counters[drive_index] =
        host->getDiskChangeCounter(host->context, drive_index);
    return 1;
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

int F1337_FIO1_13_LockHostCompat(const RedmcsbFio1HostPc34Compat *host,
                                 const RedmcsbFio1FileHandlePc34Compat *file,
                                 const char *device_name,
                                 int32_t *out_size)
{
    int32_t byte_count;

    if (!host || !host->lockAndGetSize || !valid_file(file) ||
        !device_name || !device_name[0] || !out_size)
        return REDMCSB_FIO1_RESULT_INVALID_PC34;
    /* ReDMCSB FIO1.C F1337 obtains a read lock, calls Info, returns the
     * volume-reported byte size, then releases the lock. It locks the source
     * FIO1_FILE device-name field, not its file-name field. The host callback
     * owns those platform calls and must unlock on success and failure. */
    if (!host->lockAndGetSize(host->context, device_name, &byte_count))
        return REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34;
    *out_size = byte_count;
    return REDMCSB_FIO1_RESULT_SUCCESS_PC34;
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

int16_t F1340_FIO1_09_CountOpenTrackdiskDevicesHostCompat(
    const RedmcsbFio1HostPc34Compat *host)
{
    int16_t drive_index;
    int16_t open_count = 0;

    if (!host || !host->trackdiskIsOpen) return 0;
    /* ReDMCSB FIO1.C F1340 checks the four source Trackdisk indices. */
    for (drive_index = 0; drive_index < 4; ++drive_index) {
        if (host->trackdiskIsOpen(host->context, drive_index)) ++open_count;
    }
    return open_count;
}

int16_t F1343_FIO1_06_DeviceNameHostCompat(
    const RedmcsbFio1HostPc34Compat *host, int use_default_drive_name,
    int16_t drive_index, const char *device_name, int16_t info_index,
    const char **out_device_name)
{
    const char *resolved_name;

    if (!out_device_name || (use_default_drive_name != 0 &&
                             use_default_drive_name != 1))
        return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (info_index != 14) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    if (use_default_drive_name) {
        if (!host || !host->defaultFloppyDeviceName ||
            drive_index < 0 || drive_index >= 4)
            return REDMCSB_FIO1_RESULT_INVALID_PC34;
        resolved_name = host->defaultFloppyDeviceName(host->context,
                                                       drive_index);
    } else {
        resolved_name = device_name;
    }
    if (!resolved_name) return REDMCSB_FIO1_RESULT_INVALID_PC34;
    *out_device_name = resolved_name;
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
    return "ReDMCSB FIO1.C:125-840 F1321/F1322/F1323/F1325/F1328-F1343/"
           "F1341-F1342; FILE.C PC34 F0770-F0779 callback primitives. "
           "Host callbacks abstract Amiga F1337 Lock/Info/UnLock and F1340 "
           "Trackdisk open-state routes, and F1343 device-name resolution. "
           "Floppy, format, and disk-change routes are excluded.";
}
