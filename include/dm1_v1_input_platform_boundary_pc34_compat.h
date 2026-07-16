#ifndef FIRESTAFF_DM1_V1_INPUT_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INPUT_PLATFORM_BOUNDARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* symbol;
    const char* source_anchor;
    const char* partition;
    int has_portable_pc34_equivalent;
    const char* rationale;
} DM1_V1_InputPlatformBoundaryPc34;

const DM1_V1_InputPlatformBoundaryPc34*
F0537_INPUT_ReleaseResources_PlatformBoundaryPc34(void);

const DM1_V1_InputPlatformBoundaryPc34*
F0544_INPUT_ResetPressingEyeOrMouth_PlatformBoundaryPc34(void);

int dm1_v1_input_platform_boundary_is_portable_pc34(const DM1_V1_InputPlatformBoundaryPc34* boundary);
const char* dm1_v1_input_platform_boundary_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INPUT_PLATFORM_BOUNDARY_PC34_COMPAT_H */
