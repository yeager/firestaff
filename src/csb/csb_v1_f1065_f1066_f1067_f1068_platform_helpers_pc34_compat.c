#include "csb_v1_f1065_f1066_f1067_f1068_platform_helpers_pc34_compat.h"
#include "redmcsb_f1065_set_exec_base_pc34_compat.h"
#include "redmcsb_f1067_init_amiga_stuff_pc34_compat.h"
#include "redmcsb_f1068_free_amiga_stuff_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

enum {
    REDMCSB_F1066_CHIP_MEMORY_REQUIREMENT = 16 * 1024,
    REDMCSB_F1066_LARGEST_BLOCK_REQUIREMENT = 8 * 1024
};

void redmcsb_f1065_set_exec_base_pc34_compat(void)
{
}

void F1065_SetExecBase(void)
{
    redmcsb_f1065_set_exec_base_pc34_compat();
}

const char *redmcsb_f1065_set_exec_base_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGALIB.C:838 F1065_SetExecBase; Amiga ExecBase "
           "address/register boundary, no PC34 portable host behavior";
}

int32_t redmcsb_f1066_get_usable_chip_memory_byte_count(
    const redmcsb_f1066_chip_memory_io *io,
    void *context)
{
    int32_t available_chip_memory;
    int32_t largest_chip_memory_block;
    int32_t byte_count;
    void *chip_memory_largest_block_bottom;
    int32_t second_largest_chip_memory_block;

    if (io == NULL ||
        io->available_chip_memory_byte_count == NULL ||
        io->allocate_chip_memory == NULL ||
        io->free_chip_memory == NULL) {
        return 0;
    }

    if (io->forbid != NULL) {
        io->forbid(context);
    }

    available_chip_memory =
        io->available_chip_memory_byte_count(context, 0);
    largest_chip_memory_block =
        io->available_chip_memory_byte_count(context, 1);

    if (available_chip_memory <= REDMCSB_F1066_CHIP_MEMORY_REQUIREMENT ||
        largest_chip_memory_block <= REDMCSB_F1066_LARGEST_BLOCK_REQUIREMENT) {
        if (io->permit != NULL) {
            io->permit(context);
        }
        return 0;
    }

    byte_count =
        largest_chip_memory_block - REDMCSB_F1066_LARGEST_BLOCK_REQUIREMENT;
    if (available_chip_memory - byte_count <
        REDMCSB_F1066_CHIP_MEMORY_REQUIREMENT) {
        byte_count =
            available_chip_memory - REDMCSB_F1066_CHIP_MEMORY_REQUIREMENT;
    }
    if (byte_count <= 0) {
        if (io->permit != NULL) {
            io->permit(context);
        }
        return 0;
    }

    chip_memory_largest_block_bottom =
        io->allocate_chip_memory(context, byte_count);
    if (chip_memory_largest_block_bottom == NULL) {
        if (io->permit != NULL) {
            io->permit(context);
        }
        return 0;
    }

    second_largest_chip_memory_block =
        io->available_chip_memory_byte_count(context, 1);
    io->free_chip_memory(context, chip_memory_largest_block_bottom,
                         byte_count);
    if (io->permit != NULL) {
        io->permit(context);
    }

    if (second_largest_chip_memory_block <
        REDMCSB_F1066_LARGEST_BLOCK_REQUIREMENT) {
        if (io->alert_csb_system_error != NULL) {
            io->alert_csb_system_error(context, 0);
        }
        return 0;
    }

    return byte_count;
}

int32_t F1066_GetUsableChipMemoryByteCount(
    const redmcsb_f1066_chip_memory_io *io,
    void *context)
{
    return redmcsb_f1066_get_usable_chip_memory_byte_count(io, context);
}

const char *redmcsb_f1066_get_usable_chip_memory_byte_count_source_evidence(
    void)
{
    return "ReDMCSB AMIGINIT.C:498 F1066_GetUsableChipMemoryByteCount; "
           "locals AMIGINIT.C:501-506 track largest block, 16K chip-memory "
           "requirement, 8K largest-block requirement, second-largest block, "
           "and available memory outside the largest block";
}

void redmcsb_f1067_init_amiga_stuff_pc34_compat(void)
{
}

void F1067_InitAmigaStuff(void)
{
    redmcsb_f1067_init_amiga_stuff_pc34_compat();
}

const char *redmcsb_f1067_init_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGA.H:406 F1067_InitAmigaStuff / AMIGINIT.C "
           "Amiga-only OS resource initialization boundary; no PC34 portable "
           "host behavior";
}

void redmcsb_f1068_free_amiga_stuff_pc34_compat(void)
{
}

void F1068_FreeAmigaStuff(void)
{
    redmcsb_f1068_free_amiga_stuff_pc34_compat();
}

const char *redmcsb_f1068_free_amiga_stuff_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:630 F1068_FreeAmigaStuff; Amiga-only OS "
           "resource release boundary, no PC34 portable host behavior";
}
