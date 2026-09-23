#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *file = path ? fopen(path, "rb") : NULL;
    long length;
    unsigned char *bytes;
    *size = 0u;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file); bytes[length] = 0; *size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *paths[4] = {
        getenv("FIRESTAFF_THERON_FILE_SELECT_DRIVER_CODE_SNAPSHOT"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_DRIVER_RAM_SNAPSHOT"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_SCROLL_RAM_TRACE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_SCROLL_CONTROL_TRACE")
    };
    unsigned char *data[4] = {0};
    size_t sizes[4] = {0}, i;
    Theron_V1RawLoaderTraceFileSelectScrollReceipt scroll;
    Theron_V1RawLoaderTraceFileSelectScrollDriverReceipt receipt;

    for (i = 0u; i < 4u; ++i) if (!paths[i]) {
        puts("SKIP: authentic file-select scroll-driver inputs are not configured");
        return 77;
    }
    for (i = 0u; i < 4u; ++i) if (!(data[i] = read_file(paths[i], &sizes[i]))) {
        fputs("FAIL: could not read file-select scroll-driver inputs\n", stderr);
        while (i) free(data[--i]);
        return 1;
    }
    memset(&scroll, 0, sizeof(scroll)); scroll.valid = 1;
    scroll.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(scroll.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    scroll.update_frame = 8580u; scroll.game_byr_write_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_file_select_scroll_driver(
            &scroll, data[0], sizes[0], data[1], sizes[1],
            (const char *)data[2], (const char *)data[3], &receipt) ||
        !receipt.valid || receipt.code_checksum != 0x0408d000u ||
        receipt.ram_checksum != 0x6908b113u ||
        receipt.byr_source_address != 0x2210u || receipt.byr_load_pc != 0x4993u ||
        receipt.scroll_writer_pc != 0x4184u ||
        receipt.countdown_low_writer_pc != 0x4a7bu ||
        receipt.countdown_high_writer_pc != 0x4a80u ||
        receipt.first_scroll_frame != 8522u ||
        receipt.presented_scroll_frame != 8580u ||
        receipt.final_scroll_frame != 9666u ||
        receipt.final_countdown_frame != 9668u ||
        receipt.update_interval_frames != 8u || receipt.scroll_updates != 144u ||
        receipt.countdown_updates != 144u || receipt.initial_byr != 0x00f0u ||
        receipt.final_byr != 0x0060u || receipt.initial_countdown != 0x0090u ||
        receipt.final_countdown != 0u ||
        receipt.next_phase_start_frame != 10632u ||
        receipt.next_phase_final_frame != 11274u ||
        receipt.next_phase_reset_frame != 11578u ||
        receipt.next_phase_interval_frames != 10u ||
        receipt.next_phase_updates != 64u ||
        receipt.next_phase_initial_countdown != 0x0040u ||
        receipt.next_phase_final_countdown != 0u ||
        receipt.next_phase_low_writer_pc != 0x4b1bu ||
        receipt.next_phase_high_writer_pc != 0x4b20u ||
        !receipt.code_path_verified ||
        !receipt.ram_snapshot_verified || !receipt.signed_scroll_loop_verified ||
        !receipt.zero_stop_verified || !receipt.next_control_phase_verified ||
        receipt.screen_semantics_proven) {
        fputs("FAIL: authentic file-select scroll driver rejected\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    data[0][0x0175u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_scroll_driver(
            &scroll, data[0], sizes[0], data[1], sizes[1],
            (const char *)data[2], (const char *)data[3], &receipt)) {
        fputs("FAIL: corrupted scroll-driver code accepted\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    data[0][0x0175u] ^= 1u;
    {
        char *needle = strstr((char *)data[3], "sequence=286 frame=9668");
        if (!needle) {
            fputs("FAIL: authentic zero-stop control row missing\n", stderr);
            for (i = 0u; i < 4u; ++i) free(data[i]);
            return 1;
        }
        needle[9] = '5';
    }
    if (theron_v1_raw_loader_trace_bind_file_select_scroll_driver(
            &scroll, data[0], sizes[0], data[1], sizes[1],
            (const char *)data[2], (const char *)data[3], &receipt)) {
        fputs("FAIL: corrupted zero-stop control row accepted\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    {
        char *needle = strstr((char *)data[3], "sequence=586 frame=9668");
        char *next_stop;
        if (!needle) {
            fputs("FAIL: modified first zero-stop row missing\n", stderr);
            for (i = 0u; i < 4u; ++i) free(data[i]);
            return 1;
        }
        needle[9] = '2';
        next_stop = strstr((char *)data[3], "sequence=2472 frame=11274");
        if (!next_stop) {
            fputs("FAIL: authentic second zero-stop row missing\n", stderr);
            for (i = 0u; i < 4u; ++i) free(data[i]);
            return 1;
        }
        next_stop[9] = '3';
    }
    if (theron_v1_raw_loader_trace_bind_file_select_scroll_driver(
            &scroll, data[0], sizes[0], data[1], sizes[1],
            (const char *)data[2], (const char *)data[3], &receipt)) {
        fputs("FAIL: corrupted second control phase accepted\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    for (i = 0u; i < 4u; ++i) free(data[i]);
    puts("PASS: authentic file-select scroll driver and zero stop are bound");
    return 0;
}
