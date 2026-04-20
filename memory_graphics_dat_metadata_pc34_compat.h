#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_METADATA_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_METADATA_PC34_COMPAT_H

long F0467_MEMORY_GetGraphicOffset_Compat(
    int graphicsDatFileFormat,
    unsigned int graphicCount,
    const unsigned short* compressedByteCounts,
    unsigned int graphicIndex);

#endif
