/*
 * Verify G0243 GraphicInfo values in the creature aspect table match
 * ReDMCSB DUNGEON.C reference (PC 3.4 / MEDIA385).
 */

#include <stdio.h>
#include "dm1_v1_creature_render_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } else { g_fail++; fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static const unsigned short ref_graphicInfo[27] = {
    0x623D, 0xA625, 0x6198, 0xB225, 0xA3B8, 0x539D, 0x0020,
    0x0220, 0x5225, 0x71B8, 0x11B8, 0x0225, 0x6038, 0xB23D,
    0x1638, 0x523D, 0xA038, 0xF23D, 0xA3BD, 0xE23D, 0x0225,
    0xA3BD, 0x53BD, 0x0038, 0x97BD, 0x0000, 0x0000
};

static void test_aspect_graphicInfo(void) {
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    int i;
    CHECK(aspects != NULL, "aspect table exists");
    if (!aspects) return;
    for (i = 0; i < 27; i++) {
        char msg[128];
        snprintf(msg, sizeof(msg),
            "C%02d: aspect.graphicInfo 0x%04X == ref 0x%04X",
            i, (unsigned)aspects[i].graphicInfo, ref_graphicInfo[i]);
        CHECK(aspects[i].graphicInfo == ref_graphicInfo[i], msg);
    }
}

static void test_graphicInfo_f0179_bits(void) {
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    int i;
    if (!aspects) return;
    for (i = 0; i < 27; i++) {
        unsigned gi = (unsigned)aspects[i].graphicInfo;
        int maxH = (gi >> 12) & 0x03;
        int maxV = (gi >> 14) & 0x03;
        char msg[128];
        snprintf(msg, sizeof(msg),
            "C%02d: maxH=%d maxV=%d both in [0,3]", i, maxH, maxV);
        CHECK(maxH >= 0 && maxH <= 3 && maxV >= 0 && maxV <= 3, msg);
    }
}

static void test_screamer_immobile_aspect(void) {
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    if (!aspects) return;
    unsigned gi = (unsigned)aspects[6].graphicInfo;
    CHECK(gi == 0x0020, "C06 Screamer: graphicInfo=0x0020 (attack only)");
    CHECK((gi & DM1_GI_MASK_ATTACK) != 0, "C06: ATTACK bit set");
    CHECK((gi & DM1_GI_MASK_SIDE) == 0, "C06: no SIDE bitmap");
    CHECK((gi & DM1_GI_MASK_BACK) == 0, "C06: no BACK bitmap");
}

static void test_lord_order_grey_lord_no_graphics(void) {
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    if (!aspects) return;
    CHECK(aspects[25].graphicInfo == 0x0000,
        "C25 Lord Order: graphicInfo=0 (no bitmaps)");
    CHECK(aspects[26].graphicInfo == 0x0000,
        "C26 Grey Lord: graphicInfo=0 (no bitmaps)");
}

static void test_derived_bitmap_indices(void) {
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    int i;
    int idx;
    if (!aspects) return;
    CHECK(aspects[0].firstDerivedBitmapIndex == 762,
        "C00: firstDerivedBitmapIndex == M539 (762)");
    idx = 762;
    for (i = 0; i < 27; i++) {
        char msg[128];
        unsigned gi = (unsigned)aspects[i].graphicInfo;
        int count = 2;
        if (gi & 0x0008) count += 2;
        if (gi & 0x0010) count += 2;
        if (gi & 0x0020) count += 2;
        snprintf(msg, sizeof(msg),
            "C%02d: firstDerivedBitmapIndex %d == computed %d",
            i, (int)aspects[i].firstDerivedBitmapIndex, idx);
        CHECK(aspects[i].firstDerivedBitmapIndex == idx, msg);
        idx += count;
    }
    CHECK(idx == 934, "total derived range 762..933 = 172 entries");
}

int main(void) {
    test_aspect_graphicInfo();
    test_graphicInfo_f0179_bits();
    test_screamer_immobile_aspect();
    test_lord_order_grey_lord_no_graphics();
    test_derived_bitmap_indices();
    printf("test_dm1_v1_g0243_creature_info: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
