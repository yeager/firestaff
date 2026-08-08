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
    unlink(path);
    puts("PASS: disassembly-bound spawn consumer receipt validates provenance and blocks semantics");
    return 0;
#endif
}
