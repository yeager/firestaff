#include "dm1_v1_amiga_platform_boundary_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_f0513_boundary(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F0513_DIALOG_DrawGameReadyToPlay_Unreferenced_PlatformBoundaryPc34();
    assert(b != NULL);
    assert(b->symbol != NULL);
    assert(b->platform_partition != NULL);
}

static void test_f0535_boundary(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F0535_MEMORY_GetGraphicsDatFileSize_PlatformBoundaryPc34();
    assert(b != NULL);
    assert(b->symbol != NULL);
}

static void test_f0551_boundary(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F0551_VIDEO_HatchBox_Unreferenced_PlatformBoundaryPc34();
    assert(b != NULL);
}

static void test_f0557_scroller_init(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F0557_SCROLLER_Initialize_PlatformBoundaryPc34();
    assert(b != NULL);
    assert(b->symbol != NULL);
}

static void test_f1111_cpsx(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F1111_CPSX_PlatformBoundaryPc34();
    assert(b != NULL);
}

static void test_is_portable(void)
{
    const DM1_V1_AmigaPlatformBoundaryPc34* b =
        F0535_MEMORY_GetGraphicsDatFileSize_PlatformBoundaryPc34();
    int p = dm1_v1_amiga_platform_boundary_is_portable_pc34(b);
    (void)p;
    assert(p == 0 || p == 1);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_amiga_platform_boundary_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_f0513_boundary();
    test_f0535_boundary();
    test_f0551_boundary();
    test_f0557_scroller_init();
    test_f1111_cpsx();
    test_is_portable();
    test_source_evidence();

    puts("ok: DM1 amiga platform boundary (Q-DM1-08) 7 tests passed");
    return 0;
}
