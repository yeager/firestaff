#ifndef NEXUS_V1_WARNING_DGT2_M11_PRESENTATION_H
#define NEXUS_V1_WARNING_DGT2_M11_PRESENTATION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_WARNING_M11_WIDTH 320U
#define NEXUS_V1_WARNING_M11_HEIGHT 200U

typedef struct {
    int valid;
    int canonical_source_bound;
    int pp_execution_bound;
    int host_surface_written;
    int bgr555_to_rgb6_exact;
    int fallback_permitted;
    uint64_t source_fnv1a64;
    uint64_t index_plane_fnv1a64;
    uint64_t bgr555_words_fnv1a64;
    uint64_t host_palette_rgb6_fnv1a64;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
} Nexus_V1_WarningDgt2M11PresentationReceipt;

/* Rehashes the canonical WARNING.BIN resource-0 route and writes its exact PP
 * indices into the top-left of the real 320x200 M11 indexed surface. The
 * output RGB6 table is the only palette acceptable to the host presenter. */
int nexus_v1_warning_dgt2_m11_present(
    const uint8_t *source_bytes,
    size_t source_size,
    uint8_t *m11_framebuffer,
    size_t m11_framebuffer_size,
    uint8_t out_rgb6[256][3],
    Nexus_V1_WarningDgt2M11PresentationReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
