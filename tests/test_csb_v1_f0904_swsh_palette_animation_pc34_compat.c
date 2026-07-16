#include "csb_v1_f0904_swsh_palette_animation_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_SwshSoundInitFacts_PC34 make_init_facts(void)
{
    CSB_V1_SwshSoundInitFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.source_swoosh_sample_bound = 1;
    facts.source_sample_byte_count = CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34;
    facts.source_sample_period = CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34;
    facts.source_sample_hash = 0x9078334u;
    facts.chip_memory_allocation_bound = 1;
    facts.sample_copied_to_owned_buffer = 1;
    facts.left_channel_bound = 1;
    facts.right_channel_bound = 1;
    facts.left_channel_unit = CSB_V1_SWSH_SOUND_CHANNEL_LEFT_PC34;
    facts.right_channel_unit = CSB_V1_SWSH_SOUND_CHANNEL_RIGHT_PC34;
    facts.same_sample_buffer_for_stereo = 1;
    facts.no_host_audio_device_emulation = 1;
    facts.no_synthetic_sound_data = 1;
    facts.no_legacy_swoosh_wrapper = 1;
    return facts;
}

static CSB_V1_SwshSoundPlayReceipt_PC34 make_play_receipt(void)
{
    CSB_V1_SwshSoundInitFacts_PC34 init_facts = make_init_facts();
    CSB_V1_SwshSoundInitReceipt_PC34 init;
    CSB_V1_SwshSoundPlayFacts_PC34 play_facts;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;

    CHECK(F0908_InitSound(&init_facts, &init) == 1);

    memset(&play_facts, 0, sizeof(play_facts));
    play_facts.valid = 1;
    play_facts.begin_left_channel = 1;
    play_facts.begin_right_channel = 1;
    play_facts.control_start_command = 1;
    play_facts.title_not_started_yet = 1;
    play_facts.no_host_audio_device_emulation = 1;
    play_facts.no_synthetic_sound_data = 1;
    play_facts.no_legacy_swoosh_wrapper = 1;
    play_facts.init = init;

    CHECK(F0909_PlaySwooshSound(&play_facts, &play) == 1);
    return play;
}

static CSB_V1_SwshPaletteAnimationFacts_PC34 make_palette_facts(
    CSB_V1_SwshSoundPlayReceipt_PC34 play)
{
    CSB_V1_SwshPaletteAnimationFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.source_palette_animation_data_bound = 1;
    facts.source_palette_command_stream_bound = 1;
    facts.source_palette_record_count =
        CSB_V1_SWSH_F0904_PALETTE_RECORD_COUNT_PC34;
    facts.source_records_are_two_word_pairs = 1;
    facts.source_palette_stream_hash = 0x09040027u;
    facts.title_not_started_yet = 1;
    facts.release_may_follow_after_animation = 1;
    facts.no_synthetic_palette_data = 1;
    facts.no_synthetic_graphic_bytes = 1;
    facts.no_legacy_palette_wrapper = 1;
    facts.play = play;
    return facts;
}

static void test_accepts_source_palette_animation_between_play_and_release(void)
{
    CSB_V1_SwshSoundPlayReceipt_PC34 play = make_play_receipt();
    CSB_V1_SwshPaletteAnimationFacts_PC34 facts = make_palette_facts(play);
    CSB_V1_SwshPaletteAnimationReceipt_PC34 receipt;

    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.play_consumed == 1);
    CHECK(receipt.source_palette_record_count == 27u);
    CHECK(receipt.source_palette_stream_hash == 0x09040027u);
    CHECK(receipt.source_records_are_two_word_pairs == 1);
    CHECK(receipt.title_not_started_yet == 1);
    CHECK(receipt.release_may_follow_after_animation == 1);
    CHECK(receipt.no_synthetic_palette_data == 1);
    CHECK(receipt.no_synthetic_graphic_bytes == 1);
    CHECK(receipt.no_legacy_palette_wrapper == 1);
}

static void test_rejects_unsequenced_or_synthetic_palette_routes(void)
{
    CSB_V1_SwshSoundPlayReceipt_PC34 play = make_play_receipt();
    CSB_V1_SwshPaletteAnimationFacts_PC34 facts = make_palette_facts(play);
    CSB_V1_SwshPaletteAnimationReceipt_PC34 receipt;

    facts.play.valid = 0;
    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 0);
    CHECK(receipt.no_synthetic_palette_data == 1);

    facts = make_palette_facts(play);
    facts.source_palette_record_count = 26u;
    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 0);

    facts = make_palette_facts(play);
    facts.source_palette_stream_hash = 0u;
    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 0);

    facts = make_palette_facts(play);
    facts.no_synthetic_palette_data = 0;
    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 0);

    facts = make_palette_facts(play);
    facts.no_legacy_palette_wrapper = 0;
    CHECK(F0904_PaletteAnimation(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    check_contains(csb_v1_f0904_palette_animation_source_evidence_pc34(),
                   "F0904_PaletteAnimation");
    check_contains(csb_v1_f0904_palette_animation_source_evidence_pc34(),
                   "after F0909_PlaySwooshSound");
    check_contains(csb_v1_f0904_palette_animation_source_evidence_pc34(),
                   "before F0910_ReleaseSwooshSound");
    check_contains(csb_v1_f0904_palette_animation_source_evidence_pc34(),
                   "27 two-word records");
}

int main(void)
{
    test_accepts_source_palette_animation_between_play_and_release();
    test_rejects_unsequenced_or_synthetic_palette_routes();
    test_evidence_string();
    return 0;
}
