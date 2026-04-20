#ifndef REDMCSB_MEMORY_FRONTEND_PC34_COMPAT_H
#define REDMCSB_MEMORY_FRONTEND_PC34_COMPAT_H

struct GraphicWidthHeight_Compat {
    unsigned short Width;
    unsigned short Height;
};

void F0488_MEMORY_ExpandGraphicToBitmap_Compat(const unsigned char* graphic, unsigned char* bitmap, const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
