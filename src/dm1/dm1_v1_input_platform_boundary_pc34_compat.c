#include "dm1_v1_input_platform_boundary_pc34_compat.h"

static const DM1_V1_InputPlatformBoundaryPc34 kF0537 = {
    "F0537_INPUT_ReleaseResources",
    "INPUT.C:210-230; AMIGA.H:338",
    "Amiga-host input resource teardown",
    0,
    "FreeSignal/DeletePort/DeleteStdIO/FreeMem release Amiga input resources; "
    "no I34E/I34M PC3.4 runtime route or portable host-resource substitute is evidenced."
};

static const DM1_V1_InputPlatformBoundaryPc34 kF0544 = {
    "F0544_INPUT_ResetPressingEyeOrMouth",
    "INPUT.C:754-768; INPUT.C:673-701; AMIGA.H:345",
    "Amiga-host input interrupt state",
    0,
    "MEDIA433_A20E_A20F_A20G_A21E_A22E_A22G guarded path clears Amiga "
    "mouse-interrupt pressing state under Forbid/Permit; no I34E/I34M PC3.4 caller is evidenced."
};

const DM1_V1_InputPlatformBoundaryPc34*
F0537_INPUT_ReleaseResources_PlatformBoundaryPc34(void) {
    return &kF0537;
}

const DM1_V1_InputPlatformBoundaryPc34*
F0544_INPUT_ResetPressingEyeOrMouth_PlatformBoundaryPc34(void) {
    return &kF0544;
}

int dm1_v1_input_platform_boundary_is_portable_pc34(const DM1_V1_InputPlatformBoundaryPc34* boundary) {
    if (!boundary) return 0;
    return boundary->has_portable_pc34_equivalent != 0;
}

const char* dm1_v1_input_platform_boundary_source_evidence_pc34(void) {
    return "2026-07-14_REDMCSB_CALLABLE_SYMBOL_AUDIT_AFTER_F0534.tsv; "
           "2026-07-14_REDMCSB_F0544_PC34_SOURCE_AUDIT.tsv; "
           "REDMCSB_MISSING_PLATFORM_BOUNDARIES.tsv";
}
