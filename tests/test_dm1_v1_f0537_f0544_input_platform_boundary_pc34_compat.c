#include "dm1_v1_input_platform_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void assert_boundary(const DM1_V1_InputPlatformBoundaryPc34* boundary,
                            const char* symbol,
                            const char* source_anchor_fragment,
                            const char* rationale_fragment) {
                                (void)rationale_fragment;
                                (void)source_anchor_fragment;
                                (void)symbol;
                                (void)boundary;
    assert(boundary != 0);
    assert(strcmp(boundary->symbol, symbol) == 0);
    assert(strstr(boundary->source_anchor, source_anchor_fragment) != 0);
    assert(boundary->has_portable_pc34_equivalent == 0);
    assert(dm1_v1_input_platform_boundary_is_portable_pc34(boundary) == 0);
    assert(strstr(boundary->rationale, rationale_fragment) != 0);
}

int main(void) {
    const char* evidence;
    (void)evidence;

    assert_boundary(F0537_INPUT_ReleaseResources_PlatformBoundaryPc34(),
                    "F0537_INPUT_ReleaseResources",
                    "INPUT.C:210-230",
                    "FreeSignal");
    assert_boundary(F0544_INPUT_ResetPressingEyeOrMouth_PlatformBoundaryPc34(),
                    "F0544_INPUT_ResetPressingEyeOrMouth",
                    "INPUT.C:754-768",
                    "no I34E/I34M PC3.4 caller");

    assert(dm1_v1_input_platform_boundary_is_portable_pc34(0) == 0);

    evidence = dm1_v1_input_platform_boundary_source_evidence_pc34();
    assert(evidence != 0);
    assert(strstr(evidence, "F0544_PC34_SOURCE_AUDIT") != 0);
    assert(strstr(evidence, "REDMCSB_MISSING_PLATFORM_BOUNDARIES") != 0);
    return 0;
}
