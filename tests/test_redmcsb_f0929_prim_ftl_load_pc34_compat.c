#include "redmcsb_f0929_prim_ftl_load_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const uint8_t *file;
    size_t file_size;
    size_t position;
    unsigned int allocation_count;
    unsigned int free_count;
} fixture;

static int read_file(void *context, int32_t handle, void *buffer,
                     uint32_t byte_count)
{
    fixture *state = context;

    (void)handle;
    if (state->position + byte_count > state->file_size) {
        return 1;
    }
    (void)memcpy(buffer, state->file + state->position, byte_count);
    state->position += byte_count;
    return 0;
}

static int seek_file(void *context, int32_t handle, uint32_t offset)
{
    fixture *state = context;

    (void)handle;
    if (offset > state->file_size) {
        return 1;
    }
    state->position = offset;
    return 0;
}

static void *allocate_memory(void *context, uint32_t byte_count)
{
    fixture *state = context;

    state->allocation_count++;
    return malloc(byte_count == 0u ? 1u : byte_count);
}

static void free_memory(void *context, void *memory)
{
    fixture *state = context;

    state->free_count++;
    free(memory);
}

int main(void)
{
    uint8_t bad_magic[20] = { 0 };
    uint8_t excessive_segments[20] = { 0 };
    fixture state = { bad_magic, sizeof(bad_magic), 0u, 0u, 0u };
    redmcsb_f0929_ftl_executable_pc34_compat executable = { NULL, NULL, NULL, 0u };
    (void)executable;
    redmcsb_f0929_callbacks_pc34_compat callbacks = {
        &state, read_file, seek_file, allocate_memory, free_memory, NULL
    };
    (void)callbacks;

    assert(redmcsb_f0929_prim_ftl_load_pc34_compat(7, &executable,
                                                    &callbacks) == 1u);
    assert(executable.memory_address == NULL);
    assert(state.allocation_count == 0u);

    excessive_segments[0] = 0x60u;
    excessive_segments[1] = 0x61u;
    excessive_segments[4] = 2u;
    excessive_segments[6] = 1u;
    excessive_segments[18] = 101u;
    state.file = excessive_segments;
    state.position = 0u;
    assert(redmcsb_f0929_prim_ftl_load_pc34_compat(7, &executable,
                                                    &callbacks) == 26u);
    assert(strstr(redmcsb_f0929_prim_ftl_load_source_evidence_pc34(),
                  "PRIM2B.C:267-544") != NULL);
    puts("ok: ReDMCSB F0929 source FTL loader validation gates");
    return 0;
}
