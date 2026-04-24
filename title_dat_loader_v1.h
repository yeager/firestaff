#ifndef FIRESTAFF_TITLE_DAT_LOADER_V1_H
#define FIRESTAFF_TITLE_DAT_LOADER_V1_H

#include <stddef.h>
#include <stdint.h>

#define V1_TITLE_DAT_ITEM_COUNT 59u
#define V1_TITLE_DAT_FRAME_MAX  53u

typedef enum V1_TitleItemType {
    V1_TITLE_ITEM_AN = 0,
    V1_TITLE_ITEM_BR,
    V1_TITLE_ITEM_P8,
    V1_TITLE_ITEM_PL,
    V1_TITLE_ITEM_EN,
    V1_TITLE_ITEM_DL,
    V1_TITLE_ITEM_DO,
    V1_TITLE_ITEM_UNKNOWN
} V1_TitleItemType;

typedef struct V1_TitleRecord {
    unsigned int index;
    V1_TitleItemType type;
    char tag[3];
    unsigned long fileOffset;
    unsigned int declaredBytes;
    unsigned int payloadBytes;
    unsigned int width;
    unsigned int height;
    unsigned int paletteOrdinal;
    unsigned int frameOrdinal;
} V1_TitleRecord;

typedef struct V1_TitleManifest {
    unsigned long fileBytes;
    unsigned int itemCount;
    unsigned int animationCount;
    unsigned int breakCount;
    unsigned int egaPaletteCount;
    unsigned int paletteCount;
    unsigned int encodedImageCount;
    unsigned int deltaLayerCount;
    unsigned int doneCount;
    unsigned int frameCount;
    V1_TitleRecord records[V1_TITLE_DAT_ITEM_COUNT];
} V1_TitleManifest;

typedef struct V1_TitlePlayer {
    const V1_TitleManifest* manifest;
    unsigned int cursor;
    unsigned int paletteOrdinal;
    unsigned int frameOrdinal;
    int done;
} V1_TitlePlayer;

typedef struct V1_TitlePalette {
    uint8_t rgba[16][4];
} V1_TitlePalette;

typedef struct V1_TitleRenderFrame {
    const V1_TitleRecord* record;
    unsigned int frameOrdinal;
    unsigned int paletteOrdinal;
    unsigned int durationFrames;
    unsigned int width;
    unsigned int height;
    const uint8_t* colorIndices;
    const V1_TitlePalette* palette;
} V1_TitleRenderFrame;

typedef int (*V1_TitleFrameCallback)(const V1_TitleRenderFrame* frame,
                                     void* userData,
                                     char* errMsg,
                                     size_t errMsgBytes);

const char* V1_Title_TypeLabel(V1_TitleItemType type);

int V1_Title_ParseManifest(const char* titleDatPath,
                           V1_TitleManifest* outManifest,
                           char* errMsg,
                           size_t errMsgBytes);

void V1_TitlePlayer_Init(V1_TitlePlayer* player,
                         const V1_TitleManifest* manifest);

const V1_TitleRecord* V1_TitlePlayer_NextFrame(V1_TitlePlayer* player);

int V1_Title_RenderFrames(const char* titleDatPath,
                          V1_TitleFrameCallback callback,
                          void* userData,
                          char* errMsg,
                          size_t errMsgBytes);

#endif
