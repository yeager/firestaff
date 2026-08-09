#ifndef NEXUS_V1_SATURN_RUNTIME_CAPTURE_H
#define NEXUS_V1_SATURN_RUNTIME_CAPTURE_H

#include <stddef.h>
#include <stdint.h>

/* The capture producer writes one exact VDP1/VDP2 frame after this marker.
 * This reader is transport-only: it does not assign menu, HUD, viewport or
 * DGN meaning to any captured byte. */
#define NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC \
    "FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
#define NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1 \
    "FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V1\n"
#define NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2 \
    "FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2\n"
#define NEXUS_V1_SATURN_VDP2_RAW_MAGIC "VDP2_RAW\n"
#define NEXUS_V1_SATURN_VDP1_VRAM_BYTES (0x40000U * 2U)
#define NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES (0x20000U * 2U)
#define NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES \
    (NEXUS_V1_SATURN_VDP1_VRAM_BYTES + \
     NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES * 2U + 1U)
#define NEXUS_V1_SATURN_VDP2_CRAM_BYTES (0x800U * 2U)
#define NEXUS_V1_SATURN_VDP2_VRAM_BYTES (0x40000U * 2U)
#define NEXUS_V1_SATURN_VDP2_REG_BYTES (0x100U * 2U)
#define NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES \
    (NEXUS_V1_SATURN_VDP2_CRAM_BYTES + \
     NEXUS_V1_SATURN_VDP2_VRAM_BYTES + \
     NEXUS_V1_SATURN_VDP2_REG_BYTES)

typedef struct {
    int valid;
    int frame_index;
    int vdp1_state_present;
    int vdp1_state_valid;
    uint32_t tvmr;
    uint32_t fbcr;
    uint32_t ptmr;
    uint32_t edsr;
    uint32_t lopr;
    uint32_t copr_word;
    uint32_t ret;
    uint32_t framebuffer_select;
    int vdp1_payload_nonzero;
    int vdp1_execution_active;
    const uint8_t *vdp1_vram;
    const uint8_t *vdp1_framebuffer_0;
    const uint8_t *vdp1_framebuffer_1;
    const uint8_t *vdp1_draw_which;
    const uint8_t *vdp2_cram;
    const uint8_t *vdp2_vram;
    const uint8_t *vdp2_registers;
    size_t vdp1_vram_size;
    size_t vdp1_framebuffer_size;
    size_t vdp2_cram_size;
    size_t vdp2_vram_size;
    size_t vdp2_register_size;
    int semantic_admission_blocked;
} Nexus_V1_SaturnRuntimeCaptureFrameReceipt;

/* Parse exactly one frame from an authenticated producer artifact. All
 * returned spans point into capture_bytes and remain valid only while that
 * buffer remains alive. V1 frames are accepted as transport evidence but do
 * not provide a COPR and therefore cannot enter VDP1 command replay. */
int nexus_v1_saturn_runtime_capture_frame(
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_receipt);

#endif
