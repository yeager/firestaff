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

int main(int argc, char **argv) {
#if defined(_WIN32)
    puts("SKIP: POSIX temporary VDC trace fixture");
    return 77;
#else
    const char *tmpdir = getenv("TMPDIR");
    char path[512];
    Theron_V1VdcIoTraceReceipt receipt;
    if (argc == 2) {
        Theron_V1VdcIoTrace trace;
        assert(theron_v1_mednafen_vdc_io_trace_load_file(argv[1], &trace));
        assert(trace.receipt.status == THERON_V1_VDC_IO_TRACE_READY);
        assert(trace.write_count >= THERON_V1_VDC_IO_TRACE_LEGACY_WRITES);
        assert(trace.write_count <= THERON_V1_VDC_IO_TRACE_MAX_WRITES);
        assert(trace.receipt.physical_bus_marker_verified);
        assert(trace.receipt.marked_physical_write_count > 0u);
        assert(trace.receipt.first_normalized_physical_address ==
               (trace.receipt.first_physical_address & 0x7fffffffu));
        assert(trace.receipt.timestamp_epoch_count ==
               trace.receipt.timestamp_reset_count + 1u);
        if (trace.receipt.snapshot_boundary_verified) {
            assert(trace.receipt.snapshot_boundary_sequence ==
                   trace.write_count);
        }
        if (trace.receipt.source_sequence_verified) {
            assert(trace.receipt.first_source_sequence ==
                   trace.receipt.source_sequence_base);
            assert(trace.receipt.last_source_sequence -
                       trace.receipt.first_source_sequence + 1u ==
                   trace.write_count);
        }
        assert(trace.writes[0].sequence == 0u);
        assert(trace.writes[0].normalized_physical_address ==
               (trace.writes[0].raw_physical_address & 0x7fffffffu));
        assert(trace.writes[trace.write_count - 1u].sequence ==
               trace.write_count - 1u);
        if (trace.receipt.snapshot_boundary_verified) {
            char vram_path[512];
            Theron_V1VdcIoVramReplayReceipt replay;
            size_t length = strlen(argv[1]);
            assert(length > 7u && strcmp(argv[1] + length - 7u, ".vdc-io") == 0);
            assert(length - 7u + 5u < sizeof(vram_path));
            memcpy(vram_path, argv[1], length - 7u);
            memcpy(vram_path + length - 7u, ".vram", 6u);
            assert(theron_v1_mednafen_vdc_io_verify_vram_snapshot(
                &trace, vram_path, &replay));
            assert(replay.vwr_commit_count > 0u);
            assert(replay.written_word_count > 0u);
            assert(replay.matched_word_count == replay.written_word_count);
            assert(replay.mismatched_word_count == 0u);
            assert(replay.atomic_snapshot_verified);
            assert(!replay.semantic_publication_allowed);
            printf("replay_vwr_commits=%u written_words=%u matched_words=%u "
                   "mismatched_words=%u\n",
                   replay.vwr_commit_count, replay.written_word_count,
                   replay.matched_word_count, replay.mismatched_word_count);
        }
        theron_v1_mednafen_vdc_io_trace_free(&trace);
        assert(trace.writes == NULL && trace.write_count == 0u);
        puts("PASS: real Theron VDC I/O trace accepted with bus marker");
        return 0;
    }
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-vdc-XXXXXX",
                    tmpdir) > 0);
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    assert(write_fixture(path,
        "FIRESTAFF_THERON_VDC_IO_TRACE_V1\n"
        "source=mednafen-pce-instrumented-vdc-io\n"
        "vdc_io_write sequence=0 timestamp=10 logical_address=21ef physical_address=801fe000 value=01 writer_pc=c6ee writer_physical_pc=0d26ee a=01 x=00 y=1f\n"
        "vdc_io_write sequence=1 timestamp=9 logical_address=0002 physical_address=1fe002 value=7f writer_pc=1a12 writer_physical_pc=0e1a12 a=7f x=03 y=04\n"
        "vdc_snapshot_boundary sequence=2 timestamp=14\n"));
    assert(theron_v1_mednafen_vdc_io_trace_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_VDC_IO_TRACE_READY);
    assert(receipt.source_header_verified && receipt.sequence_verified);
    assert(receipt.timestamp_verified && receipt.address_bounds_verified);
    assert(receipt.register_bounds_verified);
    assert(receipt.write_count == 2u);
    assert(receipt.timestamp_reset_count == 1u);
    assert(receipt.timestamp_epoch_count == 2u);
    assert(receipt.first_writer_physical_pc == 0x0d26eeu);
    assert(receipt.first_physical_address == 0x801fe000u);
    assert(receipt.first_normalized_physical_address == 0x001fe000u);
    assert(receipt.marked_physical_write_count == 1u);
    assert(receipt.snapshot_boundary_verified);
    assert(receipt.snapshot_boundary_sequence == 2u);
    assert(receipt.snapshot_boundary_timestamp == 14u);
    assert(receipt.last_logical_address == 2u);
    assert(!receipt.semantic_publication_allowed);

    {
        Theron_V1VdcIoTrace trace;
        assert(theron_v1_mednafen_vdc_io_trace_load_file(path, &trace));
        assert(trace.write_count == 2u);
        assert(trace.writes[0].raw_physical_address == 0x801fe000u);
        assert(trace.writes[0].normalized_physical_address == 0x001fe000u);
        assert(trace.writes[1].logical_address == 0x0002u);
        assert(trace.writes[1].value == 0x7fu);
        theron_v1_mednafen_vdc_io_trace_free(&trace);
    }
    unlink(path);

    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-vdc-XXXXXX",
                    tmpdir) > 0);
    fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    assert(write_fixture(path,
        "FIRESTAFF_THERON_VDC_IO_TRACE_V1\n"
        "source=mednafen-pce-instrumented-vdc-io\n"
        "vdc_io_write sequence=0 source_sequence=65536 timestamp=10 logical_address=21ef physical_address=801fe000 value=01 writer_pc=c6ee writer_physical_pc=0d26ee a=01 x=00 y=1f\n"
        "vdc_io_write sequence=1 source_sequence=65537 timestamp=11 logical_address=0002 physical_address=1fe002 value=7f writer_pc=1a12 writer_physical_pc=0e1a12 a=7f x=03 y=04\n"));
    assert(theron_v1_mednafen_vdc_io_trace_parse_file(path, &receipt));
    assert(receipt.source_sequence_verified);
    assert(receipt.source_sequence_base == 65536u);
    assert(receipt.first_source_sequence == 65536u);
    assert(receipt.last_source_sequence == 65537u);
    assert(!receipt.snapshot_boundary_verified);
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
