#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_palette.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *bytes;

    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    bytes[length] = 0u;
    *out_size = (size_t)length;
    return bytes;
}

static int write_generation7_background_bmp(
    const char *path, const unsigned char *vram, const unsigned char *vce)
{
    FILE *file;
    unsigned char header[54] = {0};
    unsigned int file_size = 54u + 256u * 240u * 3u;
    int y;

    if (!path || !path[0] || !vram || !vce) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    header[0] = 'B'; header[1] = 'M';
    header[2] = (unsigned char)file_size;
    header[3] = (unsigned char)(file_size >> 8);
    header[4] = (unsigned char)(file_size >> 16);
    header[5] = (unsigned char)(file_size >> 24);
    header[10] = 54u; header[14] = 40u;
    header[18] = 0u; header[19] = 1u;
    header[22] = 240u; header[26] = 1u; header[28] = 24u;
    if (fwrite(header, 1u, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return 0;
    }
    for (y = 239; y >= 0; --y) {
        int x;
        for (x = 0; x < 256; ++x) {
            size_t bat_word = (size_t)(y / 8) * 64u + (size_t)(x / 8);
            uint16_t bat = (uint16_t)vram[bat_word * 2u] |
                ((uint16_t)vram[bat_word * 2u + 1u] << 8);
            uint16_t tile = (uint16_t)(bat & 0x0fffu);
            unsigned int row = (unsigned int)y & 7u;
            unsigned int bit = 7u - ((unsigned int)x & 7u);
            const unsigned char *pattern = vram + (size_t)tile * 32u;
            unsigned int pixel = ((pattern[row * 2u] >> bit) & 1u) |
                (((pattern[row * 2u + 1u] >> bit) & 1u) << 1) |
                (((pattern[16u + row * 2u] >> bit) & 1u) << 2) |
                (((pattern[17u + row * 2u] >> bit) & 1u) << 3);
            unsigned int palette_index = pixel == 0u ? 0u :
                ((unsigned int)(bat >> 12) * 16u + pixel);
            uint16_t color = (uint16_t)vce[palette_index * 2u] |
                ((uint16_t)vce[palette_index * 2u + 1u] << 8);
            uint32_t rgba = tqr_bgr333_to_rgba(color);
            unsigned char bgr[3] = {
                (unsigned char)(rgba & 0xffu),
                (unsigned char)((rgba >> 8) & 0xffu),
                (unsigned char)((rgba >> 16) & 0xffu)
            };
            if (fwrite(bgr, 1u, sizeof(bgr), file) != sizeof(bgr)) {
                fclose(file);
                return 0;
            }
        }
    }
    return fclose(file) == 0;
}

int main(void)
{
    const char *trace_path = getenv("FIRESTAFF_THERON_E009_TRACE");
    const char *cd_path = getenv("FIRESTAFF_THERON_E009_CD_TRACE");
    const char *track02_path = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    const char *consumer_path = getenv("FIRESTAFF_THERON_E009_CONSUMER_TRACE");
    const char *loader_path = getenv("FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE");
    const char *vdc_path = getenv("FIRESTAFF_THERON_SCSI_GENERATION_VDC_TRACE");
    const char *vram_path = getenv("FIRESTAFF_THERON_GENERATION6_VRAM_SNAPSHOT");
    const char *presentation_vdc_path =
        getenv("FIRESTAFF_THERON_GENERATION7_VDC_TRACE");
    const char *presentation_vram_path =
        getenv("FIRESTAFF_THERON_GENERATION7_VRAM_SNAPSHOT");
    const char *presentation_state_path =
        getenv("FIRESTAFF_THERON_GENERATION7_VDC_STATE");
    const char *presentation_vce_path =
        getenv("FIRESTAFF_THERON_GENERATION7_VCE_SNAPSHOT");
    unsigned char *trace;
    unsigned char *cd_trace;
    unsigned char *track02;
    unsigned char *consumer;
    unsigned char *loader;
    unsigned char *vdc;
    unsigned char *vram;
    unsigned char *presentation_vdc;
    unsigned char *presentation_vram;
    unsigned char *presentation_state;
    unsigned char *presentation_vce;
    size_t trace_size;
    size_t cd_size;
    size_t track02_size;
    size_t consumer_size;
    size_t loader_size;
    size_t vdc_size;
    size_t vram_size;
    size_t presentation_vdc_size;
    size_t presentation_vram_size;
    size_t presentation_state_size;
    size_t presentation_vce_size;
    Theron_V1RawLoaderTraceGameE009DestinationReceipt receipt;
    Theron_V1RawLoaderTraceGameE009ConsumerReceipt consumer_receipt;
    Theron_V1RawLoaderTraceGameE009NextParametersReceipt parameters_receipt;
    Theron_V1RawLoaderTraceGameE009VdcReceipt vdc_receipt;
    Theron_V1RawLoaderTraceGameE009VdcPresentationReceipt presentation_receipt;
    Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt graphics_receipt;
    Theron_V1RawLoaderTraceFileSelectTextSourceReceipt text_receipt;

    if (!trace_path || !cd_path || !track02_path || !consumer_path ||
        !loader_path || !vdc_path || !vram_path || !presentation_vdc_path ||
        !presentation_vram_path || !presentation_state_path ||
        !presentation_vce_path) {
        puts("SKIP: real Theron e009 capture paths are not configured");
        return 77;
    }
    trace = read_file(trace_path, &trace_size);
    cd_trace = read_file(cd_path, &cd_size);
    track02 = read_file(track02_path, &track02_size);
    consumer = read_file(consumer_path, &consumer_size);
    loader = read_file(loader_path, &loader_size);
    vdc = read_file(vdc_path, &vdc_size);
    vram = read_file(vram_path, &vram_size);
    presentation_vdc = read_file(presentation_vdc_path,
        &presentation_vdc_size);
    presentation_vram = read_file(presentation_vram_path,
        &presentation_vram_size);
    presentation_state = read_file(presentation_state_path,
        &presentation_state_size);
    presentation_vce = read_file(presentation_vce_path,
        &presentation_vce_size);
    if (!trace || !cd_trace || !track02 || !consumer || trace_size == 0u ||
        !loader || !vdc || !vram || !presentation_vdc ||
        !presentation_vram || !presentation_state || !presentation_vce ||
        cd_size == 0u ||
        consumer_size == 0u ||
        loader_size == 0u || vdc_size == 0u) {
        free(trace);
        free(cd_trace);
        free(track02);
        free(consumer);
        free(loader);
        free(vdc);
        free(vram);
        free(presentation_vdc);
        free(presentation_vram);
        free(presentation_state);
        free(presentation_vce);
        fputs("FAIL: cannot read real Theron capture bundle\n", stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_game_e009_destination(
            (const char *)trace, (const char *)cd_trace, track02, track02_size,
            THERON_TRACK02_MD5_US_BIN, &receipt) || !receipt.valid ||
        receipt.raw_track02_record != 0x4e0u ||
        receipt.destination != 0x2800u ||
        receipt.destination_physical != 0x1f0800u ||
        receipt.payload_bytes != 2048u ||
        receipt.payload_span_checksum != 0x2723167fu ||
        receipt.payload_checksum != 0x33a90342u ||
        !receipt.asynchronous_resume_observed ||
        !receipt.next_dispatch_completion_observed ||
        !receipt.mode1_payload_verified || receipt.payload_semantics_proven) {
        free(trace);
        free(cd_trace);
        free(track02);
        free(consumer);
        fputs("FAIL: real Theron e009 destination was not media-bound\n", stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_game_e009_consumer(
            &receipt, (const char *)consumer, track02, track02_size,
            THERON_TRACK02_MD5_US_BIN, &consumer_receipt) ||
        !consumer_receipt.valid || !consumer_receipt.source_bytes_verified ||
        !consumer_receipt.read_order_verified ||
        !consumer_receipt.next_dispatch_observed ||
        !consumer_receipt.code_media_verified ||
        !consumer_receipt.address_construction_verified ||
        consumer_receipt.field_semantics_proven ||
        consumer_receipt.values[0] != 0xf9u ||
        consumer_receipt.values[4] != 0x20u ||
        consumer_receipt.code_checksum != 0x048e8620u ||
        consumer_receipt.code_first_record != 0x4c4u ||
        consumer_receipt.code_first_user_offset != 0x7c8u ||
        consumer_receipt.code_second_record != 0x4c5u ||
        consumer_receipt.code_second_user_offset != 0u ||
        consumer_receipt.media_offsets[0] != 0x513u ||
        consumer_receipt.media_offsets[4] != 0x517u) {
        free(trace); free(cd_trace); free(track02); free(consumer);
        fputs("FAIL: real Theron e009 consumer was not media-bound\n", stderr);
        return 1;
    }
    if (consumer_receipt.pointer_base != 0x2803u ||
        consumer_receipt.pointer_stride != 6u ||
        consumer_receipt.resolved_index != 0xd8u ||
        consumer_receipt.resolved_pointer != 0x2d13u) {
        free(trace); free(cd_trace); free(track02); free(consumer);
        fputs("FAIL: authentic consumer address construction is missing\n",
            stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_game_e009_next_parameters(
            &consumer_receipt, (const char *)loader, &parameters_receipt) ||
        !parameters_receipt.valid ||
        !parameters_receipt.consumer_output_writes_verified ||
        !parameters_receipt.tii_verified ||
        !parameters_receipt.next_parameters_verified ||
        parameters_receipt.parameter_semantics_proven ||
        memcmp(parameters_receipt.next_parameters,
            "\x00\x20\x00\x10\x00\x06\xf8\xfe", 8u) != 0) {
        free(trace); free(cd_trace); free(track02); free(consumer); free(loader);
        fprintf(stderr,
            "FAIL: authentic next e009 parameters are not write-bound (valid=%d outputs=%d tii=%d parameters=%d semantics=%d first=%02x)\n",
            parameters_receipt.valid,
            parameters_receipt.consumer_output_writes_verified,
            parameters_receipt.tii_verified,
            parameters_receipt.next_parameters_verified,
            parameters_receipt.parameter_semantics_proven,
            parameters_receipt.next_parameters[0]);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters_receipt, (const char *)cd_trace, (const char *)vdc,
            track02, track02_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &vdc_receipt) ||
        !vdc_receipt.valid || vdc_receipt.scsi_generation != 6u ||
        vdc_receipt.scsi_lba != 5018u || vdc_receipt.scsi_sector_count != 4u ||
        vdc_receipt.first_raw_track02_record != 0x7d9u ||
        vdc_receipt.payload_bytes != 8192u ||
        vdc_receipt.payload_checksum != 0x4859675du ||
        vdc_receipt.vdc_write_pc != 0xeb35u ||
        vdc_receipt.vdc_write_physical_pc != 0x000b35u ||
        vdc_receipt.vdc_payload_writes != 8192u ||
        vdc_receipt.first_vram_word != 0x1000u ||
        vdc_receipt.last_vram_word != 0x1fffu ||
        vdc_receipt.vram_word_count != 4096u ||
        vdc_receipt.vram_snapshot_checksum != 0x4859675du ||
        !vdc_receipt.read6_verified || !vdc_receipt.vdc_setup_verified ||
        vdc_receipt.repeated_word_writes_verified ||
        !vdc_receipt.single_word_writes_verified ||
        !vdc_receipt.vram_destination_verified ||
        !vdc_receipt.vram_snapshot_verified ||
        !vdc_receipt.mode1_payload_verified ||
        vdc_receipt.payload_semantics_proven) {
        fputs("FAIL: authentic generation-6 VDC payload is not media-bound\n",
            stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_game_e009_vdc_presentation(
            &vdc_receipt, (const char *)presentation_vdc,
            (const char *)presentation_state, vram, vram_size,
            presentation_vram, presentation_vram_size,
            presentation_vce, presentation_vce_size,
            track02, track02_size, THERON_TRACK02_MD5_US_BIN,
            &presentation_receipt) || !presentation_receipt.valid ||
        presentation_receipt.scsi_generation != 7u ||
        presentation_receipt.vdc_rows != 2187u ||
        presentation_receipt.vwr_commits != 1024u ||
        presentation_receipt.pre_vram_checksum != 0xedfc7797u ||
        presentation_receipt.post_vram_checksum != 0xd9d48117u ||
        presentation_receipt.bat_checksum != 0x4740a645u ||
        presentation_receipt.first_source_tile != 0x110u ||
        presentation_receipt.last_source_tile != 0x18fu ||
        presentation_receipt.active_bat_cells != 960u ||
        presentation_receipt.source_backed_active_cells != 960u ||
        presentation_receipt.unique_source_tiles != 124u ||
        presentation_receipt.background_index_pixels != 61440u ||
        presentation_receipt.nonzero_background_pixels != 3373u ||
        presentation_receipt.background_index_checksum != 0x2c2cfb4du ||
        presentation_receipt.vce_checksum != 0x1f116dc5u ||
        presentation_receipt.palette_zero_checksum != 0x0b2ae445u ||
        presentation_receipt.background_color_checksum != 0x3fde1dc5u ||
        presentation_receipt.code_snapshot_checksum != 0x3e3745f7u ||
        presentation_receipt.code_track02_record != 0x4d0u ||
        presentation_receipt.code_track02_user_offset != 0x600u ||
        presentation_receipt.source_code_bytes != 435u ||
        presentation_receipt.tia_pc != 0x468cu ||
        presentation_receipt.tia_source != 0x47e0u ||
        presentation_receipt.tia_destination != 0x0002u ||
        presentation_receipt.tia_length != 0x0040u ||
        presentation_receipt.generated_bat_row_checksum != 0xda633f05u ||
        !presentation_receipt.vdc_replay_verified ||
        !presentation_receipt.background_enabled ||
        !presentation_receipt.active_bat_source_verified ||
        !presentation_receipt.background_pixels_verified ||
        !presentation_receipt.vce_palette_verified ||
        !presentation_receipt.code_media_verified ||
        !presentation_receipt.stage2_l466b_verified ||
        !presentation_receipt.self_modifying_tia_verified ||
        presentation_receipt.tile_semantics_proven) {
        fputs("FAIL: authentic generation-7 BAT presentation is not source-bound\n",
            stderr);
        return 1;
    }
    if (getenv("FIRESTAFF_THERON_GENERATION7_BACKGROUND_BMP") &&
        !write_generation7_background_bmp(
            getenv("FIRESTAFF_THERON_GENERATION7_BACKGROUND_BMP"),
            presentation_vram, presentation_vce)) {
        fputs("FAIL: cannot write source-backed generation-7 background BMP\n",
            stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_file_select_text_source(
            track02, track02_size, THERON_TRACK02_MD5_US_BIN, &text_receipt) ||
        !text_receipt.valid || text_receipt.occurrence_count != 3u ||
        text_receipt.raw_track02_record[0] != 0x4eau ||
        text_receipt.raw_track02_record[1] != 0x4ecu ||
        text_receipt.raw_track02_record[2] != 0x4eeu ||
        text_receipt.play_prompt_checksum != 0xef1550adu ||
        text_receipt.load_prompt_checksum != 0xaa654403u ||
        !text_receipt.mode1_coordinates_verified ||
        !text_receipt.source_text_verified || text_receipt.screen_consumer_proven) {
        fputs("FAIL: authentic file-select source text is not bound\n", stderr);
        return 1;
    }
    track02[2959246u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_text_source(
            track02, track02_size, THERON_TRACK02_MD5_US_BIN, &text_receipt)) {
        fputs("FAIL: file-select source binder accepted corrupted media\n", stderr);
        return 1;
    }
    track02[2959246u] ^= 1u;
    if (!theron_v1_raw_loader_trace_bind_game_generation49_graphics(
            &presentation_receipt, (const char *)cd_trace,
            (const char *)presentation_vdc, track02, track02_size,
            THERON_TRACK02_MD5_US_BIN, &graphics_receipt) ||
        !graphics_receipt.valid || graphics_receipt.scsi_generation != 49u ||
        graphics_receipt.scsi_lba != 4622u ||
        graphics_receipt.scsi_sector_count != 12u ||
        graphics_receipt.first_raw_track02_record != 0x64du ||
        graphics_receipt.payload_bytes != 24576u ||
        graphics_receipt.payload_checksum != 0x01551f76u ||
        graphics_receipt.vdc_rows != 24580u ||
        graphics_receipt.vdc_payload_rows != 24576u ||
        graphics_receipt.first_vram_word != 0x1000u ||
        graphics_receipt.last_vram_word != 0x6fffu ||
        !graphics_receipt.read6_verified ||
        !graphics_receipt.vdc_setup_verified ||
        graphics_receipt.repeated_writes_verified ||
        !graphics_receipt.single_writes_verified ||
        !graphics_receipt.media_bytes_verified ||
        graphics_receipt.graphics_semantics_proven) {
        fputs("FAIL: authentic generation-49 graphics transport is not bound\n",
            stderr);
        return 1;
    }
    track02[(size_t)0x64du * 2352u + 16u] ^= 1u;
    {
        Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt corrupt_receipt;
        if (theron_v1_raw_loader_trace_bind_game_generation49_graphics(
                &presentation_receipt, (const char *)cd_trace,
                (const char *)presentation_vdc, track02, track02_size,
                THERON_TRACK02_MD5_US_BIN, &corrupt_receipt)) {
            fputs("FAIL: generation-49 binder accepted corrupted real media\n",
                stderr);
            return 1;
        }
    }
    track02[(size_t)0x64du * 2352u + 16u] ^= 1u;
    {
        const char *frame_vdc_path =
            getenv("FIRESTAFF_THERON_GENERATION51_VDC_TRACE");
        const char *frame_state_path =
            getenv("FIRESTAFF_THERON_GENERATION51_VDC_STATE");
        const char *frame_vram_path =
            getenv("FIRESTAFF_THERON_GENERATION51_VRAM_SNAPSHOT");
        const char *frame_vce_path =
            getenv("FIRESTAFF_THERON_GENERATION51_VCE_SNAPSHOT");
        const char *frame_sat_path =
            getenv("FIRESTAFF_THERON_GENERATION51_SAT_SNAPSHOT");
        int configured = !!frame_vdc_path + !!frame_state_path +
            !!frame_vram_path + !!frame_vce_path + !!frame_sat_path;
        if (configured != 0) {
            unsigned char *frame_vdc, *frame_state, *frame_vram;
            unsigned char *frame_vce, *frame_sat;
            size_t frame_vdc_size, frame_state_size, frame_vram_size;
            size_t frame_vce_size, frame_sat_size;
            Theron_V1RawLoaderTraceGameGeneration51FrameReceipt frame_receipt;
            if (configured != 5) {
                fputs("FAIL: generation-51 atomic bundle is incomplete\n", stderr);
                return 1;
            }
            frame_vdc = read_file(frame_vdc_path, &frame_vdc_size);
            frame_state = read_file(frame_state_path, &frame_state_size);
            frame_vram = read_file(frame_vram_path, &frame_vram_size);
            frame_vce = read_file(frame_vce_path, &frame_vce_size);
            frame_sat = read_file(frame_sat_path, &frame_sat_size);
            if (!frame_vdc || !frame_state || !frame_vram || !frame_vce ||
                !frame_sat || !theron_v1_raw_loader_trace_bind_game_generation51_frame(
                    &graphics_receipt, (const char *)frame_vdc,
                    (const char *)frame_state, frame_vram, frame_vram_size,
                    frame_vce, frame_vce_size, frame_sat, frame_sat_size,
                    track02, track02_size, THERON_TRACK02_MD5_US_BIN,
                    &frame_receipt) || !frame_receipt.valid ||
                frame_receipt.scsi_generation != 51u ||
                frame_receipt.generation_rows != 54842u ||
                frame_receipt.boundary_sequence != 94434u ||
                frame_receipt.vram_checksum != 0xde27fc7eu ||
                frame_receipt.vce_checksum != 0x88629e93u ||
                frame_receipt.sat_checksum != 0x4d7705c5u ||
                frame_receipt.l466b_writer_rows != 2176u ||
                frame_receipt.l4943_writer_rows != 15u ||
                frame_receipt.l50f1_writer_rows != 2064u ||
                frame_receipt.l5111_writer_rows != 2048u ||
                !frame_receipt.atomic_snapshot_verified ||
                !frame_receipt.stage2_control_flow_verified ||
                !frame_receipt.stable_loop_boundary_verified ||
                frame_receipt.screen_semantics_proven) {
                fputs("FAIL: generation-51 frame/control-flow receipt rejected\n",
                    stderr);
                return 1;
            }
            frame_vram[0] ^= 1u;
            if (theron_v1_raw_loader_trace_bind_game_generation51_frame(
                    &graphics_receipt, (const char *)frame_vdc,
                    (const char *)frame_state, frame_vram, frame_vram_size,
                    frame_vce, frame_vce_size, frame_sat, frame_sat_size,
                    track02, track02_size, THERON_TRACK02_MD5_US_BIN,
                    &frame_receipt)) {
                fputs("FAIL: frame binder accepted corrupted atomic VRAM\n",
                    stderr);
                return 1;
            }
            free(frame_vdc); free(frame_state); free(frame_vram);
            free(frame_vce); free(frame_sat);
        }
    }
    presentation_vram[0] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_presentation(
            &vdc_receipt, (const char *)presentation_vdc,
            (const char *)presentation_state, vram, vram_size,
            presentation_vram, presentation_vram_size,
            presentation_vce, presentation_vce_size,
            track02, track02_size, THERON_TRACK02_MD5_US_BIN,
            &presentation_receipt)) {
        fputs("FAIL: binder accepted corrupted generation-7 VRAM snapshot\n",
            stderr);
        return 1;
    }
    presentation_vram[0] ^= 1u;
    {
        char *row = strstr((char *)vdc,
            "logical_address=0002 physical_address=1fe002 value=00 writer_pc=eb35");
        char *value = row ? strstr(row, "value=00") : NULL;
        if (!value) {
            fputs("FAIL: authentic generation-6 VDC row is missing\n", stderr);
            return 1;
        }
        value[7] = '1';
        if (theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
                &parameters_receipt, (const char *)cd_trace,
                (const char *)vdc, track02, track02_size, vram, vram_size,
                THERON_TRACK02_MD5_US_BIN, &vdc_receipt)) {
            fputs("FAIL: binder accepted corrupted generation-6 VDC byte\n",
                stderr);
            return 1;
        }
        value[7] = '0';
    }
    track02[(size_t)0x7d9u * 2352u + 16u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters_receipt, (const char *)cd_trace, (const char *)vdc,
            track02, track02_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &vdc_receipt)) {
        fputs("FAIL: binder accepted corrupted generation-6 Track02 byte\n",
            stderr);
        return 1;
    }
    track02[(size_t)0x7d9u * 2352u + 16u] ^= 1u;
    vram[0x2000u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters_receipt, (const char *)cd_trace, (const char *)vdc,
            track02, track02_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &vdc_receipt)) {
        fputs("FAIL: binder accepted corrupted generation-6 VRAM snapshot\n",
            stderr);
        return 1;
    }
    vram[0x2000u] ^= 1u;
    {
        char *row = strstr((char *)loader,
            "logical_destination=20fe physical_destination=1f00fe value=f8 writer_pc=383d");
        char *value = row ? strstr(row, "value=f8") : NULL;
        if (!value) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            free(loader);
            fputs("FAIL: authentic next-parameter row is missing\n", stderr);
            return 1;
        }
        value[7] = '9';
        if (theron_v1_raw_loader_trace_bind_game_e009_next_parameters(
                &consumer_receipt, (const char *)loader,
                &parameters_receipt)) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            free(loader);
            fputs("FAIL: binder accepted corrupted next-parameter write\n",
                stderr);
            return 1;
        }
        value[7] = '8';
    }
    {
        char *byte = strstr((char *)consumer,
            "logical_address=2d13 physical_address=1f0d13 value=f9");
        char *value = byte ? strstr(byte, "value=f9") : NULL;
        if (!value) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            fputs("FAIL: real consumer byte row is missing\n", stderr);
            return 1;
        }
        value[7] = '8';
        if (theron_v1_raw_loader_trace_bind_game_e009_consumer(
                &receipt, (const char *)consumer, track02, track02_size,
                THERON_TRACK02_MD5_US_BIN, &consumer_receipt)) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            fputs("FAIL: binder accepted corrupted real consumer byte\n", stderr);
            return 1;
        }
        value[7] = '9';
    }
    {
        char *code = strstr((char *)consumer,
            "logical_address=37c8 physical_address=1f17c8 value=91");
        char *value = code ? strstr(code, "value=91") : NULL;
        if (!value) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            fputs("FAIL: authentic consumer code row is missing\n", stderr);
            return 1;
        }
        value[7] = '0';
        if (theron_v1_raw_loader_trace_bind_game_e009_consumer(
                &receipt, (const char *)consumer, track02, track02_size,
                THERON_TRACK02_MD5_US_BIN, &consumer_receipt)) {
            free(trace); free(cd_trace); free(track02); free(consumer);
            fputs("FAIL: binder accepted corrupted captured consumer code\n",
                stderr);
            return 1;
        }
        value[7] = '1';
    }
    track02[(size_t)0x4c4u * 2352u + 16u + 0x7c8u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_consumer(
            &receipt, (const char *)consumer, track02, track02_size,
            THERON_TRACK02_MD5_US_BIN, &consumer_receipt)) {
        free(trace); free(cd_trace); free(track02); free(consumer);
        fputs("FAIL: binder accepted corrupted authentic consumer code\n",
            stderr);
        return 1;
    }
    track02[(size_t)0x4c4u * 2352u + 16u + 0x7c8u] ^= 1u;
    track02[(size_t)0x4e0u * 2352u + 16u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_destination(
            (const char *)trace, (const char *)cd_trace, track02, track02_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(trace);
        free(cd_trace);
        free(track02);
        free(consumer);
        fputs("FAIL: binder accepted corrupted real MODE1 payload\n", stderr);
        return 1;
    }
    track02[(size_t)0x4e0u * 2352u + 16u] ^= 1u;
    {
        char *hash = strstr((char *)trace, "payload_fnv1a=33a90342");
        if (!hash) {
            free(trace);
            free(cd_trace);
            free(track02);
            free(consumer);
            fputs("FAIL: real payload checksum row is missing\n", stderr);
            return 1;
        }
        hash[17] = '3';
        if (theron_v1_raw_loader_trace_bind_game_e009_destination(
                (const char *)trace, (const char *)cd_trace, track02,
                track02_size, THERON_TRACK02_MD5_US_BIN, &receipt)) {
            free(trace);
            free(cd_trace);
                free(track02);
                free(consumer);
            fputs("FAIL: binder accepted corrupted real RAM checksum\n", stderr);
            return 1;
        }
    }
    free(trace);
    free(cd_trace);
    free(track02);
    free(consumer);
    free(loader);
    free(vdc);
    free(vram);
    free(presentation_vdc);
    free(presentation_vram);
    free(presentation_state);
    free(presentation_vce);
    puts("PASS: real Theron e009 payload and generation-7 presentation are bound");
    return 0;
}
