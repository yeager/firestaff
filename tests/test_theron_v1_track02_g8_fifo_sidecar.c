#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "theron_v1_track02_g8_fifo_sidecar.h"

static int failures;

static void expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int write_sidecar(const char* path,
                         const char* marker,
                         const char* receipt)
{
    FILE* file = fopen(path, "wb");
    if (!file) return 0;
    if (fputs(marker, file) == EOF || fputs(receipt, file) == EOF ||
        fclose(file) != 0) {
        return 0;
    }
    return 1;
}

int main(void)
{
    static const char valid_marker[] =
        "g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4\n";
    static const char valid_receipt[] =
        "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
        "source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 "
        "physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 "
        "value=ab\n";
    static const char changed_value_receipt[] =
        "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
        "source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 "
        "physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 "
        "value=ac\n";
    static const char duplicate_receipt[] =
        "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
        "source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 "
        "physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 "
        "value=ab\n"
        "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
        "source_offset=1 fifo_sequence=2 reader_pc=ea50 logical_destination=2201 "
        "physical_destination=1f0201 writer_pc=ea52 writer_physical_pc=1f0253 "
        "value=ac\n";
    char path[] = "/tmp/firestaff-theron-g8-sidecar-XXXXXX";
    Theron_V1Track02G8FifoSidecarReceipt baseline;
    Theron_V1Track02G8FifoSidecarReceipt candidate;
    int descriptor = mkstemp(path);

    expect(descriptor >= 0, "temporary sidecar allocates");
    if (descriptor < 0) return 1;
    close(descriptor);

    expect(write_sidecar(path, valid_marker, valid_receipt),
           "valid G8 sidecar writes");
    expect(theron_v1_track02_g8_fifo_sidecar_parse_file(path, &baseline) &&
               baseline.valid && baseline.generation == 8u &&
               baseline.lba == 4859u && baseline.dispatch == 4u &&
               baseline.source_offset == 0u && baseline.fifo_sequence == 1u &&
               baseline.first_fifo_sequence == 1u &&
               baseline.last_fifo_sequence == 1u &&
               baseline.capture_byte_count == 1u &&
               baseline.source_window_offset == 0u &&
               baseline.source_window_bytes == 1u &&
               baseline.sequence_window_identity != 0u &&
               baseline.reader_pc == 0xea50u &&
               baseline.logical_destination == 0x2200u &&
               baseline.physical_destination == 0x1f0200u &&
               baseline.writer_pc == 0xea52u &&
               baseline.writer_physical_pc == 0x1f0252u &&
               baseline.value == 0xabu && baseline.fingerprint != 0u &&
               baseline.capture_row_count == 1u && baseline.capture_file_fnv1a != 0u &&
               baseline.capture_file_identity != 0u &&
               baseline.dispatch_logical_pc == 0x3840u &&
               baseline.dispatch_physical_pc == 0x1f1840u &&
               baseline.dispatch_a == 0x20u && baseline.dispatch_x == 0xffu &&
               baseline.dispatch_y == 0x04u && baseline.cdb_opcode == 0x08u &&
               baseline.cdb_lba == 4859u && baseline.cdb_sector_count == 1u &&
               baseline.cdb[0] == 0x08u && baseline.cdb[2] == 0x12u &&
               baseline.cdb[3] == 0xfbu && baseline.cdb[4] == 0x01u &&
               baseline.capture_cdb_identity != 0u &&
               baseline.capture_file_md5[0] != '\0' &&
               !strcmp(baseline.capture_file_md5, baseline.trace_md5),
           "valid G8 capture file has canonical MD5/FNV and one output row");

    expect(write_sidecar(path,
                         "g8_fifo_output_capxure generation=8 source_lba=4859 dispatch_sequence=4\n",
                         valid_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "marker drift is rejected");
    expect(write_sidecar(path,
                         "g8_fifo_output_capture generation=9 source_lba=4859 dispatch_sequence=4\n",
                         valid_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "generation drift is rejected");
    expect(write_sidecar(path,
                         "g8_fifo_output_capture generation=8 source_lba=4860 dispatch_sequence=4\n",
                         valid_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "LBA drift is rejected");
    expect(write_sidecar(path,
                         "g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=5\n",
                         valid_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "dispatch drift is rejected");
    expect(write_sidecar(path, valid_marker,
                         "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
                         "source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 "
                         "physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f8252 "
                         "value=ab\n") &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "writer physical-address drift outside the observed main-RAM window is rejected");
    expect(write_sidecar(path, valid_marker, duplicate_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               !candidate.valid,
           "a second FIFO output row is rejected rather than changing the canonical row count");

    expect(write_sidecar(path, valid_marker, changed_value_receipt) &&
               theron_v1_track02_g8_fifo_sidecar_parse_file(path, &candidate) &&
               candidate.valid && candidate.fingerprint != baseline.fingerprint &&
               candidate.capture_file_fnv1a != baseline.capture_file_fnv1a &&
               candidate.capture_file_identity != baseline.capture_file_identity &&
               strcmp(candidate.capture_file_md5, baseline.capture_file_md5) != 0,
           "capture-file payload drift produces new canonical identities");

    remove(path);
    if (failures) return 1;
    puts("test_theron_v1_track02_g8_fifo_sidecar: PASS");
    return 0;
}
