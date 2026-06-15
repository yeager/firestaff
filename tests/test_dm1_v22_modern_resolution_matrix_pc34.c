#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef struct ResolutionCase {
    int resolution;
    int width;
    int height;
} ResolutionCase;

static int failures = 0;

static void check(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void run_case(const ResolutionCase* row) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    int resolvedWidth = 0;
    int resolvedHeight = 0;
    int started;

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.sourceId = "dm1-v22-matrix";
    spec.dataDir = "/tmp/firestaff-no-assets-required";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    spec.dungeonPath = "/tmp/firestaff-dm1-v22-matrix-missing.DAT";
    spec.presentationMode = M12_PRESENTATION_V22_MODERN;
    spec.presentationWidth = row->width;
    spec.presentationHeight = row->height;

    check(M12_PresentationMode_AllowsResolutionChoice(spec.presentationMode),
          "V2.2 modern should allow selected presentation resolution");
    check(M12_Resolution_Dimensions(row->resolution,
                                    &resolvedWidth,
                                    &resolvedHeight),
          "resolution dimensions should resolve");
    check(resolvedWidth == row->width, "resolved width should match matrix row");
    check(resolvedHeight == row->height, "resolved height should match matrix row");

    M11_GameView_Init(&view);
    started = M11_GameView_Start(&view, &spec);
    check(started == 0, "DM1 launch should stay blocked without game data");
    check(view.active == 0, "M11 view should remain inactive after missing-data gate");
    check(view.presentationMode == M12_PRESENTATION_V22_MODERN,
          "M11 should retain V2.2 modern presentation mode");
    check(view.presentationWidth == row->width,
          "M11 should retain selected presentation width");
    check(view.presentationHeight == row->height,
          "M11 should retain selected presentation height");

    M11_GameView_Shutdown(&view);
}

int main(void) {
    static const ResolutionCase rows[] = {
        {M12_RES_640x400, 640, 400},
        {M12_RES_1920x1080, 1920, 1080},
        {M12_RES_3840x2160, 3840, 2160}
    };
    size_t i;

    for (i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i) {
        run_case(&rows[i]);
    }

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v22_modern_resolution_matrix_pc34: ok");
    return 0;
}
