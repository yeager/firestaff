/*
 * Opt-in DM1 PC34 PANEL.C F0339 runtime probe.
 *
 * It never manufactures a Thing, party state, save, or graphic surface. A
 * configured original PC34 save must already hold a normal object in the
 * leader hand; only then can the normal F0352 -> F0342 -> F0339 eye route be
 * observed against its original DUNGEON.DAT and GRAPHICS.DAT session.
 */
#include "asset_loader_m11.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kEyeX = 20, kEyeY = 54, kFramebufferWidth = 320,
       kFramebufferHeight = 200, kTransparentIndex = 8 };

static int readable(const char *path)
{
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

static int skip(const char *reason)
{
    printf("SKIP: %s\n", reason);
    return 0;
}

int main(int argc, char **argv)
{
    const char *save_path = getenv("FIRESTAFF_DM1_ORIGINAL_SAVE");
    const char *data_dir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_DATA_DIR");
    M12_StartupMenuState menu;
    M11_GameViewState state;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    const M11_AssetSlot *eye;
    unsigned short thing;
    int type;
    int viewport_x, viewport_y, viewport_w, viewport_h;
    int indicator_x, indicator_y, indicator_w, indicator_h;
    int x, y;
    int opaque_count = 0;

    if (!readable(save_path)) return skip("set FIRESTAFF_DM1_ORIGINAL_SAVE to an external original PC34 save");
    if (!data_dir || !data_dir[0]) return skip("pass a DM1 data directory or set FIRESTAFF_DM1_DATA_DIR");
    M12_StartupMenu_InitWithDataDir(&menu, data_dir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu) ||
        !M11_GameView_LoadDm1SavePath(&state, save_path, NULL)) {
        fprintf(stderr, "FAIL: configured original PC34 session was not admitted\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (state.dm1ViewportRuntimeOrigin != DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34) {
        fprintf(stderr, "FAIL: save did not retain original-PC34 provenance\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    thing = DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state);
    if (thing == THING_NONE || thing == THING_ENDOFLIST || !state.world.things ||
        !F7018_GetThingData(state.world.things, thing)) {
        M11_GameView_Shutdown(&state);
        return skip("original save has no source-owned leader-hand Thing");
    }
    type = THING_GET_TYPE(thing);
    if (type == THING_TYPE_SCROLL || type == THING_TYPE_CONTAINER) {
        M11_GameView_Shutdown(&state);
        return skip("original save leader hand does not take F0342 object route");
    }
    if (M11_GameView_HandlePointer(&state, kEyeX, kEyeY, 1) != M11_GAME_INPUT_REDRAW ||
        !state.v1ObjectDescriptionPanelActive || !state.v1ObjectDescriptionSourceMaterialValid) {
        fprintf(stderr, "FAIL: original Thing did not reach the admitted F0342 route\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!M11_GameView_GetViewportRect(&viewport_x, &viewport_y, &viewport_w, &viewport_h) ||
        !M11_GameView_GetV1ArrowOrEyeZone(&indicator_x, &indicator_y, &indicator_w, &indicator_h)) {
        fprintf(stderr, "FAIL: source F0339 geometry is unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    (void)viewport_w; (void)viewport_h;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, kFramebufferWidth, kFramebufferHeight);
    eye = M11_AssetLoader_Load(&state.assetLoader, (unsigned int)dm1_v1_graphic_arrow_or_eye_pc34(1));
    if (!eye || !eye->loaded || !eye->pixels || eye->width != (unsigned short)indicator_w ||
        eye->height != (unsigned short)indicator_h) {
        fprintf(stderr, "FAIL: original C019 material is unavailable or malformed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (y = 0; y < indicator_h; ++y) for (x = 0; x < indicator_w; ++x) {
        unsigned char pixel = eye->pixels[y * eye->width + x];
        if (pixel == kTransparentIndex) continue;
        ++opaque_count;
        if (framebuffer[(viewport_y + indicator_y + y) * kFramebufferWidth + viewport_x + indicator_x + x] != pixel) {
            fprintf(stderr, "FAIL: C019 pixel drift at %d,%d\n", x, y);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    if (opaque_count == 0) {
        fprintf(stderr, "FAIL: original C019 has no visible source pixels\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("PASS: original PC34 Thing 0x%04X consumed C019 through F0339 (%d pixels)\n", (unsigned int)thing, opaque_count);
    M11_GameView_Shutdown(&state);
    return 0;
}
