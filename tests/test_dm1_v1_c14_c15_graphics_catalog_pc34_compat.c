#include "dm1_v1_c14_c15_graphics_catalog_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_max_surfaces(void)
{
    assert(DM1_V1_C14_C15_GRAPHICS_CATALOG_MAX_SURFACES == 64);
}

static void test_catalog_struct(void)
{
    DM1_V1_C14C15GraphicsCatalogPc34 c;
    memset(&c, 0, sizeof(c));
    assert(c.valid == 0);
    assert(c.surfaceCount == 0);
    assert(c.sourceAnchor == NULL);
}

static void test_entry_struct(void)
{
    DM1_V1_C14C15GraphicsCatalogEntryPc34 e;
    memset(&e, 0, sizeof(e));
    assert(e.surface == NULL);
    assert(e.graphicIndex == 0);
    assert(e.pixelsFNV1a == 0);
}

static void test_build_null_out(void)
{
    int ok = dm1_v1_c14_c15_graphics_catalog_build_pc34(NULL, 0, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_build_null_surfaces(void)
{
    DM1_V1_C14C15GraphicsCatalogPc34 c;
    int ok = dm1_v1_c14_c15_graphics_catalog_build_pc34(NULL, 0, &c);
    (void)ok;
    assert(ok == 1);
    assert(c.valid == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_c14_c15_graphics_catalog_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

static void test_admit_null_receipt(void)
{
    int ok = dm1_v1_c14_c15_graphics_catalog_admit_receipt_pc34(
        NULL, NULL, DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34, 0, 0, 0);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_max_surfaces();
    test_catalog_struct();
    test_entry_struct();
    test_build_null_out();
    test_build_null_surfaces();
    test_source_evidence();
    test_admit_null_receipt();

    puts("ok: DM1 C14/C15 graphics catalog (Q-DM1-03) 7 tests passed");
    return 0;
}
