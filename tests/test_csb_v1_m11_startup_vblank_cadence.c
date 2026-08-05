/* CSB preserves source cadence, with an explicit visible hold for CHAOS zoom. */

#include "m11_game_view.h"
#include "csb_v1_boot.h"

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
    view.csbState.startup_title_source_step = 1;
    view.csbState.startup_title_frame = 0;
    if (!M11_GameView_DropsIdleCatchupForStartup(&view)) {
        fprintf(stderr, "FAIL: CSB title must not batch invisible catch-up frames\n");
        ++failures;
    }
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 55u,
                    "CSB PRESENTS keeps its PC34 source cadence");

    view.csbState.startup_title_source_step = 2;
    view.csbState.startup_title_frame = 60;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 220u,
                    "CSB first CHAOS zoom raster remains visible for four PC34 slots");

    view.csbState.startup_title_source_step = 21;
    view.csbState.startup_title_frame = 79;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 220u,
                    "CSB final CHAOS zoom raster retains the visible four-slot hold");

    view.csbState.startup_title_frame = 80;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 55u,
                    "CSB source Delay(20) keeps its own cadence after the zoom");

    view.csbState.startup_title_source_step = 22;
    view.csbState.startup_title_frame = 100;
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 55u,
                    "CSB STRIKES BACK keeps TITLE.C's two-tick source hold");

    view.csbState.startup_title_active = 0;
    view.csbState.startup_entrance_active = 1;
    if (!M11_GameView_DropsIdleCatchupForStartup(&view)) {
        fprintf(stderr, "FAIL: CSB entrance must not batch invisible catch-up frames\n");
        ++failures;
    }
    expect_interval(M11_GameView_IdleTickIntervalMs(&view, 400), 55u,
                    "CSB entrance startup does not inherit QoL game speed");

    {
        CSB_V1_BootProfile profile;
        memset(&profile, 0, sizeof(profile));
        profile.tick_ms = 61u;
        view.csbBootProfile = &profile;
        view.csbState.startup_title_active = 1;
        view.csbState.startup_entrance_active = 0;
        view.csbState.startup_title_source_step = 2;
        view.csbState.startup_title_frame = 60;
        expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 244u,
                        "CSB CHAOS hold scales from the authenticated profile cadence");
        view.csbState.startup_title_active = 0;
        view.csbState.startup_entrance_active = 1;
        view.csbState.startup_title_source_step = 0;
        expect_interval(M11_GameView_IdleTickIntervalMs(&view, 100), 61u,
                        "CSB startup consumes the authenticated profile cadence");
        view.csbBootProfile = NULL;
    }

    view.csbState.startup_entrance_active = 0;
    if (M11_GameView_DropsIdleCatchupForStartup(&view)) {
        fprintf(stderr, "FAIL: CSB runtime must retain normal catch-up behavior\n");
        ++failures;
    }
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
