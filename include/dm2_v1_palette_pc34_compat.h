#ifndef FIRESTAFF_DM2_V1_PALETTE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PALETTE_PC34_COMPAT_H

/*
 * dm2_v1_palette_pc34_compat.h — DM2 default palette and conversion.
 *
 * Source: skproject/SKWINSPX/src/v5/gfxpal.cpp
 *
 * DMPAL: the 256-entry, 6-bit-per-component default palette baked into the
 * DM2 executable.  Each entry is 3 bytes (R, G, B), values 0-63.
 *
 * DM2_CONVERT_DRIVERPALETTE converts an 8-bit ARGB palette (as stored in
 * GRAPHICS.DAT) to 6-bit DMPAL format by shifting each component >> 2.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_PAL_ENTRIES 256
#define DM2_V1_PAL_SIZE    (DM2_V1_PAL_ENTRIES * 3)

extern const int8_t dm2_v1_default_palette[DM2_V1_PAL_SIZE];

/*
 * Convert an 8-bit ARGB palette (4 bytes per entry: A, R, G, B) to 6-bit
 * RGB (3 bytes per entry: R, G, B) by shifting each component >> 2 and
 * skipping the alpha byte.
 *
 * Source: gfxpal.cpp DM2_CONVERT_DRIVERPALETTE
 *
 * src must point to 1024 bytes (256 * 4).
 * dst must point to 768 bytes (256 * 3).
 */
void dm2_v1_convert_driver_palette(const uint8_t *src, int8_t *dst);

/*
 * Expand 6-bit palette (3 bytes/entry, 0-63) to 8-bit (3 bytes/entry, 0-255).
 * Factor: 255.0 / 63.0 per component.
 *
 * Source: gfxpal.cpp driver_setcolors
 */
void dm2_v1_expand_palette_6to8(const int8_t *pal6, uint8_t *pal8);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PALETTE_PC34_COMPAT_H */
