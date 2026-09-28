/*
 * Bounded in-memory adapter for the public-domain 7-Zip SDK 26.02 C reader.
 * The SDK validates archive and member CRCs; Firestaff limits archive bytes,
 * solid-folder output, entry count, and all SDK allocations before exposing
 * members to asset discovery.
 */

#include "firestaff_7z_extract.h"

#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "7z.h"
#include "7zCrc.h"

#define FIRESTAFF_7Z_MAX_ARCHIVE_BYTES (32u * 1024u * 1024u)
#define FIRESTAFF_7Z_MAX_OUTPUT_BYTES  (32u * 1024u * 1024u)
#define FIRESTAFF_7Z_MAX_ALLOC_BYTES   (96u * 1024u * 1024u)
#define FIRESTAFF_7Z_MAX_FILES         8192u
#define FIRESTAFF_7Z_MAX_NAME_CHARS    4096u

typedef struct {
    ILookInStream stream;
    const Byte *bytes;
    size_t size;
    size_t position;
} Firestaff7zMemoryStream;

typedef struct {
    size_t bytes_in_use;
    size_t limit;
} Firestaff7zAllocationBudget;

typedef union {
    max_align_t alignment;
    size_t size;
} Firestaff7zAllocationHeader;

typedef struct {
    ISzAlloc interface;
    Firestaff7zAllocationBudget *budget;
} Firestaff7zAllocator;

typedef struct {
    const char *wanted_name;
    uint8_t **out_bytes;
    size_t *out_size;
    int found;
} Firestaff7zReadRequest;

static atomic_flag firestaff_7z_crc_lock = ATOMIC_FLAG_INIT;
static atomic_bool firestaff_7z_crc_ready = false;

static void firestaff_7z_ensure_crc_table(void)
{
    if (atomic_load_explicit(&firestaff_7z_crc_ready, memory_order_acquire)) {
        return;
    }
    while (atomic_flag_test_and_set_explicit(&firestaff_7z_crc_lock,
                                              memory_order_acquire)) {
    }
    if (!atomic_load_explicit(&firestaff_7z_crc_ready, memory_order_relaxed)) {
        CrcGenerateTable();
        atomic_store_explicit(&firestaff_7z_crc_ready, True,
                              memory_order_release);
    }
    atomic_flag_clear_explicit(&firestaff_7z_crc_lock, memory_order_release);
}

static SRes firestaff_7z_memory_look(ILookInStreamPtr stream,
                                     const void **buffer,
                                     size_t *size)
{
    Firestaff7zMemoryStream *memory = (Firestaff7zMemoryStream *)stream;
    size_t available = memory->size - memory->position;
    if (*size > available) {
        *size = available;
    }
    *buffer = memory->bytes + memory->position;
    return SZ_OK;
}

static SRes firestaff_7z_memory_skip(ILookInStreamPtr stream, size_t offset)
{
    Firestaff7zMemoryStream *memory = (Firestaff7zMemoryStream *)stream;
    if (offset > memory->size - memory->position) {
        return SZ_ERROR_INPUT_EOF;
    }
    memory->position += offset;
    return SZ_OK;
}

static SRes firestaff_7z_memory_read(ILookInStreamPtr stream,
                                     void *buffer,
                                     size_t *size)
{
    Firestaff7zMemoryStream *memory = (Firestaff7zMemoryStream *)stream;
    size_t available = memory->size - memory->position;
    if (*size > available) {
        *size = available;
    }
    if (*size != 0u) {
        memcpy(buffer, memory->bytes + memory->position, *size);
        memory->position += *size;
    }
    return SZ_OK;
}

static SRes firestaff_7z_memory_seek(ILookInStreamPtr stream,
                                     Int64 *position,
                                     ESzSeek origin)
{
    Firestaff7zMemoryStream *memory = (Firestaff7zMemoryStream *)stream;
    Int64 base;
    Int64 target;
    if (origin == SZ_SEEK_SET) {
        base = 0;
    } else if (origin == SZ_SEEK_CUR) {
        base = (Int64)memory->position;
    } else if (origin == SZ_SEEK_END) {
        base = (Int64)memory->size;
    } else {
        return SZ_ERROR_PARAM;
    }
    if ((*position > 0 && base > INT64_MAX - *position) ||
        (*position < 0 && base < INT64_MIN - *position)) {
        return SZ_ERROR_PARAM;
    }
    target = base + *position;
    if (target < 0 || (UInt64)target > memory->size) {
        return SZ_ERROR_INPUT_EOF;
    }
    memory->position = (size_t)target;
    *position = target;
    return SZ_OK;
}

static void *firestaff_7z_bounded_alloc(ISzAllocPtr interface, size_t size)
{
    Firestaff7zAllocator *allocator = (Firestaff7zAllocator *)interface;
    Firestaff7zAllocationBudget *budget = allocator->budget;
    Firestaff7zAllocationHeader *header;
    size_t allocation_size;
    if (size == 0u || size > SIZE_MAX - sizeof(*header)) {
        return NULL;
    }
    allocation_size = sizeof(*header) + size;
    if (allocation_size > budget->limit ||
        budget->bytes_in_use > budget->limit - allocation_size) {
        return NULL;
    }
    header = (Firestaff7zAllocationHeader *)malloc(allocation_size);
    if (!header) {
        return NULL;
    }
    header->size = allocation_size;
    budget->bytes_in_use += allocation_size;
    return (void *)(header + 1);
}

static void firestaff_7z_bounded_free(ISzAllocPtr interface, void *address)
{
    Firestaff7zAllocator *allocator = (Firestaff7zAllocator *)interface;
    Firestaff7zAllocationHeader *header;
    if (!address) {
        return;
    }
    header = ((Firestaff7zAllocationHeader *)address) - 1;
    if (header->size <= allocator->budget->bytes_in_use) {
        allocator->budget->bytes_in_use -= header->size;
    } else {
        allocator->budget->bytes_in_use = 0u;
    }
    free(header);
}

static int firestaff_7z_append_utf8(char *destination,
                                    size_t capacity,
                                    size_t *used,
                                    UInt32 codepoint)
{
    unsigned char bytes[4];
    size_t count;
    size_t i;
    if (codepoint <= 0x7fu) {
        bytes[0] = (unsigned char)codepoint;
        count = 1u;
    } else if (codepoint <= 0x7ffu) {
        bytes[0] = (unsigned char)(0xc0u | (codepoint >> 6));
        bytes[1] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 2u;
    } else if (codepoint <= 0xffffu) {
        bytes[0] = (unsigned char)(0xe0u | (codepoint >> 12));
        bytes[1] = (unsigned char)(0x80u | ((codepoint >> 6) & 0x3fu));
        bytes[2] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 3u;
    } else if (codepoint <= 0x10ffffu) {
        bytes[0] = (unsigned char)(0xf0u | (codepoint >> 18));
        bytes[1] = (unsigned char)(0x80u | ((codepoint >> 12) & 0x3fu));
        bytes[2] = (unsigned char)(0x80u | ((codepoint >> 6) & 0x3fu));
        bytes[3] = (unsigned char)(0x80u | (codepoint & 0x3fu));
        count = 4u;
    } else {
        return 0;
    }
    if (count >= capacity || *used > capacity - count - 1u) {
        return 0;
    }
    for (i = 0u; i < count; ++i) {
        destination[(*used)++] = (char)bytes[i];
    }
    return 1;
}

static int firestaff_7z_member_name(const UInt16 *wide,
                                    size_t wide_count,
                                    char *name,
                                    size_t name_capacity)
{
    size_t i;
    size_t used = 0u;
    int terminated = 0;
    if (!wide || wide_count == 0u || !name || name_capacity < 2u) {
        return 0;
    }
    for (i = 0u; i < wide_count; ++i) {
        UInt32 codepoint = wide[i];
        if (codepoint == 0u) {
            terminated = 1;
            for (++i; i < wide_count; ++i) {
                if (wide[i] != 0u) {
                    return 0;
                }
            }
            break;
        }
        if (codepoint >= 0xd800u && codepoint <= 0xdbffu) {
            UInt32 low;
            if (i + 1u >= wide_count) {
                return 0;
            }
            low = wide[++i];
            if (low < 0xdc00u || low > 0xdfffu) {
                return 0;
            }
            codepoint = 0x10000u + ((codepoint - 0xd800u) << 10) +
                       (low - 0xdc00u);
        } else if (codepoint >= 0xdc00u && codepoint <= 0xdfffu) {
            return 0;
        }
        if (!firestaff_7z_append_utf8(name, name_capacity, &used, codepoint)) {
            return 0;
        }
    }
    if (!terminated || used == 0u) {
        return 0;
    }
    name[used] = '\0';
    return 1;
}

static int firestaff_7z_safe_member_path(const char *name)
{
    const char *component = name;
    const char *cursor;
    if (!name || !name[0] || name[0] == '/' || name[0] == '\\' ||
        strstr(name, "::") || strchr(name, ':') || strchr(name, '\\')) {
        return 0;
    }
    for (cursor = name; ; ++cursor) {
        if (*cursor == '/' || *cursor == '\0') {
            size_t length = (size_t)(cursor - component);
            if (length == 0u ||
                (length == 1u && component[0] == '.') ||
                (length == 2u && component[0] == '.' && component[1] == '.')) {
                return 0;
            }
            if (*cursor == '\0') {
                break;
            }
            component = cursor + 1;
        }
    }
    return 1;
}

static int firestaff_7z_read_archive(const char *path,
                                    uint8_t **out_bytes,
                                    size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    if (!path || !out_bytes || !out_size) {
        return 0;
    }
    *out_bytes = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 32L ||
        (unsigned long)length > FIRESTAFF_7Z_MAX_ARCHIVE_BYTES ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes) {
        fclose(file);
        return 0;
    }
    if (fread(bytes, 1u, (size_t)length, file) != (size_t)length ||
        ferror(file)) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)length;
    return 1;
}

int firestaff_7z_visit_files(const char *path,
                             Firestaff7zFileVisitor visitor,
                             void *user_data)
{
    uint8_t *archive_bytes = NULL;
    size_t archive_size = 0u;
    uint8_t *folder_bytes = NULL;
    size_t folder_capacity = 0u;
    UInt32 block_index = (UInt32)-1;
    CSzArEx database;
    Firestaff7zMemoryStream memory;
    Firestaff7zAllocationBudget budget;
    Firestaff7zAllocator main_allocator;
    Firestaff7zAllocator temp_allocator;
    uint64_t total_output = 0u;
    UInt32 i;
    int result = 0;
    int database_initialized = 0;
    if (!path || !visitor || !firestaff_7z_read_archive(path, &archive_bytes,
                                                        &archive_size)) {
        return 0;
    }
    memset(&memory, 0, sizeof(memory));
    memory.bytes = archive_bytes;
    memory.size = archive_size;
    memory.stream.Look = firestaff_7z_memory_look;
    memory.stream.Skip = firestaff_7z_memory_skip;
    memory.stream.Read = firestaff_7z_memory_read;
    memory.stream.Seek = firestaff_7z_memory_seek;
    budget.bytes_in_use = 0u;
    budget.limit = FIRESTAFF_7Z_MAX_ALLOC_BYTES;
    main_allocator.interface.Alloc = firestaff_7z_bounded_alloc;
    main_allocator.interface.Free = firestaff_7z_bounded_free;
    main_allocator.budget = &budget;
    temp_allocator.interface.Alloc = firestaff_7z_bounded_alloc;
    temp_allocator.interface.Free = firestaff_7z_bounded_free;
    temp_allocator.budget = &budget;
    firestaff_7z_ensure_crc_table();
    SzArEx_Init(&database);
    database_initialized = 1;
    {
        SRes open_result = SzArEx_Open(&database, &memory.stream,
                                       &main_allocator.interface,
                                       &temp_allocator.interface);
        if (open_result != SZ_OK ||
            database.NumFiles > FIRESTAFF_7Z_MAX_FILES ||
            database.db.NumFolders > FIRESTAFF_7Z_MAX_FILES) {
            goto cleanup;
        }
    }
    for (i = 0u; i < database.db.NumFolders; ++i) {
        UInt64 folder_size = SzAr_GetFolderUnpackSize(&database.db, i);
        if (folder_size > FIRESTAFF_7Z_MAX_OUTPUT_BYTES ||
            total_output > FIRESTAFF_7Z_MAX_OUTPUT_BYTES - folder_size) {
            goto cleanup;
        }
        total_output += folder_size;
    }
    for (i = 0u; i < database.NumFiles; ++i) {
        size_t wide_count;
        UInt16 *wide_name;
        char name[16384];
        size_t offset = 0u;
        size_t output_size = 0u;
        UInt32 folder_index;
        int visit_result;
        if (SzArEx_IsDir(&database, i) || SzArEx_GetFileSize(&database, i) == 0u) {
            continue;
        }
        folder_index = database.FileToFolder[i];
        if (folder_index == (UInt32)-1) {
            continue;
        }
        wide_count = SzArEx_GetFileNameUtf16(&database, i, NULL);
        if (wide_count == 0u || wide_count > FIRESTAFF_7Z_MAX_NAME_CHARS) {
            goto cleanup;
        }
        wide_name = (UInt16 *)malloc(wide_count * sizeof(*wide_name));
        if (!wide_name) {
            goto cleanup;
        }
        if (SzArEx_GetFileNameUtf16(&database, i, wide_name) != wide_count ||
            !firestaff_7z_member_name(wide_name, wide_count, name,
                                      sizeof(name)) ||
            !firestaff_7z_safe_member_path(name)) {
            free(wide_name);
            goto cleanup;
        }
        free(wide_name);
        if (SzArEx_Extract(&database, &memory.stream, i, &block_index,
                           &folder_bytes, &folder_capacity, &offset,
                           &output_size, &main_allocator.interface,
                           &temp_allocator.interface) != SZ_OK ||
            output_size != SzArEx_GetFileSize(&database, i) ||
            offset > folder_capacity ||
            output_size > folder_capacity - offset) {
            goto cleanup;
        }
        visit_result = visitor(name, folder_bytes + offset, output_size,
                               user_data);
        if (visit_result < 0) {
            goto cleanup;
        }
        if (visit_result > 0) {
            result = 1;
            goto cleanup;
        }
    }
    result = 1;

cleanup:
    firestaff_7z_bounded_free(&main_allocator.interface, folder_bytes);
    if (database_initialized) {
        SzArEx_Free(&database, &main_allocator.interface);
    }
    free(archive_bytes);
    return result;
}

static int firestaff_7z_case_equal(const char *left, const char *right)
{
    unsigned char a;
    unsigned char b;
    if (!left || !right) {
        return 0;
    }
    while (*left && *right) {
        a = (unsigned char)*left++;
        b = (unsigned char)*right++;
        if (a >= 'A' && a <= 'Z') {
            a = (unsigned char)(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = (unsigned char)(b - 'A' + 'a');
        }
        if (a != b) {
            return 0;
        }
    }
    return *left == '\0' && *right == '\0';
}

static int firestaff_7z_read_visitor(const char *name,
                                     const uint8_t *bytes,
                                     size_t byte_count,
                                     void *user_data)
{
    Firestaff7zReadRequest *request = (Firestaff7zReadRequest *)user_data;
    uint8_t *copy;
    if (!firestaff_7z_case_equal(name, request->wanted_name)) {
        return 0;
    }
    copy = (uint8_t *)malloc(byte_count ? byte_count : 1u);
    if (!copy) {
        return -1;
    }
    if (byte_count != 0u) {
        memcpy(copy, bytes, byte_count);
    }
    *request->out_bytes = copy;
    *request->out_size = byte_count;
    request->found = 1;
    return 1;
}

int firestaff_7z_read_member(const char *path,
                             const char *member_name,
                             uint8_t **out_bytes,
                             size_t *out_size)
{
    Firestaff7zReadRequest request;
    if (out_bytes) {
        *out_bytes = NULL;
    }
    if (out_size) {
        *out_size = 0u;
    }
    if (!path || !member_name || !out_bytes || !out_size ||
        !firestaff_7z_safe_member_path(member_name)) {
        return 0;
    }
    memset(&request, 0, sizeof(request));
    request.wanted_name = member_name;
    request.out_bytes = out_bytes;
    request.out_size = out_size;
    if (!firestaff_7z_visit_files(path, firestaff_7z_read_visitor, &request) ||
        !request.found) {
        free(*out_bytes);
        *out_bytes = NULL;
        *out_size = 0u;
        return 0;
    }
    return 1;
}
