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

#ifdef __cplusplus
}
#endif

#endif
