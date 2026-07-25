#include "dm1_v1_automap_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

static void test_record_visit_null(void)
{
    DM1_V1_AutoMap_RecordVisitPc34Compat(NULL);
}

static void test_export_current_level_null(void)
{
    int ok = DM1_V1_AutoMap_ExportCurrentLevelPc34Compat(NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_export_png_null(void)
{
    int ok = DM1_V1_AutoMap_ExportPNGPc34Compat(NULL, 0, NULL);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_record_visit_null();
    test_export_current_level_null();
    test_export_png_null();

    puts("ok: DM1 automap (Q-DM1-03) 3 tests passed");
    return 0;
}
