#ifndef NEXUS_V1_WARNING_DGT2_RESOURCE_CORPUS_H
#define NEXUS_V1_WARNING_DGT2_RESOURCE_CORPUS_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_warning_dgt2_source_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The canonical WARNING.BIN RES* directory carries four DGT2/PP resources,
 * not only resource zero. Every observed resource follows the same
 * ST-124-R1 section-6 contract already admitted for resource zero: a DGT2
 * record header, a six-byte PP header, a 256-word big-endian BGR555 CLUT, a
 * width*height one-byte-per-pixel index plane, and two trailing bytes before
 * the next descriptor. This module admits all four resources against the
 * live canonical source, binds their observed contiguous chain, and extends
 * the existing execution/M11 presentation route to each admitted resource.
 * It adds no colour conversion, subrecord grammar, trailing-byte meaning,
 * default image, or fallback presentation. */
#define NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT 4U
#define NEXUS_V1_WARNING_DGT2_CLUT_BYTES 512U
#define NEXUS_V1_WARNING_DGT2_TRAILING_BYTES 2U
#define NEXUS_V1_WARNING_DGT2_MAX_PIXELS (272U * 136U)
#define NEXUS_V1_WARNING_DGT2_M11_WIDTH 320U
#define NEXUS_V1_WARNING_DGT2_M11_HEIGHT 200U

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int descriptor_bound;
    int dgt2_header_bound;
    int pp_header_bound;
    int payload_bound;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t descriptor_index;
    uint32_t descriptor_id;
    uint32_t descriptor_offset;
    uint32_t resource_length;
    uint64_t resource_fnv1a64;
    uint32_t pp_header_offset;
    uint16_t pp_width;
    uint16_t pp_height;
    uint64_t pp_header_fnv1a64;
    uint32_t clut_offset;
    uint32_t clut_length;
    uint64_t clut_fnv1a64;
    uint32_t pixel_offset;
    uint32_t pixel_length;
    uint64_t pixel_fnv1a64;
    uint32_t trailing_offset;
    uint32_t trailing_length;
    uint64_t trailing_fnv1a64;
} Nexus_V1_WarningDgt2ResourceReceipt;

typedef struct {
    int valid;
    int all_resources_bound;
    int contiguous_chain_observed;
    int chain_covers_source_tail;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t resource_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t chain_fnv1a64;
    Nexus_V1_WarningDgt2ResourceReceipt
        resources[NEXUS_V1_WARNING_DGT2_RESOURCE_COUNT];
} Nexus_V1_WarningDgt2ResourceCorpusReceipt;

typedef struct {
    int valid;
    int resource_receipt_bound;
    int pp_256_indexed_proven;
    int bgr555_clut_proven;
    int stride_proven;
    int index_plane_copied;
    int presented;
    int fallback_permitted;
    uint64_t source_fnv1a64;
    uint32_t descriptor_index;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
    uint32_t bgr555_word_count;
    uint64_t index_plane_fnv1a64;
    uint64_t bgr555_words_fnv1a64;
    uint32_t trailing_raw_bytes;
} Nexus_V1_WarningDgt2ResourceExecutionReceipt;

typedef struct {
    int valid;
    int canonical_source_bound;
    int pp_execution_bound;
    int host_surface_written;
    int bgr555_to_rgb6_exact;
    int fallback_permitted;
    uint64_t source_fnv1a64;
    uint32_t descriptor_index;
    uint64_t index_plane_fnv1a64;
    uint64_t bgr555_words_fnv1a64;
    uint64_t host_palette_rgb6_fnv1a64;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
} Nexus_V1_WarningDgt2ResourceM11Receipt;

typedef int (*Nexus_V1_WarningDgt2ResourcePresentFn)(
    void *context,
    const uint8_t *indexed_pixels,
    uint16_t width,
    uint16_t height,
    uint32_t stride,
    const uint16_t *bgr555_words,
    uint32_t bgr555_word_count);

/* Admits one canonical DGT2/PP resource (index 0..3) against the live
 * SHA-256-attested or canonical-FNV-witnessed WARNING.BIN source. Every
 * offset, length, and dimension is rechecked against the bytes; the receipt
 * grants no draw route. Returns 1 only for a fully matching receipt. */
int nexus_v1_warning_dgt2_resource_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    uint32_t descriptor_index,
    Nexus_V1_WarningDgt2ResourceReceipt *out_receipt);

/* Admits all four canonical resources and binds their observed contiguous
 * chain [0x48, 101256). Fails closed when any single resource drifts. */
int nexus_v1_warning_dgt2_resource_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    Nexus_V1_WarningDgt2ResourceCorpusReceipt *out_receipt);

/* Executes the admitted ST-124 section-6 PP contract for one admitted
 * resource: copies its exact index bytes and original BGR555 words into
 * caller-owned buffers and invokes the explicit presentation callback. No
 * default presentation, host-RGBA conversion, CLUT substitution,
 * trailing-byte interpretation, or fallback. */
int nexus_v1_warning_dgt2_resource_execute(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2ResourceReceipt *resource_receipt,
    uint8_t *indexed_pixels_out,
    size_t indexed_pixels_out_size,
    uint16_t bgr555_words_out[256],
    Nexus_V1_WarningDgt2ResourcePresentFn present,
    void *present_context,
    Nexus_V1_WarningDgt2ResourceExecutionReceipt *out_receipt);

/* Rehashes the canonical source, admits the requested resource, and writes
 * its exact PP indices into the top-left of the real 320x200 M11 indexed
 * surface with the ST-124-ordered BGR555->RGB6 exact palette expansion. A
 * changed source or failed receipt leaves the caller-cleared frame
 * unpresented; no substitute surface or fallback is used. */
int nexus_v1_warning_dgt2_resource_m11_present(
    const uint8_t *source_bytes,
    size_t source_size,
    uint32_t descriptor_index,
    uint8_t *m11_framebuffer,
    size_t m11_framebuffer_size,
    uint8_t out_rgb6[256][3],
    Nexus_V1_WarningDgt2ResourceM11Receipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
