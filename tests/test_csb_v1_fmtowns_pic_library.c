#include "csb_v1_fmtowns_pic_library.h"
#include "csb_v1_fmtowns_cd.h"
#include "firestaff_zip_extract.h"

/* The FM Towns archive is real-media coverage, not an optional synthetic
 * probe.  Its reads and receipts live in assertions, so retain them in
 * Release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_null_gate(void) {
    dm1_v1_fmtowns_pic_library_view_t v;
    uint8_t buf[10] = {0};
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(NULL, 10, &v) == 0);
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 10, NULL) == 0);
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 3, &v) == 0);
}

static void test_wrong_sig_rejects(void) {
    dm1_v1_fmtowns_pic_library_view_t v;
    /* sig != 0x8001. */
    uint8_t buf[10] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 10, &v) == 0);
}

static void verify_all_original_spans(const dm1_v1_fmtowns_pic_library_view_t *view) {
    size_t next = view->payload_offset;
    const uint8_t *bytes = NULL;
    uint16_t size = 0;
    for (uint16_t index = 0; index < view->asset_count; ++index) {
        size_t offset = 0;
        assert(dm1_v1_fmtowns_pic_library_asset_offset_pc34(view, index, &offset) ==
               DM1_V1_FMTOWNS_PIC_LIB_OK);
        assert(offset == next);
        assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(view, index, &bytes, &size) ==
               DM1_V1_FMTOWNS_PIC_LIB_OK);
        assert(next <= view->data_size && size <= view->data_size - next);
        assert(bytes == view->data + next);
        next += size;
    }
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(view, view->asset_count,
        &bytes, &size) == DM1_V1_FMTOWNS_PIC_LIB_ERR_INDEX);
    /* Container spans are not all necessarily pixel graphics; do not infer
     * DECODEGRAPHIC dimensions or visual parity from successful indexing. */
    printf("PASS: all %u original asset spans are contiguous and bounded\n",
           (unsigned)view->asset_count);
}

static void test_real_data_csb(void) {
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    uint8_t *image = NULL;
    uint8_t *buf = NULL;
    size_t image_size = 0u;
    size_t size = 0u;
    CSB_V1_FmtownsCdLayout layout;
    const CSB_V1_FmtownsCdFile *entry;
    if (!archive || !archive[0]) { puts("SKIP: no CSB FM Towns archive"); return; }
    assert(firestaff_zip_extract_by_suffix(archive, ".img", &image,
                                            &image_size) == 0 && image);
    assert(csb_v1_fmtowns_cd_parse(image, image_size, &layout) == 0);
    entry = csb_v1_fmtowns_cd_find(&layout, "CDATA", "GRAPHICS.DAT");
    assert(entry && entry->size != 0u);
    buf = (uint8_t *)malloc(entry->size);
    assert(buf && csb_v1_fmtowns_cd_extract(image, image_size, entry, buf,
                                             entry->size) == 0);
    size = entry->size;
    dm1_v1_fmtowns_pic_library_view_t v;
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, size, &v) == 1);
    /* CSB has 728 assets, first asset size = 0xef (239). */
    assert(v.asset_count == 728);
    verify_all_original_spans(&v);
    uint16_t first_size = 0;
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&v, 0, &first_size) ==
           DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(first_size == 0xef);
    /* Asset 10 = size 0x38 (56 bytes, byte-verified earlier). */
    uint16_t sz10 = 0;
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&v, 9, &sz10) ==
           DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(sz10 == 0x38);
    free(buf);
    entry = csb_v1_fmtowns_cd_find(&layout, "CJDATA", "GRAPHICS.DAT");
    assert(entry && entry->size != 0u);
    buf = (uint8_t *)malloc(entry->size);
    assert(buf && csb_v1_fmtowns_cd_extract(image, image_size, entry, buf,
                                             entry->size) == 0);
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, entry->size,
                                                        &v) == 1);
    assert(v.asset_count == 728);
    verify_all_original_spans(&v);
    free(buf);
    free(image);
    puts("PASS: CSB CDATA/CJDATA GRAPHICS.DAT open from original ZIP/IMG in RAM");
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 77;
    }
    test_null_gate();
    test_wrong_sig_rejects();
    test_real_data_csb();
    puts("All csb_v1_fmtowns_pic_library tests passed.");
    return 0;
}
