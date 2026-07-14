#ifndef FIRESTAFF_REDMCSB_F0709_START_SOUND_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0709_START_SOUND_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IO.C:3832-3843, PC 3.4 I34E/I34M branch. */
typedef struct {
    int16_t graphic_index;
    void *buffer;
} redmcsb_f0709_sound_descriptor_pc34_compat;

typedef const redmcsb_f0709_sound_descriptor_pc34_compat *
    (*redmcsb_f0709_sound_descriptor_lookup_pc34_compat)(
        void *context, int16_t sound_index);

typedef void (*redmcsb_f0709_play_cpsx_pc34_compat)(
    void *context,
    void *sound_buffer,
    int16_t sound_volume,
    int16_t period);

typedef struct {
    redmcsb_f0709_sound_descriptor_lookup_pc34_compat lookup;
    redmcsb_f0709_play_cpsx_pc34_compat play_cpsx;
    void *context;
} redmcsb_f0709_sound_route_pc34_compat;

/*
 * Executes F0709's I34E/I34M branch.  The source calls F0060_SOUND_Play_CPSX
 * with the descriptor buffer, the unmodified volume, and period 6000 when
 * the original source condition admits the descriptor.
 */
bool redmcsb_f0709_start_sound_pc34_compat(
    const redmcsb_f0709_sound_route_pc34_compat *route,
    int16_t sound_index,
    int16_t sound_volume);

const char *redmcsb_f0709_start_sound_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
