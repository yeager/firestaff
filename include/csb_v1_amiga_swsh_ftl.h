#ifndef CSB_V1_AMIGA_SWSH_FTL_H
#define CSB_V1_AMIGA_SWSH_FTL_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_AMIGA_SWSH_WIDTH = 320,
    CSB_V1_AMIGA_SWSH_HEIGHT = 200,
    CSB_V1_AMIGA_SWSH_GRAPHIC_BYTES = 2310,
    CSB_V1_AMIGA_SWSH_PALETTE_EVENT_COUNT = 27,
    CSB_V1_AMIGA_SWSH_SAMPLE_BYTES = 9078
};

typedef struct {
    int16_t command;
    uint16_t parameter;
} CSB_V1_AmigaSwshPaletteEvent;

typedef struct {
    uint8_t logo_graphic[CSB_V1_AMIGA_SWSH_GRAPHIC_BYTES];
    CSB_V1_AmigaSwshPaletteEvent
        palette[CSB_V1_AMIGA_SWSH_PALETTE_EVENT_COUNT];
    uint8_t sample[CSB_V1_AMIGA_SWSH_SAMPLE_BYTES];
    uint16_t sample_period;
} CSB_V1_AmigaSwshFtlAssets;

/* Decode CSB's Amiga SWSH.FTL data hunk. Container checksums and full
 * fingerprints of the source palette, logo and sample must match before media
 * is exposed. The selected edition is authenticated by its boot profile. */
int csb_v1_amiga_swsh_ftl_decode(const uint8_t *bytes, size_t byte_count,
                                  CSB_V1_AmigaSwshFtlAssets *out_assets);

#endif
