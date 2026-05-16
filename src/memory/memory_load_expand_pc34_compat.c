#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_load_expand_pc34_compat.h"
#include "expand_frontend_pc34_compat.h"

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
int                                 graphicIndexFlags SEPARATOR
const unsigned char*                loadedGraphic     SEPARATOR
unsigned long                       loadedByteCount   SEPARATOR
unsigned char*                      destinationBitmap SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo      FINAL_SEPARATOR
{
        (void)sizeInfo;
        if ((graphicIndexFlags & MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED) != 0) {
                memcpy(destinationBitmap, loadedGraphic, loadedByteCount);
                return;
        }
        if ((graphicIndexFlags & MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS) == 0) {
                memcpy(destinationBitmap - 4, loadedGraphic, 4);
                F0466_EXPAND_GraphicToBitmap_Compat(loadedGraphic, destinationBitmap);
                return;
        }
        F0466_EXPAND_GraphicToBitmap_Compat(loadedGraphic, destinationBitmap);
}
