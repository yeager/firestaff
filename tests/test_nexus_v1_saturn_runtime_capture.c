#include "nexus_v1_saturn_runtime_capture.h"
#include "nexus_v1_vdp2_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t append_bytes(uint8_t *blob, size_t offset, const void *data,
                           size_t size)
{
    memcpy(blob + offset, data, size);
    return offset + size;
}

static size_t append_frame(uint8_t *blob, size_t offset, unsigned int index,
                           int with_state)
{
    char marker[32];
    size_t payload_offset;
    snprintf(marker, sizeof(marker), "frame=%u\n", index);
    offset = append_bytes(blob, offset, marker, strlen(marker));
    if (with_state) {
        static const char state[] =
            "state=tvmr:00,fbcr:00,ptmr:02,edsr:03,lopr:0000,"
            "copr:000374,ret:ffffffff,fb:0,sysclipx:013f,sysclipy:00df\n";
        offset = append_bytes(blob, offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2,
                              sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U);
        offset = append_bytes(blob, offset, state, sizeof(state) - 1U);
    } else {
        offset = append_bytes(blob, offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1,
                              sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U);
    }
    payload_offset = offset;
    memset(blob + offset, 0, NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES);
    if (with_state) blob[payload_offset] = 0x5aU;
    offset += NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES;
    offset = append_bytes(blob, offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC,
                          sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U);
    memset(blob + offset, 0, NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES);
    return offset + NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
}

static int read_external_capture(const char *path, uint8_t **out_data,
                                 size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;
    if (!path || !out_data || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0) {
        if (file) fclose(file);
        return 0;
    }
    size = ftell(file);
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

int main(void)
{
    static const char test_state[] =
        "state=tvmr:00,fbcr:00,ptmr:02,edsr:03,lopr:0000,"
        "copr:000374,ret:ffffffff,fb:0,sysclipx:013f,sysclipy:00df\n";
    const size_t frame0_size = strlen("frame=0\n") +
        (sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U) +
        NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES +
        (sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U) +
        NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
    const size_t frame1_size = strlen("frame=1\n") +
        (sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U) +
        sizeof(test_state) - 1U +
        NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES +
        (sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U) +
        NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
    const size_t blob_size =
        (sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U) +
        frame0_size + frame1_size;
    uint8_t *blob = (uint8_t *)calloc(1U, blob_size);
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt receipt;
    Nexus_V1_SaturnVdp2RegisterReceipt register_receipt;
    Nexus_V1_Vdp2BitmapCaptureFramebuffer nbg0_framebuffer;
    Nexus_V1_Vdp2BitmapCaptureReceipt nbg0_receipt;
    size_t offset;
    const char *external = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");

    if (!blob) return 1;
    offset = 0U;
    offset = append_bytes(blob, offset, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC,
                          sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U);
    offset = append_frame(blob, offset, 0U, 0);
    offset = append_frame(blob, offset, 1U, 1);
    {
        const size_t vdp2_register_offset =
            (sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U) +
            frame0_size + strlen("frame=1\n") +
            (sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U) +
            sizeof(test_state) - 1U + NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES +
            (sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U);
        /* Firestaff V2 stores the VDP2 register words in host/little-endian
         * order. Make the producer order observable instead of testing zeros. */
        blob[vdp2_register_offset + 0x00U] = 0x00U;
        blob[vdp2_register_offset + 0x01U] = 0x80U;
        blob[vdp2_register_offset + 0x20U] = 0x02U;
        blob[vdp2_register_offset + 0x21U] = 0x00U;
        blob[vdp2_register_offset + 0x28U] = 0x13U;
        blob[vdp2_register_offset + 0x29U] = 0x00U;
        blob[vdp2_register_offset + 0x2cU] = 0x05U;
        blob[vdp2_register_offset + 0xf8U] = 0x05U;
        blob[vdp2_register_offset + 0xf9U] = 0x03U;
        blob[vdp2_register_offset + 0xd0U] = 0x11U;
        blob[vdp2_register_offset + 0xd1U] = 0x22U;
        blob[vdp2_register_offset + 0xd2U] = 0x33U;
        blob[vdp2_register_offset + 0xd3U] = 0x44U;
        blob[vdp2_register_offset + 0xd4U] = 0x55U;
        blob[vdp2_register_offset + 0xd5U] = 0x66U;
        blob[vdp2_register_offset + 0xd6U] = 0x77U;
        blob[vdp2_register_offset + 0xd7U] = 0x88U;
        blob[vdp2_register_offset + 0xe0U] = 0x99U;
        blob[vdp2_register_offset + 0xe1U] = 0xaaU;
        blob[vdp2_register_offset + 0xecU] = 0xbbU;
        blob[vdp2_register_offset + 0xedU] = 0xccU;
    }
    if (offset != blob_size ||
        !nexus_v1_saturn_runtime_capture_frame(blob, blob_size, 1U,
                                               &receipt) ||
        !receipt.valid || !receipt.vdp1_state_present ||
        !receipt.vdp1_state_valid || receipt.copr_word != 0x374U ||
        !receipt.vdp1_system_clip_state_present ||
        receipt.system_clip_x != 0x13fU || receipt.system_clip_y != 0xdfU ||
        !receipt.vdp1_execution_active || receipt.vdp1_vram_size !=
            NEXUS_V1_SATURN_VDP1_VRAM_BYTES || receipt.vdp1_draw_which ||
            !receipt.vdp2_registers ||
        receipt.vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        receipt.vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        !receipt.semantic_admission_blocked ||
        !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
            &receipt, &register_receipt) || !register_receipt.valid ||
        register_receipt.byte_order !=
            NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE ||
        register_receipt.tvmd != 0x8000U || register_receipt.bgon != 0x0002U ||
        register_receipt.nbg0_enabled || !register_receipt.nbg0_bitmap_mode ||
        register_receipt.nbg0_colour_code != 1 ||
        register_receipt.nbg0_bitmap_palette_number != 5 ||
        register_receipt.prina != 0x0305U ||
        register_receipt.nbg0_priority != 5U ||
        register_receipt.nbg1_priority != 3U ||
        register_receipt.wctla != 0x2211U ||
        register_receipt.wctlb != 0x4433U ||
        register_receipt.wctlc != 0x6655U ||
        register_receipt.wctld != 0x8877U ||
        register_receipt.spctl != 0xaa99U ||
        register_receipt.ccctl != 0xccbbU ||
        register_receipt.semantic_admission_blocked != 1 ||
        nexus_v1_saturn_runtime_capture_frame(blob, blob_size, 2U,
                                               &receipt)) {
        free(blob);
        fprintf(stderr, "FAIL: synthetic Saturn raw frame parser\n");
        return 1;
    }
    /* This is a raw-capture schema fixture, not substitute game art. The
     * retail title's NBG0 bytes are checked separately against real media. */
    {
        const size_t vdp2_register_offset =
            (sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U) +
            frame0_size + strlen("frame=1\n") +
            (sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V2) - 1U) +
            sizeof(test_state) - 1U + NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES +
            (sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U);
        const size_t vdp2_vram_offset = vdp2_register_offset +
            NEXUS_V1_SATURN_VDP2_REG_BYTES;
        const size_t vdp2_cram_offset = vdp2_vram_offset +
            NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
        blob[vdp2_register_offset + 0x20U] = 0x03U;
        blob[vdp2_register_offset + 0x2cU] = 0x00U;
        blob[vdp2_vram_offset] = 1U;
        blob[vdp2_cram_offset + 2U] = 0x1fU;
        blob[vdp2_cram_offset + 3U] = 0x80U;
        memset(&nbg0_framebuffer, 0, sizeof(nbg0_framebuffer));
        memset(&nbg0_receipt, 0, sizeof(nbg0_receipt));
        if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg0_bitmap(
                &nbg0_framebuffer, blob, blob_size, 1U, &receipt,
                &register_receipt, &nbg0_receipt) || !nbg0_receipt.valid ||
            !nbg0_receipt.capture_only || nbg0_receipt.renderer_permitted ||
            !nbg0_receipt.nbg0_bitmap_mode || !nbg0_receipt.colour_code_256 ||
            nbg0_receipt.bitmap_vram_offset != 0U ||
            nbg0_receipt.cram_offset != 0U || nbg0_receipt.written_pixels != 1 ||
            nbg0_framebuffer.rgba_buffer[0] == 0U) {
            free(blob);
            fprintf(stderr, "FAIL: NBG0 capture-only bitmap decoder\n");
            return 1;
        }
    }
    free(blob);

    {
        const char *generic_state =
            "state=tvmr:0,fbcr:2,ptmr:2,edsr:3,lopr:dd0,copr:dd4,"
            "ret:ffffffff,fb:1,sysclipx:13f,sysclipy:df\n";
        const size_t generic_size =
            (sizeof(NEXUS_V1_SATURN_MDFN_RUNTIME_CAPTURE_MAGIC) - 1U) +
            strlen("frame=0\n") +
            (sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_MDFN) - 1U) +
            strlen(generic_state) +
            NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES +
            (sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U) +
            NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
        uint8_t *generic = (uint8_t *)calloc(1U, generic_size);
        size_t generic_offset = 0U;
        if (!generic) return 1;
        generic_offset = append_bytes(
            generic, generic_offset,
            NEXUS_V1_SATURN_MDFN_RUNTIME_CAPTURE_MAGIC,
            sizeof(NEXUS_V1_SATURN_MDFN_RUNTIME_CAPTURE_MAGIC) - 1U);
        generic_offset = append_bytes(generic, generic_offset,
                                      "frame=0\n", strlen("frame=0\n"));
        generic_offset = append_bytes(
            generic, generic_offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_MDFN,
            sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_MDFN) - 1U);
        generic_offset = append_bytes(generic, generic_offset,
                                      generic_state, strlen(generic_state));
        generic[generic_offset] = 0x5aU;
        generic_offset += NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES;
        generic_offset = append_bytes(
            generic, generic_offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC,
            sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U);
        /* Mednafen's candidate PR emits explicit big-endian Saturn words. */
        generic[generic_offset + 0x00U] = 0x80U;
        generic[generic_offset + 0x01U] = 0x00U;
        generic[generic_offset + 0x20U] = 0x00U;
        generic[generic_offset + 0x21U] = 0x02U;
        generic[generic_offset + 0xf8U] = 0x03U;
        generic[generic_offset + 0xf9U] = 0x05U;
        generic[generic_offset + 0xd0U] = 0x22U;
        generic[generic_offset + 0xd1U] = 0x11U;
        generic[generic_offset + 0xecU] = 0xccU;
        generic[generic_offset + 0xedU] = 0xbbU;
        generic_offset += NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
        if (generic_offset != generic_size ||
            !nexus_v1_saturn_runtime_capture_frame(
                generic, generic_size, 0U, &receipt) || !receipt.valid ||
            !receipt.vdp1_state_present || !receipt.vdp1_state_valid ||
            receipt.copr_word != 0xdd4U ||
            receipt.vdp1_word_order !=
                NEXUS_V1_SATURN_VDP1_WORD_ORDER_BIG || !receipt.vdp1_vram ||
            receipt.vdp1_draw_which || !receipt.semantic_admission_blocked ||
            receipt.vdp2_word_order !=
                NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
            !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
                &receipt, &register_receipt) || !register_receipt.valid ||
            register_receipt.byte_order !=
                NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
            register_receipt.tvmd != 0x8000U || register_receipt.bgon != 0x0002U ||
            register_receipt.prina != 0x0305U ||
            register_receipt.nbg0_priority != 5U ||
            register_receipt.nbg1_priority != 3U ||
            register_receipt.wctla != 0x2211U ||
            register_receipt.ccctl != 0xccbbU) {
            free(generic);
            fprintf(stderr, "FAIL: generic Mednafen raw frame parser\n");
            return 1;
        }
        free(generic);
    }

    if (external) {
        uint8_t *external_data = NULL;
        size_t external_size = 0U;
        unsigned int frame = 0U;
        const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
        const char *require_nbg1_bitmap =
            getenv("FIRESTAFF_NEXUS_REQUIRE_NBG1_BITMAP");
        const char *require_nbg0_bitmap =
            getenv("FIRESTAFF_NEXUS_REQUIRE_NBG0_BITMAP");
        if (frame_text) frame = (unsigned int)strtoul(frame_text, NULL, 0);
        if (!read_external_capture(external, &external_data, &external_size) ||
            !nexus_v1_saturn_runtime_capture_frame(
                external_data, external_size, frame, &receipt) ||
            !receipt.valid || !receipt.vdp1_vram ||
            !receipt.semantic_admission_blocked ||
            !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
                &receipt, &register_receipt) || !register_receipt.valid) {
            free(external_data);
            fprintf(stderr, "FAIL: external Saturn raw frame parser\n");
            return 1;
        }
        if (require_nbg1_bitmap && require_nbg1_bitmap[0] &&
            (!register_receipt.nbg1_enabled ||
             !register_receipt.nbg1_bitmap_mode ||
             register_receipt.nbg1_colour_code != 1 ||
             register_receipt.nbg1_bitmap_palette_number != 0U ||
             register_receipt.nbg1_scroll_x != 0U ||
             register_receipt.nbg1_scroll_y != 0U)) {
            free(external_data);
            fprintf(stderr, "FAIL: external NBG1 bitmap palette/origin receipt\n");
            return 1;
        }
        if (require_nbg0_bitmap && require_nbg0_bitmap[0]) {
            memset(&nbg0_framebuffer, 0, sizeof(nbg0_framebuffer));
            memset(&nbg0_receipt, 0, sizeof(nbg0_receipt));
            if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg0_bitmap(
                    &nbg0_framebuffer, external_data, external_size, frame,
                    &receipt, &register_receipt, &nbg0_receipt) ||
                !nbg0_receipt.valid || !nbg0_receipt.capture_only ||
                nbg0_receipt.renderer_permitted ||
                !nbg0_receipt.nbg0_bitmap_mode ||
                !nbg0_receipt.colour_code_256 ||
                !nbg0_receipt.original_saturn_capture_verified ||
                nbg0_receipt.bitmap_vram_offset != 0U ||
                nbg0_receipt.cram_offset != 0U) {
                free(external_data);
                fprintf(stderr, "FAIL: external NBG0 capture-only decoder\n");
                return 1;
            }
        }
        printf("external_frame=%u state=%d active=%d copr=0x%x "
               "sysclip_present=%d sysclip=(%u,%u)\n", frame,
               receipt.vdp1_state_present, receipt.vdp1_execution_active,
               receipt.copr_word, receipt.vdp1_system_clip_state_present,
               receipt.system_clip_x, receipt.system_clip_y);
        printf("external_vdp2_order=%d tvmd=0x%04x bgon=0x%04x nbg1=%d bitmap=%d "
               "bmpna=0x%04x palette=%u scroll=(%u,%u)\n",
               register_receipt.byte_order, register_receipt.tvmd,
               register_receipt.bgon, register_receipt.nbg1_enabled,
               register_receipt.nbg1_bitmap_mode, register_receipt.bmpna,
               (unsigned int)register_receipt.nbg1_bitmap_palette_number,
               (unsigned int)register_receipt.nbg1_scroll_x,
               (unsigned int)register_receipt.nbg1_scroll_y);
        free(external_data);
    }
    puts("test_nexus_v1_saturn_runtime_capture: PASS");
    return 0;
}
