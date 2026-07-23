#ifndef FIRESTAFF_DM1_V1_PLATFORM_TIMING_EXCEPTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PLATFORM_TIMING_EXCEPTION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host equivalent of the Atari ST exception bundle used by ReDMCSB.
 *
 * This is deliberately a scheduler contract, not an emulator for the ST
 * vector table.  Every delivery is admitted only when the PC34/PAL source
 * cadence is known.  A host without the required capability is rejected; it
 * may not invent a timer, palette change, audio service, DMA completion, or
 * floppy state.
 *
 * Source anchors:
 *   E0013 BASE.C:726  Timer C (explicit no-op)
 *   E0014 BASE.C:728  Timer D keyboard/MIDI service
 *   E0015 BASE.C:738  Timer B palette switcher
 *   E0017 BASE.C:787  vertical blank CPSDF
 *   E0061 SOUND.C / ATARIST.H:5 Timer A sound player
 *   S0080/S0081 ATARIST.H:250-251 DMA completion / floppy power.
 */

#define DM1_V1_PLATFORM_TIMING_PC34_VBLANK_MS 20u

typedef void (*DM1_V1_PlatformTimingCallbackPc34)(void *context,
                                                    uint32_t source_tick);

typedef struct DM1_V1_PlatformTimingHostPc34 {
    int original_pc34_contract_verified;
    uint32_t source_vblank_ms;
    int monotonic_cadence_available;
    int keyboard_midi_available;
    int palette_switch_available;
    int audio_cadence_available;
    int dma_completion_available;
    int floppy_control_available;
    DM1_V1_PlatformTimingCallbackPc34 keyboard_midi;
    DM1_V1_PlatformTimingCallbackPc34 palette_switch;
    DM1_V1_PlatformTimingCallbackPc34 audio_player;
    DM1_V1_PlatformTimingCallbackPc34 dma_completion;
    DM1_V1_PlatformTimingCallbackPc34 floppy_power_off;
    void *context;
} DM1_V1_PlatformTimingHostPc34;

typedef struct DM1_V1_PlatformTimingStatePc34 {
    DM1_V1_PlatformTimingHostPc34 host;
    uint32_t source_tick;
    uint32_t timer_c_noop_count;
    uint32_t keyboard_midi_count;
    uint32_t palette_switch_count;
    uint32_t vblank_count;
    uint32_t audio_tick_count;
    uint32_t dma_completion_count;
    uint32_t floppy_power_off_count;
    int fail_closed;
    int last_delivery_permitted;
} DM1_V1_PlatformTimingStatePc34;

void dm1_v1_platform_timing_exception_init_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
void dm1_v1_platform_timing_exception_configure_pc34(
    DM1_V1_PlatformTimingStatePc34 *state,
    const DM1_V1_PlatformTimingHostPc34 *host);

/* E0017 delivery.  A successful source VBlank always runs E0013's explicit
 * no-op and increments the source tick. */
int dm1_v1_e0017_vertical_blank_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
/* E0014/E0015/E0061 are demand-driven original services. */
int dm1_v1_e0014_keyboard_midi_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
int dm1_v1_e0015_palette_switch_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
int dm1_v1_e0061_sound_timer_a_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
/* S0080/S0081 are never simulated when the host cannot perform them. */
int dm1_v1_s0080_dma_completion_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);
int dm1_v1_s0081_floppy_power_off_pc34(
    DM1_V1_PlatformTimingStatePc34 *state);

int dm1_v1_platform_timing_exception_is_ready_pc34(
    const DM1_V1_PlatformTimingStatePc34 *state);
const char *dm1_v1_platform_timing_exception_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
