#include "csb_v1_viewport_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static void write_le16(uint8_t *bytes, size_t word_index, uint16_t value)
{
    const size_t offset = word_index * 2u;
    bytes[offset] = (uint8_t)(value & 0xffu);
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void build_skin_def(uint8_t *skin_def, size_t size)
{
    memset(skin_def, 0, size);
    write_le16(skin_def, 0, 0x0001u); /* LARGE bitmap and CSB background graphic id */
    write_le16(skin_def, 1, 0x2221u); /* NEAR bitmap */
    write_le16(skin_def, 2, 0x3331u); /* MIDDLE bitmap */
    write_le16(skin_def, 4, 0x4441u); /* LARGE mask */
    write_le16(skin_def, 5, 0x5551u); /* NEAR mask */
    write_le16(skin_def, 6, 0x6661u); /* MIDDLE mask */
}

static int expect_valid_selection(
    const char *label,
    const uint8_t *skin_def,
    CSB_V1_ViewportCustomBackgroundViewIndex view_index,
    size_t want_bitmap_offset,
    size_t want_mask_offset,
    uint8_t want_bitmap_first,
    uint8_t want_mask_first)
{
    int ok = 1;
    CSB_V1_ViewportCustomBackgroundSelection selection =
        csb_v1_viewport_custom_background_load_and_select_pc34(
            skin_def, 32u, view_index);
    char id[128];

    snprintf(id, sizeof(id), "%s.valid", label);
    ok &= expect_int(id, selection.is_valid, 1);
    snprintf(id, sizeof(id), "%s.bitmap_ptr", label);
    ok &= expect_int(id, selection.bitmap == skin_def + want_bitmap_offset, 1);
    snprintf(id, sizeof(id), "%s.mask_ptr", label);
    ok &= expect_int(id, selection.mask == skin_def + want_mask_offset, 1);
    snprintf(id, sizeof(id), "%s.bitmap_first", label);
    ok &= expect_int(id, selection.bitmap ? selection.bitmap[0] : -1,
                     want_bitmap_first);
    snprintf(id, sizeof(id), "%s.mask_first", label);
    ok &= expect_int(id, selection.mask ? selection.mask[0] : -1,
                     want_mask_first);
    snprintf(id, sizeof(id), "%s.byte_width_nonzero", label);
    ok &= expect_int(id, selection.byte_width > 0, 1);
    snprintf(id, sizeof(id), "%s.height_nonzero", label);
    ok &= expect_int(id, selection.height > 0, 1);
    snprintf(id, sizeof(id), "%s.within_large_limit", label);
    ok &= expect_int(id, selection.byte_width * selection.height <= 7840, 1);
    return ok;
}

int main(void)
{
    int ok = 1;
    uint8_t skin_def[32];
    uint8_t truncated[8];
    CSB_V1_ViewportCustomBackgroundSelection invalid;

    printf("probe=csb_v1_viewport_custom_background_pc34_compat\n");
    printf("sourceEvidence=%s\n", csb_v1_viewport_source_evidence());

    build_skin_def(skin_def, sizeof(skin_def));
    ok &= expect_valid_selection("d3l.large",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L,
                                 0u,
                                 8u,
                                 0x01u,
                                 0x41u);
    ok &= expect_valid_selection("d2c.middle",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D2C,
                                 4u,
                                 12u,
                                 0x31u,
                                 0x61u);
    ok &= expect_valid_selection("d1r.near",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D1R,
                                 2u,
                                 10u,
                                 0x21u,
                                 0x51u);
    ok &= expect_valid_selection("d0l.near",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D0L,
                                 2u,
                                 10u,
                                 0x21u,
                                 0x51u);
    ok &= expect_valid_selection("d3l2.large",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L2,
                                 0u,
                                 8u,
                                 0x01u,
                                 0x41u);
    ok &= expect_valid_selection("d3r2.large",
                                 skin_def,
                                 CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R2,
                                 0u,
                                 8u,
                                 0x01u,
                                 0x41u);

    memset(truncated, 0, sizeof(truncated));
    invalid = csb_v1_viewport_custom_background_load_and_select_pc34(
        truncated, sizeof(truncated), CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L);
    ok &= expect_int("truncated.invalid", invalid.is_valid, 0);
    ok &= expect_int("truncated.zero_bitmap", invalid.bitmap == NULL, 1);
    ok &= expect_int("truncated.zero_mask", invalid.mask == NULL, 1);
    ok &= expect_int("truncated.zero_byte_width", invalid.byte_width, 0);
    ok &= expect_int("truncated.zero_height", invalid.height, 0);

    build_skin_def(skin_def, sizeof(skin_def));
    skin_def[0] = 0xffu;
    invalid = csb_v1_viewport_custom_background_load_and_select_pc34(
        skin_def, sizeof(skin_def), CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L);
    ok &= expect_int("wrong_graphic.invalid", invalid.is_valid, 0);
    ok &= expect_int("wrong_graphic.zero_bitmap", invalid.bitmap == NULL, 1);
    ok &= expect_int("wrong_graphic.zero_mask", invalid.mask == NULL, 1);
    ok &= expect_int("wrong_graphic.zero_byte_width", invalid.byte_width, 0);
    ok &= expect_int("wrong_graphic.zero_height", invalid.height, 0);

    return ok ? 0 : 1;
}
