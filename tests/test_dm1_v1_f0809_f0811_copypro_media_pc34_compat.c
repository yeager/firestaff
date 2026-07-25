#include "dm1_v1_f0809_f0811_copypro_media_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_diagnostic_constants(void)
{
    assert(DM1_V1_F0809_COPYPRO_DIAGNOSTIC_BYTES_PC34 == 1088);
    assert(DM1_V1_F0810_COPYPRO_DIAGNOSTIC_BYTES_PC34 == 5840);
    assert(DM1_V1_F0810_COPYPRO_CHECKED_BYTES_PC34 == 5836);
}

static void test_media_kind_enum(void)
{
    assert(DM1_V1_COPYPRO_MEDIA_NONE_PC34 == 0);
    assert(DM1_V1_COPYPRO_MEDIA_INSTALL_FILES_PC34 == 1);
    assert(DM1_V1_COPYPRO_MEDIA_SECTOR_IMAGE_PC34 == 2);
    assert(DM1_V1_COPYPRO_MEDIA_AUTHENTICATED_FLUX_CAPTURE_PC34 == 3);
}

static void test_read_id_struct(void)
{
    DM1_V1_F0811ReadIdPc34 id;
    memset(&id, 0, sizeof(id));
    assert(id.dl == 0);
    assert(id.cl == 0);
    assert(id.dh == 0);
    assert(id.ch == 0);
}

static void test_capture_struct(void)
{
    DM1_V1_CopyProtectionCapturePc34 c;
    memset(&c, 0, sizeof(c));
    assert(c.mediaKind == DM1_V1_COPYPRO_MEDIA_NONE_PC34);
    assert(c.mediaHashVerified == 0);
    assert(c.track0Diagnostic == NULL);
    assert(c.track2Diagnostic == NULL);
}

static void test_receipt_struct(void)
{
    DM1_V1_CopyProtectionReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticDiskResponse == 0);
    assert(r.functionId == 0);
}

static void test_verify_null_receipt(void)
{
    int ok = dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(809, NULL, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_verify_null_capture(void)
{
    DM1_V1_CopyProtectionReceiptPc34 r;
    int ok = dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(809, NULL, &r);
    (void)ok;
    assert(ok == 0);
    assert(r.valid == 0);
}

static void test_verify_invalid_function(void)
{
    DM1_V1_CopyProtectionCapturePc34 c;
    DM1_V1_CopyProtectionReceiptPc34 r;
    memset(&c, 0, sizeof(c));
    c.mediaKind = DM1_V1_COPYPRO_MEDIA_AUTHENTICATED_FLUX_CAPTURE_PC34;
    c.mediaHashVerified = 1;
    int ok = dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(999, &c, &r);
    (void)ok;
    assert(ok == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_f0809_f0811_copypro_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_diagnostic_constants();
    test_media_kind_enum();
    test_read_id_struct();
    test_capture_struct();
    test_receipt_struct();
    test_verify_null_receipt();
    test_verify_null_capture();
    test_verify_invalid_function();
    test_source_evidence();

    puts("ok: DM1 F0809/F0811 copy protection media (Q-DM1-08) 9 tests passed");
    return 0;
}
