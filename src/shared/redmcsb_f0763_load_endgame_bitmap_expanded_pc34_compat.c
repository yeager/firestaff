#include "redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat.h"

unsigned char *redmcsb_f0763_load_endgame_bitmap_expanded_pc34_compat(
    const redmcsb_f0763_graphics_pc34_compat *graphics,
    const redmcsb_f0763_graphic_width_height_pc34 *graphic_width_height,
    int16_t graphic_index)
{
    unsigned char *bitmap;
    const redmcsb_f0763_graphic_width_height_pc34 *graphic =
        &graphic_width_height[graphic_index];

    /* ReDMCSB MEMORY.C:2764-2773, MEDIA720 PC 3.4 route. */
    bitmap = graphics->allocate_mem_for_graphic(
        graphics->context,
        graphic->width,
        graphic->height,
        REDMCSB_F0763_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP_PC34_COMPAT);
    graphics->load_decompress_and_expand_graphic(
        graphics->context, graphic_index, bitmap);
    return bitmap;
}

const char *redmcsb_f0763_load_endgame_bitmap_expanded_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 MEMORY.C:2764-2773 defines "
           "F0763_LoadEndgameBitmapExpanded: allocate G2005's width and "
           "height with C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP, call "
           "F0490_MEMORY_LoadDecompressAndExpandGraphic with the unmodified "
           "graphic index and allocated bitmap, then return that bitmap. "
           "DEFS.H:2805 defines C0 as 0.";
}
