#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"

#include <string.h>

static int csb_v1_swsh_init_receipt_matches_pc34(
    const CSB_V1_SwshSoundInitReceipt_PC34 *receipt)
{
    return receipt && receipt->valid &&
        receipt->source_sample_byte_count ==
            CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34 &&
        receipt->source_sample_period == CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34 &&
        receipt->source_sample_hash != 0u &&
        receipt->stereo_channels_bound &&
        receipt->owned_sample_buffer_bound &&
        receipt->no_host_audio_device_emulation &&
        receipt->no_synthetic_sound_data &&
        receipt->no_legacy_swoosh_wrapper;
}

static int csb_v1_swsh_play_receipt_matches_pc34(
    const CSB_V1_SwshSoundPlayReceipt_PC34 *receipt)
{
    return receipt && receipt->valid &&
        receipt->init_consumed &&
        receipt->left_channel_started &&
        receipt->right_channel_started &&
        receipt->control_start_command_sent &&
        receipt->title_not_started_yet &&
        receipt->no_host_audio_device_emulation &&
        receipt->no_synthetic_sound_data &&
        receipt->no_legacy_swoosh_wrapper;
}

void csb_v1_swsh_sound_init_receipt_init_pc34(
    CSB_V1_SwshSoundInitReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

void csb_v1_swsh_sound_play_receipt_init_pc34(
    CSB_V1_SwshSoundPlayReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

void csb_v1_swsh_sound_release_receipt_init_pc34(
    CSB_V1_SwshSoundReleaseReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0908_InitSound(
    const CSB_V1_SwshSoundInitFacts_PC34 *facts,
    CSB_V1_SwshSoundInitReceipt_PC34 *out_receipt)
{
    const char *evidence = csb_v1_f0908_init_sound_source_evidence_pc34();

    csb_v1_swsh_sound_init_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->source_swoosh_sample_bound ||
        facts->source_sample_byte_count !=
            CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34 ||
        facts->source_sample_period != CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34 ||
        facts->source_sample_hash == 0u ||
        !facts->chip_memory_allocation_bound ||
        !facts->sample_copied_to_owned_buffer ||
        !facts->left_channel_bound ||
        !facts->right_channel_bound ||
        facts->left_channel_unit != CSB_V1_SWSH_SOUND_CHANNEL_LEFT_PC34 ||
        facts->right_channel_unit != CSB_V1_SWSH_SOUND_CHANNEL_RIGHT_PC34 ||
        !facts->same_sample_buffer_for_stereo ||
        !facts->no_host_audio_device_emulation ||
        !facts->no_synthetic_sound_data ||
        !facts->no_legacy_swoosh_wrapper) {
        if (out_receipt) {
            out_receipt->no_host_audio_device_emulation = 1;
            out_receipt->no_synthetic_sound_data = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->source_sample_byte_count = facts->source_sample_byte_count;
    out_receipt->source_sample_period = facts->source_sample_period;
    out_receipt->source_sample_hash = facts->source_sample_hash;
    out_receipt->stereo_channels_bound = 1;
    out_receipt->owned_sample_buffer_bound = 1;
    out_receipt->no_host_audio_device_emulation = 1;
    out_receipt->no_synthetic_sound_data = 1;
    out_receipt->no_legacy_swoosh_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

int F0909_PlaySwooshSound(
    const CSB_V1_SwshSoundPlayFacts_PC34 *facts,
    CSB_V1_SwshSoundPlayReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0909_play_swoosh_sound_source_evidence_pc34();

    csb_v1_swsh_sound_play_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->begin_left_channel ||
        !facts->begin_right_channel ||
        !facts->control_start_command ||
        !facts->title_not_started_yet ||
        !facts->no_host_audio_device_emulation ||
        !facts->no_synthetic_sound_data ||
        !facts->no_legacy_swoosh_wrapper ||
        !csb_v1_swsh_init_receipt_matches_pc34(&facts->init)) {
        if (out_receipt) {
            out_receipt->no_host_audio_device_emulation = 1;
            out_receipt->no_synthetic_sound_data = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->init_consumed = 1;
    out_receipt->left_channel_started = 1;
    out_receipt->right_channel_started = 1;
    out_receipt->control_start_command_sent = 1;
    out_receipt->title_not_started_yet = 1;
    out_receipt->no_host_audio_device_emulation = 1;
    out_receipt->no_synthetic_sound_data = 1;
    out_receipt->no_legacy_swoosh_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

int F0910_ReleaseSwooshSound(
    const CSB_V1_SwshSoundReleaseFacts_PC34 *facts,
    CSB_V1_SwshSoundReleaseReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0910_release_swoosh_sound_source_evidence_pc34();

    csb_v1_swsh_sound_release_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->control_finish_command ||
        !facts->sync_cycle_set_before_finish ||
        !facts->wait_left_channel ||
        !facts->wait_right_channel ||
        !facts->sync_cycle_cleared_after_wait ||
        !facts->control_stop_command ||
        !facts->owned_sample_buffer_released ||
        !facts->title_may_consume_after_release ||
        !facts->no_host_audio_device_emulation ||
        !facts->no_synthetic_sound_data ||
        !facts->no_legacy_swoosh_wrapper ||
        !csb_v1_swsh_play_receipt_matches_pc34(&facts->play)) {
        if (out_receipt) {
            out_receipt->no_host_audio_device_emulation = 1;
            out_receipt->no_synthetic_sound_data = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->play_consumed = 1;
    out_receipt->finish_before_stop = 1;
    out_receipt->channels_waited = 1;
    out_receipt->owned_sample_buffer_released = 1;
    out_receipt->title_may_consume_after_release = 1;
    out_receipt->no_host_audio_device_emulation = 1;
    out_receipt->no_synthetic_sound_data = 1;
    out_receipt->no_legacy_swoosh_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0908_init_sound_source_evidence_pc34(void)
{
    return "ReDMCSB SWSHSND.C:10-24/74-143 F0908_InitSound binds the real "
           "G0746_auc_SwooshSoundData sample to left/right audio channels "
           "with G0745 byte count 9078 and G0744 period 334; SWSHSDAT.C:462-464 "
           "defines the period and byte count";
}

const char *csb_v1_f0909_play_swoosh_sound_source_evidence_pc34(void)
{
    return "ReDMCSB SWSHSND.C:26-32/207-213 F0909_PlaySwooshSound starts "
           "the left and right swoosh channels before sending CMD_START on "
           "the control request; SWSH.C:2189-2191 places it before the "
           "palette animation and TITLE handoff";
}

const char *csb_v1_f0910_release_swoosh_sound_source_evidence_pc34(void)
{
    return "ReDMCSB SWSHSND.C:35-48/216-226 F0910_ReleaseSwooshSound sends "
           "ADCMD_FINISH with ADIOF_SYNCCYCLE, waits for both channels, clears "
           "sync, stops the control request, and releases the owned swoosh "
           "buffer before TITLE.C F0437 consumes C001";
}
