#include "nexus_v1_saturn_runtime_capture.h"

#include <stdio.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[1] << 8U) | p[0]);
}

static int vdp2_score(const uint8_t *registers, int little)
{
    uint16_t tvmd = little ? read_le16(registers) : read_be16(registers);
    uint16_t bgon = little ? read_le16(registers + 0x20U) :
        read_be16(registers + 0x20U);
    uint16_t chctla = little ? read_le16(registers + 0x28U) :
        read_be16(registers + 0x28U);
    int score = 0;
    if (tvmd & 0x8000U) score += 3;
    if (bgon & 0x001fU) score += 4;
    if ((bgon & ~0x1f3fU) == 0U) score += 1;
    if (bgon & 0x0002U) {
        score += 2;
        if (chctla & 0x0200U) score += 1;
    }
    return score;
}

static uint16_t vdp2_read16(const uint8_t *registers, size_t offset,
                            Nexus_V1_SaturnVdp2RegisterByteOrder order)
{
    return order == NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE
        ? read_le16(registers + offset) : read_be16(registers + offset);
}

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
    unsigned int system_clip_x;
    unsigned int system_clip_y;
    int consumed = 0;
    int suffix_consumed = 0;
    int parsed;

    if (!line || !receipt || line_size == 0U || line_size >= sizeof(text)) {
        return 0;
    }
    memcpy(text, line, line_size);
    text[line_size] = '\0';
    parsed = sscanf(text,
        "state=tvmr:%x,fbcr:%x,ptmr:%x,edsr:%x,lopr:%x,copr:%x,ret:%x,fb:%u%n",
        &receipt->tvmr, &receipt->fbcr, &receipt->ptmr, &receipt->edsr,
        &receipt->lopr, &receipt->copr_word, &receipt->ret, &fb, &consumed);
    if (parsed != 8 || fb > 1U) return 0;
    receipt->framebuffer_select = fb;
    if (text[consumed] != '\0') {
        if (sscanf(text + consumed, ",sysclipx:%x,sysclipy:%x%n",
                   &system_clip_x, &system_clip_y, &suffix_consumed) != 2 ||
            text[consumed + suffix_consumed] != '\0') return 0;
        receipt->vdp1_system_clip_state_present = 1;
        receipt->system_clip_x = system_clip_x;
        receipt->system_clip_y = system_clip_y;
    }
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
    const size_t mednafen_runtime_magic =
        sizeof(NEXUS_V1_SATURN_MDFN_RUNTIME_CAPTURE_MAGIC) - 1U;
    const size_t vdp1_magic_v1 =
        sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U;
    const size_t vdp1_magic_v2 =
        sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U;
    const size_t vdp1_magic_mednafen =
        sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_MDFN) - 1U;
    const size_t vdp2_magic = sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U;
    size_t offset;
    unsigned int current_frame = 0U;
    int firestaff_magic;
    int mednafen_magic_present;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    firestaff_magic = capture_bytes && capture_byte_count >= runtime_magic &&
        memcmp(capture_bytes, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC,
               runtime_magic) == 0;
    mednafen_magic_present = capture_bytes &&
        capture_byte_count >= mednafen_runtime_magic &&
        memcmp(capture_bytes, NEXUS_V1_SATURN_MDFN_RUNTIME_CAPTURE_MAGIC,
               mednafen_runtime_magic) == 0;
    if (!capture_bytes || (!firestaff_magic && !mednafen_magic_present)) {
        *out_receipt = receipt;
        return 0;
    }
    offset = mednafen_magic_present
        ? mednafen_runtime_magic : runtime_magic;
    while (offset < capture_byte_count) {
        char frame_marker[32];
        size_t marker_size;
        size_t line_end;
        int state_present = 0;
        const uint8_t *vdp1_payload;
        const uint8_t *vdp1_draw_which = NULL;
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
        } else if (has_bytes(capture_bytes, capture_byte_count, offset,
                             vdp1_magic_mednafen) &&
                   memcmp(capture_bytes + offset,
                          NEXUS_V1_SATURN_VDP1_RAW_MAGIC_MDFN,
                          vdp1_magic_mednafen) == 0) {
            offset += vdp1_magic_mednafen;
            /* The generic Mednafen producer may include the VDP1 execution
             * registers after its marker. Older candidate captures omitted
             * this line, so keep it optional for transport compatibility. */
            if (find_newline(capture_bytes, capture_byte_count, offset,
                             &line_end) &&
                line_end > offset &&
                memcmp(capture_bytes + offset, "state=", 6U) == 0) {
                state_present = parse_state(capture_bytes + offset,
                                            line_end - offset, &receipt);
                if (!state_present) {
                    *out_receipt = receipt;
                    return 0;
                }
                offset = line_end + 1U;
            }
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
        /* An early VDP1 V2 producer appended the draw-buffer selector even
         * though the state line already carried the same fact. Accept that
         * transport variant without including the byte in the VDP1 regions. */
        if (!has_bytes(capture_bytes, capture_byte_count, offset,
                       vdp2_magic) ||
            memcmp(capture_bytes + offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC,
                   vdp2_magic) != 0) {
            if (has_bytes(capture_bytes, capture_byte_count, offset + 1U,
                          vdp2_magic) &&
                memcmp(capture_bytes + offset + 1U,
                       NEXUS_V1_SATURN_VDP2_RAW_MAGIC, vdp2_magic) == 0) {
                vdp1_draw_which = capture_bytes + offset;
                ++offset;
            }
        }
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
            receipt.vdp1_draw_which = vdp1_draw_which;
            /* Producer order is RawRegs, VRAM, CRAM. Keep these pointers
             * aligned with scripts/mednafen_1.32.1_nexus_saturn_capture.patch
             * and validate them through the external frame witness. */
            receipt.vdp2_registers = vdp2_payload;
            receipt.vdp2_vram = vdp2_payload +
                NEXUS_V1_SATURN_VDP2_REG_BYTES;
            receipt.vdp2_cram = receipt.vdp2_vram +
                NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
            receipt.vdp1_vram_size = NEXUS_V1_SATURN_VDP1_VRAM_BYTES;
            receipt.vdp1_framebuffer_size =
                NEXUS_V1_SATURN_VDP1_FRAMEBUFFER_BYTES;
            receipt.vdp2_cram_size = NEXUS_V1_SATURN_VDP2_CRAM_BYTES;
            receipt.vdp2_vram_size = NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
            receipt.vdp2_register_size = NEXUS_V1_SATURN_VDP2_REG_BYTES;
            receipt.vdp1_payload_nonzero = payload_nonzero(
                vdp1_payload, NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES);
            receipt.vdp1_word_order = mednafen_magic_present
                ? NEXUS_V1_SATURN_VDP1_WORD_ORDER_BIG
                : NEXUS_V1_SATURN_VDP1_WORD_ORDER_LITTLE;
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

int nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
    const Nexus_V1_SaturnRuntimeCaptureFrameReceipt *frame,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_receipt)
{
    Nexus_V1_SaturnVdp2RegisterReceipt receipt;
    int big_score;
    int little_score;

    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    if (!out_receipt) return 0;
    if (!frame || !frame->valid || !frame->vdp2_registers ||
        frame->vdp2_register_size < 0x2aU) {
        *out_receipt = receipt;
        return 0;
    }
    big_score = vdp2_score(frame->vdp2_registers, 0);
    little_score = vdp2_score(frame->vdp2_registers, 1);
    receipt.byte_order = little_score >= big_score
        ? NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE
        : NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    receipt.tvmd = vdp2_read16(frame->vdp2_registers, 0x00U,
                               receipt.byte_order);
    receipt.bgon = vdp2_read16(frame->vdp2_registers, 0x20U,
                               receipt.byte_order);
    receipt.chctla = vdp2_read16(frame->vdp2_registers, 0x28U,
                                 receipt.byte_order);
    receipt.chctlb = vdp2_read16(frame->vdp2_registers, 0x2aU,
                                 receipt.byte_order);
    receipt.bmpna = vdp2_read16(frame->vdp2_registers, 0x2cU,
                                receipt.byte_order);
    receipt.pncn1 = vdp2_read16(frame->vdp2_registers, 0x32U,
                                receipt.byte_order);
    receipt.craofa = vdp2_read16(frame->vdp2_registers, 0xe4U,
                                 receipt.byte_order);
    receipt.nbg1_enabled = (receipt.bgon & 0x0002U) != 0U;
    receipt.nbg1_bitmap_mode = (receipt.chctla & 0x0200U) != 0U;
    receipt.nbg1_16x16_character_mode = (receipt.chctla & 0x0100U) != 0U;
    receipt.nbg1_colour_code = (receipt.chctla >> 12U) & 3U;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}
