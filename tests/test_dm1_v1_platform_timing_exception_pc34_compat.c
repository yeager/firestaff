#include "dm1_v1_platform_timing_exception_pc34_compat.h"
#include "dm1_v1_vblank_timing.h"

#include <stdio.h>
#include <string.h>

typedef struct Capture {
    unsigned calls[5];
    uint32_t tick[5];
} Capture;

static int assertions;
static int failures;
#define CHECK(expr) do { ++assertions; if (!(expr)) { ++failures; \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); } } while (0)

static void callback(void *context, uint32_t tick)
{
    Capture *capture = (Capture *)context;
    unsigned index = capture->calls[0]++;
    if (index < 5u) capture->tick[index] = tick;
}

int main(void)
{
    DM1_V1_PlatformTimingStatePc34 state;
    DM1_V1_PlatformTimingHostPc34 host;
    DM1_V1_VBlankTimingState vblank;
    Capture capture;
    memset(&capture, 0, sizeof(capture));
    memset(&host, 0, sizeof(host));
    host.original_pc34_contract_verified = 1;
    host.source_vblank_ms = DM1_V1_PLATFORM_TIMING_PC34_VBLANK_MS;
    host.monotonic_cadence_available = 1;
    host.keyboard_midi_available = 1;
    host.palette_switch_available = 1;
    host.audio_cadence_available = 1;
    host.dma_completion_available = 1;
    host.floppy_control_available = 1;
    host.keyboard_midi = callback;
    host.palette_switch = callback;
    host.audio_player = callback;
    host.dma_completion = callback;
    host.floppy_power_off = callback;
    host.context = &capture;

    dm1_v1_platform_timing_exception_init_pc34(&state);
    dm1_v1_platform_timing_exception_configure_pc34(&state, &host);
    CHECK(dm1_v1_platform_timing_exception_is_ready_pc34(&state));
    CHECK(dm1_v1_e0017_vertical_blank_pc34(&state));
    CHECK(state.source_tick == 1u && state.vblank_count == 1u &&
          state.timer_c_noop_count == 1u);
    CHECK(dm1_v1_e0014_keyboard_midi_pc34(&state));
    CHECK(dm1_v1_e0015_palette_switch_pc34(&state));
    CHECK(dm1_v1_e0061_sound_timer_a_pc34(&state));
    CHECK(dm1_v1_s0080_dma_completion_pc34(&state));
    CHECK(dm1_v1_s0081_floppy_power_off_pc34(&state));
    CHECK(capture.calls[0] == 5u && capture.tick[0] == 1u &&
          capture.tick[4] == 1u);
    CHECK(state.keyboard_midi_count == 1u && state.palette_switch_count == 1u &&
          state.audio_tick_count == 1u && state.dma_completion_count == 1u &&
          state.floppy_power_off_count == 1u);

    host.audio_player = NULL;
    dm1_v1_platform_timing_exception_configure_pc34(&state, &host);
    CHECK(!dm1_v1_e0061_sound_timer_a_pc34(&state));
    CHECK(state.fail_closed && !state.last_delivery_permitted);
    host.audio_player = callback;
    host.monotonic_cadence_available = 0;
    dm1_v1_platform_timing_exception_configure_pc34(&state, &host);
    CHECK(!dm1_v1_e0017_vertical_blank_pc34(&state));
    CHECK(state.fail_closed && state.source_tick == 1u);
    host.monotonic_cadence_available = 1;
    host.source_vblank_ms = 16u;
    dm1_v1_platform_timing_exception_configure_pc34(&state, &host);
    CHECK(!dm1_v1_platform_timing_exception_is_ready_pc34(&state));
    CHECK(!dm1_v1_s0080_dma_completion_pc34(&state));
    CHECK(strstr(dm1_v1_platform_timing_exception_source_evidence_pc34(),
                 "E0017") != NULL);

    /* The live DM1 timing owner cannot mint a game tick when E0017's host
     * cadence is unavailable.  Restoring a verified binding resumes the
     * original 20 ms source tick. */
    DM1_V1_VBlankTiming_Init(&vblank);
    host.source_vblank_ms = DM1_V1_PLATFORM_TIMING_PC34_VBLANK_MS;
    host.monotonic_cadence_available = 0;
    DM1_V1_VBlankTiming_ConfigurePlatformHost(&vblank, &host);
    CHECK(!DM1_V1_VBlankTiming_Update(&vblank, 20u));
    CHECK(vblank.vblankCount == 0 && vblank.platformTiming.fail_closed);
    DM1_V1_VBlankTiming_ConfigurePlatformHost(&vblank, NULL);
    CHECK(!DM1_V1_VBlankTiming_Update(&vblank, 20u));
    CHECK(vblank.vblankCount == 1 && vblank.platformTiming.vblank_count == 1u);
    printf("test_dm1_v1_platform_timing_exception_pc34_compat: %d assertions, %d failures\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
