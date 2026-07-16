#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"

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

static CSB_V1_SwshSoundPlayFacts_PC34 make_play_facts(
    CSB_V1_SwshSoundInitReceipt_PC34 init)
{
    CSB_V1_SwshSoundPlayFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.begin_left_channel = 1;
    facts.begin_right_channel = 1;
    facts.control_start_command = 1;
    facts.title_not_started_yet = 1;
    facts.no_host_audio_device_emulation = 1;
    facts.no_synthetic_sound_data = 1;
    facts.no_legacy_swoosh_wrapper = 1;
    facts.init = init;
    return facts;
}

static CSB_V1_SwshSoundReleaseFacts_PC34 make_release_facts(
    CSB_V1_SwshSoundPlayReceipt_PC34 play)
{
    CSB_V1_SwshSoundReleaseFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.control_finish_command = 1;
    facts.sync_cycle_set_before_finish = 1;
    facts.wait_left_channel = 1;
    facts.wait_right_channel = 1;
    facts.sync_cycle_cleared_after_wait = 1;
    facts.control_stop_command = 1;
    facts.owned_sample_buffer_released = 1;
    facts.title_may_consume_after_release = 1;
    facts.no_host_audio_device_emulation = 1;
    facts.no_synthetic_sound_data = 1;
    facts.no_legacy_swoosh_wrapper = 1;
    facts.play = play;
    return facts;
}

static void test_accepts_source_swoosh_lifecycle(void)
{
    CSB_V1_SwshSoundInitFacts_PC34 init_facts = make_init_facts();
    CSB_V1_SwshSoundInitReceipt_PC34 init;
    CSB_V1_SwshSoundPlayFacts_PC34 play_facts;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;
    CSB_V1_SwshSoundReleaseFacts_PC34 release_facts;
    CSB_V1_SwshSoundReleaseReceipt_PC34 release;

    CHECK(F0908_InitSound(&init_facts, &init) == 1);
    CHECK(init.valid == 1);
    CHECK(init.source_sample_byte_count == 9078);
    CHECK(init.source_sample_period == 334);
    CHECK(init.source_sample_hash == 0x9078334u);
    CHECK(init.stereo_channels_bound == 1);
    CHECK(init.owned_sample_buffer_bound == 1);

    play_facts = make_play_facts(init);
    CHECK(F0909_PlaySwooshSound(&play_facts, &play) == 1);
    CHECK(play.valid == 1);
    CHECK(play.init_consumed == 1);
    CHECK(play.left_channel_started == 1);
    CHECK(play.right_channel_started == 1);
    CHECK(play.control_start_command_sent == 1);
    CHECK(play.title_not_started_yet == 1);

    release_facts = make_release_facts(play);
    CHECK(F0910_ReleaseSwooshSound(&release_facts, &release) == 1);
    CHECK(release.valid == 1);
    CHECK(release.play_consumed == 1);
    CHECK(release.finish_before_stop == 1);
    CHECK(release.channels_waited == 1);
    CHECK(release.owned_sample_buffer_released == 1);
    CHECK(release.title_may_consume_after_release == 1);
}

static void test_rejects_synthetic_or_wrong_sample_facts(void)
{
    CSB_V1_SwshSoundInitFacts_PC34 facts = make_init_facts();
    CSB_V1_SwshSoundInitReceipt_PC34 receipt;

    facts.source_sample_byte_count = 9079;
    CHECK(F0908_InitSound(&facts, &receipt) == 0);
    CHECK(receipt.no_synthetic_sound_data == 1);

    facts = make_init_facts();
    facts.source_sample_period = 333;
    CHECK(F0908_InitSound(&facts, &receipt) == 0);

    facts = make_init_facts();
    facts.source_sample_hash = 0u;
    CHECK(F0908_InitSound(&facts, &receipt) == 0);

    facts = make_init_facts();
    facts.no_host_audio_device_emulation = 0;
    CHECK(F0908_InitSound(&facts, &receipt) == 0);
}

static void test_rejects_unsequenced_play_or_release(void)
{
    CSB_V1_SwshSoundInitReceipt_PC34 init;
    CSB_V1_SwshSoundPlayFacts_PC34 play_facts;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;
    CSB_V1_SwshSoundReleaseFacts_PC34 release_facts;
    CSB_V1_SwshSoundReleaseReceipt_PC34 release;

    memset(&init, 0, sizeof(init));
    play_facts = make_play_facts(init);
    CHECK(F0909_PlaySwooshSound(&play_facts, &play) == 0);
    CHECK(play.no_synthetic_sound_data == 1);

    {
        CSB_V1_SwshSoundInitFacts_PC34 init_facts = make_init_facts();
        CHECK(F0908_InitSound(&init_facts, &init) == 1);
    }
    play_facts = make_play_facts(init);
    play_facts.title_not_started_yet = 0;
    CHECK(F0909_PlaySwooshSound(&play_facts, &play) == 0);

    play_facts = make_play_facts(init);
    CHECK(F0909_PlaySwooshSound(&play_facts, &play) == 1);
    release_facts = make_release_facts(play);
    release_facts.wait_right_channel = 0;
    CHECK(F0910_ReleaseSwooshSound(&release_facts, &release) == 0);

    release_facts = make_release_facts(play);
    release_facts.title_may_consume_after_release = 0;
    CHECK(F0910_ReleaseSwooshSound(&release_facts, &release) == 0);
}

static void test_evidence_strings(void)
{
    check_contains(csb_v1_f0908_init_sound_source_evidence_pc34(),
                   "SWSHSND.C:10-24/74-143");
    check_contains(csb_v1_f0908_init_sound_source_evidence_pc34(),
                   "byte count 9078 and G0744 period 334");

    check_contains(csb_v1_f0909_play_swoosh_sound_source_evidence_pc34(),
                   "SWSHSND.C:26-32/207-213");
    check_contains(csb_v1_f0909_play_swoosh_sound_source_evidence_pc34(),
                   "before the palette animation and TITLE handoff");

    check_contains(csb_v1_f0910_release_swoosh_sound_source_evidence_pc34(),
                   "SWSHSND.C:35-48/216-226");
    check_contains(csb_v1_f0910_release_swoosh_sound_source_evidence_pc34(),
                   "before TITLE.C F0437 consumes C001");
}

int main(void)
{
    test_accepts_source_swoosh_lifecycle();
    test_rejects_synthetic_or_wrong_sample_facts();
    test_rejects_unsequenced_play_or_release();
    test_evidence_strings();
    return 0;
}
