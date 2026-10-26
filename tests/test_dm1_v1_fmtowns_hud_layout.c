#include "dm1_v1_fmtowns_hud_layout.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #x); ++failures; \
} } while (0)

typedef struct ExpectedRecord {
    uint16_t zone;
    uint8_t type;
    uint16_t parent;
    int16_t data1;
    int16_t data2;
} ExpectedRecord;

static const ExpectedRecord expected_leaves[] = {
    {113,1,112,0,0}, {114,1,112,20,0},
    {115,1,112,20,15}, {116,1,112,0,15},
    {151,1,150,0,0}, {152,1,150,69,0},
    {153,1,150,138,0}, {154,1,150,207,0},
    {155,9,151,43,7}, {156,9,152,43,7},
    {157,9,153,43,7}, {158,9,154,43,7},
    {159,1,155,0,0}, {160,1,156,0,0},
    {161,1,157,0,0}, {162,1,158,0,0},
    {175,1,171,7,0}, {176,1,172,7,0},
    {177,1,173,7,0}, {178,1,174,7,0},
    {179,10,175,0,0}, {180,10,176,0,0},
    {181,10,177,0,0}, {182,10,178,0,0},
    {183,9,151,24,29}, {184,9,152,24,29},
    {185,9,153,24,29}, {186,9,154,24,29},
    {187,1,183,43,0}, {188,1,184,43,0},
    {189,1,185,43,0}, {190,1,186,43,0},
    {191,9,187,4,25}, {192,9,188,4,25},
    {193,9,189,4,25}, {194,9,190,4,25},
    {195,7,191,5,26}, {196,7,192,5,26},
    {197,7,193,5,26}, {198,7,194,5,26},
    {199,7,191,12,26}, {200,7,192,12,26},
    {201,7,193,12,26}, {202,7,194,12,26},
    {203,7,191,19,26}, {204,7,192,19,26},
    {205,7,193,19,26}, {206,7,194,19,26},
    {207,9,151,16,16}, {208,9,152,16,16},
    {209,9,153,16,16}, {210,9,154,16,16},
    {211,1,207,4,10}, {212,1,207,24,10},
    {213,1,208,4,10}, {214,1,208,24,10},
    {215,1,209,4,10}, {216,1,209,24,10},
    {217,1,210,4,10}, {218,1,210,24,10},
    {553,4,4,3,7}, {554,4,4,104,132}, {555,3,4,218,132}
};

static void check_chain(DM1_V1_FmtownsHudEdition edition,
                        unsigned int zone,
                        const ExpectedRecord *expected,
                        size_t count)
{
    DM1_V1_FmtownsHudLayoutChain actual;
    size_t i;
    CHECK(dm1_v1_fmtowns_hud_layout_chain(edition, zone, &actual));
    CHECK(actual.record_count == count);
    if (actual.record_count != count) return;
    for (i = 0; i < count; ++i) {
        CHECK(actual.records[i].zone_index == expected[i].zone);
        CHECK(actual.records[i].record_type == expected[i].type);
        CHECK(actual.records[i].parent_index == expected[i].parent);
        CHECK(actual.records[i].data1 == expected[i].data1);
        CHECK(actual.records[i].data2 == expected[i].data2);
    }
}

int main(void)
{
    static const ExpectedRecord icon113[] = {
        {113,1,112,0,0}, {112,9,111,19,14}, {111,2,110,319,0},
        {110,9,2,39,29}, {2,1,1,0,0}, {1,9,0,320,200}
    };
    static const ExpectedRecord status151[] = {
        {151,1,150,0,0}, {150,9,0,67,29}
    };
    static const ExpectedRecord status175[] = {
        {175,1,171,7,0}, {171,9,151,32,29},
        {151,1,150,0,0}, {150,9,0,67,29}
    };
    static const ExpectedRecord bar187[] = {
        {187,1,183,43,0}, {183,9,151,24,29},
        {151,1,150,0,0}, {150,9,0,67,29}
    };
    static const ExpectedRecord loadLabel554[] = {
        {554,4,4,104,132}, {4,1,3,0,0}, {3,9,0,224,136}
    };
    static const ExpectedRecord loadValue555[] = {
        {555,3,4,218,132}, {4,1,3,0,0}, {3,9,0,224,136}
    };
    DM1_V1_FmtownsHudLayoutChain chain;
    static const struct {
        unsigned int zone;
        int width;
        int height;
        int x;
        int y;
        int clipped_width;
        int clipped_height;
        int source_x;
        int source_y;
    } expected_rects[] = {
        {151,67,29,0,0,67,29,0,0},
        {152,67,29,69,0,67,29,0,0},
        {153,67,29,138,0,67,29,0,0},
        {154,67,29,207,0,67,29,0,0},
        {159,43,7,0,0,43,7,0,0},
        {160,43,7,69,0,43,7,0,0},
        {161,43,7,138,0,43,7,0,0},
        {162,43,7,207,0,43,7,0,0},
        {175,32,29,7,0,32,29,0,0},
        {176,32,29,76,0,32,29,0,0},
        {195,4,25,46,2,4,25,0,0},
        {199,4,25,53,2,4,25,0,0},
        {203,4,25,60,2,4,25,0,0},
        {211,16,16,4,10,16,16,0,0},
        {212,16,16,24,10,16,16,0,0},
        {213,16,16,73,10,16,16,0,0},
        {214,16,16,93,10,16,16,0,0}
    };

    /* Source oracle: ReDMCSB COORD.C G3038 110..116, G3034 150..218,
     * G3033 500..613 and G3016 1..17, MEDIA487 F20E/MEDIA569 F20J. */
    for (int edition = DM1_V1_FMTOWNS_HUD_EDITION_F20E;
         edition <= DM1_V1_FMTOWNS_HUD_EDITION_F20J; ++edition) {
        for (size_t i = 0; i < sizeof(expected_leaves) /
                                  sizeof(expected_leaves[0]); ++i) {
            CHECK(dm1_v1_fmtowns_hud_layout_chain(
                (DM1_V1_FmtownsHudEdition)edition,
                expected_leaves[i].zone, &chain));
            CHECK(chain.record_count > 0);
            if (chain.record_count > 0) {
                CHECK(chain.records[0].zone_index == expected_leaves[i].zone);
                CHECK(chain.records[0].record_type == expected_leaves[i].type);
                CHECK(chain.records[0].parent_index == expected_leaves[i].parent);
                CHECK(chain.records[0].data1 == expected_leaves[i].data1);
                CHECK(chain.records[0].data2 == expected_leaves[i].data2);
                CHECK(chain.records[chain.record_count - 1].parent_index == 0);
            }
        }
        check_chain((DM1_V1_FmtownsHudEdition)edition, 113, icon113,
                    sizeof(icon113) / sizeof(icon113[0]));
        check_chain((DM1_V1_FmtownsHudEdition)edition, 151, status151,
                    sizeof(status151) / sizeof(status151[0]));
        check_chain((DM1_V1_FmtownsHudEdition)edition, 175, status175,
                    sizeof(status175) / sizeof(status175[0]));
        check_chain((DM1_V1_FmtownsHudEdition)edition, 187, bar187,
                    sizeof(bar187) / sizeof(bar187[0]));
        check_chain((DM1_V1_FmtownsHudEdition)edition, 554, loadLabel554,
                    sizeof(loadLabel554) / sizeof(loadLabel554[0]));
        check_chain((DM1_V1_FmtownsHudEdition)edition, 555, loadValue555,
                    sizeof(loadValue555) / sizeof(loadValue555[0]));
        for (size_t i = 0; i < sizeof(expected_rects) /
                                  sizeof(expected_rects[0]); ++i) {
            DM1_V1_FmtownsHudRect rect;
            CHECK(dm1_v1_fmtowns_hud_layout_rect(
                (DM1_V1_FmtownsHudEdition)edition,
                expected_rects[i].zone,
                expected_rects[i].width,
                expected_rects[i].height, &rect));
            CHECK(rect.x == expected_rects[i].x);
            CHECK(rect.y == expected_rects[i].y);
            CHECK(rect.width == expected_rects[i].clipped_width);
            CHECK(rect.height == expected_rects[i].clipped_height);
            CHECK(rect.source_x == expected_rects[i].source_x);
            CHECK(rect.source_y == expected_rects[i].source_y);
        }
        {
            uint8_t icons[76 * 14];
            uint8_t portrait[32 * 29];
            uint8_t pixels[320 * 200];
            memset(icons, 0, sizeof(icons));
            memset(portrait, 0, sizeof(portrait));
            memset(pixels, 0xEE, sizeof(pixels));
            icons[2 * 76 + 38] = 3u;
            icons[6 * 76 + 41] = 12u;
            icons[13 * 76 + 56] = 9u;
            portrait[0] = 4u;
            portrait[31 + 28 * 32] = 12u;
            CHECK(dm1_v1_fmtowns_hud_blit_indexed(
                (DM1_V1_FmtownsHudEdition)edition, 115u,
                icons, 76, 14, 19, 14, 38, 0,
                pixels, 320, 200, 12));
            CHECK(pixels[17 * 320 + 301] == 3u);
            CHECK(pixels[21 * 320 + 304] == 0xEEu);
            CHECK(pixels[28 * 320 + 319] == 9u);
            CHECK(pixels[0] == 0xEEu);
            CHECK(!dm1_v1_fmtowns_hud_blit_indexed(
                (DM1_V1_FmtownsHudEdition)edition, 163u,
                icons, 76, 14, 0, 0, 0, 0,
                pixels, 320, 200, 12));
            CHECK(dm1_v1_fmtowns_hud_blit_indexed(
                (DM1_V1_FmtownsHudEdition)edition, 175u,
                portrait, 32, 29, 32, 29, 0, 0,
                pixels, 320, 200, -1));
            CHECK(pixels[0 * 320 + 7] == 4u);
            CHECK(pixels[28 * 320 + 38] == 12u);
        }
    }
    CHECK(!dm1_v1_fmtowns_hud_layout_chain(
        (DM1_V1_FmtownsHudEdition)0, 113, &chain));
    CHECK(!dm1_v1_fmtowns_hud_layout_chain(
        DM1_V1_FMTOWNS_HUD_EDITION_F20E, 500, &chain));
    CHECK(dm1_v1_fmtowns_hud_layout_chain(
        DM1_V1_FMTOWNS_HUD_EDITION_F20J, 191, &chain));
    CHECK(!dm1_v1_fmtowns_hud_layout_chain(
        DM1_V1_FMTOWNS_HUD_EDITION_F20E, 113, NULL));
    {
        DM1_V1_FmtownsHudRect rect;
        CHECK(!dm1_v1_fmtowns_hud_layout_rect(
            DM1_V1_FMTOWNS_HUD_EDITION_F20E, 163, 0, 0, &rect));
        CHECK(!dm1_v1_fmtowns_hud_layout_rect(
            DM1_V1_FMTOWNS_HUD_EDITION_F20E, 151, 67, 0, &rect));
        CHECK(!dm1_v1_fmtowns_hud_layout_rect(
            DM1_V1_FMTOWNS_HUD_EDITION_F20E, 113, 19, 14, NULL));
    }
    if (failures) return 1;
    puts("PASS: F20E/F20J source-native DM1 HUD layout records and parent chains");
    return 0;
}
