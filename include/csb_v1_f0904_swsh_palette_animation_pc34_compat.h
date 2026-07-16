#ifndef FIRESTAFF_CSB_V1_F0904_SWSH_PALETTE_ANIMATION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0904_SWSH_PALETTE_ANIMATION_PC34_COMPAT_H

#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_SWSH_F0904_PALETTE_RECORD_COUNT_PC34 = 27
};

typedef struct CSB_V1_SwshPaletteAnimationFacts_PC34 {
    int valid;
    int source_palette_animation_data_bound;
    int source_palette_command_stream_bound;
    unsigned int source_palette_record_count;
    int source_records_are_two_word_pairs;
    uint32_t source_palette_stream_hash;
    int title_not_started_yet;
    int release_may_follow_after_animation;
    int no_synthetic_palette_data;
    int no_synthetic_graphic_bytes;
    int no_legacy_palette_wrapper;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;
} CSB_V1_SwshPaletteAnimationFacts_PC34;

typedef struct CSB_V1_SwshPaletteAnimationReceipt_PC34 {
    int valid;
    int play_consumed;
    unsigned int source_palette_record_count;
    uint32_t source_palette_stream_hash;
    int source_records_are_two_word_pairs;
    int title_not_started_yet;
    int release_may_follow_after_animation;
    int no_synthetic_palette_data;
    int no_synthetic_graphic_bytes;
    int no_legacy_palette_wrapper;
    const char *source_evidence;
} CSB_V1_SwshPaletteAnimationReceipt_PC34;

void csb_v1_swsh_palette_animation_receipt_init_pc34(
    CSB_V1_SwshPaletteAnimationReceipt_PC34 *receipt);

int F0904_PaletteAnimation(
    const CSB_V1_SwshPaletteAnimationFacts_PC34 *facts,
    CSB_V1_SwshPaletteAnimationReceipt_PC34 *out_receipt);

const char *csb_v1_f0904_palette_animation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0904_SWSH_PALETTE_ANIMATION_PC34_COMPAT_H */
