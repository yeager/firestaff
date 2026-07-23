#include "dm1_v1_platform_timing_exception_pc34_compat.h"

#include <string.h>

static int dm1_v1_platform_timing_contract_valid_pc34(
    const DM1_V1_PlatformTimingStatePc34 *state)
{
    return state && state->host.original_pc34_contract_verified &&
        state->host.source_vblank_ms == DM1_V1_PLATFORM_TIMING_PC34_VBLANK_MS;
}

static int dm1_v1_platform_timing_fail_closed_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    if (state) {
        state->fail_closed = 1;
        state->last_delivery_permitted = 0;
    }
    return 0;
}

void dm1_v1_platform_timing_exception_init_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    DM1_V1_PlatformTimingHostPc34 default_host;
    if (!state) return;
    memset(state, 0, sizeof(*state));
    memset(&default_host, 0, sizeof(default_host));
    /* A monotonic host clock is the portable equivalent of receiving the
     * source 50 Hz VBlank.  Optional platform services stay unavailable
     * until their real host binding is explicitly supplied. */
    default_host.original_pc34_contract_verified = 1;
    default_host.source_vblank_ms = DM1_V1_PLATFORM_TIMING_PC34_VBLANK_MS;
    default_host.monotonic_cadence_available = 1;
    dm1_v1_platform_timing_exception_configure_pc34(state, &default_host);
}

void dm1_v1_platform_timing_exception_configure_pc34(
    DM1_V1_PlatformTimingStatePc34 *state,
    const DM1_V1_PlatformTimingHostPc34 *host)
{
    uint32_t tick;
    if (!state) return;
    tick = state->source_tick;
    memset(state, 0, sizeof(*state));
    state->source_tick = tick;
    if (host) state->host = *host;
    if (!dm1_v1_platform_timing_contract_valid_pc34(state)) {
        state->fail_closed = 1;
    }
}

int dm1_v1_e0017_vertical_blank_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    if (!dm1_v1_platform_timing_contract_valid_pc34(state) ||
        !state->host.monotonic_cadence_available) {
        return dm1_v1_platform_timing_fail_closed_pc34(state);
    }
    state->source_tick++;
    state->vblank_count++;
    /* E0013 Timer C is explicitly a source no-op.  It is recorded so a
     * host never substitutes arbitrary timer work for it. */
    state->timer_c_noop_count++;
    state->last_delivery_permitted = 1;
    return 1;
}

static int dm1_v1_platform_timing_dispatch_pc34(
    DM1_V1_PlatformTimingStatePc34 *state, int capability,
    DM1_V1_PlatformTimingCallbackPc34 callback, uint32_t *counter)
{
    if (!dm1_v1_platform_timing_contract_valid_pc34(state) || !capability ||
        !callback || state->source_tick == 0u) {
        return dm1_v1_platform_timing_fail_closed_pc34(state);
    }
    callback(state->host.context, state->source_tick);
    (*counter)++;
    state->last_delivery_permitted = 1;
    return 1;
}

int dm1_v1_e0014_keyboard_midi_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_dispatch_pc34(state,
        state ? state->host.keyboard_midi_available : 0,
        state ? state->host.keyboard_midi : NULL,
        state ? &state->keyboard_midi_count : NULL);
}

int dm1_v1_e0015_palette_switch_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_dispatch_pc34(state,
        state ? state->host.palette_switch_available : 0,
        state ? state->host.palette_switch : NULL,
        state ? &state->palette_switch_count : NULL);
}

int dm1_v1_e0061_sound_timer_a_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_dispatch_pc34(state,
        state ? state->host.audio_cadence_available : 0,
        state ? state->host.audio_player : NULL,
        state ? &state->audio_tick_count : NULL);
}

int dm1_v1_s0080_dma_completion_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_dispatch_pc34(state,
        state ? state->host.dma_completion_available : 0,
        state ? state->host.dma_completion : NULL,
        state ? &state->dma_completion_count : NULL);
}

int dm1_v1_s0081_floppy_power_off_pc34(
    DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_dispatch_pc34(state,
        state ? state->host.floppy_control_available : 0,
        state ? state->host.floppy_power_off : NULL,
        state ? &state->floppy_power_off_count : NULL);
}

int dm1_v1_platform_timing_exception_is_ready_pc34(
    const DM1_V1_PlatformTimingStatePc34 *state)
{
    return dm1_v1_platform_timing_contract_valid_pc34(state) &&
        state->host.monotonic_cadence_available && !state->fail_closed;
}

const char *dm1_v1_platform_timing_exception_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:E0013 TimerC no-op, E0014 keyboard/MIDI, "
           "E0015 TimerB palette, E0017 VBlank; SOUND.C:E0061 TimerA; "
           "ATARIST.H:S0080 DMA completion/S0081 floppy power. "
           "Firestaff accepts only authenticated 20 ms PC34/PAL cadence "
           "and fails closed for unavailable host services.";
}
