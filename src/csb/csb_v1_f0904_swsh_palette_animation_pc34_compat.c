#include "csb_v1_f0904_swsh_palette_animation_pc34_compat.h"

#include <string.h>

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

void csb_v1_swsh_palette_animation_receipt_init_pc34(
    CSB_V1_SwshPaletteAnimationReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0904_PaletteAnimation(
    const CSB_V1_SwshPaletteAnimationFacts_PC34 *facts,
    CSB_V1_SwshPaletteAnimationReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0904_palette_animation_source_evidence_pc34();

    csb_v1_swsh_palette_animation_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->source_palette_animation_data_bound ||
        !facts->source_palette_command_stream_bound ||
        facts->source_palette_record_count !=
            CSB_V1_SWSH_F0904_PALETTE_RECORD_COUNT_PC34 ||
        !facts->source_records_are_two_word_pairs ||
        facts->source_palette_stream_hash == 0u ||
        !facts->title_not_started_yet ||
        !facts->release_may_follow_after_animation ||
        !facts->no_synthetic_palette_data ||
        !facts->no_synthetic_graphic_bytes ||
        !facts->no_legacy_palette_wrapper ||
        !csb_v1_swsh_play_receipt_matches_pc34(&facts->play)) {
        if (out_receipt) {
            out_receipt->no_synthetic_palette_data = 1;
            out_receipt->no_synthetic_graphic_bytes = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->play_consumed = 1;
    out_receipt->source_palette_record_count =
        facts->source_palette_record_count;
    out_receipt->source_palette_stream_hash =
        facts->source_palette_stream_hash;
    out_receipt->source_records_are_two_word_pairs = 1;
    out_receipt->title_not_started_yet = 1;
    out_receipt->release_may_follow_after_animation = 1;
    out_receipt->no_synthetic_palette_data = 1;
    out_receipt->no_synthetic_graphic_bytes = 1;
    out_receipt->no_legacy_palette_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0904_palette_animation_source_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C F0904_PaletteAnimation consumes the source "
           "G0741_aai_PaletteAnimationData command records after "
           "F0909_PlaySwooshSound and before F0910_ReleaseSwooshSound; "
           "the CSB gate requires the caller-bound 27 two-word records and "
           "does not synthesize palette or graphic bytes";
}
