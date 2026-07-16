#include "redmcsb_f1007_add_memory_chunk_pc34_compat.h"
#include "redmcsb_f1008_get_largest_available_memory_chunk.h"
#include "redmcsb_f1017_malloc_pc34_compat.h"
#include "redmcsb_f1018_mfree_pc34_compat.h"

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

static int test_f1007_source_named_wrapper_prepends_memory_chunk(void)
{
    unsigned char first_storage[128];
    unsigned char second_storage[160];
    redmcsb_f1007_memory_chunk_pc34_compat *first =
        (redmcsb_f1007_memory_chunk_pc34_compat *)first_storage;
    redmcsb_f1007_memory_chunk_pc34_compat *second =
        (redmcsb_f1007_memory_chunk_pc34_compat *)second_storage;

    memset(first_storage, 0xcc, sizeof(first_storage));
    memset(second_storage, 0xdd, sizeof(second_storage));
    redmcsb_f1007_first_memory_chunk_pc34_compat = 0;

    F1007_AddMemoryChunk(first, (int32_t)sizeof(first_storage), 0x0003);
    CHECK(redmcsb_f1007_first_memory_chunk_pc34_compat == first);
    CHECK(first->next == 0);
    CHECK(first->total_byte_count == (int32_t)sizeof(first_storage));
    CHECK(first->top == first_storage + sizeof(first_storage));
    CHECK(first->available_byte_count ==
          (int32_t)(sizeof(first_storage) - sizeof(*first)));
    CHECK(first->requirements == 0x0003);
    CHECK(first->in_use == 0);
    CHECK(first->backup_top == first->top);
    CHECK(first->backup_available_byte_count == first->available_byte_count);

    F1007_AddMemoryChunk(second, (int32_t)sizeof(second_storage), 0x0007);
    CHECK(redmcsb_f1007_first_memory_chunk_pc34_compat == second);
    CHECK(second->next == first);
    CHECK(second->requirements == 0x0007);
    return 0;
}

static int test_f1007_rejects_tiny_chunk_without_mutation(void)
{
    redmcsb_f1007_memory_chunk_pc34_compat existing;
    redmcsb_f1007_memory_chunk_pc34_compat tiny;

    memset(&existing, 0, sizeof(existing));
    memset(&tiny, 0x5a, sizeof(tiny));
    redmcsb_f1007_first_memory_chunk_pc34_compat = &existing;

    F1007_AddMemoryChunk(&tiny, (int32_t)sizeof(tiny), 0x0001);
    CHECK(redmcsb_f1007_first_memory_chunk_pc34_compat == &existing);
    CHECK(tiny.next != &existing);
    return 0;
}

static redmcsb_f1008_memory_chunk make_f1008_chunk(
    int32_t byte_count,
    int16_t requirements,
    int16_t in_use)
{
    redmcsb_f1008_memory_chunk chunk;

    memset(&chunk, 0, sizeof(chunk));
    chunk.total_byte_count = byte_count;
    chunk.requirements = requirements;
    chunk.in_use = in_use;
    return chunk;
}

static int test_f1008_source_named_wrapper_selects_largest_eligible_chunk(void)
{
    redmcsb_f1008_memory_chunk first = make_f1008_chunk(80, 0x0007, 0);
    redmcsb_f1008_memory_chunk second = make_f1008_chunk(160, 0x0003, 0);
    redmcsb_f1008_memory_chunk third = make_f1008_chunk(160, 0x0007, 0);
    redmcsb_f1008_memory_chunk fourth = make_f1008_chunk(300, 0x0007, 1);
    redmcsb_f1008_memory_chunk *selected;

    first.next = &second;
    second.next = &third;
    third.next = &fourth;

    selected = F1008_GetLargestAvailableMemoryChunk(&first, 0x0005);
    CHECK(selected == &third);
    CHECK(third.in_use == 1);
    CHECK(fourth.in_use == 1);
    CHECK(first.in_use == 0);
    CHECK(second.in_use == 0);
    return 0;
}

static int test_f1017_and_f1018_pc34_boundaries_fail_closed(void)
{
    CHECK(redmcsb_f1017_malloc_pc34_compat(32u) == 0);
    CHECK(F1017_Malloc(64u) == 0);
    CHECK(redmcsb_f1018_mfree_pc34_compat() == false);
    CHECK(F1018_Mfree() == false);
    return 0;
}

static int test_source_evidence_names_bundle(void)
{
    const char *f1007 = redmcsb_f1007_add_memory_chunk_source_evidence();
    const char *f1008 =
        redmcsb_f1008_get_largest_available_memory_chunk_source_evidence();
    const char *f1017 = redmcsb_f1017_malloc_source_evidence_pc34();
    const char *f1018 = redmcsb_f1018_mfree_source_evidence_pc34();

    CHECK(f1007 != 0);
    CHECK(strstr(f1007, "F1007_AddMemoryChunk") != 0);
    CHECK(strstr(f1007, "MEMORY.C:177") != 0);
    CHECK(f1008 != 0);
    CHECK(strstr(f1008, "F1008_GetLargestAvailableMemoryChunk") != 0);
    CHECK(strstr(f1008, "MEMORY.C:191") != 0);
    CHECK(f1017 != 0);
    CHECK(strstr(f1017, "F1017_Malloc") != 0);
    CHECK(strstr(f1017, "CEDT018.C:208") != 0);
    CHECK(f1018 != 0);
    CHECK(strstr(f1018, "F1018_Mfree") != 0);
    CHECK(strstr(f1018, "CEDT018.C:216") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_f1007_source_named_wrapper_prepends_memory_chunk() == 0);
    CHECK(test_f1007_rejects_tiny_chunk_without_mutation() == 0);
    CHECK(test_f1008_source_named_wrapper_selects_largest_eligible_chunk() == 0);
    CHECK(test_f1017_and_f1018_pc34_boundaries_fail_closed() == 0);
    CHECK(test_source_evidence_names_bundle() == 0);
    return 0;
}
