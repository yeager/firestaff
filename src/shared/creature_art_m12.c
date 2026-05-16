#include "creature_art_m12.h"
#include "creature_art_data_m12.h"

#include <string.h>

void M12_CreatureArt_Init(M12_CreatureArtState* state,
                          const char* dataDir,
                          unsigned int seed) {
    int i;
    (void)dataDir; /* Thumbnails are now embedded, no filesystem access needed */

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));

    for (i = 0; i < M12_CREATURE_COUNT; ++i) {
        state->creatures[i].name = g_creatureDataEntries[i].displayName;
        memcpy(state->creatures[i].pixels,
               g_creatureDataEntries[i].pixels,
               M12_CREATURE_THUMB_WIDTH * M12_CREATURE_THUMB_HEIGHT);
        state->creatures[i].loaded = 1;
    }

    state->anyLoaded = 1;
    state->selectedIndex = (int)(seed % (unsigned int)M12_CREATURE_COUNT);
}

int M12_CreatureArt_HasSelection(const M12_CreatureArtState* state) {
    return state && state->selectedIndex >= 0 && state->anyLoaded;
}

const char* M12_CreatureArt_GetSelectedName(const M12_CreatureArtState* state) {
    if (!state || state->selectedIndex < 0 || state->selectedIndex >= M12_CREATURE_COUNT) {
        return "";
    }
    return state->creatures[state->selectedIndex].name;
}

void M12_CreatureArt_Draw(const M12_CreatureArtState* state,
                          unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int dstX,
                          int dstY,
                          int dstW,
                          int dstH) {
    const M12_CreatureThumb* thumb;
    int y;
    int x;

    if (!state || !framebuffer || !M12_CreatureArt_HasSelection(state)) {
        return;
    }
    if (dstW <= 0 || dstH <= 0) {
        return;
    }

    thumb = &state->creatures[state->selectedIndex];

    for (y = 0; y < dstH; ++y) {
        int srcY = y * M12_CREATURE_THUMB_HEIGHT / dstH;
        int py = dstY + y;
        if (py < 0 || py >= framebufferHeight) {
            continue;
        }
        for (x = 0; x < dstW; ++x) {
            int srcX = x * M12_CREATURE_THUMB_WIDTH / dstW;
            int px = dstX + x;
            unsigned char pixel;
            if (px < 0 || px >= framebufferWidth) {
                continue;
            }
            pixel = thumb->pixels[srcY * M12_CREATURE_THUMB_WIDTH + srcX];
            if (pixel != 0) {
                framebuffer[py * framebufferWidth + px] = pixel;
            }
        }
    }
}
