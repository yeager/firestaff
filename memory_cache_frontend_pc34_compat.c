#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_frontend_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned char* F0489_MEMORY_BuildNativeBitmapInPlace_Compat(
const unsigned char*                    graphic  SEPARATOR
unsigned char*                          bitmap   SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo FINAL_SEPARATOR
{
        F0488_MEMORY_ExpandGraphicToBitmap_Compat(graphic, bitmap, sizeInfo);
        return bitmap;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Heap free operations (MEMORY.C:399-453)
 *
 * F0469_MEMORY_FreeAtHeapTop (MEMORY.C:399-411):
 *   - Frees temporary allocation: increments G0462 by byte count
 *   - No coalescing; strictly LIFO for temporary allocations
 *
 * F0470_MEMORY_FreeAtHeapBottom (MEMORY.C:413-425):
 *   - PC-34/I34E variant: releases permanent bottom allocation
 *   - Decrements G0461_puc_HeapEnd
 *
 * F0606_AllocateMemForGraphic (MEMORY.C:427-445):
 *   - Wrapper: allocates bitmap + copies dimensions header
 *
 * F0607_FreeMemForGraphic (MEMORY.C:447-452):
 *   - Frees bitmap from heap top, accounting for dimension header
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_free_source_evidence(void)
{
    return
        "MEMORY.C:399-411 F0469_MEMORY_FreeAtHeapTop LIFO temp free\n"
        "MEMORY.C:413-425 F0470_MEMORY_FreeAtHeapBottom permanent free\n"
        "MEMORY.C:427-445 F0606_AllocateMemForGraphic bitmap+header\n"
        "MEMORY.C:447-452 F0607_FreeMemForGraphic bitmap free\n";
}

