#include "nexus_v1_saturn_runtime_capture.h"

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
            "copr:000374,ret:ffffffff,fb:0\n";
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
        "copr:000374,ret:ffffffff,fb:0\n";
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
    size_t offset;
    const char *external = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");

    if (!blob) return 1;
    offset = 0U;
    offset = append_bytes(blob, offset, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC,
                          sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U);
    offset = append_frame(blob, offset, 0U, 0);
    offset = append_frame(blob, offset, 1U, 1);
    if (offset != blob_size ||
        !nexus_v1_saturn_runtime_capture_frame(blob, blob_size, 1U,
                                               &receipt) ||
        !receipt.valid || !receipt.vdp1_state_present ||
        !receipt.vdp1_state_valid || receipt.copr_word != 0x374U ||
        !receipt.vdp1_execution_active || receipt.vdp1_vram_size !=
            NEXUS_V1_SATURN_VDP1_VRAM_BYTES || !receipt.vdp2_registers ||
        receipt.vdp2_cram_size != NEXUS_V1_SATURN_VDP2_CRAM_BYTES ||
        receipt.vdp2_vram_size != NEXUS_V1_SATURN_VDP2_VRAM_BYTES ||
        !receipt.semantic_admission_blocked ||
        !nexus_v1_saturn_runtime_capture_vdp2_register_receipt(
            &receipt, &register_receipt) || !register_receipt.valid ||
        register_receipt.byte_order !=
            NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE ||
        register_receipt.semantic_admission_blocked != 1 ||
        nexus_v1_saturn_runtime_capture_frame(blob, blob_size, 2U,
                                               &receipt)) {
        free(blob);
        fprintf(stderr, "FAIL: synthetic Saturn raw frame parser\n");
        return 1;
    }
    free(blob);

    if (external) {
        uint8_t *external_data = NULL;
        size_t external_size = 0U;
        unsigned int frame = 0U;
        const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
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
        printf("external_frame=%u state=%d active=%d copr=0x%x\n", frame,
               receipt.vdp1_state_present, receipt.vdp1_execution_active,
               receipt.copr_word);
        printf("external_vdp2_order=%d tvmd=0x%04x bgon=0x%04x nbg1=%d bitmap=%d\n",
               register_receipt.byte_order, register_receipt.tvmd,
               register_receipt.bgon, register_receipt.nbg1_enabled,
               register_receipt.nbg1_bitmap_mode);
        free(external_data);
    }
    puts("test_nexus_v1_saturn_runtime_capture: PASS");
    return 0;
}
