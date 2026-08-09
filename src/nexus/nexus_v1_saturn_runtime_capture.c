#include "nexus_v1_saturn_runtime_capture.h"

#include <stdio.h>
#include <string.h>

static int has_bytes(const uint8_t *base, size_t size, size_t offset,
                     size_t count)
{
    return base && offset <= size && count <= size - offset;
}

static int find_newline(const uint8_t *data, size_t size, size_t start,
                        size_t *out_end)
{
    size_t i;
    if (!data || !out_end || start > size) return 0;
    for (i = start; i < size; ++i) {
        if (data[i] == '\n') {
            *out_end = i;
            return 1;
        }
    }
    return 0;
}

static int parse_state(const uint8_t *line, size_t line_size,
                       Nexus_V1_SaturnRuntimeCaptureFrameReceipt *receipt)
{
    char text[192];
    unsigned int fb;
    int parsed;

    if (!line || !receipt || line_size == 0U || line_size >= sizeof(text)) {
        return 0;
    }
    memcpy(text, line, line_size);
    text[line_size] = '\0';
    parsed = sscanf(text,
        "state=tvmr:%x,fbcr:%x,ptmr:%x,edsr:%x,lopr:%x,copr:%x,ret:%x,fb:%u",
        &receipt->tvmr, &receipt->fbcr, &receipt->ptmr, &receipt->edsr,
        &receipt->lopr, &receipt->copr_word, &receipt->ret, &fb);
    if (parsed != 8 || fb > 1U) return 0;
    receipt->framebuffer_select = fb;
    receipt->vdp1_state_present = 1;
    receipt->vdp1_state_valid = 1;
    return 1;
}

static int payload_nonzero(const uint8_t *data, size_t size)
{
    size_t i;
    if (!data) return 0;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0U) return 1;
    }
    return 0;
}

int nexus_v1_saturn_runtime_capture_frame(
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt receipt;
    const size_t runtime_magic =
        sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U;
    const size_t vdp1_magic_v1 =
        sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U;
    const size_t vdp1_magic_v2 =
        sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U;
    const size_t vdp2_magic = sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U;
    size_t offset;
    unsigned int current_frame = 0U;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    if (!capture_bytes || capture_byte_count < runtime_magic ||
        memcmp(capture_bytes, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC,
               runtime_magic) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    offset = runtime_magic;
    while (offset < capture_byte_count) {
        char frame_marker[32];
        size_t marker_size;
        size_t line_end;
        int state_present = 0;
        const uint8_t *vdp1_payload;
        const uint8_t *vdp2_payload;

        snprintf(frame_marker, sizeof(frame_marker), "frame=%u\n",
                 current_frame);
        marker_size = strlen(frame_marker);
        if (!has_bytes(capture_bytes, capture_byte_count, offset,
                       marker_size) ||
            memcmp(capture_bytes + offset, frame_marker, marker_size) != 0) {
            *out_receipt = receipt;
            return 0;
        }
        offset += marker_size;
        if (has_bytes(capture_bytes, capture_byte_count, offset,
                      vdp1_magic_v2) &&
            memcmp(capture_bytes + offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2,
                   vdp1_magic_v2) == 0) {
            offset += vdp1_magic_v2;
            if (!find_newline(capture_bytes, capture_byte_count, offset,
                              &line_end) ||
                line_end == offset + 1U ||
                memcmp(capture_bytes + offset, "state=", 6U) != 0) {
                *out_receipt = receipt;
                return 0;
            }
            state_present = parse_state(capture_bytes + offset,
                                        line_end - offset, &receipt);
            if (!state_present) {
                *out_receipt = receipt;
                return 0;
            }
            offset = line_end + 1U;
        } else if (has_bytes(capture_bytes, capture_byte_count, offset,
                             vdp1_magic_v1) &&
                   memcmp(capture_bytes + offset,
                          NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1,
                          vdp1_magic_v1) == 0) {
            offset += vdp1_magic_v1;
        } else {
            *out_receipt = receipt;
            return 0;
        }
        if (!has_bytes(capture_bytes, capture_byte_count, offset,
                       NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES)) {
            *out_receipt = receipt;
            return 0;
        }
        vdp1_payload = capture_bytes + offset;
        offset += NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES;
        if (!has_bytes(capture_bytes, capture_byte_count, offset,
                       vdp2_magic) ||
            memcmp(capture_bytes + offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC,
                   vdp2_magic) != 0) {
            *out_receipt = receipt;
            return 0;
        }
        offset += vdp2_magic;
        if (!has_bytes(capture_bytes, capture_byte_count, offset,
                       NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES)) {
            *out_receipt = receipt;
            return 0;
        }
        vdp2_payload = capture_bytes + offset;
        offset += NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
        if (current_frame == frame_index) {
            receipt.frame_index = (int)current_frame;
            receipt.vdp1_vram = vdp1_payload;
            receipt.vdp1_framebuffer_0 =
                vdp1_payload + NEXUS_V1_SATURN_VDP1_VRAM_BYTES;
            receipt.vdp1_framebuffer_1 = receipt.vdp1_framebuffer_0 +
                NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES;
            receipt.vdp1_draw_which = receipt.vdp1_framebuffer_1 +
                NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES;
            receipt.vdp2_cram = vdp2_payload;
            receipt.vdp2_vram = vdp2_payload + NEXUS_V1_SATURN_VDP2_CRAM_BYTES;
            receipt.vdp2_registers = receipt.vdp2_vram +
                NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
            receipt.vdp1_vram_size = NEXUS_V1_SATURN_VDP1_VRAM_BYTES;
            receipt.vdp1_framebuffer_size =
                NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES;
            receipt.vdp2_cram_size = NEXUS_V1_SATURN_VDP2_CRAM_BYTES;
            receipt.vdp2_vram_size = NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
            receipt.vdp2_register_size = NEXUS_V1_SATURN_VDP2_REG_BYTES;
            receipt.vdp1_payload_nonzero = payload_nonzero(
                vdp1_payload, NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES);
            receipt.vdp1_execution_active = state_present &&
                receipt.ptmr != 0U && receipt.edsr != 0U &&
                receipt.vdp1_payload_nonzero;
            receipt.valid = 1;
            *out_receipt = receipt;
            return 1;
        }
        ++current_frame;
    }
    *out_receipt = receipt;
    return 0;
}
