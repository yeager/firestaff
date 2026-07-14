#include "redmcsb_f0940_copy_copper_instructions_pc34_compat.h"

void redmcsb_f0940_copy_copper_instructions_pc34_compat(
    redmcsb_f0940_view_port_pc34_compat *view_port,
    long instruction_count,
    redmcsb_f0940_copy_mem_pc34_compat copy_mem,
    void *context)
{
    redmcsb_f0940_copper_list_pc34_compat *display_instructions;
    int16_t remaining_count;
    uint8_t *destination;
    const uint8_t *source;

    display_instructions = view_port->display_instructions;
    destination = display_instructions->copper_instructions +
                  REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT;
    source = destination +
             (instruction_count * REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT);
    remaining_count = (int16_t)(display_instructions->max_count -
                                instruction_count);
    display_instructions->max_count = remaining_count;
    display_instructions->count = remaining_count;
    copy_mem(context, source, destination,
             (long)(remaining_count - 1) *
                 REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT);
}

const char *redmcsb_f0940_copy_copper_instructions_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:333-348 defines F0940_CopyCopperInstructions "
           "inside MEDIA442_A20E_A21E: DspIns->CopIns + 1 is the "
           "destination, source is destination plus P0996_l_ CopIns, then "
           "MaxCount and Count become MaxCount - P0996_l_ before "
           "CopyMem(source, destination, (remainingCount - 1) * "
           "sizeof(struct CopIns)). Toolchains/Commodore Amiga/Base/"
           "HARDDISK/AztecC/include/graphics/copper.h:11-30 defines "
           "CopIns as a six-byte 68000 structure. No PC 3.4 route is "
           "supplied; CopyMem is an explicit host boundary.";
}
