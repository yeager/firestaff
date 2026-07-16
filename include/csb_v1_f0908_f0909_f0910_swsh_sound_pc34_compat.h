#ifndef FIRESTAFF_CSB_V1_F0908_F0909_F0910_SWSH_SOUND_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0908_F0909_F0910_SWSH_SOUND_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34 = 9078,
    CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34 = 334,
    CSB_V1_SWSH_SOUND_CHANNEL_LEFT_PC34 = 8,
    CSB_V1_SWSH_SOUND_CHANNEL_RIGHT_PC34 = 4
};

typedef struct CSB_V1_SwshSoundInitFacts_PC34 {
    int valid;
    int source_swoosh_sample_bound;
    long source_sample_byte_count;
    int source_sample_period;
    uint32_t source_sample_hash;
    int chip_memory_allocation_bound;
    int sample_copied_to_owned_buffer;
    int left_channel_bound;
    int right_channel_bound;
    int left_channel_unit;
    int right_channel_unit;
    int same_sample_buffer_for_stereo;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
} CSB_V1_SwshSoundInitFacts_PC34;

typedef struct CSB_V1_SwshSoundInitReceipt_PC34 {
    int valid;
    long source_sample_byte_count;
    int source_sample_period;
    uint32_t source_sample_hash;
    int stereo_channels_bound;
    int owned_sample_buffer_bound;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
    const char *source_evidence;
} CSB_V1_SwshSoundInitReceipt_PC34;

typedef struct CSB_V1_SwshSoundPlayFacts_PC34 {
    int valid;
    int begin_left_channel;
    int begin_right_channel;
    int control_start_command;
    int title_not_started_yet;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
    CSB_V1_SwshSoundInitReceipt_PC34 init;
} CSB_V1_SwshSoundPlayFacts_PC34;

typedef struct CSB_V1_SwshSoundPlayReceipt_PC34 {
    int valid;
    int init_consumed;
    int left_channel_started;
    int right_channel_started;
    int control_start_command_sent;
    int title_not_started_yet;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
    const char *source_evidence;
} CSB_V1_SwshSoundPlayReceipt_PC34;

typedef struct CSB_V1_SwshSoundReleaseFacts_PC34 {
    int valid;
    int control_finish_command;
    int sync_cycle_set_before_finish;
    int wait_left_channel;
    int wait_right_channel;
    int sync_cycle_cleared_after_wait;
    int control_stop_command;
    int owned_sample_buffer_released;
    int title_may_consume_after_release;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;
} CSB_V1_SwshSoundReleaseFacts_PC34;

typedef struct CSB_V1_SwshSoundReleaseReceipt_PC34 {
    int valid;
    int play_consumed;
    int finish_before_stop;
    int channels_waited;
    int owned_sample_buffer_released;
    int title_may_consume_after_release;
    int no_host_audio_device_emulation;
    int no_synthetic_sound_data;
    int no_legacy_swoosh_wrapper;
    const char *source_evidence;
} CSB_V1_SwshSoundReleaseReceipt_PC34;

void csb_v1_swsh_sound_init_receipt_init_pc34(
    CSB_V1_SwshSoundInitReceipt_PC34 *receipt);
void csb_v1_swsh_sound_play_receipt_init_pc34(
    CSB_V1_SwshSoundPlayReceipt_PC34 *receipt);
void csb_v1_swsh_sound_release_receipt_init_pc34(
    CSB_V1_SwshSoundReleaseReceipt_PC34 *receipt);

int F0908_InitSound(
    const CSB_V1_SwshSoundInitFacts_PC34 *facts,
    CSB_V1_SwshSoundInitReceipt_PC34 *out_receipt);
int F0909_PlaySwooshSound(
    const CSB_V1_SwshSoundPlayFacts_PC34 *facts,
    CSB_V1_SwshSoundPlayReceipt_PC34 *out_receipt);
int F0910_ReleaseSwooshSound(
    const CSB_V1_SwshSoundReleaseFacts_PC34 *facts,
    CSB_V1_SwshSoundReleaseReceipt_PC34 *out_receipt);

const char *csb_v1_f0908_init_sound_source_evidence_pc34(void);
const char *csb_v1_f0909_play_swoosh_sound_source_evidence_pc34(void);
const char *csb_v1_f0910_release_swoosh_sound_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0908_F0909_F0910_SWSH_SOUND_PC34_COMPAT_H */
