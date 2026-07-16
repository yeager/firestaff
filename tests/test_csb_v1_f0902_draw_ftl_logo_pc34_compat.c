#include "csb_v1_f0902_draw_ftl_logo_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_FtlLogoFacts_PC34 make_logo_facts(void)
{
    CSB_V1_FtlLogoFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.source_ftl_logo_bitmap_bound = 1;
    facts.source_ftl_logo_palette_bound = 1;
    facts.width = CSB_V1_F0902_FTL_LOGO_WIDTH_PC34;
    facts.height = CSB_V1_F0902_FTL_LOGO_HEIGHT_PC34;
    facts.packed_stride_bytes =
        CSB_V1_F0902_FTL_LOGO_PACKED_STRIDE_BYTES_PC34;
    facts.palette_color_count = CSB_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34;
    facts.source_bitmap_hash = 0x09023200u;
    facts.source_palette_hash = 0x09020016u;
    facts.before_swoosh_sound_init = 1;
    facts.title_not_started_yet = 1;
    facts.no_synthetic_graphic_bytes = 1;
    facts.no_synthetic_palette_data = 1;
    facts.no_legacy_logo_wrapper = 1;
    return facts;
}

static void test_accepts_source_logo_shape_before_swoosh(void)
{
    CSB_V1_FtlLogoFacts_PC34 facts = make_logo_facts();
    CSB_V1_FtlLogoReceipt_PC34 receipt;

    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.source_bitmap_consumed == 1);
    CHECK(receipt.source_palette_consumed == 1);
    CHECK(receipt.width == 320);
    CHECK(receipt.height == 200);
    CHECK(receipt.packed_stride_bytes == 160);
    CHECK(receipt.palette_color_count == 16);
    CHECK(receipt.source_bitmap_hash == 0x09023200u);
    CHECK(receipt.source_palette_hash == 0x09020016u);
    CHECK(receipt.before_swoosh_sound_init == 1);
    CHECK(receipt.title_not_started_yet == 1);
    CHECK(receipt.no_synthetic_graphic_bytes == 1);
    CHECK(receipt.no_synthetic_palette_data == 1);
    CHECK(receipt.no_legacy_logo_wrapper == 1);
}

static void test_rejects_wrong_or_synthetic_logo_routes(void)
{
    CSB_V1_FtlLogoFacts_PC34 facts = make_logo_facts();
    CSB_V1_FtlLogoReceipt_PC34 receipt;

    facts.width = 319;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);
    CHECK(receipt.no_synthetic_graphic_bytes == 1);

    facts = make_logo_facts();
    facts.packed_stride_bytes = 161;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);

    facts = make_logo_facts();
    facts.source_bitmap_hash = 0u;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);

    facts = make_logo_facts();
    facts.before_swoosh_sound_init = 0;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);

    facts = make_logo_facts();
    facts.no_synthetic_palette_data = 0;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);

    facts = make_logo_facts();
    facts.no_legacy_logo_wrapper = 0;
    CHECK(F0902_DrawFTLLogo(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    check_contains(csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(),
                   "F0902_DrawFTLLogo");
    check_contains(csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(),
                   "320x200 packed frame");
    check_contains(csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(),
                   "160-byte rows");
    check_contains(csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(),
                   "before F0908_InitSound");
}

int main(void)
{
    test_accepts_source_logo_shape_before_swoosh();
    test_rejects_wrong_or_synthetic_logo_routes();
    test_evidence_string();
    return 0;
}
