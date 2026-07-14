#include "redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat.h"

void redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_pc34_compat(
    const redmcsb_f0752_graphics_pc34_compat *graphics,
    int16_t negative_bitmap_index,
    int16_t width,
    int16_t height)
{
    void *bitmap;

    /* STARTUP2.C:477-490, MEDIA720_I34E_I34M PC 3.4 branch. */
    bitmap = graphics->allocate_mem_for_graphic(
        graphics->context,
        width,
        height,
        REDMCSB_F0752_ALLOCATION_PERMANENT_PC34_COMPAT);
    graphics->set_negative_bitmap_pointer(
        graphics->context, negative_bitmap_index, bitmap);
}

const char *redmcsb_f0752_allocate_and_set_negative_bitmap_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C:477-490 (MEDIA720_I34E_I34M) calls "
           "F0606_AllocateMemForGraphic with "
           "G2005_GraphicWidthHeight[graphic_index].Width, .Height, and "
           "C1_ALLOCATION_PERMANENT, then unconditionally passes that "
           "result to F0632_COORD_SetNegativeBitmapPointer. COORD.C:2005-"
           "2017 owns negative-index and entry-type guarding; MEMORY.C:427-"
           "446 owns the native bitmap allocation and dimensions.";
}
