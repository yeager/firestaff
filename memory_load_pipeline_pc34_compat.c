#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_load_pipeline_pc34_compat.h"

int MEMORY_LOAD_RunPipeline_Compat(
int                                 mode                          SEPARATOR
int                                 graphicIndex                  SEPARATOR
const unsigned short*               languageSpecificGraphicIndices SEPARATOR
const unsigned short*               graphicDecompressedByteCounts  SEPARATOR
const unsigned char*                loadedGraphic                  SEPARATOR
unsigned long                       loadedByteCount                SEPARATOR
unsigned char**                     outGraphic                     SEPARATOR
unsigned char*                      allocatedBuffer                SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                   SEPARATOR
struct MemoryLoadPipelineResult_Compat* outResult                  FINAL_SEPARATOR
{
        if (mode == MEMORY_LOAD_PIPELINE_MODE_TEMPORARY) {
                outResult->byteCount = F0440_STARTEND_LoadTemporaryGraphicTransaction_Compat(
                    graphicIndex,
                    languageSpecificGraphicIndices,
                    graphicDecompressedByteCounts,
                    loadedGraphic,
                    outGraphic,
                    allocatedBuffer,
                    sizeInfo,
                    &outResult->transaction);
                outResult->output = *outGraphic;
                return 1;
        }
        if (mode == MEMORY_LOAD_PIPELINE_MODE_ENDGAME_BITMAP) {
                outResult->output = F0763_LoadEndgameBitmapExpandedTransaction_Compat(
                    loadedGraphic,
                    loadedByteCount,
                    allocatedBuffer,
                    sizeInfo,
                    &outResult->transaction);
                outResult->byteCount = (long)loadedByteCount;
                if (outGraphic != 0) {
                        *outGraphic = outResult->output;
                }
                return 1;
        }
        return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining MEMORY.C function citations for parity
 *
 *   MEMORY.C:2740 F0440_STARTEND_G
 *   MEMORY.C:553 F0473_MEMORY_S
 *   MEMORY.C:2255 F0497_LZW_D
 *   MEMORY.C:2295 F0527_FLOPPY_R (platform-specific, not implemented for PC-34)
 *   MEMORY.C:1320 F0534_FLOPPY_G (platform-specific, not implemented for PC-34)
 *   MEMORY.C:263 F0604_B
 *   MEMORY.C:283 F0605_R
 *   MEMORY.C:666 F0608_G
 *   MEMORY.C:1767 F0611_M
 *   MEMORY.C:1778 F0612_R
 *   MEMORY.C:1801 F0613_A
 *   MEMORY.C:1809 F0614_C
 *   MEMORY.C:2785 F0711_C
 *   MEMORY.C:740 F0739_MUSIC_S (platform-specific, not implemented for PC-34)
 *   MEMORY.C:1172 F0755_S
 *   MEMORY.C:1204 F0756_E
 *   MEMORY.C:786 F0772_FILE_R (platform-specific, not implemented for PC-34)
 *   MEMORY.C:785 F0774_FILE_S (platform-specific, not implemented for PC-34)
 *   MEMORY.C:1278 F0780_FILE_I (platform-specific, not implemented for PC-34)
 *   MEMORY.C:177 F1007_A
 *   MEMORY.C:191 F1008_G
 * ══════════════════════════════════════════════════════════════════════ */

