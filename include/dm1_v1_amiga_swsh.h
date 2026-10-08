#ifndef FIRESTAFF_DM1_V1_AMIGA_SWSH_H
#define FIRESTAFF_DM1_V1_AMIGA_SWSH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_AMIGA_SWSH_WIDTH = 320,
    DM1_V1_AMIGA_SWSH_HEIGHT = 200,
    DM1_V1_AMIGA_SWSH_PLANAR_BYTES = 32000,
    DM1_V1_AMIGA_SWSH_PALETTE_PAIRS = 27,
    DM1_V1_AMIGA_SWSH_PALETTE_BYTES = 108,
    DM1_V1_AMIGA_SWSH_SOUND_BYTES = 9078,
    DM1_V1_AMIGA_SWSH_PAULA_PERIOD = 334
};

typedef struct {
    const uint8_t* planarLogo;
    const uint8_t* palettePairs;
    const uint8_t* sound;
    size_t soundBytes;
    const char* executableMd5;
} DM1_V1_AmigaSwshAssets;

/* Validate an authenticated ReDMCSB Amiga A20 SWSH executable and expose
 * its source-owned logo, palette sequence, and PCM as views into `bytes`.
 * The caller keeps `bytes` alive for as long as these views are used. */
int dm1_v1_amiga_swsh_parse(const uint8_t* bytes,
                            size_t byteCount,
                            DM1_V1_AmigaSwshAssets* outAssets);

/* Expand the four original 320x200 Amiga bitplanes to 8-bit color indexes. */
int dm1_v1_amiga_swsh_decode_logo(const DM1_V1_AmigaSwshAssets* assets,
                                  uint8_t* outIndexed,
                                  size_t outCapacity);

/* Read one source row. A palette index of -1 denotes a VBlank wait; all
 * other valid rows set one of the 16 Amiga color registers. */
int dm1_v1_amiga_swsh_palette_event(const DM1_V1_AmigaSwshAssets* assets,
                                     unsigned int eventIndex,
                                     int* outPaletteIndex,
                                     unsigned int* outValue);

#ifdef __cplusplus
}
#endif

#endif
