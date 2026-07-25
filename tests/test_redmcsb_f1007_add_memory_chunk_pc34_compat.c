#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1007_add_memory_chunk_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

enum {
    REDMCSB_F1007_PAYLOAD_BYTE_COUNT = 64
};

typedef union {
    max_align_t alignment;
    unsigned char bytes[sizeof(redmcsb_f1007_memory_chunk_pc34_compat) +
                        REDMCSB_F1007_PAYLOAD_BYTE_COUNT];
} redmcsb_f1007_chunk_storage;

int main(void)
{
    redmcsb_f1007_memory_chunk_pc34_compat existing = { 0 };
    redmcsb_f1007_chunk_storage storage = { 0 };
    redmcsb_f1007_memory_chunk_pc34_compat *memory_chunk =
        (redmcsb_f1007_memory_chunk_pc34_compat *)(void *)storage.bytes;
    const char *evidence;
    (void)evidence;
    int32_t byte_count = (int32_t)sizeof(storage.bytes);

    redmcsb_f1007_first_memory_chunk_pc34_compat = &existing;
    F1007_AddMemoryChunk_PC34_Compat(memory_chunk, byte_count, INT16_C(0x0700));

    assert(redmcsb_f1007_first_memory_chunk_pc34_compat == memory_chunk);
    assert(memory_chunk->next == &existing);
    assert(memory_chunk->total_byte_count == REDMCSB_F1007_PAYLOAD_BYTE_COUNT);
    assert(memory_chunk->requirements == INT16_C(0x0700));
    assert(memory_chunk->in_use == 0);
    assert(memory_chunk->top == storage.bytes + sizeof(*memory_chunk) +
                                      REDMCSB_F1007_PAYLOAD_BYTE_COUNT);
    assert(memory_chunk->backup_top == memory_chunk->top);
    assert(memory_chunk->available_byte_count == REDMCSB_F1007_PAYLOAD_BYTE_COUNT);
    assert(memory_chunk->backup_available_byte_count ==
           memory_chunk->available_byte_count);

    evidence = redmcsb_f1007_add_memory_chunk_source_evidence();
    assert(strstr(evidence, "MEMORY.C:177-188") != NULL);
    assert(strstr(evidence, "MEDIA758_A36M_A35E_A35M") != NULL);
    puts("ok: ReDMCSB F1007 memory chunk initialization and prepend");
    return 0;
}
