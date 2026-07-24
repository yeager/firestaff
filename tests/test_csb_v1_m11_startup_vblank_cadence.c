/* CSB startup must consume one source VBlank per host idle tick. */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static uint32_t fnv1a(const unsigned char* bytes, int count) {
    uint32_t hash = 2166136261u;
    int index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static void expect_interval(uint32_t actual, uint32_t expected,
                            const char* label) {
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: expected %u, got %u\n", label,
                (unsigned int)expected, (unsigned int)actual);
        ++failures;
    }
}

int main(void) {
    M11_GameViewState view;
    unsigned char swoosh[9078];
    int index;

    memset(&view, 0, sizeof(view));
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 200u,
                    "ordinary gameplay keeps the 200 ms source tick");
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 200), 100u,
                    "ordinary gameplay still follows speed multiplier");

    view.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    view.csbState.startup_title_active = 1;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 20u,
                    "CSB title startup receives one VBlank per 20 ms");

    view.csbState.startup_title_active = 0;
    view.csbState.startup_entrance_active = 1;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 400), 20u,
                    "CSB entrance startup does not inherit QoL game speed");

    view.csbState.startup_entrance_active = 0;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 200u,
                    "CSB runtime returns to ordinary gameplay cadence");

    for (index = 0; index < (int)sizeof(swoosh); ++index) {
        swoosh[index] = (unsigned char)((index * 13 + 7) & 0xff);
    }
    if (!M11_GameView_SetCsbStartupSwooshSource(
            &view, swoosh, (int)sizeof(swoosh), fnv1a(swoosh,
                                                       (int)sizeof(swoosh))) ||
        !view.csbStartupSwooshBytesBound ||
        view.csbStartupSwooshHash != fnv1a(swoosh, (int)sizeof(swoosh))) {
        fprintf(stderr, "FAIL: authenticated raw CSB swoosh binds to M11\n");
        ++failures;
    }
    swoosh[0] ^= 1u;
    if (M11_GameView_SetCsbStartupSwooshSource(
            &view, swoosh, (int)sizeof(swoosh),
            view.csbStartupSwooshHash) || view.csbStartupSwooshBytesBound) {
        fprintf(stderr, "FAIL: changed CSB source must clear M11 binding\n");
        ++failures;
    }

    if (failures != 0) return 1;
    puts("csb startup VBlank cadence tests passed");
    return 0;
}
