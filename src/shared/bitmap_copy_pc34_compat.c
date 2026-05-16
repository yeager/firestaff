#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "bitmap_copy_pc34_compat.h"

static unsigned short bitmap_copy_read_u16_le(const unsigned char* p) {
        return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static unsigned long bitmap_copy_byte_count(const unsigned char* bitmap) {
        unsigned short width;
        unsigned short height;
        unsigned short bytesPerRow;


        width = bitmap_copy_read_u16_le(bitmap - 4);
        height = bitmap_copy_read_u16_le(bitmap - 2);
        bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
        return (unsigned long)bytesPerRow * (unsigned long)height;
}

void F0615_CopyBitmapDimensions_Compat(
const unsigned char* sourceBitmap      SEPARATOR
unsigned char*       destinationBitmap FINAL_SEPARATOR
{
        memcpy(destinationBitmap - 4, sourceBitmap - 4, 4);
}

void F0616_CopyBitmap_Compat(
const unsigned char* sourceBitmap      SEPARATOR
unsigned char*       destinationBitmap FINAL_SEPARATOR
{
        memcpy(destinationBitmap - 4, sourceBitmap - 4, bitmap_copy_byte_count(sourceBitmap) + 4);
}
