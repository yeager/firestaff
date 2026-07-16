#include "dm1_v1_portrait_panel_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

static void test_guard_paths(void)
{
    uint8_t planar[DM1_PORTRAIT_PLANAR_BYTES];
    uint8_t chunky[DM1_PORTRAIT_CHUNKY_BYTES];

    memset(planar, 0, sizeof(planar));
    memset(chunky, 0, sizeof(chunky));
    ASSERT_TRUE(!DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
                    NULL, DM1_PORTRAIT_PLANAR_BYTES,
                    chunky, DM1_PORTRAIT_CHUNKY_BYTES),
                "planar-to-chunky rejects null planar");
    ASSERT_TRUE(!DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
                    planar, DM1_PORTRAIT_PLANAR_BYTES - 1u,
                    chunky, DM1_PORTRAIT_CHUNKY_BYTES),
                "planar-to-chunky rejects short planar");
    ASSERT_TRUE(!DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
                    chunky, DM1_PORTRAIT_CHUNKY_BYTES,
                    planar, DM1_PORTRAIT_PLANAR_BYTES - 1u),
                "chunky-to-planar rejects short planar output");
}

static void test_single_byte_msb_first_plane_order(void)
{
    uint8_t planar[DM1_PORTRAIT_PLANAR_BYTES];
    uint8_t chunky[DM1_PORTRAIT_CHUNKY_BYTES];
    const unsigned plane_size = (DM1_PORTRAIT_W / 8u) * DM1_PORTRAIT_H;

    memset(planar, 0, sizeof(planar));
    memset(chunky, 0xAA, sizeof(chunky));
    planar[0 * plane_size + 0] = 0x80u; /* pixel 0: bitplane 0 */
    planar[1 * plane_size + 0] = 0x40u; /* pixel 1: bitplane 1 */
    planar[2 * plane_size + 0] = 0x20u; /* pixel 2: bitplane 2 */
    planar[3 * plane_size + 0] = 0x10u; /* pixel 3: bitplane 3 */
    planar[0 * plane_size + 0] |= 0x08u;
    planar[1 * plane_size + 0] |= 0x08u;
    planar[2 * plane_size + 0] |= 0x08u;
    planar[3 * plane_size + 0] |= 0x08u; /* pixel 4: colour 15 */

    ASSERT_TRUE(DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
                    planar, DM1_PORTRAIT_PLANAR_BYTES,
                    chunky, DM1_PORTRAIT_CHUNKY_BYTES),
                "planar-to-chunky accepts native portrait span");
    ASSERT_EQ(chunky[0], 1, "pixel 0 reads bitplane 0 from MSB");
    ASSERT_EQ(chunky[1], 2, "pixel 1 reads bitplane 1 from next bit");
    ASSERT_EQ(chunky[2], 4, "pixel 2 reads bitplane 2");
    ASSERT_EQ(chunky[3], 8, "pixel 3 reads bitplane 3");
    ASSERT_EQ(chunky[4], 15, "pixel 4 combines all four bitplanes");
    ASSERT_EQ(chunky[8], 0, "next byte starts at pixel 8");
}

static void test_chunky_to_planar_masks_to_4bpp(void)
{
    uint8_t planar[DM1_PORTRAIT_PLANAR_BYTES];
    uint8_t chunky[DM1_PORTRAIT_CHUNKY_BYTES];
    const unsigned plane_size = (DM1_PORTRAIT_W / 8u) * DM1_PORTRAIT_H;

    memset(planar, 0xCC, sizeof(planar));
    memset(chunky, 0, sizeof(chunky));
    chunky[0] = 0x1Fu;
    chunky[1] = 0x02u;
    chunky[7] = 0x08u;
    ASSERT_TRUE(DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
                    chunky, DM1_PORTRAIT_CHUNKY_BYTES,
                    planar, DM1_PORTRAIT_PLANAR_BYTES),
                "chunky-to-planar accepts native portrait span");
    ASSERT_EQ(planar[0 * plane_size + 0], 0x80u, "plane 0 masks high chunky bits");
    ASSERT_EQ(planar[1 * plane_size + 0], 0xC0u, "plane 1 packs pixels 0 and 1");
    ASSERT_EQ(planar[2 * plane_size + 0], 0x80u, "plane 2 packs pixel 0");
    ASSERT_EQ(planar[3 * plane_size + 0], 0x81u, "plane 3 packs pixels 0 and 7");
    ASSERT_EQ(planar[0 * plane_size + 1], 0, "unused bytes are cleared");
}

static void test_roundtrip_through_portrait_struct(void)
{
    DM1_V1_PortraitPanelPortraitPc34 portrait;
    uint8_t source[DM1_PORTRAIT_CHUNKY_BYTES];
    uint8_t roundtrip[DM1_PORTRAIT_CHUNKY_BYTES];
    uint8_t planar[DM1_PORTRAIT_PLANAR_BYTES];

    memset(&portrait, 0, sizeof(portrait));
    for (int i = 0; i < DM1_PORTRAIT_CHUNKY_BYTES; ++i) {
        source[i] = (uint8_t)((i * 7 + i / 3) & 0x0F);
    }
    ASSERT_TRUE(DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
                    source, DM1_PORTRAIT_CHUNKY_BYTES,
                    planar, DM1_PORTRAIT_PLANAR_BYTES),
                "roundtrip chunky-to-planar");
    ASSERT_TRUE(DM1_V1_PortraitPanel_LoadPortraitPc34Compat(
                    &portrait, planar, DM1_PORTRAIT_PLANAR_BYTES),
                "load portrait copies native planar bytes");
    DM1_V1_PortraitPanel_ConvertPlanarToChunkyPc34Compat(&portrait);
    ASSERT_TRUE(memcmp(source, portrait.chunky_data, sizeof(source)) == 0,
                "portrait struct planar-to-chunky roundtrip");
    memset(roundtrip, 0, sizeof(roundtrip));
    ASSERT_TRUE(DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
                    portrait.planar_data, DM1_PORTRAIT_PLANAR_BYTES,
                    roundtrip, DM1_PORTRAIT_CHUNKY_BYTES),
                "direct planar-to-chunky roundtrip");
    ASSERT_TRUE(memcmp(source, roundtrip, sizeof(source)) == 0,
                "direct roundtrip matches source");
}

int main(void)
{
    test_guard_paths();
    test_single_byte_msb_first_plane_order();
    test_chunky_to_planar_masks_to_4bpp();
    test_roundtrip_through_portrait_struct();

    if (g_fail) {
        fprintf(stderr, "dm1_v1_portrait_planar_conversion_pc34_compat: %d failed, %d passed\n",
                g_fail, g_pass);
        return 1;
    }
    printf("dm1_v1_portrait_planar_conversion_pc34_compat: %d passed\n", g_pass);
    return 0;
}
