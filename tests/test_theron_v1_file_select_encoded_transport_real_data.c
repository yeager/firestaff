#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *bytes;

    *out_size = 0u;
    file = path ? fopen(path, "rb") : NULL;
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
    bytes[length] = 0;
    *out_size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *media_path = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    const char *cd_path = getenv("FIRESTAFF_THERON_FILE_SELECT_CD_TRACE");
    const char *write_path =
        getenv("FIRESTAFF_THERON_FILE_SELECT_SOURCE_WRITE_TRACE");
    const char *read_path =
        getenv("FIRESTAFF_THERON_FILE_SELECT_SOURCE_READ_TRACE");
    const char *consumer_path =
        getenv("FIRESTAFF_THERON_FILE_SELECT_CONSUMER_READ_TRACE");
    const char *vdc_path = getenv("FIRESTAFF_THERON_FILE_SELECT_VDC_TRACE");
    unsigned char *media, *cd, *write_trace, *read_trace, *consumer_trace, *vdc;
    size_t media_size, cd_size, write_size, read_size, consumer_size, vdc_size;
    char *corruption;
    Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt receipt;

    if (!media_path || !cd_path || !write_path || !read_path ||
        !consumer_path || !vdc_path) {
        puts("SKIP: authentic file-select transport inputs are not configured");
        return 77;
    }
    media = read_file(media_path, &media_size);
    cd = read_file(cd_path, &cd_size);
    write_trace = read_file(write_path, &write_size);
    read_trace = read_file(read_path, &read_size);
    consumer_trace = read_file(consumer_path, &consumer_size);
    vdc = read_file(vdc_path, &vdc_size);
    if (!media || !cd || !write_trace || !read_trace || !cd_size ||
        !consumer_trace || !vdc || !write_size || !read_size ||
        !consumer_size || !vdc_size) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: could not read authentic transport inputs\n", stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_file_select_encoded_transport(
            (const char *)cd, (const char *)write_trace,
            (const char *)read_trace, (const char *)consumer_trace,
            (const char *)vdc, media, media_size,
            THERON_TRACK02_MD5_US_BIN, &receipt) || !receipt.valid ||
        receipt.scsi_generation != 12u || receipt.scsi_lba != 4668u ||
        receipt.raw_track02_record != 0x67bu || receipt.byte_count != 195u ||
        receipt.raw_sector_checksum != 0xfcc73c77u ||
        receipt.payload_checksum != 0xefad54b3u ||
        receipt.loader_pc != 0xea9eu ||
        receipt.loader_physical_pc != 0x000a9eu ||
        receipt.source_physical_first != 0x0ddc5bu ||
        receipt.transfer_pc != 0x3446u ||
        receipt.transfer_physical_pc != 0x1f1446u ||
        receipt.destination_physical_first != 0x0d1d3du ||
        !receipt.read6_verified || !receipt.media_to_source_ram_verified ||
        !receipt.source_to_destination_ram_verified ||
        !receipt.destination_consumer_verified ||
        !receipt.destination_to_vdc_verified ||
        receipt.presentation_source_reader_pc != 0x514bu ||
        receipt.vdc_writer_pc != 0x5110u || receipt.vdc_setup_rows != 13u ||
        receipt.vdc_payload_rows != 512u ||
        receipt.vdc_payload_checksum != 0xa8007f15u ||
        receipt.first_vram_word != 0x0800u ||
        receipt.last_vram_word != 0x08ffu ||
        receipt.vram_word_count != 256u ||
        !receipt.vdc_destination_replay_verified ||
        receipt.text_or_glyph_semantics_proven) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: authentic encoded transport receipt rejected\n", stderr);
        return 1;
    }
    media[3901984u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_encoded_transport(
            (const char *)cd, (const char *)write_trace,
            (const char *)read_trace, (const char *)consumer_trace,
            (const char *)vdc, media, media_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: corrupted authentic media was accepted\n", stderr);
        return 1;
    }
    media[3901984u] ^= 1u;
    corruption = strstr((char *)vdc,
        "sequence=13 frame=8580 logical_address=0002 physical_address=1fe002 value=30");
    if (!corruption || !(corruption = strstr(corruption, "value=30"))) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: authentic first VWR row was not found\n", stderr);
        return 1;
    }
    corruption[6] = '2';
    if (theron_v1_raw_loader_trace_bind_file_select_encoded_transport(
            (const char *)cd, (const char *)write_trace,
            (const char *)read_trace, (const char *)consumer_trace,
            (const char *)vdc, media, media_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: corrupted VWR row was accepted\n", stderr);
        return 1;
    }
    corruption[6] = '3';
    corruption = strstr((char *)write_trace,
        "physical_address=0ddc5b value=08 writer_pc=ea9e");
    if (!corruption) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: authentic loader row was not found\n", stderr);
        return 1;
    }
    corruption = strstr(corruption, "value=08");
    if (!corruption) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: authentic loader value was not found\n", stderr);
        return 1;
    }
    corruption[6] = '9';
    if (theron_v1_raw_loader_trace_bind_file_select_encoded_transport(
            (const char *)cd, (const char *)write_trace,
            (const char *)read_trace, (const char *)consumer_trace,
            (const char *)vdc, media, media_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(write_trace); free(read_trace);
        free(consumer_trace); free(vdc);
        fputs("FAIL: corrupted loader row was accepted\n", stderr);
        return 1;
    }
    free(media); free(cd); free(write_trace); free(read_trace);
    free(consumer_trace); free(vdc);
    puts("PASS: authentic Theron file-select encoded transport is media-bound");
    return 0;
}
