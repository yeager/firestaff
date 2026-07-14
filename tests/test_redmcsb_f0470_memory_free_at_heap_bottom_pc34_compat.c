#include "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat heap = {
        .heap_begin = 100U,
        .permanent_end = 140U,
        .temporary_top = 180U,
        .heap_limit = 200U,
        .available_heap_byte_count = 20U,
    };
    ReDMCSBF0470MemoryHeapBoundsPc34Compat before;
    int ok = 1;

    before = heap;
    ok &= check(F0470_MEMORY_FreeAtHeapBottom_PC34(&heap, 5U),
                "odd release succeeds");
    ok &= check(heap.available_heap_byte_count == 26U,
                "odd release is rounded up before accounting");
    ok &= check(heap.heap_begin == before.heap_begin &&
                    heap.permanent_end == before.permanent_end &&
                    heap.temporary_top == before.temporary_top &&
                    heap.heap_limit == before.heap_limit,
                "release does not move allocation boundaries");

    ok &= check(F0470_MEMORY_FreeAtHeapBottom_PC34(&heap, 4U),
                "even release succeeds");
    ok &= check(heap.available_heap_byte_count == 30U,
                "even release increases available bytes exactly");

    before = heap;
    ok &= check(!F0470_MEMORY_FreeAtHeapBottom_PC34(&heap, 71U),
                "release beyond heap capacity rejects");
    ok &= check(heap.available_heap_byte_count == before.available_heap_byte_count &&
                    heap.permanent_end == before.permanent_end &&
                    heap.temporary_top == before.temporary_top,
                "capacity rejection preserves all accounting and boundaries");

    before = heap;
    ok &= check(!F0470_MEMORY_FreeAtHeapBottom_PC34(&heap, SIZE_MAX),
                "odd maximum release rejects before rounding overflow");
    ok &= check(heap.available_heap_byte_count == before.available_heap_byte_count &&
                    heap.permanent_end == before.permanent_end &&
                    heap.temporary_top == before.temporary_top,
                "overflow rejection preserves all accounting and boundaries");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat");
    return 0;
}
