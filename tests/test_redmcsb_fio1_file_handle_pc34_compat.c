#include "redmcsb_fio1_file_handle_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct TestHost {
    unsigned char bytes[64];
    size_t length;
    size_t offset;
    int16_t nextHandle;
    int closeCount;
    int deleteCount;
    int renameCount;
    int failOpen;
    int failCreate;
    int failRead;
    int failWrite;
    int failSeek;
    int failTell;
    int failSize;
    int failDelete;
    int failRename;
} TestHost;

static __attribute__((unused)) int valid_handle(int16_t handle) { return handle == 7; }

static bool open_file(void *context, const char *name, uint8_t mode,
                      int16_t *out_handle)
{
    (void)mode;
    (void)name;
    TestHost *host = context;
    assert(strcmp(name, "SAVES/DM.DAT") == 0);
    assert(mode == REDMCSB_F0770_DOS_OPEN_READ_WRITE_PC34);
    if (host->failOpen) return 0;
    *out_handle = host->nextHandle;
    return true;
}

static bool create_file(void *context, const char *name, uint16_t attributes,
                        int16_t *out_handle)
{
    (void)attributes;
    (void)name;
    TestHost *host = context;
    assert(strcmp(name, "SAVES/NEW.DAT") == 0 ||
           strcmp(name, "SAVES/DM.DAT") == 0);
    assert(attributes == REDMCSB_F0776_DOS_CREATE_ATTRIBUTES_PC34);
    if (host->failCreate) return false;
    *out_handle = host->nextHandle;
    host->length = 0;
    host->offset = 0;
    return 1;
}

static void close_file(void *context, int16_t handle)
{
    (void)handle;
    TestHost *host = context;
    assert(valid_handle(handle));
    host->closeCount++;
}

static size_t read_file(void *context, int16_t handle, unsigned char *buffer,
                        uint16_t count)
{
    (void)handle;
    TestHost *host = context;
    size_t available;
    assert(valid_handle(handle));
    if (host->failRead) return 0;
    available = host->length - host->offset;
    if (available < count) return 0;
    memcpy(buffer, host->bytes + host->offset, count);
    host->offset += count;
    return count;
}

static struct RedmcsbF0773FileWriteResult write_file(
    void *context, int16_t handle, const unsigned char *buffer, uint16_t count)
{
    (void)handle;
    TestHost *host = context;
    struct RedmcsbF0773FileWriteResult result = {0, 1};
    assert(valid_handle(handle));
    if (host->failWrite) return result;
    if (host->offset + count > sizeof(host->bytes)) return result;
    memcpy(host->bytes + host->offset, buffer, count);
    host->offset += count;
    if (host->offset > host->length) host->length = host->offset;
    result.transferred = count;
    result.carry = 0;
    return result;
}

static bool seek_begin(void *context, int16_t handle, int32_t offset)
{
    (void)handle;
    TestHost *host = context;
    assert(valid_handle(handle));
    if (host->failSeek || offset < 0 || (size_t)offset > host->length) return false;
    host->offset = (size_t)offset;
    return true;
}

static int32_t tell_file(void *context, int16_t handle)
{
    (void)handle;
    TestHost *host = context;
    assert(valid_handle(handle));
    return host->failTell ? -1 : (int32_t)host->offset;
}

static uint32_t seek_end(void *context, int16_t handle)
{
    (void)handle;
    TestHost *host = context;
    assert(valid_handle(handle));
    if (host->failSize) return UINT32_MAX;
    host->offset = host->length;
    return (uint32_t)host->length;
}

static int delete_file(void *context, const char *name)
{
    (void)name;
    TestHost *host = context;
    assert(strcmp(name, "SAVES/DM.DAT") == 0);
    host->deleteCount++;
    return !host->failDelete;
}

static int rename_file(void *context, const char *source, const char *destination)
{
    (void)destination;
    (void)source;
    TestHost *host = context;
    assert(strcmp(source, "SAVES/DM.DAT") == 0);
    assert(strcmp(destination, "SAVES/NEW.DAT") == 0);
    host->renameCount++;
    return !host->failRename;
}

static int enumerate_matches(void *context, const char *pattern, char *out_names,
                             uint16_t stride, uint16_t max_names,
                             uint16_t *out_count)
{
    (void)max_names;
    (void)pattern;
    (void)context;
    assert(strcmp(pattern, "SAVES/*.DAT") == 0);
    assert(stride >= 8 && max_names >= 2);
    strcpy(out_names, "DM.DAT");
    strcpy(out_names + stride, "CSB.DAT");
    *out_count = 2;
    return 1;
}

int main(void)
{
    TestHost state;
    RedmcsbFio1HostPc34Compat host;
    RedmcsbFio1FileHandlePc34Compat file;
    RedmcsbFio1FileHandlePc34Compat destination;
    unsigned char input[] = {1, 2, 3, 4};
    (void)input;
    unsigned char output[4] = {0};
    (void)output;
    char names[32] = {0};
    (void)names;
    uint16_t count = 0;
    (void)count;
    uint32_t size = 0;
    (void)size;
    int32_t offset = -1;
    (void)offset;

    memset(&state, 0, sizeof(state));
    state.nextHandle = 7;
    memset(&host, 0, sizeof(host));
    host.context = &state;
    host.openFile = open_file;
    host.closeFile = close_file;
    host.readFile = read_file;
    host.writeFile = write_file;
    host.seekFromBeginning = seek_begin;
    host.tellFile = tell_file;
    host.seekToEnd = seek_end;
    host.createFile = create_file;
    host.deleteFile = delete_file;
    host.renameFile = rename_file;
    host.enumerateMatches = enumerate_matches;

    RedmcsbFio1FileHandleInitPc34Compat(&file, "ROOT/*");
    F1342_FIO1_08_SubstituteWildcardPc34Compat(&file, "DM.DAT");
    assert(strcmp(file.fileName, "ROOT/DM.DAT") == 0);
    F1321_FIO1_07_SubstituteStringInFileNamePc34Compat(&file, "ROOT", "SAVES");
    assert(strcmp(file.fileName, "SAVES/DM.DAT") == 0);
    assert(F1329_FIO1_21_OpenPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1329_FIO1_21_OpenPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34);
    assert(F1336_FIO1_14_WritePc34Compat(&host, &file, sizeof(input), input) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1333_FIO1_17_TellPc34Compat(&host, &file, &offset) == REDMCSB_FIO1_RESULT_SUCCESS_PC34 && offset == 4);
    assert(F1332_FIO1_18_GetSizePc34Compat(&host, &file, &size) == REDMCSB_FIO1_RESULT_SUCCESS_PC34 && size == 4);
    assert(F1333_FIO1_17_TellPc34Compat(&host, &file, &offset) == REDMCSB_FIO1_RESULT_SUCCESS_PC34 && offset == 4);
    assert(F1334_FIO1_16_SeekPc34Compat(&host, &file, 0) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1335_FIO1_15_ReadPc34Compat(&host, &file, sizeof(output), output) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(memcmp(input, output, sizeof(input)) == 0);
    assert(F1331_FIO1_19_ClosePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1341_FIO1_10_IsClosedPc34Compat(&file) == 1);
    assert(F1328_FIO1_22_ProbeClosedPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(state.closeCount == 2);
    assert(F1338_FIO1_12_DeletePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34 && state.deleteCount == 1);

    RedmcsbFio1FileHandleInitPc34Compat(&destination, "SAVES/NEW.DAT");
    assert(F1339_FIO1_11_RenamePc34Compat(&host, &file, &destination) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(state.renameCount == 1 && strcmp(file.fileName, "SAVES/NEW.DAT") == 0);
    assert(F1330_FIO1_20_CreatePc34Compat(&host, &destination) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1331_FIO1_19_ClosePc34Compat(&host, &destination) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);

    RedmcsbFio1FileHandleInitPc34Compat(&file, "SAVES/*.DAT");
    assert(F1323_FIO1_25_EnumerateMatchesPc34Compat(&host, &file, names, 16, 2,
                                                      &count) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(count == 2 && strcmp(names, "DM.DAT") == 0 &&
           strcmp(names + 16, "CSB.DAT") == 0);
    RedmcsbFio1FileHandleInitPc34Compat(&file, NULL);
    assert(F1328_FIO1_22_ProbeClosedPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1328_FIO1_22_ProbeClosedPc34Compat(&host, NULL) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1329_FIO1_21_OpenPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1330_FIO1_20_CreatePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1331_FIO1_19_ClosePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1332_FIO1_18_GetSizePc34Compat(&host, &file, &size) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1333_FIO1_17_TellPc34Compat(&host, &file, &offset) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1334_FIO1_16_SeekPc34Compat(&host, &file, 0) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1335_FIO1_15_ReadPc34Compat(&host, &file, 0, NULL) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1336_FIO1_14_WritePc34Compat(&host, &file, 0, NULL) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1338_FIO1_12_DeletePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1339_FIO1_11_RenamePc34Compat(&host, &file, &destination) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    assert(F1341_FIO1_10_IsClosedPc34Compat(NULL) == 1);

    RedmcsbFio1FileHandleInitPc34Compat(&file, "SAVES/DM.DAT");
    assert(F1331_FIO1_19_ClosePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1332_FIO1_18_GetSizePc34Compat(&host, &file, &size) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1333_FIO1_17_TellPc34Compat(&host, &file, &offset) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1334_FIO1_16_SeekPc34Compat(&host, &file, 0) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1335_FIO1_15_ReadPc34Compat(&host, &file, 0, NULL) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1336_FIO1_14_WritePc34Compat(&host, &file, 0, NULL) == REDMCSB_FIO1_RESULT_NOT_OPEN_PC34);
    assert(F1338_FIO1_12_DeletePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    state.failOpen = 1;
    assert(F1328_FIO1_22_ProbeClosedPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34);
    assert(F1329_FIO1_21_OpenPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34);
    state.failOpen = 0;
    assert(F1329_FIO1_21_OpenPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    assert(F1328_FIO1_22_ProbeClosedPc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_ALREADY_OPEN_PC34);
    assert(F1341_FIO1_10_IsClosedPc34Compat(&file) == 0);
    state.failRead = 1;
    assert(F1335_FIO1_15_ReadPc34Compat(&host, &file, 1, output) == REDMCSB_FIO1_RESULT_READ_FAILED_PC34);
    state.failRead = 0;
    state.failWrite = 1;
    assert(F1336_FIO1_14_WritePc34Compat(&host, &file, 1, input) == REDMCSB_FIO1_RESULT_WRITE_FAILED_PC34);
    state.failWrite = 0;
    state.failTell = 1;
    assert(F1333_FIO1_17_TellPc34Compat(&host, &file, &offset) == REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34);
    state.failTell = 0;
    state.failSize = 1;
    assert(F1332_FIO1_18_GetSizePc34Compat(&host, &file, &size) == REDMCSB_FIO1_RESULT_QUERY_FAILED_PC34);
    state.failSize = 0;
    state.failSeek = 1;
    assert(F1334_FIO1_16_SeekPc34Compat(&host, &file, 0) == REDMCSB_FIO1_RESULT_INVALID_PC34);
    state.failSeek = 0;
    assert(F1331_FIO1_19_ClosePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_SUCCESS_PC34);
    state.failCreate = 1;
    assert(F1330_FIO1_20_CreatePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_WRITE_FAILED_PC34);
    state.failCreate = 0;
    state.failDelete = 1;
    assert(F1338_FIO1_12_DeletePc34Compat(&host, &file) == REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34);
    state.failDelete = 0;
    state.failRename = 1;
    assert(F1339_FIO1_11_RenamePc34Compat(&host, &file, &destination) == REDMCSB_FIO1_RESULT_OPEN_FAILED_PC34);
    state.failRename = 0;
    F1321_FIO1_07_SubstituteStringInFileNamePc34Compat(&file, "missing", "x");
    assert(strcmp(file.fileName, "SAVES/DM.DAT") == 0);
    return 0;
}
