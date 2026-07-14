#include "f0917_prim_15_memory_allocate_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    size_t call_count;
    size_t byte_count;
    void *result;
} AllocationCapture;

static void *capture_allocate(void *context, size_t byte_count)
{
    AllocationCapture *capture = context;

    ++capture->call_count;
    capture->byte_count = byte_count;
    return capture->result;
}

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
    unsigned char storage[1];
    AllocationCapture capture = {0u, 0u, storage};
    f0917_prim_15_memory_allocator_pc34_compat allocator = {
        &capture,
        capture_allocate};
    int ok = 1;

    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(&allocator, 27) ==
                    storage &&
                    capture.call_count == 1u && capture.byte_count == 27u,
                "positive byte count is forwarded once with its callback result");

    capture.result = NULL;
    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(&allocator, 0) ==
                    NULL &&
                    capture.call_count == 2u && capture.byte_count == 0u,
                "zero is forwarded and callback failure is preserved");

    capture.result = storage;
    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(&allocator, INT32_MAX) ==
                    storage &&
                    capture.call_count == 3u &&
                    capture.byte_count == (size_t)INT32_MAX,
                "largest PC signed-long count is forwarded without narrowing");

    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(&allocator, -1) ==
                    NULL &&
                    capture.call_count == 3u,
                "negative count is rejected without a callback");
    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(NULL, 1) == NULL &&
                    capture.call_count == 3u,
                "absent allocator is rejected without a callback");

    allocator.allocate = NULL;
    ok &= check(f0917_prim_15_memory_allocate_pc34_compat(&allocator, 1) ==
                    NULL &&
                    capture.call_count == 3u,
                "absent allocation callback is rejected without a callback");
    ok &= check(strstr(f0917_prim_15_memory_allocate_source_evidence_pc34(),
                       "F0917_PRIM_15_Memory_Allocate") != NULL,
                "source evidence identifies the mapped routine");

    if (!ok) {
        return 1;
    }

    puts("PASS f0917_prim_15_memory_allocate_pc34_compat");
    return 0;
}
