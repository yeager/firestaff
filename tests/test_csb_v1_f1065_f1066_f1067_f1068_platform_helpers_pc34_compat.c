#include "csb_v1_f1065_f1066_f1067_f1068_platform_helpers_pc34_compat.h"
#include "redmcsb_f1065_set_exec_base_pc34_compat.h"
#include "redmcsb_f1067_init_amiga_stuff_pc34_compat.h"
#include "redmcsb_f1068_free_amiga_stuff_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

typedef struct fake_chip_memory {
    int32_t total;
    int32_t largest;
    int32_t largest_after_allocation;
    int32_t allocated_byte_count;
    int forbid_count;
    int permit_count;
    int available_total_count;
    int available_largest_count;
    int allocate_count;
    int free_count;
    int alert_count;
    uint32_t alert_code;
    unsigned char storage[4];
} fake_chip_memory;

static void fake_forbid(void *context)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;
    memory->forbid_count++;
}

static int32_t fake_available(void *context, int largest_block)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;

    if (largest_block) {
        memory->available_largest_count++;
        if (memory->allocated_byte_count > 0) {
            return memory->largest_after_allocation;
        }
        return memory->largest;
    }

    memory->available_total_count++;
    return memory->total - memory->allocated_byte_count;
}

static void *fake_allocate(void *context, int32_t byte_count)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;

    memory->allocate_count++;
    if (byte_count <= 0 || byte_count > memory->largest) {
        return NULL;
    }
    memory->allocated_byte_count = byte_count;
    return memory->storage;
}

static void fake_free(void *context, void *pointer, int32_t byte_count)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;

    CHECK(pointer == memory->storage);
    CHECK(byte_count == memory->allocated_byte_count);
    memory->free_count++;
    memory->allocated_byte_count = 0;
}

static void fake_alert(void *context, uint32_t error_code)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;

    memory->alert_count++;
    memory->alert_code = error_code;
}

static void fake_permit(void *context)
{
    fake_chip_memory *memory = (fake_chip_memory *)context;
    memory->permit_count++;
}

static redmcsb_f1066_chip_memory_io fake_io(void)
{
    redmcsb_f1066_chip_memory_io io;

    io.forbid = fake_forbid;
    io.available_chip_memory_byte_count = fake_available;
    io.allocate_chip_memory = fake_allocate;
    io.free_chip_memory = fake_free;
    io.alert_csb_system_error = fake_alert;
    io.permit = fake_permit;
    return io;
}

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_platform_boundaries_are_noop(void)
{
    int sentinel = 0x1065;

    redmcsb_f1065_set_exec_base_pc34_compat();
    F1065_SetExecBase();
    redmcsb_f1067_init_amiga_stuff_pc34_compat();
    F1067_InitAmigaStuff();
    redmcsb_f1068_free_amiga_stuff_pc34_compat();
    F1068_FreeAmigaStuff();

    CHECK(sentinel == 0x1065);
    check_contains(redmcsb_f1065_set_exec_base_source_evidence_pc34(),
                   "AMIGALIB.C:838");
    check_contains(redmcsb_f1065_set_exec_base_source_evidence_pc34(),
                   "F1065_SetExecBase");
    check_contains(redmcsb_f1067_init_amiga_stuff_source_evidence_pc34(),
                   "AMIGA.H:406");
    check_contains(redmcsb_f1067_init_amiga_stuff_source_evidence_pc34(),
                   "F1067_InitAmigaStuff");
    check_contains(redmcsb_f1068_free_amiga_stuff_source_evidence_pc34(),
                   "AMIGINIT.C:630");
    check_contains(redmcsb_f1068_free_amiga_stuff_source_evidence_pc34(),
                   "F1068_FreeAmigaStuff");
}

static void test_f1066_usable_chip_memory_success(void)
{
    redmcsb_f1066_chip_memory_io io = fake_io();
    fake_chip_memory memory;
    int32_t result;

    memset(&memory, 0, sizeof(memory));
    memory.total = 64 * 1024;
    memory.largest = 32 * 1024;
    memory.largest_after_allocation = 8 * 1024;

    result = F1066_GetUsableChipMemoryByteCount(&io, &memory);

    CHECK(result == 24 * 1024);
    CHECK(memory.forbid_count == 1);
    CHECK(memory.permit_count == 1);
    CHECK(memory.available_total_count == 1);
    CHECK(memory.available_largest_count == 2);
    CHECK(memory.allocate_count == 1);
    CHECK(memory.free_count == 1);
    CHECK(memory.alert_count == 0);
    CHECK(memory.allocated_byte_count == 0);
}

static void test_f1066_preserves_total_requirement(void)
{
    redmcsb_f1066_chip_memory_io io = fake_io();
    fake_chip_memory memory;
    int32_t result;

    memset(&memory, 0, sizeof(memory));
    memory.total = 24 * 1024;
    memory.largest = 20 * 1024;
    memory.largest_after_allocation = 8 * 1024;

    result = redmcsb_f1066_get_usable_chip_memory_byte_count(&io, &memory);

    CHECK(result == 8 * 1024);
    CHECK(memory.free_count == 1);
    CHECK(memory.alert_count == 0);
}

static void test_f1066_fails_closed_on_missing_memory(void)
{
    redmcsb_f1066_chip_memory_io io = fake_io();
    fake_chip_memory memory;
    int32_t result;

    memset(&memory, 0, sizeof(memory));
    memory.total = 16 * 1024;
    memory.largest = 16 * 1024;

    result = F1066_GetUsableChipMemoryByteCount(&io, &memory);

    CHECK(result == 0);
    CHECK(memory.forbid_count == 1);
    CHECK(memory.permit_count == 1);
    CHECK(memory.allocate_count == 0);
    CHECK(memory.free_count == 0);
    CHECK(memory.alert_count == 0);
}

static void test_f1066_alerts_when_second_largest_block_is_too_small(void)
{
    redmcsb_f1066_chip_memory_io io = fake_io();
    fake_chip_memory memory;
    int32_t result;

    memset(&memory, 0, sizeof(memory));
    memory.total = 64 * 1024;
    memory.largest = 32 * 1024;
    memory.largest_after_allocation = 4 * 1024;

    result = F1066_GetUsableChipMemoryByteCount(&io, &memory);

    CHECK(result == 0);
    CHECK(memory.free_count == 1);
    CHECK(memory.alert_count == 1);
    CHECK(memory.alert_code == 0);
    CHECK(memory.allocated_byte_count == 0);
}

static void test_f1066_rejects_incomplete_io(void)
{
    redmcsb_f1066_chip_memory_io io = fake_io();
    fake_chip_memory memory;

    memset(&memory, 0, sizeof(memory));
    io.free_chip_memory = NULL;

    CHECK(F1066_GetUsableChipMemoryByteCount(&io, &memory) == 0);
    CHECK(memory.forbid_count == 0);
    CHECK(memory.permit_count == 0);
}

static void test_f1066_evidence(void)
{
    const char *evidence =
        redmcsb_f1066_get_usable_chip_memory_byte_count_source_evidence();

    check_contains(evidence, "AMIGINIT.C:498");
    check_contains(evidence, "F1066_GetUsableChipMemoryByteCount");
    check_contains(evidence, "AMIGINIT.C:501-506");
    check_contains(evidence, "16K");
    check_contains(evidence, "8K");
}

int main(void)
{
    test_platform_boundaries_are_noop();
    test_f1066_usable_chip_memory_success();
    test_f1066_preserves_total_requirement();
    test_f1066_fails_closed_on_missing_memory();
    test_f1066_alerts_when_second_largest_block_is_too_small();
    test_f1066_rejects_incomplete_io();
    test_f1066_evidence();
    return 0;
}
