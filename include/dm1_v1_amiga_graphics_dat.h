#ifndef DM1_V1_AMIGA_GRAPHICS_DAT_H
#define DM1_V1_AMIGA_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM1 Amiga GRAPHICS.DAT classifier and admission gate.
 *
 * The Amiga releases use the legacy format (no 0x8001 new-format marker)
 * with big-endian byte order. The file has 575 graphics, all uncompressed
 * (comp == decomp), with width/height embedded at the start of each
 * graphic's data rather than in a header array.
 *
 * Known variants:
 *   6a2f135b53c2220f0251fa103e2a6e7e  DM Amiga 2.0 English      411960
 *   dd373954b3fb127db7387946131ea322  DM Amiga 2.0 French       411960
 *   0679e39da9dcc2e855cb33c6c64ddcb5  DM Amiga 2.0/2.2 German   411960
 *   b35931b55db649a1bd2d415b61b29801  DM Amiga 2.1/2.2 English  411960
 *   7f9458e4a3972d06e649a6fa85a7f34b  DM Amiga 3.6 Multilang    411960
 *   491ca939f9abb33ceeb26619b841fe91  DM Amiga Demo English
 *
 * Source: extracted from ADF disk images via AmigaOS OFS filesystem. */

#define DM1_AMIGA_GRAPHICS_EXPECTED_COUNT  575U
#define DM1_AMIGA_GRAPHICS_MIN_SIZE        395000U
#define DM1_AMIGA_GRAPHICS_MAX_SIZE        415000U

typedef enum {
    DM1_AMIGA_LANG_UNKNOWN = 0,
    DM1_AMIGA_LANG_EN      = 1,
    DM1_AMIGA_LANG_FR      = 2,
    DM1_AMIGA_LANG_DE      = 3,
    DM1_AMIGA_LANG_MULTI   = 4
} DM1_V1_AmigaLang;

typedef enum {
    DM1_AMIGA_VER_UNKNOWN = 0,
    DM1_AMIGA_VER_2_0     = 1,
    DM1_AMIGA_VER_2_1     = 2,
    DM1_AMIGA_VER_2_2     = 3,
    DM1_AMIGA_VER_3_6     = 4,
    DM1_AMIGA_VER_DEMO    = 5
} DM1_V1_AmigaVersion;

typedef struct {
    int                 is_amiga;
    uint16_t            graphic_count;
    uint32_t            file_size;
    DM1_V1_AmigaLang    lang;
    DM1_V1_AmigaVersion version;
    uint8_t             md5[16];
} DM1_V1_AmigaGraphicsReceipt;

int dm1_v1_amiga_graphics_probe(const uint8_t *data, size_t size);

int dm1_v1_amiga_graphics_receipt(const uint8_t *data, size_t size,
                                  DM1_V1_AmigaGraphicsReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_AMIGA_GRAPHICS_DAT_H */
