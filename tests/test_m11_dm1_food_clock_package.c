/* Original-media package admission, not an emulator timing capture. */
#include "m11_game_view.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    M11_GameViewState *state;
    int expected, bound, ok;
    size_t archiveLength;
    if (argc != 3) return 2;
    expected = strcmp(argv[2], "bound") == 0;
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    if (!M11_GameView_StartDm1(state, argv[1]) || !state->active) {
        fprintf(stderr, "FAIL: original-media startup rejected %s\n", argv[1]);
        M11_GameView_Shutdown(state);
        free(state);
        return 1;
    }
    archiveLength = strlen(argv[1]);
    /* A fallback sibling installation must not make a negative case pass. */
    ok = strncmp(state->assetLoader.graphicsDatPath, argv[1], archiveLength) == 0 &&
         strcmp(state->assetLoader.graphicsDatPath + archiveLength,
                "::DATA/GRAPHICS.DAT") == 0;
    bound = state->v1FoodVblankHzNumerator == 25175000u &&
            state->v1FoodVblankHzDenominator == 359200u;
    ok = ok && (bound == expected);
    if (!expected)
        ok = ok && state->v1FoodVblankHzNumerator == 0 &&
                   state->v1FoodVblankHzDenominator == 0;
    if (!ok)
        fprintf(stderr, "FAIL: selected=%s clock=%u/%u expected=%s\n",
                state->assetLoader.graphicsDatPath,
                state->v1FoodVblankHzNumerator, state->v1FoodVblankHzDenominator,
                argv[2]);
    M11_GameView_Shutdown(state);
    if (state->v1FoodCommandPending || state->v1FoodVblankHzNumerator ||
        state->v1FoodVblankHzDenominator) ok = 0;
    free(state);
    return ok ? 0 : 1;
}
