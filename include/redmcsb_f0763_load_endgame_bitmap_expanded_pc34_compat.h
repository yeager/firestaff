/*
 * ReDMCSB MEMORY.C F0763_LoadEndgameBitmapExpanded, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0763_LOAD_ENDGAME_BITMAP_EXPANDED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0763_LOAD_ENDGAME_BITMAP_EXPANDED_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redmcsb_f0763_graphic_width_height_pc34 {
    int16_t width;
    int16_t height;
} redmcsb_f0763_graphic_width_height_pc34;

typedef unsigned char *(*redmcsb_f0763_allocate_mem_for_graphic_pc34_compat)(
    void *context,
    int16_t width,
    int16_t height,
    int16_t allocation_type);

typedef void (*redmcsb_f0763_load_decompress_and_expand_graphic_pc34_compat)(
    void *context,
    int16_t graphic_index,
    unsigned char *bitmap);

typedef struct redmcsb_f0763_graphics_pc34_compat {
    redmcsb_f0763_allocate_mem_for_graphic_pc34_compat allocate_mem_for_graphic;
    redmcsb_f0763_load_decompress_and_expand_graphic_pc34_compat
        load_decompress_and_expand_graphic;
    void *context;
} redmcsb_f0763_graphics_pc34_compat;

/* C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP in ReDMCSB DEFS.H:2805. */
#define REDMCSB_F0763_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP_PC34_COMPAT INT16_C(0)

/*
 * Executes the PC 3.4 F0763 route. As in MEMORY.C, the caller supplies a
 * valid graphic table and index; allocation failure is still passed to the
 * loader and no synthetic bitmap bytes are created here.
 */
unsigned char *redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat(
    const redmcsb_f0763_graphics_pc34_compat *graphics,
    const redmcsb_f0763_graphic_width_height_pc34 *graphic_width_height,
    int16_t graphic_index);

const char *redmcsb_f0763_load_endgame_bitmap_expanded_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
