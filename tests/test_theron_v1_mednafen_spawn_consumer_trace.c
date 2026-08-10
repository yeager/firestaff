#include "theron_v1_mednafen_spawn_consumer_trace.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static void write_fixture(const char *path, int bad_flags) {
    FILE *file = fopen(path, "wb");
    assert(file);
    fputs("source=mednafen-pce-instrumented-spawn-consumer\n", file);
    fprintf(file,
            "spawn_consumer_read sequence=0 logical_address=5d64 physical_address=1f2c54 value=ad reader_pc=c96b reader_physical_pc=1f196b target_5d64=%d target_5d6a=0 c96b_window=1 cc4c_window=0\n",
            bad_flags ? 0 : 1);
    fputs("spawn_consumer_read sequence=1 logical_address=2800 physical_address=1f2c55 value=08 reader_pc=cc4c reader_physical_pc=1f1c4c target_5d64=0 target_5d6a=0 c96b_window=0 cc4c_window=1\n", file);
    fclose(file);
}

static void write_register_fixture(const char *path, int bad_flags,
                                   int include_spawn_entry) {
    FILE *file = fopen(path, "wb");
    assert(file);
    fputs("source=mednafen-pce-instrumented-spawn-registers-v3\n", file);
    fprintf(file,
            "spawn_consumer_registers sequence=0 pc=4644 physical_pc=114644 a=01 x=02 y=03 sp=fe p=04 mpr0=1f mpr_pc=8a b3=10 b4=20 b5=30 b6=40 b8=50 ba=60 bb=70 c96b_window=0 cc4c_window=0 preconsumer_4644=%d helper_4667=0 spawn_entry_b0e5=0\n",
            bad_flags ? 0 : 1);
    fputs("spawn_consumer_registers sequence=1 pc=4667 physical_pc=114667 a=11 x=12 y=13 sp=fd p=05 mpr0=1f mpr_pc=8a b3=14 b4=21 b5=31 b6=41 b8=51 ba=61 bb=71 c96b_window=0 cc4c_window=0 preconsumer_4644=0 helper_4667=1 spawn_entry_b0e5=0\n", file);
    fputs("spawn_consumer_registers sequence=2 pc=c96b physical_pc=13496b a=21 x=22 y=23 sp=fc p=06 mpr0=1f mpr_pc=9a b3=12 b4=22 b5=32 b6=42 b8=52 ba=62 bb=72 c96b_window=1 cc4c_window=0 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=0\n", file);
    fputs("spawn_consumer_registers sequence=3 pc=cc4c physical_pc=134c4c a=31 x=32 y=33 sp=fb p=07 mpr0=1f mpr_pc=9a b3=13 b4=23 b5=33 b6=43 b8=53 ba=63 bb=73 c96b_window=0 cc4c_window=1 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=0\n", file);
    if (include_spawn_entry) {
        fputs("spawn_consumer_registers sequence=4 pc=b0e5 physical_pc=1130e5 a=41 x=42 y=43 sp=fa p=08 mpr0=1f mpr_pc=89 b3=14 b4=24 b5=34 b6=44 b8=54 ba=64 bb=74 c96b_window=0 cc4c_window=0 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=1\n", file);
    }
    fclose(file);
}

static void write_execution_window_fixture(const char *path) {
    FILE *file = fopen(path, "wb");
    assert(file);
    fputs("source=mednafen-pce-instrumented-spawn-registers-v3\n", file);
    fputs("spawn_consumer_registers sequence=0 pc=ca00 physical_pc=0d4a00 a=01 x=02 y=03 sp=fe p=04 mpr0=1f mpr_pc=6a b3=10 b4=20 b5=30 b6=40 b8=50 ba=60 bb=70 c96b_window=1 cc4c_window=0 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=0\n", file);
    fputs("spawn_consumer_registers sequence=1 pc=cd00 physical_pc=0d4d00 a=11 x=12 y=13 sp=fd p=05 mpr0=1f mpr_pc=6a b3=11 b4=21 b5=31 b6=41 b8=51 ba=61 bb=71 c96b_window=0 cc4c_window=1 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=0\n", file);
    fclose(file);
}

static void write_rng_fixture(const char *path, int bad_step) {
    FILE *file = fopen(path, "wb");
    assert(file);
    fputs("source=mednafen-pce-instrumented-rng-consumer\n", file);
    fputs("rng_consumer_sample_limit=512\n", file);
    fprintf(file,
            "rng_consumer_window sequence=1 step=0 pc=5d64 physical_pc=114d64 entry=5d64 a=01 x=02 y=03 sp=fe p=04 mpr0=1f b3=10 b4=20 b5=30 b6=40 b8=50 ba=60 bb=70 entry_sp=fe return_pc=5d90 return_boundary=0\n");
    fprintf(file,
            "rng_consumer_window sequence=1 step=%u pc=5d70 physical_pc=114d70 entry=window a=11 x=12 y=13 sp=fd p=05 mpr0=1f b3=11 b4=21 b5=31 b6=41 b8=51 ba=61 bb=71 entry_sp=fe return_pc=5d90 return_boundary=1\n",
            bad_step ? 2u : 1u);
    fclose(file);
}

int main(void) {
#if defined(_WIN32)
    puts("SKIP: temporary trace fixture requires POSIX mkstemp");
    return 77;
#else
    char path[512];
    char path2[512];
    const char *tmpdir = getenv("TMPDIR");
    Theron_V1SpawnConsumerTraceReceipt receipt;
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-spawn-XXXXXX",
                    tmpdir) > 0);
    assert(snprintf(path2, sizeof(path2), "%s/firestaff-theron-spawn-XXXXXX",
                    tmpdir) > 0);
    int fd = mkstemp(path);
    int fd2 = mkstemp(path2);
    assert(fd >= 0);
    assert(fd2 >= 0);
    close(fd);
    close(fd2);
    write_fixture(path, 0);
    assert(theron_v1_mednafen_spawn_consumer_trace_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
    assert(receipt.source_header_verified && receipt.sequence_verified);
    assert(receipt.bank_coordinates_verified && receipt.boundary_flags_verified);
    assert(receipt.target_5d64_seen && receipt.c96b_window_seen);
    assert(receipt.cc4c_window_seen && receipt.read_count == 2u);
    assert(!receipt.semantic_publication_allowed);
    write_fixture(path, 1);
    assert(!theron_v1_mednafen_spawn_consumer_trace_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);

    write_register_fixture(path, 0, 1);
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        assert(theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
        assert(registers.sample_count == 5u);
        assert(registers.c96b_window_seen && registers.cc4c_window_seen);
        assert(registers.preconsumer_4644_seen && registers.helper_4667_seen);
        assert(registers.helper_4667_special_branch_seen);
        assert(registers.last_a == 0x41u && registers.last_bb == 0x74u);
        assert(registers.last_mpr_pc == 0x89u);
        assert(registers.spawn_entry_b0e5_seen);
        assert(!registers.semantic_publication_allowed);
    }
    {
        Theron_V1SpawnCaptureCorrelationReceipt correlation;
        write_fixture(path2, 0);
        assert(theron_v1_mednafen_spawn_capture_correlate_files(
            path2, path, &correlation));
        assert(correlation.ready && correlation.source_windows_paired);
        assert(correlation.consumer_read_count == 2u &&
               correlation.register_sample_count == 5u);
        assert(!correlation.dynamic_return_contract_verified &&
               !correlation.semantic_publication_allowed);
        assert(theron_v1_mednafen_spawn_capture_correlate_files(
            path, path, &correlation) == 0);
        /* The same path cannot contain both sidecar headers. */
        assert(!correlation.ready && !correlation.semantic_publication_allowed);
    }
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        /* The helper/window edges alone are insufficient: the exact regular
         * spawn entry must be observed in the same run. */
        write_register_fixture(path, 0, 0);
        assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
        assert(theron_v1_mednafen_spawn_register_trace_parse_execution_window_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
        assert(!registers.spawn_entry_b0e5_seen);
    }
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        FILE *legacy = fopen(path, "wb");
        assert(legacy);
        fputs("source=mednafen-pce-instrumented-spawn-registers-v2\n", legacy);
        fclose(legacy);
        assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
    }
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        write_execution_window_fixture(path);
        assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
        assert(theron_v1_mednafen_spawn_register_trace_parse_execution_window_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
        assert(registers.sample_count == 2u);
        assert(registers.c96b_window_seen && registers.cc4c_window_seen);
        assert(!registers.preconsumer_4644_seen && !registers.helper_4667_seen);
        assert(!registers.semantic_publication_allowed);
    }
    {
        const char *real_trace = getenv("THERON_REAL_SPAWN_REGISTER_TRACE");
        if (real_trace && real_trace[0]) {
            Theron_V1SpawnRegisterTraceReceipt registers;
            assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
                real_trace, &registers));
            assert(theron_v1_mednafen_spawn_register_trace_parse_execution_window_file(
                real_trace, &registers));
            assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
            assert(registers.sample_count == 2048u);
            assert(registers.c96b_window_seen && registers.cc4c_window_seen);
            assert(!registers.preconsumer_4644_seen && !registers.helper_4667_seen);
            assert(!registers.semantic_publication_allowed);
        }
    }
    write_register_fixture(path, 1, 1);
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
    }
    {
        Theron_V1RngConsumerTraceReceipt rng;
        write_rng_fixture(path, 0);
        assert(theron_v1_mednafen_rng_consumer_trace_parse_file(path, &rng));
        assert(rng.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
        assert(rng.source_header_verified && rng.sequence_verified);
        assert(rng.step_verified && rng.physical_pc_bounds_verified);
        assert(rng.boundary_flags_verified && rng.target_5d64_seen);
        assert(rng.sample_limit == 512u);
        assert(rng.sample_count == 2u && rng.window_count == 1u);
        assert(rng.return_boundary_seen && rng.last_return_pc == 0x5d90u);
        assert(rng.last_pc == 0x5d70u && rng.last_bb == 0x71u);
        assert(!rng.semantic_publication_allowed);
        write_rng_fixture(path, 1);
        assert(!theron_v1_mednafen_rng_consumer_trace_parse_file(path, &rng));
        assert(rng.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
    }
    {
        const char *real_rng = getenv("THERON_REAL_RNG_CONSUMER_TRACE");
        if (real_rng && real_rng[0]) {
            Theron_V1RngConsumerTraceReceipt rng;
            assert(theron_v1_mednafen_rng_consumer_trace_parse_file(
                real_rng, &rng));
            assert(rng.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
            assert(rng.source_header_verified && rng.sequence_verified);
            assert(rng.step_verified && rng.physical_pc_bounds_verified);
            assert(rng.boundary_flags_verified && rng.target_5d64_seen);
            assert(rng.sample_limit >= 512u && rng.sample_limit <= 65536u);
            assert(rng.sample_count == rng.sample_limit);
            assert(!rng.semantic_publication_allowed);
        }
    }
    {
        const char *real_code = getenv("THERON_REAL_RNG_CODE_TRACE");
        const char *real_track02 = getenv("THERON_REAL_US_TRACK02");
        if (real_code && real_code[0] && real_track02 && real_track02[0]) {
            Theron_V1RngCodeSourceCorrelationReceipt source;
            assert(theron_v1_mednafen_rng_code_correlate_us_track02_file(
                real_code, real_track02, &source));
            assert(source.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
            assert(source.source_header_verified && source.format_verified);
            assert(source.track02_size_verified &&
                   source.source_bytes_match_verified);
            assert(source.target_5d64_seen && source.window_count == 1u);
            assert(source.source_match_count == 1u);
            assert(source.first_source_offset == 0x975c4u);
            assert(!source.semantic_publication_allowed);
        }
    }
    unlink(path);
    unlink(path2);
    puts("PASS: disassembly-bound spawn receipts validate provenance and block semantics");
    return 0;
#endif
}
