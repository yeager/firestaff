#ifndef FIRESTAFF_DM1_V1_SELECTOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SELECTOR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Extract the original I34E selector's embedded 224x136 dungeon preview
 * from its hash-authenticated SELECTOR executable. outPixels is allocated
 * with malloc and contains one VGA palette index per pixel. */
int DM1_V1_Selector_ExtractDungeonViewPc34(const char* selectorPath,
                                           unsigned char** outPixels,
                                           unsigned int* outPixelCount);

/* Bounded decoder used after a caller has authenticated its original-media
 * input. Raw input alone does not establish the I34E identity. */
int DM1_V1_Selector_DecodeDungeonViewPc34(const unsigned char* executable,
                                          unsigned int executableBytes,
                                          unsigned char** outPixels,
                                          unsigned int* outPixelCount);

/* Compose one original I34E SELECTOR frame from the authenticated source
 * GRAPHICS.DAT entries 4/2/3 and the embedded SELECTOR viewport. Pixels are
 * one-byte VGA indices. doorStep follows SELECTOR.C F8360: four pixels per
 * step; zero is the closed entrance. */
int DM1_V1_Selector_ComposeFramePc34(
    const unsigned char* entrance, unsigned int entranceWidth,
    unsigned int entranceHeight,
    const unsigned char* leftDoor, unsigned int leftDoorWidth,
    unsigned int leftDoorHeight,
    const unsigned char* rightDoor, unsigned int rightDoorWidth,
    unsigned int rightDoorHeight,
    const unsigned char* selectorView, unsigned int selectorViewWidth,
    unsigned int selectorViewHeight, unsigned int doorStep,
    unsigned char* outFrame, unsigned int outFrameBytes);

#ifdef __cplusplus
}
#endif

#endif
