#ifndef FIRESTAFF_REDMCSB_F0940_COPY_COPPER_INSTRUCTIONS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0940_COPY_COPPER_INSTRUCTIONS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source layout from graphics/copper.h: an Amiga 68000 CopIns contains a
 * short opcode and a four-byte union, so CopyMem advances in six-byte
 * instruction units. The external CopyMem call remains a host boundary.
 */
#define REDMCSB_F0940_COPPER_INSTRUCTION_BYTE_COUNT 6L

typedef void (*redmcsb_f0940_copy_mem_pc34_compat)(
    void *context,
    const void *source,
    void *destination,
    long byte_count);

typedef struct {
    uint8_t *copper_instructions;
    int16_t count;
    int16_t max_count;
} redmcsb_f0940_copper_list_pc34_compat;

typedef struct {
    redmcsb_f0940_copper_list_pc34_compat *display_instructions;
} redmcsb_f0940_view_port_pc34_compat;

/*
 * ReDMCSB EXEC.C F0940_CopyCopperInstructions. This adapter exposes the
 * Amiga graphics.library CopyMem operation at the host boundary and assumes
 * source-valid ViewPort, DspIns, CopIns, and instruction_count values.
 */
void redmcsb_f0940_copy_copper_instructions_pc34_compat(
    redmcsb_f0940_view_port_pc34_compat *view_port,
    long instruction_count,
    redmcsb_f0940_copy_mem_pc34_compat copy_mem,
    void *context);

const char *redmcsb_f0940_copy_copper_instructions_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0940_COPY_COPPER_INSTRUCTIONS_PC34_COMPAT_H */
