#include "theron_v1_trace_v3_schema.h"
#include <stdio.h>
#include <string.h>

static int check(const char *text, int expected) {
    Theron_V1TraceV3SchemaReceipt receipt;
    int got = theron_v1_trace_v3_schema_validate(text, &receipt);
    return got == expected && receipt.valid == expected;
}
int main(void) {
    const char *ok = "trace_format=theron_irq2_capture_v3\n"
        "loader_registers phase=pre4090 fc=00\n"
        "loader_registers phase=post4093 fc=00\n"
        "pcecd_epoch transfer_epoch=physical_stage2_stage3 first_io_seq=4\n";
    Theron_V1TraceV3SchemaReceipt schema;
    Theron_V1Stage2RuntimeHandoff stage3 = {0};
    Theron_V1TraceV3BindingReceipt binding;
    Theron_V1TraceV3ImportRequest request;
    Theron_V1TraceV3CaptureLaunch launch;
    char command[256];
    Theron_V1TraceV3CommandPlan plan;
    Theron_V1_TrackMediaAvailabilityReceipt media={THERON_V1_TRACK_MEDIA_RAW_READY,1};
    Theron_V1TraceV3LauncherReceipt launcher;
    if (strcmp(theron_v1_trace_v3_preflight_status("", "", 0),
               "system_card_missing") ||
        strcmp(theron_v1_trace_v3_preflight_status("card", "bad", 0),
               "system_card_hash_mismatch") ||
        strcmp(theron_v1_trace_v3_preflight_status("card",
               "0123456789abcdef0123456789abcdef", 0),
               "trace_capture_required")) return 1;
    if (strcmp(theron_v1_trace_v3_raw_track02_status("", "card"),
               "raw_track02_missing") ||
        strcmp(theron_v1_trace_v3_raw_track02_status(THERON_TRACK02_MD5_US_ISO,
               "card"), "raw_track02_iso_end_variant") ||
        strcmp(theron_v1_trace_v3_raw_track02_status(THERON_TRACK02_MD5_US_BIN,
               ""), "system_card_missing")) return 1;
    if (!check(ok, 1) || !check("trace_format=theron_irq2_capture_v2\n", 0) ||
        !check("trace_format=theron_irq2_capture_v3\nloader_registers phase=post4093 x\n", 0)) {
        return 1;
    }
    theron_v1_trace_v3_schema_validate(ok, &schema);
    stage3.valid = 1;
    stage3.stage3_cd_read_record_proven = 1;
    stage3.stage3_cd_read_record = 0x4dfu;
    if (!theron_v1_trace_v3_bind_stage3(&schema, &stage3, "cue", "track", 1,
                                         &binding) || !binding.valid ||
        binding.runtime_allowed || !binding.source_hashes_bound ||
        !binding.epoch_matches_stage3 ||
        theron_v1_trace_v3_bind_stage3(&schema, &stage3, "", "track", 1,
                                        &binding) ||
        theron_v1_trace_v3_bind_stage3(&schema, &stage3, "cue", "track", 0,
                                        &binding)) return 1;
    if (!theron_v1_trace_v3_bind_stage3(&schema, &stage3,
                                         "0123456789abcdef0123456789abcdef",
                                         "fedcba9876543210fedcba9876543210", 1,
                                         &binding) ||
        !theron_v1_trace_v3_import_request(&binding, &stage3,
                                             "0123456789abcdef0123456789abcdef",
                                             "fedcba9876543210fedcba9876543210",
                                             &request) || !request.runtime_blocked ||
        theron_v1_trace_v3_import_request(&binding, &stage3, "bad",
                                           "fedcba9876543210fedcba9876543210", &request)) return 1;
    if (!theron_v1_trace_v3_import_request(&binding, &stage3,
                                             "0123456789abcdef0123456789abcdef",
                                             "fedcba9876543210fedcba9876543210",
                                             &request) ||
        !theron_v1_trace_v3_capture_launch(&request, "cue", "track", "card",
                                            "0123456789abcdef0123456789abcdef", &launch) ||
        !launch.runtime_blocked ||
        theron_v1_trace_v3_capture_launch(&request, "cue", "track", "",
                                           "0123456789abcdef0123456789abcdef", &launch)) return 1;
    if (!theron_v1_trace_v3_capture_launch(&request, "cue", "track", "card",
                                            "0123456789abcdef0123456789abcdef", &launch) ||
        !theron_v1_trace_v3_format_command(&launch, "mednafen", "cue", "track",
                                            "card", "trace", command, sizeof(command)) ||
        !strstr(command, "FIRESTAFF_THERON_IRQ2_TRACE=trace") ||
        theron_v1_trace_v3_format_command(&launch, "bad\npath", "cue", "track",
                                           "card", "trace", command, sizeof(command))) return 1;
    if (!theron_v1_trace_v3_plan_command(&request, &launch, "mednafen", "cue",
                                         "track", "card", "trace", &plan) ||
        !plan.runtime_blocked ||
        theron_v1_trace_v3_plan_command(&request, &launch, "mednafen", "cue",
                                         "track", "card", "bad\ntrace", &plan)) return 1;
    media.availability=THERON_V1_TRACK_MEDIA_END_VARIANT;
    if(theron_v1_trace_v3_plan_command_with_media(&media,&request,&launch,"mednafen","cue","track","card","trace",&plan))return 1;
    media.availability=THERON_V1_TRACK_MEDIA_MISSING;
    media.loader_usable=0;
    memset(&plan,0,sizeof(plan));
    if(theron_v1_trace_v3_plan_command_with_media(&media,&request,&launch,"mednafen","cue","track","card","trace",&plan) ||
       theron_v1_trace_v3_launcher_receipt(&plan,&launcher))return 1;
    if (!theron_v1_trace_v3_plan_command(&request, &launch, "mednafen", "cue",
                                         "track", "card", "trace", &plan) ||
        !theron_v1_trace_v3_launcher_receipt(&plan, &launcher) ||
        !launcher.capture_command_ready || launcher.trace_validated ||
        !launcher.runtime_blocked) return 1;
    if (theron_v1_trace_v3_preflight_adapter(&launcher, "system_card_missing") !=
            THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING ||
        theron_v1_trace_v3_preflight_adapter(&launcher, "trace_capture_required") !=
            THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID) return 1;
    puts("PASS trace v3 schema fail-closed");
    return 0;
}
