#ifndef FIRESTAFF_DUNGEON_DECOMPRESSOR_FTL_H
#define FIRESTAFF_DUNGEON_DECOMPRESSOR_FTL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decompress FTL-compressed dungeon data.
 * Source: ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon.
 *
 * compressed: raw compressed data (starts with 20-byte frequency table)
 * compressedSize: size in bytes
 * output: pre-allocated buffer of decompressedSize bytes
 * decompressedSize: expected decompressed byte count (from COMPRESSED_DUNGEON_HEADER)
 *
 * Returns 1 on success, 0 on failure. */
int ftl_decompress_dungeon(const unsigned char* compressed, size_t compressedSize,
                           unsigned char* output, long decompressedSize);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DUNGEON_DECOMPRESSOR_FTL_H */
