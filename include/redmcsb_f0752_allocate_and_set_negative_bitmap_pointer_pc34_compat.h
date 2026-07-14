/*
 * ReDMCSB STARTUP2.C F0752_AllocateAndSetNegativeBitmapPointer,
 * PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0752_ALLOCATE_AND_SET_NEGATIVE_BITMAP_POINTER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0752_ALLOCATE_AND_SET_NEGATIVE_BITMAP_POINTER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*redmcsb_f0752_allocate_mem_for_graphic_pc34_compat)(
    void *context,
    int16_t width,
    int16_t height,
    int16_t allocation_type);

typedef void (*redmcsb_f0752_set_negative_bitmap_pointer_pc34_compat)(
    void *context,
    int16_t negative_bitmap_index,
    void *bitmap);

typedef struct {
    redmcsb_f0752_allocate_mem_for_graphic_pc34_compat allocate_mem_for_graphic;
    redmcsb_f0752_set_negative_bitmap_pointer_pc34_compat
        set_negative_bitmap_pointer;
    void *context;
} redmcsb_f0752_graphics_pc34_compat;

/* C1_ALLOCATION_PERMANENT in STARTUP2.C. */
#define REDMCSB_F0752_ALLOCATION_PERMANENT_PC34_COMPAT INT16_C(1)

/*
 * Executes the PC 3.4 branch of F0752. Width and height are the selected
 * G2005_GraphicWidthHeight[graphic_index] entry, supplied by the host's
 * genuine graphic table. The source does not suppress the setter on a NULL
 * allocation result, so neither does this adapter.
 */
void redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat(
    const redmcsb_f0752_graphics_pc34_compat *graphics,
    int16_t negative_bitmap_index,
    int16_t width,
    int16_t height);

const char *redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
