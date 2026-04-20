#ifndef REDMCSB_HOST_VIDEO_PGM_BACKEND_PC34_COMPAT_H
#define REDMCSB_HOST_VIDEO_PGM_BACKEND_PC34_COMPAT_H

struct HostVideoPgmBackendResult_Compat {
    unsigned int frameNumber;
    unsigned short width;
    unsigned short height;
    unsigned long pixelCount;
    const char* outputPath;
};

int F9002_HOSTVIDEO_PublishFrameToPgm_Compat(
    const unsigned char* screenBitmap,
    unsigned int frameNumber,
    const char* outputPath,
    struct HostVideoPgmBackendResult_Compat* outResult);

#endif
