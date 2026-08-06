/* Opt-in HME-242 TITLE presentation regression.  It reads the user-selected
 * FM Towns CD and English companion through Firestaff's RAM-only launch path.
 * No archive member is ever materialised on disk. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "dm2_v1_boot.h"

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

static unsigned int fnv1a32(const unsigned char* bytes, size_t size)
{
    unsigned int hash = 2166136261u;
    size_t index;
    for (index = 0u; bytes && index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
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
               view.dm2FmtownsTitleFrameReceipt.requested_frame == 0u &&
               view.dm2FmtownsFrameCount == 19u,
           "M11 starts with AUTOEXEC's real SWOOSH frame zero, PL palette and EN/DL count");
    expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
               M11_GAME_INPUT_IGNORED,
           "SWOOSH prevents a click from reaching SKULL menu input early");
    expect(M11_GameView_HandlePointerButton(
               &view, 160, 100, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
               M11_GAME_INPUT_IGNORED,
           "SWOOSH prevents pointer input from reaching SKULL menu input early");
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
    for (step = 0; step < 10000 && view.dm2FmtownsSwooshActive; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleBound && !view.dm2FmtownsSwooshActive &&
               view.dm2FmtownsTitleFrameReceipt.requested_frame == 0u &&
               view.dm2FmtownsFrameCount == 225u,
           "M11 advances real SWOOSH before binding TITLE's source EN/DL count through Timer-A units");
    for (step = 0; step < 10000 &&
                    view.dm2FmtownsTitleFrameIndex < 13u; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleFrameIndex == 13u &&
               view.audioState.dm2FmtownsTitleSoundPlayCount == 0,
           "TITLE keeps SND2 silent before SO's 14 preceding source frames");
    for (step = 0; step < 10000 &&
                    view.dm2FmtownsTitleFrameIndex < 14u; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleFrameIndex == 14u &&
               view.audioState.dm2FmtownsTitleSoundPlayCount == 1,
           "TITLE starts its first authenticated SND2 playback at SO's frame boundary");
    for (step = 0; step < 10000 && !view.dm2FmtownsTitleFinished; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleFinished && !view.dm2FmtownsTitleBound &&
               view.dm2FmtownsFrameCount == 0u &&
               view.audioState.dm2FmtownsTitleSoundAccepted &&
               view.audioState.dm2FmtownsTitleSoundByteCount == 12862 &&
               view.audioState.dm2FmtownsTitleSoundHash == 0x0b829ae7u &&
               view.audioState.dm2FmtownsTitleSoundPlayCount == 5,
           "M11 reaches the real TITLE stream end before handing off to SKULL");
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) != 0u,
           "TITLE hands off to the original FM Towns GDAT menu");
    expect(fnv1a32(framebuffer, sizeof(framebuffer)) == 0x63310e49u,
           "TITLE handoff decodes the selected HME-242 IMG2 menu bytes exactly");
    {
        DM2_V1_StartupMenuPointerLayout layout;
        DM2_V1_StartupMenuAuxPointerLayout aux_layout;
        M11_GameInputResult pointer_result;
        int x;
        int y;
        memset(&layout, 0, sizeof(layout));
        memset(&aux_layout, 0, sizeof(aux_layout));
        expect(dm2_v1_boot_startup_menu_pointer_layout(
                   (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) &&
                   layout.valid && layout.new_game.w > 0 &&
                   layout.new_game.h > 0 && layout.resume_game.w > 0 &&
                   layout.resume_game.h > 0,
               "HME-242 GDAT provides the source NEW GAME and RESUME hit rectangles");
        x = layout.new_game.x + layout.new_game.w / 2;
        y = layout.new_game.y + layout.new_game.h / 2;
        pointer_result = M11_GameView_HandlePointerButton(
            &view, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        expect(pointer_result == M11_GAME_INPUT_REDRAW &&
                   view.dm2State.startup_menu_active &&
                   view.world.party.championCount == 0 &&
                   view.world.party.activeChampionIndex == -1,
               "HME-242 NEW GAME rectangle dispatches 0xD7 but cannot create a fake party");
        x = layout.resume_game.x + layout.resume_game.w / 2;
        y = layout.resume_game.y + layout.resume_game.h / 2;
        pointer_result = M11_GameView_HandlePointerButton(
            &view, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        expect(pointer_result == M11_GAME_INPUT_IGNORED &&
                   view.dm2State.startup_menu_active &&
                   view.world.party.championCount == 0 &&
                   view.world.party.activeChampionIndex == -1,
               "HME-242 RESUME rectangle dispatches 0xD9 only when an admitted real save exists");
        expect(dm2_v1_boot_startup_menu_aux_pointer_layout(
                   (DM2_V1_BootProfile *)view.dm2BootProfile, &aux_layout) &&
                   aux_layout.valid && aux_layout.show_credits.w > 0 &&
                   aux_layout.show_credits.h > 0 &&
                   aux_layout.dismiss_credits.w > 0 &&
                   aux_layout.dismiss_credits.h > 0 &&
                   aux_layout.quit_game.w > 0 && aux_layout.quit_game.h > 0,
               "HME-242 GDAT provides source credits, dismissal and quit rectangles");
        x = aux_layout.show_credits.x + aux_layout.show_credits.w / 2;
        y = aux_layout.show_credits.y + aux_layout.show_credits.h / 2;
        pointer_result = M11_GameView_HandlePointerButton(
            &view, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        expect(pointer_result == M11_GAME_INPUT_REDRAW &&
                   view.dm2State.startup_credits_active,
               "HME-242 credits rectangle enters the source-owned credits state");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
        expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) != 0u,
               "FM Towns credits draw real TITLE GDAT pixels rather than a host status panel");
        x = aux_layout.dismiss_credits.x + aux_layout.dismiss_credits.w / 2;
        y = aux_layout.dismiss_credits.y + aux_layout.dismiss_credits.h / 2;
        pointer_result = M11_GameView_HandlePointerButton(
            &view, x, y, DM1_V1_MOUSE_MASK_RIGHT_PC34);
        expect(pointer_result == M11_GAME_INPUT_REDRAW &&
                   !view.dm2State.startup_credits_active,
               "HME-242 credits accepts the source dismissal event from either mouse button");
        x = aux_layout.quit_game.x + aux_layout.quit_game.w / 2;
        y = aux_layout.quit_game.y + aux_layout.quit_game.h / 2;
        pointer_result = M11_GameView_HandlePointerButton(
            &view, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        expect(pointer_result == M11_GAME_INPUT_RETURN_TO_MENU,
               "HME-242 quit rectangle dispatches the source-owned launcher return");
    }
    expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
               M11_GAME_INPUT_IGNORED,
           "untranslated keyboard tokens cannot invent FM Towns menu input");
    M11_GameView_Shutdown(&view);
    expect(view.dm2FmtownsTitleBytes == NULL &&
               view.dm2FmtownsTitleByteCount == 0u &&
               view.dm2FmtownsFrameCount == 0u &&
               !view.dm2FmtownsTitleBound &&
               !view.dm2FmtownsSwooshActive,
           "shutdown releases the RAM-only FM Towns animation member");
    if (failures) return 1;
    puts("PASS: DM2 FM Towns TITLE M11 presentation uses authenticated real media");
    return 0;
}
