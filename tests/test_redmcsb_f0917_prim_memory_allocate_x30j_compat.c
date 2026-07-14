#include "redmcsb_f0917_prim_memory_allocate_x30j_compat.h"

typedef struct {
    int calls;
    int32_t byte_count;
    uintptr_t allocation_result;
    int error_calls;
    int error_number;
} test_prim;

static uintptr_t test_allocate(void *context, int32_t byte_count)
{
    test_prim *prim = context;

    prim->calls++;
    prim->byte_count = byte_count;
    return prim->allocation_result;
}

static void test_print_os_error(void *context, int error_number)
{
    test_prim *prim = context;

    prim->error_calls++;
    prim->error_number = error_number;
}

int main(void)
{
    test_prim prim = {0, 0, (uintptr_t)UINT32_C(0x00012340), 0, 0};

    if (redmcsb_f0917_prim_memory_allocate_x30j_compat(
            test_allocate, test_print_os_error, &prim, INT32_C(4096)) !=
            (uintptr_t)UINT32_C(0x00012340) ||
        prim.calls != 1 || prim.byte_count != INT32_C(4096) ||
        prim.error_calls != 0) {
        return 1;
    }

    prim.allocation_result = (uintptr_t)UINT32_C(0xf0000012);
    if (redmcsb_f0917_prim_memory_allocate_x30j_compat(
            test_allocate, test_print_os_error, &prim, INT32_C(-8)) !=
            (uintptr_t)UINT32_C(0xf0000012) ||
        prim.calls != 2 || prim.byte_count != INT32_C(-8) ||
        prim.error_calls != 1 || prim.error_number != 1) {
        return 2;
    }
    return 0;
}
