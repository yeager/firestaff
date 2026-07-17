#include "theron_v1_track02_g8_fifo_capture_binding.h"

#include <stdio.h>
#include <string.h>

static uint32_t binding_capture_fingerprint(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt)
{
    return (receipt->source_offset * 16777619u) ^ receipt->fifo_sequence ^
        ((uint32_t)receipt->reader_pc << 16) ^ receipt->logical_destination ^
        receipt->physical_destination ^ ((uint32_t)receipt->writer_pc << 16) ^
        receipt->writer_physical_pc ^ receipt->value;
}

static uint32_t sidecar_capture_fingerprint(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar)
{
    return (sidecar->source_offset * 16777619u) ^ sidecar->fifo_sequence ^
        ((uint32_t)sidecar->reader_pc << 16) ^ sidecar->logical_destination ^
        sidecar->physical_destination ^ ((uint32_t)sidecar->writer_pc << 16) ^
        sidecar->writer_physical_pc ^ sidecar->value;
}

static uint32_t binding_sequence_window_identity(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt)
{
    uint32_t identity = 2166136261u;
    const uint32_t fields[] = {
        receipt->generation, receipt->lba, receipt->dispatch,
        receipt->first_fifo_sequence, receipt->last_fifo_sequence,
        receipt->capture_byte_count, receipt->source_window_offset,
        receipt->source_window_bytes
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    return identity;
}

static uint32_t sidecar_sequence_window_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar)
{
    uint32_t identity = 2166136261u;
    const uint32_t fields[] = {
        sidecar->generation, sidecar->lba, sidecar->dispatch,
        sidecar->first_fifo_sequence, sidecar->last_fifo_sequence,
        sidecar->capture_byte_count, sidecar->source_window_offset,
        sidecar->source_window_bytes
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    return identity;
}

static uint32_t binding_capture_file_identity(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt)
{
    uint32_t identity = 2166136261u;
    size_t i;

    for (i = 0u; receipt->capture_file_md5[i]; ++i) {
        identity = (identity ^ (uint8_t)receipt->capture_file_md5[i]) * 16777619u;
    }
    identity = (identity ^ receipt->capture_file_fnv1a) * 16777619u;
    return (identity ^ receipt->capture_row_count) * 16777619u;
}

static uint32_t sidecar_capture_file_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar)
{
    uint32_t identity = 2166136261u;
    size_t i;

    for (i = 0u; sidecar->capture_file_md5[i]; ++i) {
        identity = (identity ^ (uint8_t)sidecar->capture_file_md5[i]) * 16777619u;
    }
    identity = (identity ^ sidecar->capture_file_fnv1a) * 16777619u;
    return (identity ^ sidecar->capture_row_count) * 16777619u;
}

static uint32_t binding_capture_cdb_identity(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt)
{
    uint32_t identity = receipt->capture_file_identity;
    const uint32_t fields[] = {
        receipt->generation, receipt->dispatch, receipt->dispatch_logical_pc,
        receipt->dispatch_physical_pc, receipt->dispatch_a, receipt->dispatch_x,
        receipt->dispatch_y, receipt->cdb_opcode, receipt->cdb_lba,
        receipt->cdb_sector_count
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    for (i = 0u; i < sizeof(receipt->cdb); ++i) {
        identity = (identity ^ receipt->cdb[i]) * 16777619u;
    }
    return identity;
}

static uint32_t sidecar_capture_cdb_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar)
{
    uint32_t identity = sidecar->capture_file_identity;
    const uint32_t fields[] = {
        sidecar->generation, sidecar->dispatch, sidecar->dispatch_logical_pc,
        sidecar->dispatch_physical_pc, sidecar->dispatch_a, sidecar->dispatch_x,
        sidecar->dispatch_y, sidecar->cdb_opcode, sidecar->cdb_lba,
        sidecar->cdb_sector_count
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    for (i = 0u; i < sizeof(sidecar->cdb); ++i) {
        identity = (identity ^ sidecar->cdb[i]) * 16777619u;
    }
    return identity;
}

static int binding_is_capture_only(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt)
{
    static const uint8_t expected_cdb[] = {0x08u, 0x00u, 0x12u, 0xfbu, 0x01u, 0x00u};
    return receipt &&
        receipt->status == THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_CAPTURE_REQUIRED &&
        receipt->sidecar_consumed &&
        receipt->artifact_corpus_source_trace_md5_consumed &&
        receipt->m11_lifecycle_source_trace_md5_consumed &&
        receipt->capture_required_only && receipt->no_draw_only &&
        receipt->lifecycle_scan_epoch && receipt->generation == 8u &&
        receipt->lba == 4859u && receipt->dispatch == 4u &&
        receipt->source_offset < 2352u && receipt->fifo_sequence &&
        receipt->first_fifo_sequence == receipt->fifo_sequence &&
        receipt->last_fifo_sequence == receipt->fifo_sequence &&
        receipt->capture_byte_count == 1u &&
        receipt->source_window_offset == receipt->source_offset &&
        receipt->source_window_bytes == 1u && receipt->sequence_window_identity &&
        receipt->sequence_window_identity == binding_sequence_window_identity(receipt) &&
        receipt->reader_pc && receipt->writer_pc &&
        receipt->physical_destination >= 0x1f0000u &&
        receipt->physical_destination <= 0x1f7fffu &&
        receipt->writer_physical_pc >= 0x1f0000u &&
        receipt->writer_physical_pc <= 0x1f7fffu &&
        receipt->fingerprint &&
        receipt->fingerprint == binding_capture_fingerprint(receipt) &&
        receipt->capture_row_count == 1u && receipt->capture_file_fnv1a &&
        receipt->capture_file_identity &&
        receipt->capture_file_identity == binding_capture_file_identity(receipt) &&
        receipt->dispatch_logical_pc == 0x3840u &&
        receipt->dispatch_physical_pc == 0x1f1840u &&
        receipt->dispatch_a == 0x20u && receipt->dispatch_x == 0xffu &&
        receipt->dispatch_y == 0x04u && receipt->cdb_opcode == 0x08u &&
        receipt->cdb_lba == 4859u && receipt->cdb_sector_count == 1u &&
        !memcmp(receipt->cdb, expected_cdb, sizeof(expected_cdb)) &&
        receipt->capture_cdb_identity &&
        receipt->capture_cdb_identity == binding_capture_cdb_identity(receipt) &&
        receipt->capture_file_md5[0] && receipt->capture_target_plan_identity &&
        receipt->sidecar_trace_md5[0] &&
        !strcmp(receipt->capture_file_md5, receipt->sidecar_trace_md5) &&
        receipt->source_trace_md5[0] && receipt->artifact_bundle_md5[0];
}

int theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    const char* m11_lifecycle_source_trace_md5,
    uint32_t lifecycle_scan_epoch)
{
    if (!binding_is_capture_only(receipt) || !artifact_corpus ||
        !m11_lifecycle_source_trace_md5 || !m11_lifecycle_source_trace_md5[0] ||
        !lifecycle_scan_epoch ||
        receipt->lifecycle_scan_epoch != lifecycle_scan_epoch ||
        strcmp(receipt->source_trace_md5, m11_lifecycle_source_trace_md5) ||
        strcmp(artifact_corpus->source_trace_md5, m11_lifecycle_source_trace_md5) ||
        receipt->capture_target_plan_identity !=
            artifact_corpus->capture_target_plan_identity ||
        strcmp(receipt->artifact_bundle_md5, artifact_corpus->artifact.bundle_md5) ||
        !theron_v1_track02_handoff_artifact_corpus_matches_identity(
            artifact_corpus, artifact_corpus->track02_md5,
            m11_lifecycle_source_trace_md5,
            artifact_corpus->capture_target_plan_identity)) {
        return 0;
    }
    return 1;
}

int theron_v1_track02_g8_fifo_capture_binding_bind(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    const char* m11_lifecycle_source_trace_md5,
    uint32_t lifecycle_scan_epoch,
    Theron_V1Track02G8FifoCaptureBindingReceipt* out)
{
    Theron_V1Track02G8FifoCaptureBindingReceipt receipt = {0};
    static const uint8_t expected_cdb[] = {0x08u, 0x00u, 0x12u, 0xfbu, 0x01u, 0x00u};

    if (!out) return 0;
    *out = receipt;
    if (!sidecar || !artifact_corpus || !m11_lifecycle_source_trace_md5 ||
        !m11_lifecycle_source_trace_md5[0] || !lifecycle_scan_epoch ||
        !sidecar->valid || sidecar->generation != 8u || sidecar->lba != 4859u ||
        sidecar->dispatch != 4u || sidecar->source_offset >= 2352u ||
        !sidecar->fifo_sequence || !sidecar->reader_pc || !sidecar->writer_pc ||
        sidecar->first_fifo_sequence != sidecar->fifo_sequence ||
        sidecar->last_fifo_sequence != sidecar->fifo_sequence ||
        sidecar->capture_byte_count != 1u ||
        sidecar->source_window_offset != sidecar->source_offset ||
        sidecar->source_window_bytes != 1u || !sidecar->sequence_window_identity ||
        sidecar->sequence_window_identity != sidecar_sequence_window_identity(sidecar) ||
        sidecar->physical_destination < 0x1f0000u ||
        sidecar->physical_destination > 0x1f7fffu ||
        sidecar->writer_physical_pc < 0x1f0000u ||
        sidecar->writer_physical_pc > 0x1f7fffu || !sidecar->fingerprint ||
        sidecar->fingerprint != sidecar_capture_fingerprint(sidecar) ||
        sidecar->capture_row_count != 1u || !sidecar->capture_file_fnv1a ||
        !sidecar->capture_file_identity ||
        sidecar->capture_file_identity != sidecar_capture_file_identity(sidecar) ||
        sidecar->dispatch_logical_pc != 0x3840u ||
        sidecar->dispatch_physical_pc != 0x1f1840u ||
        sidecar->dispatch_a != 0x20u || sidecar->dispatch_x != 0xffu ||
        sidecar->dispatch_y != 0x04u || sidecar->cdb_opcode != 0x08u ||
        sidecar->cdb_lba != 4859u || sidecar->cdb_sector_count != 1u ||
        memcmp(sidecar->cdb, expected_cdb, sizeof(expected_cdb)) ||
        !sidecar->capture_cdb_identity ||
        sidecar->capture_cdb_identity != sidecar_capture_cdb_identity(sidecar) ||
        !sidecar->trace_md5[0] || !sidecar->capture_file_md5[0] ||
        strcmp(sidecar->trace_md5, sidecar->capture_file_md5) ||
        !theron_v1_track02_handoff_artifact_corpus_matches_identity(
            artifact_corpus, artifact_corpus->track02_md5,
            m11_lifecycle_source_trace_md5,
            artifact_corpus->capture_target_plan_identity)) {
        receipt.status = THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_REJECTED;
        *out = receipt;
        return 1;
    }

    receipt.status = THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_CAPTURE_REQUIRED;
    receipt.sidecar_consumed = 1;
    receipt.artifact_corpus_source_trace_md5_consumed = 1;
    receipt.m11_lifecycle_source_trace_md5_consumed = 1;
    receipt.capture_required_only = 1;
    receipt.no_draw_only = 1;
    receipt.lifecycle_scan_epoch = lifecycle_scan_epoch;
    receipt.generation = sidecar->generation;
    receipt.lba = sidecar->lba;
    receipt.dispatch = sidecar->dispatch;
    receipt.fifo_sequence = sidecar->fifo_sequence;
    receipt.fingerprint = sidecar->fingerprint;
    receipt.capture_target_plan_identity = artifact_corpus->capture_target_plan_identity;
    receipt.source_offset = sidecar->source_offset;
    receipt.first_fifo_sequence = sidecar->first_fifo_sequence;
    receipt.last_fifo_sequence = sidecar->last_fifo_sequence;
    receipt.capture_byte_count = sidecar->capture_byte_count;
    receipt.source_window_offset = sidecar->source_window_offset;
    receipt.source_window_bytes = sidecar->source_window_bytes;
    receipt.sequence_window_identity = sidecar->sequence_window_identity;
    receipt.dispatch_logical_pc = sidecar->dispatch_logical_pc;
    receipt.dispatch_physical_pc = sidecar->dispatch_physical_pc;
    receipt.dispatch_a = sidecar->dispatch_a;
    receipt.dispatch_x = sidecar->dispatch_x;
    receipt.dispatch_y = sidecar->dispatch_y;
    receipt.cdb_opcode = sidecar->cdb_opcode;
    receipt.cdb_lba = sidecar->cdb_lba;
    receipt.cdb_sector_count = sidecar->cdb_sector_count;
    memcpy(receipt.cdb, sidecar->cdb, sizeof(receipt.cdb));
    receipt.capture_cdb_identity = sidecar->capture_cdb_identity;
    receipt.reader_pc = sidecar->reader_pc;
    receipt.logical_destination = sidecar->logical_destination;
    receipt.physical_destination = sidecar->physical_destination;
    receipt.writer_pc = sidecar->writer_pc;
    receipt.writer_physical_pc = sidecar->writer_physical_pc;
    receipt.value = sidecar->value;
    receipt.capture_file_fnv1a = sidecar->capture_file_fnv1a;
    receipt.capture_row_count = sidecar->capture_row_count;
    receipt.capture_file_identity = sidecar->capture_file_identity;
    snprintf(receipt.sidecar_trace_md5, sizeof(receipt.sidecar_trace_md5), "%s",
             sidecar->trace_md5);
    snprintf(receipt.capture_file_md5, sizeof(receipt.capture_file_md5), "%s",
             sidecar->capture_file_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s",
             m11_lifecycle_source_trace_md5);
    snprintf(receipt.artifact_bundle_md5, sizeof(receipt.artifact_bundle_md5), "%s",
             artifact_corpus->artifact.bundle_md5);
    *out = receipt;
    return 1;
}
