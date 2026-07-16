#include "theron_v1_runtime_admission.h"

#include <stdlib.h>
#include <string.h>

static int build_payload_receipt(
    Theron_V1RawLoaderTraceGamePayloadReceipt *out)
{
    enum { source_lba = 4165u, raw_record = source_lba - 3009u };
    static const char capture[] =
        "source=mednafen-pce-instrumented-cd\n"
        "main_ram_loader_e009_dispatch sequence=7 logical_pc=3840 physical_pc=1f1840 a=20 x=00 y=00\n"
        "pce_cd_register_write cpu_pc=e90d physical=1801 data=81\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=08\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=00\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=10\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=45\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=01\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=00\n"
        "scsi_read_command generation=4 opcode=08 cdb=080010450100 start_lba=4165 sector_count=1\n"
        "pce_cd_fifo_origin_main_ram_receipt generation=4 source_lba=4165 source_offset=17 fifo_sequence=42 reader_pc=e98a logical_destination=2300 physical_destination=1f2300 writer_pc=3844 writer_physical_pc=1f1844 value=5a\n"
        "pce_cd_fifo_origin_main_ram_consumer sequence=1 generation=4 source_lba=4165 source_offset=17 fifo_sequence=42 logical_address=2300 physical_address=1f2300 value=5a reader_pc=3900 reader_physical_pc=1f1900\n";
    size_t raw_size = (raw_record + 1u) * THERON_TRACK02_RAW_SECTOR_BYTES;
    uint8_t *raw = (uint8_t *)calloc(raw_size, 1u);
    int ok;

    if (!raw) {
        return 0;
    }
    raw[(size_t)raw_record * THERON_TRACK02_RAW_SECTOR_BYTES + 17u] = 0x5au;
    ok = theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
        capture, raw, raw_size, THERON_TRACK02_MD5_US_BIN, out);
    free(raw);
    return ok;
}

int main(void)
{
    Theron_V1RuntimeAdmissionReceipt runtime;
    Theron_V1RuntimeSessionHandoffReceipt session;
    Theron_V1TraceSourceProvenanceReceipt provenance;
    Theron_V1RawLoaderTraceGamePayloadReceipt payload;
    Theron_V1RawLoaderTraceGamePayloadReceipt mutated;
    Theron_V1CaptureConfig config = {
        1, "raw", "card", "raw_track_required_ready", 1, 1
    };

    theron_v1_runtime_admission_init(&runtime);
    if (runtime.admitted || runtime.game_owned_fifo_payload_admitted ||
        runtime.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_session_handoff_init(&session);
    if (session.valid || session.startup_session_handoff_ready ||
        session.runtime_capture_required ||
        session.fallback_visuals_allowed) {
        return 1;
    }
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    if (theron_v1_runtime_admission_attach(&runtime, "synthetic", 0)) {
        return 1;
    }
    if (!theron_v1_runtime_trace_identity_valid("v3:raw:card", &config)) {
        return 1;
    }
    if (!theron_v1_trace_source_provenance(
            "capture-1", "v3:raw:card", &provenance) ||
        !provenance.valid || provenance.runtime_admitted) {
        return 1;
    }
    if (theron_v1_trace_source_provenance(
            "v3:raw:card", "v3:raw:card", &provenance)) {
        return 1;
    }
    if (!theron_v1_runtime_admission_attach(
            &runtime, "real-v3-trace", 0) || runtime.admitted) {
        return 1;
    }

    if (!build_payload_receipt(&payload) ||
        !theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &payload)) {
        return 1;
    }
    if (!runtime.attached || !runtime.admitted ||
        !runtime.game_owned_fifo_payload_attached ||
        !runtime.game_owned_fifo_payload_admitted ||
        runtime.game_owned_fifo_payload_variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(runtime.game_owned_fifo_payload_track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        runtime.game_owned_fifo_payload_record != payload.raw_track02_record ||
        runtime.game_owned_fifo_payload_source_offset != payload.source_offset ||
        runtime.game_owned_fifo_payload_source_byte != payload.source_byte ||
        !runtime.cdb_read6_verified ||
        !runtime.fifo_to_game_ram_verified ||
        !runtime.game_ram_consumer_verified ||
        runtime.payload_semantics_proven ||
        runtime.visual_semantics_proven ||
        runtime.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session)) {
        return 1;
    }
    if (!session.valid ||
        !session.startup_session_handoff_ready ||
        !session.runtime_capture_required ||
        !session.game_owned_fifo_payload_admitted ||
        session.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(session.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        session.record != payload.raw_track02_record ||
        session.source_offset != payload.source_offset ||
        session.source_byte != payload.source_byte ||
        !session.cdb_read6_verified ||
        !session.fifo_to_game_ram_verified ||
        !session.game_ram_consumer_verified ||
        session.payload_semantics_proven ||
        session.visual_semantics_proven ||
        session.fallback_visuals_allowed ||
        session.object_table_admission_allowed ||
        session.level_admission_allowed) {
        return 1;
    }

    mutated = payload;
    mutated.payload_semantics_proven = 1;
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    mutated = payload;
    mutated.game_ram_consumer_verified = 0;
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    mutated = payload;
    strcpy(mutated.track02_md5, THERON_TRACK02_MD5_JP_BIN);
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    if (!theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &payload)) {
        return 1;
    }
    runtime.payload_semantics_proven = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    runtime.payload_semantics_proven = 0;
    runtime.visual_semantics_proven = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    runtime.visual_semantics_proven = 0;
    runtime.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    return 0;
}
