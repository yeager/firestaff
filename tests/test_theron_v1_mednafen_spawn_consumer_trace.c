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

static void write_register_fixture(const char *path, int bad_flags) {
    FILE *file = fopen(path, "wb");
    assert(file);
    fputs("source=mednafen-pce-instrumented-spawn-registers\n", file);
    fprintf(file,
            "spawn_consumer_registers sequence=0 pc=4644 physical_pc=1f4644 a=01 x=02 y=03 sp=fe p=04 mpr0=1f b3=10 b4=20 b5=30 b6=40 b8=50 ba=60 bb=70 c96b_window=0 cc4c_window=0 preconsumer_4644=%d helper_4667=0\n",
            bad_flags ? 0 : 1);
    fputs("spawn_consumer_registers sequence=1 pc=4667 physical_pc=1f4667 a=11 x=12 y=13 sp=fd p=05 mpr0=1f b3=11 b4=21 b5=31 b6=41 b8=51 ba=61 bb=71 c96b_window=0 cc4c_window=0 preconsumer_4644=0 helper_4667=1\n", file);
    fputs("spawn_consumer_registers sequence=2 pc=c96b physical_pc=1f596b a=21 x=22 y=23 sp=fc p=06 mpr0=1f b3=12 b4=22 b5=32 b6=42 b8=52 ba=62 bb=72 c96b_window=1 cc4c_window=0 preconsumer_4644=0 helper_4667=0\n", file);
    fputs("spawn_consumer_registers sequence=3 pc=cc4c physical_pc=1f5c4c a=31 x=32 y=33 sp=fb p=07 mpr0=1f b3=13 b4=23 b5=33 b6=43 b8=53 ba=63 bb=73 c96b_window=0 cc4c_window=1 preconsumer_4644=0 helper_4667=0\n", file);
    fclose(file);
}

int main(void) {
#if defined(_WIN32)
    puts("SKIP: temporary trace fixture requires POSIX mkstemp");
    return 77;
#else
    char path[] = "/tmp/firestaff-theron-spawn-XXXXXX";
    Theron_V1SpawnConsumerTraceReceipt receipt;
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
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

    write_register_fixture(path, 0);
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        assert(theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_READY);
        assert(registers.sample_count == 4u);
        assert(registers.c96b_window_seen && registers.cc4c_window_seen);
        assert(registers.preconsumer_4644_seen && registers.helper_4667_seen);
        assert(registers.last_a == 0x31u && registers.last_bb == 0x73u);
        assert(!registers.semantic_publication_allowed);
    }
    write_register_fixture(path, 1);
    {
        Theron_V1SpawnRegisterTraceReceipt registers;
        assert(!theron_v1_mednafen_spawn_register_trace_parse_file(
            path, &registers));
        assert(registers.status == THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED);
    }
    unlink(path);
    puts("PASS: disassembly-bound spawn receipts validate provenance and block semantics");
    return 0;
#endif
}
