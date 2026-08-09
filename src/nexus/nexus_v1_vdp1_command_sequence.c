#include "nexus_v1_vdp1_command_sequence.h"

#include <string.h>

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8U);
}

static int valid_offset(uint32_t offset, int size)
{
    return offset < (uint32_t)size && (offset % NEXUS_V1_VDP1_COMMAND_BYTES) == 0U &&
        offset <= (uint32_t)size - NEXUS_V1_VDP1_COMMAND_BYTES;
}

static int signed_11(uint16_t value)
{
    value &= 0x07ffU;
    return (value & 0x0400U) != 0U ? (int)value - 0x0800 : (int)value;
}

static int contains_offset(const uint32_t *offsets, int count, uint32_t target)
{
    int i;
    for (i = 0; i < count; ++i) {
        if (offsets[i] == target) return 1;
    }
    return 0;
}

static int follow_candidate(const uint8_t *vram, int vram_size,
                            uint32_t start, uint32_t copr,
                            uint32_t offsets[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS],
                            int *out_count, int *out_draw_count,
                            int *out_user_clip_count,
                            int *out_system_clip_count,
                            int *out_local_count, int *out_has_copr)
{
    uint32_t returns[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    int return_count = 0;
    int count = 0;
    int draws = 0;
    int user_clips = 0;
    int system_clips = 0;
    int locals = 0;
    int has_copr = 0;
    uint32_t offset = start;

    while (1) {
        uint16_t control;
        uint16_t link;
        unsigned jump_mode;
        uint8_t type;

        if (!valid_offset(offset, vram_size) ||
            contains_offset(offsets, count, offset) ||
            count >= NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS) return 0;
        control = read_le16(vram + offset);
        link = read_le16(vram + offset + 2U);
        if (control == 0U && link == 0U &&
            !memcmp(vram + offset, "\0\0\0\0", 4U)) return 0;
        offsets[count++] = offset;
        if (offset == copr) has_copr = 1;
        type = (uint8_t)(control & 0x000fU);
        if (control & 0x8000U) break;
        if (type <= 7U) ++draws;
        else if (type == 8U) ++user_clips;
        else if (type == 9U) ++system_clips;
        else if (type == 10U) ++locals;

        jump_mode = (unsigned)((control >> 12U) & 0x3U);
        if (jump_mode == 0U) {
            offset += NEXUS_V1_VDP1_COMMAND_BYTES;
        } else if (jump_mode == 1U) {
            offset = (uint32_t)link << 3U;
        } else if (jump_mode == 2U) {
            if (return_count >= NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS) return 0;
            returns[return_count++] = offset + NEXUS_V1_VDP1_COMMAND_BYTES;
            offset = (uint32_t)link << 3U;
        } else {
            if (return_count <= 0) return 0;
            offset = returns[--return_count];
        }
    }

    if (out_count) *out_count = count;
    if (out_draw_count) *out_draw_count = draws;
    if (out_user_clip_count) *out_user_clip_count = user_clips;
    if (out_system_clip_count) *out_system_clip_count = system_clips;
    if (out_local_count) *out_local_count = locals;
    if (out_has_copr) *out_has_copr = has_copr;
    return count > 0 && (read_le16(vram + offsets[count - 1]) & 0x8000U) != 0U;
}

int nexus_v1_vdp1_command_sequence_frame(
    const Nexus_V1_Vdp1CommandSequenceInput *input,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_receipt)
{
    Nexus_V1_Vdp1CommandSequenceReceipt receipt;
    uint32_t best[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    int best_count = 0;
    int best_draws = 0;
    int best_user_clips = 0;
    int best_system_clips = 0;
    int best_locals = 0;
    int best_score_draws = -1;
    int best_score_length = 0;
    uint32_t copr;
    uint32_t start;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    if (!input || !input->vdp1_vram ||
        input->vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        input->copr_word > (uint32_t)NEXUS_V1_VDP1_VRAM_BYTES / 8U ||
        (input->system_clip_state_present &&
         (input->system_clip_x > 0x1fffU || input->system_clip_y > 0x1fffU))) {
        *out_receipt = receipt;
        return 0;
    }
    copr = input->copr_word << 3U;
    receipt.copr_byte_offset = copr;
    if (!valid_offset(copr, input->vdp1_vram_size)) {
        *out_receipt = receipt;
        return 0;
    }

    for (start = 0U;
         start <= (uint32_t)input->vdp1_vram_size - NEXUS_V1_VDP1_COMMAND_BYTES;
         start += NEXUS_V1_VDP1_COMMAND_BYTES) {
        uint32_t candidate[NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
        int count;
        int draws;
        int user_clips;
        int system_clips;
        int locals;
        int has_copr;
        int score_length;

        if (!follow_candidate(input->vdp1_vram, input->vdp1_vram_size,
                              start, copr, candidate, &count, &draws,
                              &user_clips, &system_clips, &locals,
                              &has_copr) || !has_copr || draws <= 0 ||
            (user_clips + system_clips <= 0 &&
             !input->system_clip_state_present) || locals <= 0) continue;
        score_length = -count;
        if (draws > best_score_draws ||
            (draws == best_score_draws && score_length > -best_score_length)) {
            memcpy(best, candidate, (size_t)count * sizeof(*best));
            best_count = count;
            best_draws = draws;
            best_user_clips = user_clips;
            best_system_clips = system_clips;
            best_locals = locals;
            best_score_draws = draws;
            best_score_length = count;
        }
    }
    if (best_count <= 0) {
        *out_receipt = receipt;
        return 0;
    }

    memcpy(receipt.command_byte_offsets, best,
           (size_t)best_count * sizeof(*best));
    receipt.start_byte_offset = best[0];
    receipt.end_byte_offset = best[best_count - 1];
    receipt.command_count = best_count;
    receipt.draw_count = best_draws;
    receipt.user_clip_count = best_user_clips;
    receipt.system_clip_count = best_system_clips;
    receipt.system_clip_state_verified = best_system_clips > 0 ||
        input->system_clip_state_present;
    if (input->system_clip_state_present) {
        receipt.system_clip_x = input->system_clip_x;
        receipt.system_clip_y = input->system_clip_y;
    }
    receipt.local_coordinate_count = best_locals;
    receipt.command_order_verified = 1;
    receipt.end_record_verified =
        (read_le16(input->vdp1_vram + receipt.end_byte_offset) & 0x8000U) != 0U;
    receipt.complete = receipt.command_order_verified && receipt.end_record_verified;

    /* VDP1 Local Coordinate is display-space origin state.  Keep the first
     * observed state as framing data; camera/mesh meaning remains blocked. */
    {
        int i;
        for (i = 0; i < best_count; ++i) {
            const uint8_t *record = input->vdp1_vram + best[i];
            if ((read_le16(record) & 0x000fU) == 10U &&
                !(read_le16(record) & 0x8000U)) {
                receipt.display_origin_x = signed_11(read_le16(record + 12U));
                receipt.display_origin_y = signed_11(read_le16(record + 14U));
                receipt.display_origin_verified = 1;
                break;
            }
        }
        for (i = 0; i < best_count; ++i) {
            const uint8_t *record = input->vdp1_vram + best[i];
            if ((read_le16(record) & 0x000fU) == 8U &&
                !(read_le16(record) & 0x8000U)) {
                receipt.user_clip_x0 = read_le16(record + 12U) & 0x1fffU;
                receipt.user_clip_y0 = read_le16(record + 14U) & 0x1fffU;
                receipt.user_clip_x1 = read_le16(record + 20U) & 0x1fffU;
                receipt.user_clip_y1 = read_le16(record + 22U) & 0x1fffU;
                break;
            }
        }
    }
    receipt.valid = receipt.complete && receipt.display_origin_verified;
    *out_receipt = receipt;
    return receipt.valid;
}
