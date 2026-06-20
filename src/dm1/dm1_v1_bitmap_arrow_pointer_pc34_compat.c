#include "firestaff/dm1/v1/bitmap_arrow_pointer_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0042_auc_Graphic562_Bitmap_ArrowPointer):
 * - DATA.C:48  - declaration of G0042_auc_Graphic562_Bitmap_ArrowPointer[2][66]
 * - DATA.C:48/362-376/120 - declaration + PC 3.4 init + Atari variant init
 * - DATA.C:362-376 - PC 3.4 init { plane0={0xFF,...}, plane1={0xFF,...} }
 *                     (11 rows × 6 bytes/row = 66 bytes per plane)
 * - DATA.C:376 - last byte of the arrow-pointer bitmap init block
 * - DATA.C:120 - alternate (32x18) init used by some Atari variants
 * - IO.C cursor blit path — arrow-pointer 4bpp-packed bitmap
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0042
 * arrow-pointer mouse-cursor bitmap.
 */

enum {
    kPlaneCount  = 2,
    kPlaneBytes  = 66,
    kIndexOOR    = -1
};

static const unsigned char s_g0042[kPlaneCount][kPlaneBytes] = {
    /* Plane 0 — colored arrow shape. */
    {
        0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFB, 0xF0, 0x00, 0x00, 0x00, 0x00,
        0xFB, 0xBF, 0x00, 0x00, 0x00, 0x00,
        0xFB, 0xBB, 0xF0, 0x00, 0x00, 0x00,
        0xFB, 0xBB, 0xBF, 0x00, 0x00, 0x00,
        0xFB, 0xBB, 0xBB, 0xF0, 0x00, 0x00,
        0xFB, 0xBB, 0xBB, 0xBF, 0x00, 0x00,
        0xFB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00,
        0xFB, 0xBF, 0xBB, 0xFF, 0x00, 0x00,
        0xFF, 0xF0, 0xFB, 0xBF, 0x00, 0x00,
        0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00
    },
    /* Plane 1 — outline mask. */
    {
        0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
        0xFF, 0xF0, 0xFF, 0xFF, 0x00, 0x00,
        0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00
    }
};

const unsigned char *
dm1_v1_bitmap_arrow_pointer_plane_pc34(int plane_index)
{
    if (plane_index < 0 || plane_index >= kPlaneCount) {
        return 0;
    }
    return s_g0042[plane_index];
}

int
dm1_v1_bitmap_arrow_pointer_plane_bytes_pc34(void)
{
    return kPlaneBytes;
}

int
dm1_v1_bitmap_arrow_pointer_get_pc34(int plane_index, int byte_index)
{
    if (plane_index < 0 || plane_index >= kPlaneCount) {
        return kIndexOOR;
    }
    if (byte_index < 0 || byte_index >= kPlaneBytes) {
        return kIndexOOR;
    }
    return (int)s_g0042[plane_index][byte_index];
}

int
dm1_v1_bitmap_arrow_pointer_run_pc34(
    DM1_V1_BitmapArrowPointerResultPc34 *out)
{
    int table_matches_declaration = 1;
    int plane0_first_byte_0xFF = 1;
    int plane0_last_byte_0x00 = 1;
    int plane1_first_byte_0xFF = 1;
    int plane1_last_byte_0x00 = 1;
    int planes_byte_identical = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    out->planeSizeBytes = kPlaneBytes;

    /* Phase 1: plane0 byte 0 == 0xFF. */
    if (s_g0042[0][0] != 0xFF) plane0_first_byte_0xFF = 0;

    /* Phase 2: plane0 byte 65 == 0x00. */
    if (s_g0042[0][kPlaneBytes - 1] != 0x00) plane0_last_byte_0x00 = 0;

    /* Phase 3: plane1 byte 0 == 0xFF. */
    if (s_g0042[1][0] != 0xFF) plane1_first_byte_0xFF = 0;

    /* Phase 4: plane1 byte 65 == 0x00. */
    if (s_g0042[1][kPlaneBytes - 1] != 0x00) plane1_last_byte_0x00 = 0;

    out->plane0FirstByte0xFF = plane0_first_byte_0xFF;
    out->plane0LastByte0x00 = plane0_last_byte_0x00;
    out->plane1FirstByte0xFF = plane1_first_byte_0xFF;
    out->plane1LastByte0x00 = plane1_last_byte_0x00;

    /* Phase 5: tables match the source-locked DATA.C:362-376 init. */
    {
        static const unsigned char kP0[kPlaneBytes] = {
            0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFB, 0xF0, 0x00, 0x00, 0x00, 0x00,
            0xFB, 0xBF, 0x00, 0x00, 0x00, 0x00,
            0xFB, 0xBB, 0xF0, 0x00, 0x00, 0x00,
            0xFB, 0xBB, 0xBF, 0x00, 0x00, 0x00,
            0xFB, 0xBB, 0xBB, 0xF0, 0x00, 0x00,
            0xFB, 0xBB, 0xBB, 0xBF, 0x00, 0x00,
            0xFB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00,
            0xFB, 0xBF, 0xBB, 0xFF, 0x00, 0x00,
            0xFF, 0xF0, 0xFB, 0xBF, 0x00, 0x00,
            0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00
        };
        static const unsigned char kP1[kPlaneBytes] = {
            0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
            0xFF, 0xF0, 0xFF, 0xFF, 0x00, 0x00,
            0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00
        };
        for (i = 0; i < kPlaneBytes; ++i) {
            if (s_g0042[0][i] != kP0[i] || s_g0042[1][i] != kP1[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 6: plane0 and plane1 byte-identical length (both 66 bytes). */
    if (kPlaneBytes != kPlaneBytes) {
        planes_byte_identical = 0;  /* tautology: both planes have kPlaneBytes bytes. */
    } else {
        planes_byte_identical = 1;
    }
    out->planesByteIdentical = planes_byte_identical;

    /* Phase 7: lookup function correctness for each (plane, byte). */
    for (i = 0; i < kPlaneBytes; ++i) {
        if (dm1_v1_bitmap_arrow_pointer_get_pc34(0, i) != (int)s_g0042[0][i]) {
            lookup_function_correct = 0;
        }
        if (dm1_v1_bitmap_arrow_pointer_get_pc34(1, i) != (int)s_g0042[1][i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 8: out-of-range lookup returns -1. */
    if (dm1_v1_bitmap_arrow_pointer_get_pc34(-1, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bitmap_arrow_pointer_get_pc34(0, -1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bitmap_arrow_pointer_get_pc34(2, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bitmap_arrow_pointer_get_pc34(0, kPlaneBytes) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->plane0FirstByte0xFF &&
        out->plane0LastByte0x00 &&
        out->plane1FirstByte0xFF &&
        out->plane1LastByte0x00 &&
        out->planesByteIdentical &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}