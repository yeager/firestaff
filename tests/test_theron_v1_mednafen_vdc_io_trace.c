#include "theron_v1_mednafen_vdc_io_trace.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static int write_fixture(const char *path, const char *body) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    fputs(body, file);
    return fclose(file) == 0;
}

int main(void) {
#if defined(_WIN32)
    puts("SKIP: POSIX temporary VDC trace fixture");
    return 77;
#else
    const char *tmpdir = getenv("TMPDIR");
    char path[512];
    Theron_V1VdcIoTraceReceipt receipt;
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-vdc-XXXXXX",
                    tmpdir) > 0);
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    assert(write_fixture(path,
        "FIRESTAFF_THERON_VDC_IO_TRACE_V1\n"
        "source=mednafen-pce-instrumented-vdc-io\n"
        "vdc_io_write sequence=0 timestamp=10 logical_address=0000 physical_address=1fe000 value=02 writer_pc=1a10 writer_physical_pc=0e1a10 a=02 x=03 y=04\n"
        "vdc_io_write sequence=1 timestamp=11 logical_address=0002 physical_address=1fe002 value=7f writer_pc=1a12 writer_physical_pc=0e1a12 a=7f x=03 y=04\n"));
    assert(theron_v1_mednafen_vdc_io_trace_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_VDC_IO_TRACE_READY);
    assert(receipt.source_header_verified && receipt.sequence_verified);
    assert(receipt.timestamp_verified && receipt.address_bounds_verified);
    assert(receipt.register_bounds_verified);
    assert(receipt.write_count == 2u);
    assert(receipt.first_writer_physical_pc == 0x0e1a10u);
    assert(receipt.last_logical_address == 2u);
    assert(!receipt.semantic_publication_allowed);
    unlink(path);

    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-vdc-XXXXXX",
                    tmpdir) > 0);
    fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    assert(write_fixture(path,
        "FIRESTAFF_THERON_VDC_IO_TRACE_V1\n"
        "source=mednafen-pce-instrumented-vdc-io\n"
        "vdc_io_write sequence=1 timestamp=10 logical_address=0000 physical_address=1fe000 value=02 writer_pc=1a10 writer_physical_pc=0e1a10 a=02 x=03 y=04\n"));
    assert(!theron_v1_mednafen_vdc_io_trace_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_VDC_IO_TRACE_REJECTED);
    assert(!receipt.sequence_verified);
    unlink(path);
    puts("PASS: Theron VDC I/O trace provenance validates and blocks semantics");
    return 0;
#endif
}
