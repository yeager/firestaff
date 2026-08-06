/* Opt-in HME-242 TITLE presentation regression.  It reads the user-selected
 * FM Towns CD and English companion through Firestaff's RAM-only launch path.
 * No archive member is ever materialised on disk. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int failures;

static void expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static unsigned int nonzero_pixels(const unsigned char* pixels, size_t size)
{
    unsigned int count = 0u;
    size_t index;
    for (index = 0u; pixels && index < size; ++index) {
        if (pixels[index] != 0u) ++count;
    }
    return count;
}

int main(void)
{
    const char* root = getenv("FIRESTAFF_DM2_FMTOWNS_ROOT");
    const char* companion = getenv("FIRESTAFF_DM2_ENGLISH_COMPANION");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    unsigned char framebuffer[M11_FB_BYTES];
    int step;

    if (!root || !root[0] || !companion || !companion[0]) {
        puts("SKIP: FIRESTAFF_DM2_FMTOWNS_ROOT and English companion are required");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    spec.dataDir = root;
    spec.dm2EnglishCompanionPath = companion;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;
    M11_GameView_Init(&view);
    expect(M11_GameView_Start(&view, &spec) == 1,
           "selected HME-242 archive enters the DM2 M11 path");
    expect(view.dm2FmtownsTitleBound && view.dm2FmtownsSwooshActive &&
               view.dm2FmtownsTitlePalette.valid &&
               view.dm2FmtownsTitleFrameReceipt.valid &&
               view.dm2FmtownsTitleFrameReceipt.requested_frame == 0u,
           "M11 starts with AUTOEXEC's real SWOOSH frame zero and PL palette");
    expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
               M11_GAME_INPUT_IGNORED,
           "SWOOSH prevents a click from reaching SKULL menu input early");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) == 13u,
           "M11 presents HME-242 SWOOSH's sparse source first EN canvas");
    for (step = 0; step < 6; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) == 59u,
           "M11 presents real HME-242 SWOOSH delta pixels before TITLE");
    for (step = 0; step < 254; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleBound && !view.dm2FmtownsSwooshActive &&
               view.dm2FmtownsTitleFrameReceipt.requested_frame > 0u,
           "M11 advances real SWOOSH before binding TITLE through Timer-A units");
    view.dm2FmtownsTitleBound = 0;
    view.dm2FmtownsTitleFinished = 1;
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) == 0u &&
               M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                   M11_GAME_INPUT_IGNORED,
           "unimplemented FM Towns SKULL.EXP stays black and cannot use PC menu input");
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    puts("PASS: DM2 FM Towns TITLE M11 presentation uses authenticated real media");
    return 0;
}
