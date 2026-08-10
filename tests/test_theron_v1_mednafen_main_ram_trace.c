#include "theron_v1_mednafen_main_ram_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

static int test_escaped_newline_normalization(void) {
#if defined(_WIN32)
    return 1;
#else
    char path[] = "/tmp/firestaff-theron-mainram-XXXXXX";
    const char *trace =
        "source=mednafen-pce-instrumented-main-ram-loader\\n"
        "main_ram_loader_block_transfer logical_pc=2286 physical_pc=1f0286 operation=tia source=c800 destination=0404 length=80\\n"
        "main_ram_loader_rts logical_pc=228d physical_pc=1f028d\\n"
        "main_ram_loader_post_rts source_logical_pc=228d source_physical_pc=1f028d logical_pc=2286 physical_pc=1f0286 opcode=e3\\n"
        "main_ram_loader_write sequence=0 dispatch_sequence=unbound logical_destination=3084 physical_destination=1f1084 value=00 writer_pc=318b writer_physical_pc=1f118b\\n"
        "_pc=1f118b\\n";
    Theron_V1MednafenMainRamTraceReceipt receipt;
    int fd = mkstemp(path);
    FILE *file;
    int result;

    if (fd < 0) return 0;
    file = fdopen(fd, "wb");
    if (!file) {
        close(fd);
        unlink(path);
        return 0;
    }
    if (fputs(trace, file) == EOF || fclose(file) != 0) {
        unlink(path);
        return 0;
    }
    result = theron_v1_mednafen_main_ram_trace_parse_file(path, &receipt) &&
             receipt.status == THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY &&
             receipt.first_length == 0x80u &&
             receipt.vce_tia_transfer_count == 1u &&
             receipt.vce_tia_coordinates_verified &&
             receipt.vce_tia_source == 0xc800u &&
             receipt.vce_tia_destination == 0x0404u &&
             receipt.vce_tia_length == 0x80u;
    unlink(path);
    return result;
#endif
}

static const char *resolve_real_trace_path(char *path, size_t path_size) {
    const char *configured = getenv("THERON_MEDNAFEN_MAIN_RAM_TRACE");
    const char *home = getenv("HOME");

    if (configured && configured[0]) return configured;
#if !defined(_WIN32)
    if (home && home[0] && path && path_size > 0u) {
        char directory_path[512];
        DIR *directory;
        struct dirent *entry;

        if (snprintf(directory_path, sizeof(directory_path),
                     "%s/.firestaff/firestaff-probe-screenshots", home) < 0)
            return NULL;
        directory = opendir(directory_path);
        if (!directory) return NULL;
        while ((entry = readdir(directory)) != NULL) {
            size_t name_length = strlen(entry->d_name);
            const char *suffix = ".trace.main-ram-loader";
            size_t suffix_length = strlen(suffix);
            struct stat info;

            if (name_length <= suffix_length ||
                strcmp(entry->d_name + name_length - suffix_length, suffix) != 0)
                continue;
            if (snprintf(path, path_size, "%s/%s", directory_path,
                         entry->d_name) < 0 ||
                stat(path, &info) != 0 || !S_ISREG(info.st_mode))
                continue;
            /* Prefer a real, parseable capture. Older failed captures can
             * remain beside the valid one and must not become the default. */
            Theron_V1MednafenMainRamTraceReceipt candidate;
            if (theron_v1_mednafen_main_ram_trace_parse_file(path, &candidate) &&
                candidate.status == THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY) {
                closedir(directory);
                return path;
            }
        }
        closedir(directory);
    }
#else
    (void)path;
    (void)path_size;
#endif
    return NULL;
}

int main(void) {
    char discovered_path[512];
    const char *path = resolve_real_trace_path(discovered_path,
                                               sizeof(discovered_path));
    Theron_V1MednafenMainRamTraceReceipt receipt;

    if (!test_escaped_newline_normalization()) {
        fprintf(stderr, "FAIL: escaped newline trace normalization\n");
        return 1;
    }

    if (!path || !path[0]) {
        puts("SKIP: no real Theron Main-RAM loader capture was supplied");
        return 77;
    }
    if (!theron_v1_mednafen_main_ram_trace_parse_file(path, &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY ||
        !receipt.source_trace_md5_verified || !receipt.source_header_verified ||
        !receipt.transfer_coordinates_verified || receipt.target_2600_bytes_present ||
        receipt.semantic_publication_allowed || receipt.first_length != 0x80u ||
        receipt.vce_tia_transfer_count < 1u ||
        !receipt.vce_tia_coordinates_verified ||
        receipt.vce_tia_source != 0xc800u ||
        receipt.vce_tia_destination != 0x0404u ||
        receipt.vce_tia_length != 0x80u) {
        fprintf(stderr, "FAIL: Main-RAM loader trace was not accepted\n");
        return 1;
    }
    printf("PASS: md5=%s transfers=%u rts=%u post_rts=%u "
           "pc=%x physical=%x source=%x destination=%x length=%x "
           "target_2600=absent semantic_publication=blocked\n",
           receipt.source_trace_md5, receipt.block_transfer_count,
           receipt.rts_count, receipt.post_rts_count, receipt.first_logical_pc,
           receipt.first_physical_pc, receipt.first_source,
           receipt.first_destination, receipt.first_length);
    return 0;
}
