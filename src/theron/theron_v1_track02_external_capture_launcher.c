#include <stdio.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_track02_external_capture_launcher.h"

static int theron_v1_external_capture_file_exists(const char *path) {
    FILE *file;
    if (!path || !path[0] || !(file = fopen(path, "rb"))) return 0;
    fclose(file);
    return 1;
}

static const char *theron_v1_external_capture_variant_name(
    Theron_Track02Variant variant) {
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) return "jp_bin";
    if (variant == THERON_TRACK02_VARIANT_US_BIN) return "us_bin";
    if (variant == THERON_TRACK02_VARIANT_US_ISO) return "us_iso";
    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) return "jp_rev1_iso";
    return NULL;
}

static int theron_v1_external_capture_prepare(
    const Theron_V1Track02ExternalCaptureRequest *request,
    Theron_V1Track02RawMediaIntakeReceipt *intake,
    Theron_V1Track02ExternalCaptureReceipt *out) {
    Theron_V1Track02ExternalCaptureReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!request || !request->emulator_path || !request->media_path ||
        !request->expected_track02_md5 || !request->manifest_path ||
        !request->emulator_path[0] || !request->media_path[0] ||
        !request->expected_track02_md5[0] || !request->manifest_path[0] ||
        !theron_v1_external_capture_file_exists(request->emulator_path)) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
        *out = receipt;
        return 1;
    }
    if (!theron_v1_track02_raw_media_intake_discover(request->media_path, intake) ||
        intake->status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (intake->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        strcmp(request->expected_track02_md5, intake->track02_md5) ||
        !theron_v1_external_capture_variant_name(intake->variant)) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
        *out = receipt;
        return 1;
    }
    receipt.raw_media_intake_verified = 1;
    receipt.track02_variant = intake->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", intake->track02_md5);
    snprintf(receipt.emulator_path, sizeof(receipt.emulator_path), "%s", request->emulator_path);
    snprintf(receipt.media_path, sizeof(receipt.media_path), "%s", request->media_path);
    snprintf(receipt.manifest_path, sizeof(receipt.manifest_path), "%s", request->manifest_path);
    *out = receipt;
    return 1;
}

int theron_v1_track02_external_capture_write_skeleton(
    const Theron_V1Track02ExternalCaptureRequest *request,
    Theron_V1Track02ExternalCaptureReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt intake;
    Theron_V1Track02ExternalCaptureReceipt receipt;
    FILE *file;
    const char *variant;

    if (!theron_v1_external_capture_prepare(request, &intake, &receipt)) return 0;
    if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE &&
        receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED &&
        (file = fopen(request->manifest_path, "wb"))) {
        variant = theron_v1_external_capture_variant_name(intake.variant);
        fprintf(file,
                "# Original external emulator capture required.\n"
                "# Do not invent any values below; remove leading # only for observed rows.\n"
                "format=theron_track02_capture_trace_v1\n"
                "track02_md5=%s\n"
                "track02_variant=%s\n"
                "# loader_record=\n# loader_destination=\n# loader_payload_bytes=\n"
                "# loader_payload_checksum=\n# consumer_trace_checksum=\n"
                "# dungeon_record_consumer_pc=\n# dungeon_record_payload_offset=\n"
                "# dungeon_record_byte_count=\n# dungeon_record_window_checksum=\n"
                "# object_table_consumer_pc=\n# object_table_payload_offset=\n"
                "# object_table_byte_count=\n# object_table_window_checksum=\n",
                intake.track02_md5, variant);
        fclose(file);
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_SKELETON_WRITTEN;
        receipt.manifest_skeleton_written = 1;
    } else if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE &&
               receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
    }
    *out = receipt;
    return 1;
}

int theron_v1_track02_external_capture_validate(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt intake;
    Theron_V1Track02RawTraceMediaInput raw_trace;
    Theron_V1Track02CaptureTraceManifest manifest;
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt evidence;
    Theron_V1Track02ExternalCaptureReceipt receipt;

    if (!out || !out_admission ||
        !theron_v1_external_capture_prepare(request, &intake, &receipt)) return 0;
    memset(out_admission, 0, sizeof(*out_admission));
    if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE &&
        receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED &&
        theron_v1_track02_raw_media_intake_prepare_trace_input(&intake, &raw_trace) &&
        theron_v1_track02_capture_trace_manifest_parse(request->manifest_path, &manifest) &&
        theron_v1_track02_capture_trace_manifest_bind(
            &intake, &raw_trace, provenance, preparation, &manifest, &evidence) &&
        theron_v1_track02_capture_trace_runtime_admit(
            &evidence, provenance, preparation, out_admission)) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY;
        receipt.strict_manifest_consumed = 1;
        receipt.runtime_admission_consumed = 1;
    } else if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
    }
    *out = receipt;
    return 1;
}

int theron_v1_track02_external_capture_validate_huc6280_log(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const char *event_log_path,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt intake;
    Theron_V1Track02RawTraceMediaInput raw_trace;
    Theron_V1Track02Huc6280CaptureEventLog event_log;
    Theron_V1Track02CaptureTraceManifest manifest;
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt evidence;
    Theron_V1Track02ExternalCaptureReceipt receipt;

    if (!out || !out_admission ||
        !theron_v1_external_capture_prepare(request, &intake, &receipt)) return 0;
    memset(out_admission, 0, sizeof(*out_admission));
    if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE &&
        receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED &&
        theron_v1_track02_raw_media_intake_prepare_trace_input(&intake, &raw_trace) &&
        theron_v1_track02_huc6280_capture_event_log_parse(event_log_path, &event_log) &&
        theron_v1_track02_huc6280_capture_event_log_bind_manifest(
            &event_log, provenance, preparation, &manifest) &&
        theron_v1_track02_capture_trace_manifest_bind(
            &intake, &raw_trace, provenance, preparation, &manifest, &evidence) &&
        theron_v1_track02_capture_trace_runtime_admit(
            &evidence, provenance, preparation, out_admission)) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY;
        receipt.strict_manifest_consumed = 1;
        receipt.runtime_admission_consumed = 1;
    } else if (receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
    }
    *out = receipt;
    return 1;
}

int theron_v1_track02_external_capture_validate_mednafen_trace(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const Theron_V1Track02MednafenTraceConvertRequest *trace_request,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt intake;
    Theron_V1Track02ExternalCaptureReceipt receipt;
    Theron_V1Track02MednafenTraceConvertReceipt conversion;
    char event_log_md5[33];

    if (!out || !out_admission ||
        !theron_v1_external_capture_prepare(request, &intake, &receipt)) return 0;
    memset(out_admission, 0, sizeof(*out_admission));
    if (receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE ||
        receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED) {
        *out = receipt;
        return 1;
    }
    if (!trace_request || !theron_v1_track02_mednafen_trace_convert_file(
            trace_request, &conversion) ||
        conversion.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED ||
        !conversion.huc6280_event_log_md5_verified ||
        !m12_file_md5_hex(trace_request->event_log_path, event_log_md5) ||
        strcmp(event_log_md5, conversion.event_log_md5) ||
        !theron_v1_track02_external_capture_validate_huc6280_log(
            request, trace_request->event_log_path, provenance, preparation,
            out_admission, &receipt) ||
        receipt.status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY ||
        !m12_file_md5_hex(trace_request->event_log_path, event_log_md5) ||
        strcmp(event_log_md5, conversion.event_log_md5)) {
        receipt.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED;
    } else {
        receipt.mednafen_trace_source_verified = 1;
        receipt.mednafen_event_log_verified = 1;
        snprintf(receipt.mednafen_trace_source_path,
                 sizeof(receipt.mednafen_trace_source_path), "%s",
                 conversion.source_trace_path);
        snprintf(receipt.mednafen_trace_source_md5,
                 sizeof(receipt.mednafen_trace_source_md5), "%s",
                 conversion.source_trace_md5);
        snprintf(receipt.mednafen_event_log_md5,
                 sizeof(receipt.mednafen_event_log_md5), "%s",
                 conversion.event_log_md5);
    }
    *out = receipt;
    return 1;
}
