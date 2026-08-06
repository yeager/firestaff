#ifndef CSB_V1_AMIGA_TITL_DAT_H
#define CSB_V1_AMIGA_TITL_DAT_H

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* CSB Amiga TITL.DAT title-animation schedule.
 *
 * TITL.DAT is an ANIM stream, not a GRAPHICS.DAT item.  The verified Amiga
 * 3.1 EN/FR/GE catalogue has one 320x200 4-bit encoded image followed by
 * 31 delta layers.  Their display durations are vertical blanks and must be
 * retained exactly; a renderer may not replace this with a generic timer.
 *
 * Source references:
 *   Greatstone, CSB Amiga 3.1 EN/FR/GE TITL.DAT catalogue:
 *   http://greatstone.free.fr/dm/db_data/csb_amiga_31_enfrge_originala/titl.dat/titl.dat.html
 *   ReDMCSB APPA.C:51-53 loads SWSH then passes FTL_TITL to ANIM.
 */

#define CSB_V1_AMIGA_TITL_WIDTH                 320u
#define CSB_V1_AMIGA_TITL_HEIGHT                200u
#define CSB_V1_AMIGA_TITL_BIT_DEPTH               4u
#define CSB_V1_AMIGA_TITL_DELTA_COUNT            31u
#define CSB_V1_AMIGA_TITL_FRAME_COUNT            32u

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t bit_depth;
    uint16_t initial_duration_vbl;
    uint16_t delta_durations_vbl[CSB_V1_AMIGA_TITL_DELTA_COUNT];
    uint16_t delta_count;
    uint32_t total_duration_vbl;
} CSB_V1_AmigaTitlSchedule;

/* ReDMCSB ANIM.C F1181 PL_DATA: one 16-bit colour count followed by indexed
 * 8-bit RGB components. Values are the original four-bit Amiga components. */
typedef struct {
    uint8_t rgb4[16][3];
    uint16_t color_count;
} CSB_V1_AmigaTitlPalette;

/* Decode the strict AN/PL/EN/DL.../DO record envelope and its real VBL
 * schedule.  Image and delta payloads stay opaque here; their decompression
 * belongs to the renderer once the IMGA delta operation is source-locked. */
int csb_v1_amiga_titl_dat_decode(const uint8_t *data, size_t size,
                                 CSB_V1_AmigaTitlSchedule *out);

/* Decode TITL.DAT's real PL step. The caller receives only the encoded
 * source components; applying them to a display remains the renderer's job. */
int csb_v1_amiga_titl_dat_decode_palette(const uint8_t *data, size_t size,
                                         CSB_V1_AmigaTitlPalette *out);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_AMIGA_TITL_DAT_H */
