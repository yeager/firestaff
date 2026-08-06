#ifndef NEXUS_V1_FONT256_S2D_ADMISSION_H
#define NEXUS_V1_FONT256_S2D_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_saturn_font.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_FONT256_S2D_BYTES 25012U
#define NEXUS_V1_FONT256_S2D_SHA256 \
    "b820d606b4de4fbaa21d4e32f1df56b4cce6898939fb04f73cb6f55f4ebd13af"
#define NEXUS_V1_FONT256_S2D_SHA256_ENGLISH \
    "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb"

/* An upstream direct-file discovery must verify this SHA-256 before calling
 * the byte admission. The admission itself rechecks all source-backed SCR
 * framing and binds the supplied bytes by FNV; it does not implement SHA-256
 * a second time or retain the asset payload. */
typedef struct {
    int sha256_verified;
    const char *sha256_hex;
    uint64_t source_fnv1a64;
} Nexus_V1_Font256S2DSourceIdentity;

typedef struct {
    int valid;
    int source_identity_bound;
    int scr_header_bound;
    int section_table_bound;
    int glyph_layout_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint32_t source_bytes;
    uint64_t source_fnv1a64;
    uint32_t char_count;
    uint32_t header_descriptor;
    uint32_t section_count;
    uint64_t section_table_fnv1a64;
    Nexus_V1_FontSection sections[4];
    uint64_t section_fnv1a64[4];
} Nexus_V1_Font256S2DAdmissionReceipt;

/* Admits only an externally SHA-256-attested retail FONT256.S2D SCR
 * envelope (canonical capture or the documented English Saturn revision).
 * The four admitted spans are source provenance, not glyph layout:
 * glyph encoding, character mapping, pixels, and drawing remain unavailable.
 * Returns 1 only for a fully matching receipt, otherwise 0. */
int nexus_v1_font256_s2d_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DSourceIdentity *identity,
    Nexus_V1_Font256S2DAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
