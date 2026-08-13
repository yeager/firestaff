#include "theron_v1_mednafen_main_ram_consumer_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static int test_escaped_newline_trace(void) {
#if defined(_WIN32)
    return 1;
#else
    char path[512];
    const char *tmpdir = getenv("TMPDIR");
    const char *trace =
        "source=mednafen-pce-instrumented-main-ram-consumer\\n"
        "main_ram_consumer_read sequence=0 logical_address=2c54 physical_address=1f2c54 value=ad reader_pc=2c54 reader_physical_pc=1f2c54\\n"
        "main_ram_consumer_read sequence=1 logical_address=2c55 physical_address=1f2c55 value=08 reader_pc=2c54 reader_physical_pc=1f2c54\\n";
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;
    static const unsigned char code[] = { 0xad };
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    if (snprintf(path, sizeof(path), "%s/firestaff-theron-consumer-XXXXXX",
                 tmpdir) <= 0) return 0;
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
    result = theron_v1_mednafen_main_ram_consumer_trace_parse_file(path, &receipt) &&
             receipt.status == THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY &&
             receipt.read_count == 2u &&
             theron_v1_mednafen_main_ram_consumer_trace_verify_code_window(
                 path, 0x2c54u, code, sizeof(code));
    unlink(path);
    return result;
#endif
}

static int test_code_bank_reader_trace(void) {
#if defined(_WIN32)
    return 1;
#else
    char path[512];
    const char *tmpdir = getenv("TMPDIR");
    const char *trace =
        "source=mednafen-pce-instrumented-main-ram-consumer\n"
        "main_ram_consumer_read sequence=0 logical_address=21f9 physical_address=1f01f9 value=46 reader_pc=4630 reader_physical_pc=0d0630\n";
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    if (snprintf(path, sizeof(path), "%s/firestaff-theron-consumer-bank-XXXXXX",
                 tmpdir) <= 0) return 0;
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
    result = theron_v1_mednafen_main_ram_consumer_trace_parse_file(path, &receipt) &&
             receipt.status == THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY &&
             receipt.first_reader_physical_pc == 0x0d0630u;
    unlink(path);
    return result;
#endif
}

static int test_target_window_provenance(void) {
#if defined(_WIN32)
    return 1;
#else
    char path[512];
    const char *tmpdir = getenv("TMPDIR");
    const char *trace =
        "source=mednafen-pce-instrumented-main-ram-consumer\n"
        "main_ram_consumer_read sequence=0 logical_address=25ff physical_address=1f05ff value=00 reader_pc=cb22 reader_physical_pc=002b22 a=01 x=ff y=00 sp=fa p=04\n"
        "main_ram_consumer_read sequence=1 logical_address=2600 physical_address=1f0600 value=00 reader_pc=cb22 reader_physical_pc=002b22 a=01 x=ff y=00 sp=fa p=04\n"
        "main_ram_consumer_read sequence=2 logical_address=271e physical_address=1f071e value=df reader_pc=c3f1 reader_physical_pc=0d23f1 a=a0 x=00 y=06 sp=f7 p=90\n";
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    if (snprintf(path, sizeof(path), "%s/firestaff-theron-target-window-XXXXXX",
                 tmpdir) <= 0) return 0;
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
    result = theron_v1_mednafen_main_ram_consumer_trace_parse_file(path, &receipt) &&
             receipt.status == THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY &&
             receipt.target_2600_bytes_present &&
             receipt.target_2600_read_count == 2u &&
             receipt.target_2600_nonzero_read_count == 1u &&
             receipt.target_2600_distinct_reader_pc_count == 2u &&
             receipt.target_2600_init_read_count == 1u &&
             receipt.target_2600_runtime_read_count == 1u &&
             receipt.target_2600_c3a0_read_count == 1u &&
             receipt.target_2600_c3a0_nonzero_read_count == 1u &&
             receipt.target_2600_c3a0_distinct_reader_pc_count == 1u &&
             !receipt.semantic_publication_allowed;
    unlink(path);
    return result;
#endif
}

int main(void) {
    const char *path = getenv("THERON_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE");
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;
    static const unsigned char code_window[] = {
        0xad, 0x08, 0x30, 0x18, 0x6d, 0xf5, 0xff, 0x53,
        0x04, 0xad, 0x09, 0x30, 0x18, 0x6d, 0xf5, 0xff,
        0x53, 0x08, 0xad, 0x0a, 0x30, 0x18
    };
    unsigned char wrong_window[sizeof(code_window)];

    if (!test_escaped_newline_trace()) {
        fprintf(stderr, "FAIL: escaped newline consumer trace normalization\n");
        return 1;
    }
    if (!test_code_bank_reader_trace()) {
        fprintf(stderr, "FAIL: HuC6280 code-bank reader address\n");
        return 1;
    }
    if (!test_target_window_provenance()) {
        fprintf(stderr, "FAIL: target RAM window provenance retention\n");
        return 1;
    }

    if (!path || !path[0]) {
        puts("SKIP: THERON_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE is not set");
        return 77;
    }
    if (!theron_v1_mednafen_main_ram_consumer_trace_parse_file(path, &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY ||
        !receipt.source_trace_md5_verified || !receipt.source_header_verified ||
        !receipt.bank_coordinates_verified ||
        receipt.semantic_publication_allowed || receipt.read_count == 0u) {
        fprintf(stderr, "FAIL: real Mednafen consumer trace was not accepted\n");
        return 1;
    }
    printf("PASS: md5=%s reads=%u first_physical=%x last_physical=%x "
           "first_reader=%x last_reader=%x target_2600=%s target_reads=%u "
           "target_nonzero=%u target_readers=%u "
           "target_init=%u target_runtime=%u target_c3a0=%u "
           "target_c3a0_nonzero=%u target_c3a0_readers=%u "
           "semantic_publication=blocked\n",
           receipt.source_trace_md5, receipt.read_count,
           receipt.first_physical_address, receipt.last_physical_address,
           receipt.first_reader_physical_pc, receipt.last_reader_physical_pc,
           receipt.target_2600_bytes_present ? "present" : "absent",
           receipt.target_2600_read_count,
           receipt.target_2600_nonzero_read_count,
           receipt.target_2600_distinct_reader_pc_count,
           receipt.target_2600_init_read_count,
           receipt.target_2600_runtime_read_count,
           receipt.target_2600_c3a0_read_count,
           receipt.target_2600_c3a0_nonzero_read_count,
           receipt.target_2600_c3a0_distinct_reader_pc_count);
    if (getenv("THERON_MEDNAFEN_MAIN_RAM_CONSUMER_PARSE_ONLY")) {
        puts("PASS: parser-only capture admission; code-window semantics not requested");
        return 0;
    }
    if (!theron_v1_mednafen_main_ram_consumer_trace_verify_code_window(
            path, 0x2c54u, code_window, sizeof(code_window))) {
        fprintf(stderr, "FAIL: captured HuC6280 code window was not verified\n");
        return 1;
    }
    memcpy(wrong_window, code_window, sizeof(wrong_window));
    wrong_window[0] ^= 0x01u;
    if (theron_v1_mednafen_main_ram_consumer_trace_verify_code_window(
            path, 0x2c54u, wrong_window, sizeof(wrong_window))) {
        fprintf(stderr, "FAIL: mutated HuC6280 code window was accepted\n");
        return 1;
    }
    puts("PASS: executed HuC6280 code window $2c54-$2c69 verified; semantics blocked");
    return 0;
}
