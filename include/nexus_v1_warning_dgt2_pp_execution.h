#ifndef NEXUS_V1_WARNING_DGT2_PP_EXECUTION_H
#define NEXUS_V1_WARNING_DGT2_PP_EXECUTION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_warning_dgt2_pp_payload_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Receives exactly one source-proven PP image. Palette words retain their
 * original big-endian BGR555 values; no host colour conversion is implied. */
typedef int (*Nexus_V1_WarningDgt2PpPresentFn)(
    void *context,
    const uint8_t *indexed_pixels,
    uint16_t width,
    uint16_t height,
    uint32_t stride,
    const uint16_t *bgr555_words,
    uint32_t bgr555_word_count);

typedef struct {
    int valid;
    int payload_admission_bound;
    int pp_256_indexed_proven;
    int bgr555_clut_proven;
    int stride_proven;
    int index_plane_copied;
    int presented;
    int fallback_permitted;
    uint64_t source_fnv1a64;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
    uint32_t bgr555_word_count;
    uint64_t index_plane_fnv1a64;
    uint64_t bgr555_words_fnv1a64;
    uint32_t trailing_raw_bytes;
} Nexus_V1_WarningDgt2PpExecutionReceipt;

/* Executes only the Sega ST-124 section-6 PP contract already admitted for
 * WARNING.BIN resource 0. The caller supplies exact-sized output buffers and
 * an explicit renderer; there is no default image, palette, or fallback. */
int nexus_v1_warning_dgt2_pp_execute(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt *payload_admission,
    uint8_t *indexed_pixels_out,
    size_t indexed_pixels_out_size,
    uint16_t bgr555_words_out[256],
    Nexus_V1_WarningDgt2PpPresentFn present,
    void *present_context,
    Nexus_V1_WarningDgt2PpExecutionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
