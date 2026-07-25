#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1008_get_largest_available_memory_chunk.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static redmcsb_f1008_memory_chunk make_chunk(int32_t total_byte_count,
                                              int16_t requirements,
                                              int16_t in_use)
{
    redmcsb_f1008_memory_chunk chunk = { 0 };

    chunk.total_byte_count = total_byte_count;
    chunk.requirements = requirements;
    chunk.in_use = in_use;
    return chunk;
}

int main(void)
{
    redmcsb_f1008_memory_chunk first = make_chunk(80, 0x0007, 0);
    redmcsb_f1008_memory_chunk second = make_chunk(160, 0x0003, 0);
    redmcsb_f1008_memory_chunk third = make_chunk(160, 0x0007, 0);
    redmcsb_f1008_memory_chunk fourth = make_chunk(300, 0x0007, 1);
    redmcsb_f1008_memory_chunk fifth = make_chunk(160, 0x0007, 0);
    redmcsb_f1008_memory_chunk *selected;
    (void)selected;
    const char *evidence;
    (void)evidence;

    first.next = &second;
    second.next = &third;
    third.next = &fourth;
    fourth.next = &fifth;

    assert(redmcsb_f1008_get_largest_available_memory_chunk(0, 0x0001) ==
           0);

    selected = redmcsb_f1008_get_largest_available_memory_chunk(&first,
                                                                  0x0005);
    assert(selected == &third);
    assert(first.in_use == 0);
    assert(second.in_use == 0);
    assert(third.in_use == 1);
    assert(fourth.in_use == 1);
    assert(fifth.in_use == 0);

    selected = redmcsb_f1008_get_largest_available_memory_chunk(&first,
                                                                  0x0001);
    assert(selected == &second);
    assert(second.in_use == 1);

    assert(redmcsb_f1008_get_largest_available_memory_chunk(&first,
                                                              0x0007) ==
           &fifth);
    assert(fifth.in_use == 1);
    assert(redmcsb_f1008_get_largest_available_memory_chunk(&first,
                                                              0x0007) ==
           &first);
    assert(first.in_use == 1);
    assert(redmcsb_f1008_get_largest_available_memory_chunk(&first,
                                                              0x0001) ==
           0);

    evidence =
        redmcsb_f1008_get_largest_available_memory_chunk_source_evidence();
    assert(strstr(evidence, "MEMORY.C:191-220") != 0);
    assert(strstr(evidence, "F1008_GetLargestAvailableMemoryChunk") != 0);
    puts("ok: ReDMCSB F1008 largest available memory chunk");
    return 0;
}
