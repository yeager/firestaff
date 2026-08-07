#ifndef DM1_V1_FMTOWNS_ICON_GEOMETRY_H
#define DM1_V1_FMTOWNS_ICON_GEOMETRY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 icon geometry constants.
 *
 * Every value here is a byte-verified word read from the initialised
 * data segment of the shipping HMA-240 English `EDM.EXP` (Phar Lap
 * P3 load offset 0x200). Recovered 2026-08-07 by directly reading
 * the file bytes at the vaddrs recovered in
 * parity-evidence/dm1_fmtowns_menu_p3_disassembly.md, then
 * cross-referenced against DRAW_ICN_BUTTON (0x44f0) and
 * LOAD_ICON (0xbb74) which consume them at runtime:
 *
 *   DRAW_ICN_BUTTON @ 0x44f0:
 *     movsx eax, word ptr [0x26c76]      ; ICON_SIZE = malloc arg
 *     push  eax
 *     call  MALLOC_ICON_BUFFER
 *     ...
 *     mov   ax, word ptr [0x26c7a]       ; ICON_Y_SIZE
 *     mov   word ptr [esi - 2], ax       ; buffer header y
 *     mov   ax, word ptr [0x26c78]       ; ICON_X_SIZE
 *     mov   word ptr [esi - 4], ax       ; buffer header x
 *
 *   LOAD_ICON @ 0xbb74:
 *     mov   ax, word ptr [0x26c78]       ; ICON_X_SIZE
 *     mov   word ptr [esi - 4], ax
 *     mov   ax, word ptr [0x26c7a]       ; ICON_Y_SIZE
 *     mov   word ptr [esi - 2], ax
 *
 * ICON_SIZE is the raster storage in bytes: for the shipping DM1 FM
 * Towns build, this equals ICON_X_SIZE * ICON_Y_SIZE (16 * 16 = 256)
 * because each icon pixel is one 8bpp palette byte. Consumers must
 * treat the trio as a coherent record — a mismatch means the source
 * geometry drifted and downstream draws must fail closed.
 */

#define DM1_V1_FMTOWNS_ICON_SCR_X_SIZE  320   /* SCR_X_SIZE  @ 0x26c68 */
#define DM1_V1_FMTOWNS_ICON_SIZE_BYTES  256   /* ICON_SIZE   @ 0x26c76 */
#define DM1_V1_FMTOWNS_ICON_X_SIZE       16   /* ICON_X_SIZE @ 0x26c78 */
#define DM1_V1_FMTOWNS_ICON_Y_SIZE       16   /* ICON_Y_SIZE @ 0x26c7a */

/* Return non-zero if the three-way invariant holds:
 *   ICON_SIZE == ICON_X_SIZE * ICON_Y_SIZE.
 * The M11 icon consumer must gate icon draws on this check so any
 * future edit that breaks the invariant fails closed instead of
 * writing off the end of a smaller icon buffer. */
int dm1_v1_fmtowns_icon_geometry_is_self_consistent_pc34(void);

/* Fill *out_bytes with ICON_SIZE, *out_x with ICON_X_SIZE, *out_y
 * with ICON_Y_SIZE. Returns 1 on success (all pointers non-NULL and
 * invariant holds), 0 otherwise. */
int dm1_v1_fmtowns_icon_geometry_get_pc34(
    uint16_t *out_bytes, uint16_t *out_x, uint16_t *out_y);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_ICON_GEOMETRY_H */
