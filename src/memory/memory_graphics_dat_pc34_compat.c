#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_pc34_compat.h"

static long graphics_dat_file_size(FILE* file) {
        long current;
        long size;


        current = ftell(file);
        if (current < 0) {
                return -1;
        }
        if (fseek(file, 0, SEEK_END) != 0) {
                return -1;
        }
        size = ftell(file);
        if (size < 0) {
                return -1;
        }
        if (fseek(file, current, SEEK_SET) != 0) {
                return -1;
        }
        return size;
}

int F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(
const char*                            path  SEPARATOR
struct MemoryGraphicsDatState_Compat*  state FINAL_SEPARATOR
{
        if (state->referenceCount++) {
                return 1;
        }
        state->file = fopen(path, "rb");
        if (state->file == 0) {
                state->referenceCount = 0;
                return 0;
        }
        state->fileSize = graphics_dat_file_size(state->file);
        state->cachedChunkIndex = -1;
        state->cacheContainsGraphicData = 0;
        return state->fileSize >= 0;
}

int F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(
struct MemoryGraphicsDatState_Compat* state FINAL_SEPARATOR
{
        if (state->referenceCount <= 0) {
                return 0;
        }
        if (--state->referenceCount) {
                return 1;
        }
        if (state->file != 0) {
                fclose(state->file);
                state->file = 0;
        }
        state->fileSize = 0;
        state->cachedChunkIndex = -1;
        state->cacheContainsGraphicData = 0;
        return 1;
}

int F0474_MEMORY_LoadGraphic_CPSDF_Compat(
long                               graphicOffset        SEPARATOR
int                                compressedByteCount  SEPARATOR
struct MemoryGraphicsDatState_Compat* state             SEPARATOR
unsigned char*                      destination         FINAL_SEPARATOR
{
        int remainingByteCount;
        long readFromOffset;
        long offset;
        int chunkIndex;
        int startOffsetInBuffer;
        int bufferByteCount;
        long freadByteCount;


        if (state->file == 0) {
                return 0;
        }
        offset = graphicOffset;
        remainingByteCount = compressedByteCount;
        chunkIndex = (int)(offset / 1024);
        readFromOffset = (long)chunkIndex << 10;
        while (remainingByteCount > 0) {
                if ((chunkIndex != state->cachedChunkIndex) || !state->cacheContainsGraphicData) {
                        state->cachedChunkIndex = chunkIndex;
                        freadByteCount = state->fileSize - readFromOffset;
                        if (freadByteCount > 1024) {
                                freadByteCount = 1024;
                        }
                        if (freadByteCount < 0) {
                                return 0;
                        }
                        if (fseek(state->file, readFromOffset, SEEK_SET) != 0) {
                                return 0;
                        }
                        if ((long)fread(state->chunkBuffer, 1, (size_t)freadByteCount, state->file) != freadByteCount) {
                                return 0;
                        }
                        state->cacheContainsGraphicData = 1;
                }
                startOffsetInBuffer = (int)(offset - readFromOffset);
                bufferByteCount = remainingByteCount;
                if (bufferByteCount > 1024 - startOffsetInBuffer) {
                        bufferByteCount = 1024 - startOffsetInBuffer;
                }
                memcpy(destination, state->chunkBuffer + startOffsetInBuffer, (size_t)bufferByteCount);
                remainingByteCount -= bufferByteCount;
                offset += bufferByteCount;
                destination += bufferByteCount;
                chunkIndex++;
                readFromOffset += 1024;
        }
        return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Graphics.dat I/O source-lock (MEMORY.C:1212-1486)
 *
 * F0477_MEMORY_OpenGraphicsDat_CPSDF (MEMORY.C:1212-1285):
 *   - Opens GRAPHICS.DAT file
 *   - Platform-specific: PC disk, Amiga disk, Atari ST disk
 *   - Reads and validates file header
 *
 * F0478_MEMORY_CloseGraphicsDat_CPSDF (MEMORY.C:1287-1311):
 *   - Closes GRAPHICS.DAT file handle
 *
 * F0479_MEMORY_ReadGraphicsDatHeader (MEMORY.C:1330-1486):
 *   - Reads graphic entry header from DAT file
 *   - Decodes dimensions, compression, offset table
 *   - G0467_MEMORY_GetGraphicOffset for file seek position
 *
 * F0535_MEMORY_GetGraphicsDatFileSize (MEMORY.C:1313-1328):
 *   - Returns file size for validation
 *
 * F0484_MEMORY_LoadGraphics_CPSDEF (MEMORY.C:1823-2306):
 *   - Bulk load + decompress + expand graphics
 *   - Main graphics loading pipeline for game startup
 *   - Decompresses RLE/LZ data from GRAPHICS.DAT entries
 *   - Expands pixel data to display format
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_gfxdat_source_evidence(void)
{
    return
        "MEMORY.C:1212-1285 F0477_MEMORY_OpenGraphicsDat_CPSDF\n"
        "MEMORY.C:1287-1311 F0478_MEMORY_CloseGraphicsDat_CPSDF\n"
        "MEMORY.C:1330-1486 F0479_MEMORY_ReadGraphicsDatHeader\n"
        "MEMORY.C:1313-1328 F0535_MEMORY_GetGraphicsDatFileSize\n"
        "MEMORY.C:1823-2306 F0484_MEMORY_LoadGraphics_CPSDEF bulk pipeline\n";
}

