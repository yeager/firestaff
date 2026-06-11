#include "main_loop_m11.h"

enum {
    M11_SOURCE_FB_WIDTH = 320,
    M11_SOURCE_FB_HEIGHT = 200
};

int M11_MapPresentedGamePointToSourceForPresentation(int presentationMode,
                                                     int presentationWidth,
                                                     int presentationHeight,
                                                     int* x,
                                                     int* y) {
    int targetW = M11_SOURCE_FB_WIDTH;
    int targetH = M11_SOURCE_FB_HEIGHT;
    if (!x || !y) {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_SOURCE_FB_WIDTH * 2;
        targetH = M11_SOURCE_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V21_UPSCALED ||
                presentationMode == M12_PRESENTATION_V22_MODERN) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    } else {
        return 0;
    }
    if (targetW > 0 && targetH > 0) {
        *x = (*x * M11_SOURCE_FB_WIDTH) / targetW;
        *y = (*y * M11_SOURCE_FB_HEIGHT) / targetH;
    }
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= M11_SOURCE_FB_WIDTH) *x = M11_SOURCE_FB_WIDTH - 1;
    if (*y >= M11_SOURCE_FB_HEIGHT) *y = M11_SOURCE_FB_HEIGHT - 1;
    return 1;
}
