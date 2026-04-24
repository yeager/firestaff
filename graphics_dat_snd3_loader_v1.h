#ifndef FIRESTAFF_GRAPHICS_DAT_SND3_LOADER_V1_H
#define FIRESTAFF_GRAPHICS_DAT_SND3_LOADER_V1_H

/*
 * graphics_dat_snd3_loader_v1 — Pass 51 bounded V1 audio loader.
 *
 * Parses the DM PC v3.4 English GRAPHICS.DAT DMCSB2 header and decodes
 * only the 33 SND3 in-game SFX items.  SND3 is unsigned 8-bit mono PCM
 * at 6000 Hz with a big-endian sample-count word at the start of each
 * item and 0..3 trailing bytes of unknown use.
 *
 * This module deliberately does not wire SDL/runtime playback.  It is a
 * pure file/asset decoder so the runtime mapping/integration can be a
 * later, separately evidenced pass.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define V1_GRAPHICS_DAT_SIGNATURE        0x8001u
#define V1_GRAPHICS_DAT_ITEM_COUNT       713u
#define V1_GRAPHICS_DAT_HEADER_BYTES     5708u
#define V1_GRAPHICS_DAT_SND3_COUNT       33u
#define V1_GRAPHICS_DAT_SND3_SAMPLE_RATE_HZ 6000u

const unsigned short* V1_GraphicsSnd3_ItemIndices(unsigned int* outCount);
int V1_GraphicsSnd3_IsItemIndex(unsigned int itemIndex);
const char* V1_GraphicsSnd3_ItemLabel(unsigned int itemIndex);

/* Raw GRAPHICS.DAT header entry for a SND3 item. */
typedef struct {
    unsigned int   itemIndex;
    unsigned int   soundOrdinal;       /* 0..32, Greatstone Sound NN */
    unsigned long  fileOffset;
    unsigned int   itemBytes;
    unsigned int   decompressedBytes;
    unsigned short attr0;
    unsigned short attr1;
    unsigned int   declaredSampleCount;
    unsigned int   trailingBytes;      /* itemBytes - 2 - declaredSampleCount */
} V1_GraphicsSnd3Item;

typedef struct {
    unsigned short signature;
    unsigned short itemCount;
    unsigned long  headerBytes;
    unsigned long  fileBytes;
    V1_GraphicsSnd3Item items[V1_GRAPHICS_DAT_SND3_COUNT];
} V1_GraphicsSnd3Manifest;

typedef struct {
    unsigned int   itemIndex;
    unsigned int   soundOrdinal;
    unsigned int   declaredSampleCount;
    unsigned int   decodedSampleCount;
    unsigned char* samples;            /* malloc'd unsigned 8-bit PCM */
    unsigned int   sampleRateHz;
} V1_GraphicsSnd3Buffer;

int V1_GraphicsSnd3_ParseManifest(const char* graphicsDatPath,
                                  V1_GraphicsSnd3Manifest* outManifest,
                                  char* errMsg,
                                  size_t errMsgBytes);

int V1_GraphicsSnd3_DecodeItem(const char* graphicsDatPath,
                               const V1_GraphicsSnd3Manifest* manifest,
                               unsigned int itemIndex,
                               V1_GraphicsSnd3Buffer* outBuffer,
                               char* errMsg,
                               size_t errMsgBytes);

void V1_GraphicsSnd3_FreeBuffer(V1_GraphicsSnd3Buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_GRAPHICS_DAT_SND3_LOADER_V1_H */
