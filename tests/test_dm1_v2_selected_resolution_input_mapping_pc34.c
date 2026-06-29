#include "main_loop_m11.h"

#include <stdio.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

typedef struct ResolutionCase {
    int width;
    int height;
} ResolutionCase;

static int presented_center_for_source_axis(int sourceCoord,
                                            int sourceExtent,
                                            int presentedExtent) {
    int lo;
    int hi;
    if (sourceCoord < 0) {
        sourceCoord = 0;
    }
    if (sourceCoord >= sourceExtent) {
        sourceCoord = sourceExtent - 1;
    }
    lo = (sourceCoord * presentedExtent + sourceExtent - 1) / sourceExtent;
    hi = (((sourceCoord + 1) * presentedExtent) - 1) / sourceExtent;
    return (lo + hi) / 2;
}

static void expect_source_point(int presentationMode,
                                int presentationWidth,
                                int presentationHeight,
                                int sourceX,
                                int sourceY) {
    int mappedX = presented_center_for_source_axis(sourceX, 320, presentationWidth);
    int mappedY = presented_center_for_source_axis(sourceY, 200, presentationHeight);
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode,
              presentationWidth,
              presentationHeight,
              &mappedX,
              &mappedY) == 1);
    CHECK(mappedX == sourceX);
    CHECK(mappedY == sourceY);
}

static void expect_selected_resolution(int presentationMode,
                                       int presentationWidth,
                                       int presentationHeight) {
    int x;
    int y;

    expect_source_point(presentationMode, presentationWidth, presentationHeight, 0, 0);
    expect_source_point(presentationMode, presentationWidth, presentationHeight, 159, 99);
    expect_source_point(presentationMode, presentationWidth, presentationHeight, 160, 100);
    expect_source_point(presentationMode, presentationWidth, presentationHeight, 319, 199);

    x = presentationWidth - 1;
    y = presentationHeight - 1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode,
              presentationWidth,
              presentationHeight,
              &x,
              &y) == 1);
    CHECK(x == 319);
    CHECK(y == 199);

    x = presentationWidth;
    y = presentationHeight;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode,
              presentationWidth,
              presentationHeight,
              &x,
              &y) == 1);
    CHECK(x == 319);
    CHECK(y == 199);
}

static void expect_mode_matrix(int presentationMode) {
    static const ResolutionCase rows[] = {
        {640, 400},
        {1920, 1080},
        {2560, 1440},
        {3200, 2000},
        {3840, 2160}
    };
    size_t i;

    for (i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i) {
        expect_selected_resolution(presentationMode, rows[i].width, rows[i].height);
    }
}

static void expect_non_selected_resolution_modes(void) {
    int x = 639;
    int y = 399;

    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED,
              3840,
              2160,
              &x,
              &y) == 1);
    CHECK(x == 319);
    CHECK(y == 199);

    x = 319;
    y = 199;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V1_ORIGINAL,
              320,
              200,
              &x,
              &y) == 0);
    CHECK(x == 319);
    CHECK(y == 199);
}

static void expect_boundary_clamps_and_failures(void) {
    int x;
    int y;

    x = -1;
    y = -1;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V20_FILTERED,
              640,
              400,
              &x,
              &y) == 1);
    CHECK(x == 0);
    CHECK(y == 0);

    x = -3840;
    y = -2160;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED,
              3840,
              2160,
              &x,
              &y) == 1);
    CHECK(x == 0);
    CHECK(y == 0);

    x = 3840 * 2;
    y = 2160 * 2;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN,
              3840,
              2160,
              &x,
              &y) == 1);
    CHECK(x == 319);
    CHECK(y == 199);

    x = 12;
    y = 34;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED,
              0,
              2160,
              &x,
              &y) == 0);
    CHECK(x == 12);
    CHECK(y == 34);

    x = 12;
    y = 34;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V22_MODERN,
              3840,
              0,
              &x,
              &y) == 0);
    CHECK(x == 12);
    CHECK(y == 34);

    x = 12;
    y = 34;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED,
              3840,
              2160,
              NULL,
              &y) == 0);
    CHECK(y == 34);
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              M12_PRESENTATION_V21_UPSCALED,
              3840,
              2160,
              &x,
              NULL) == 0);
    CHECK(x == 12);
}

int main(void) {
    expect_mode_matrix(M12_PRESENTATION_V21_UPSCALED);
    expect_mode_matrix(M12_PRESENTATION_V22_MODERN);
    expect_non_selected_resolution_modes();
    expect_boundary_clamps_and_failures();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_selected_resolution_input_mapping_pc34: ok");
    return 0;
}
