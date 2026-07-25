#include "dm1_v1_input_platform_boundary_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_f0537_boundary(void)
{
    const DM1_V1_InputPlatformBoundaryPc34* b =
        F0537_INPUT_ReleaseResources_PlatformBoundaryPc34();
    assert(b != NULL);
    assert(b->symbol != NULL);
    assert(b->source_anchor != NULL);
    assert(b->partition != NULL);
    assert(b->has_portable_pc34_equivalent == 0);
    assert(b->rationale != NULL);
}

static void test_f0544_boundary(void)
{
    const DM1_V1_InputPlatformBoundaryPc34* b =
        F0544_INPUT_ResetPressingEyeOrMouth_PlatformBoundaryPc34();
    assert(b != NULL);
    assert(b->symbol != NULL);
    assert(b->has_portable_pc34_equivalent == 0);
}

static void test_is_portable_null(void)
{
    int p = dm1_v1_input_platform_boundary_is_portable_pc34(NULL);
    (void)p;
    assert(p == 0);
}

static void test_is_portable_f0537(void)
{
    const DM1_V1_InputPlatformBoundaryPc34* b =
        F0537_INPUT_ReleaseResources_PlatformBoundaryPc34();
    int p = dm1_v1_input_platform_boundary_is_portable_pc34(b);
    (void)p;
    assert(p == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_input_platform_boundary_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_f0537_boundary();
    test_f0544_boundary();
    test_is_portable_null();
    test_is_portable_f0537();
    test_source_evidence();

    puts("ok: DM1 input platform boundary (Q-DM1-08) 5 tests passed");
    return 0;
}
