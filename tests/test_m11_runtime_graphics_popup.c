#include "m11_game_view.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    M11_GameInputResult result;
    size_t i;
    int nonzero = 0;

    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;

    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);
    assert(state.graphicsPopupSelectedRow == 0);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.presentationMode == M12_PRESENTATION_V20_FILTERED);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupSelectedRow == 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_DrawGraphicsPopup(&state, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0) {
            nonzero = 1;
            break;
        }
    }
    assert(nonzero == 1);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);
    puts("m11 runtime graphics popup: ok");
    return 0;
}
