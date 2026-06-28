#include "csb_v1_audio_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, name) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s (line %d)\n", name, __LINE__); \
    } \
} while (0)

static CsbV1AudioRequest req(int16_t soundIndex,
                             int16_t mode,
                             int16_t volume,
                             uint8_t priority)
{
    CsbV1AudioRequest request;
    memset(&request, 0, sizeof(request));
    request.soundIndex = soundIndex;
    request.mapX = 4;
    request.mapY = 7;
    request.mode = mode;
    request.volume = volume;
    request.priority = priority;
    return request;
}

static void test_init_contract(void)
{
    CsbV1AudioRuntime runtime;
    csb_v1_audio_runtime_init(&runtime);

    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE, "init has no pending sound");
    CHECK(runtime.pendingVolume == 0, "init pending volume zero");
    CHECK(runtime.pendingPriority == 0, "init pending priority zero");
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_NONE, "init has no last played sound");
    CHECK(runtime.lastCreatureAttackTime == -200, "init last creature attack time source default");
    CHECK(CSB_V1_SOUND_COUNT == 35, "CSB PC/Amiga sound count is 35");
    CHECK(CSB_V1_MODE_PLAY_IF_PRIORITIZED == 1, "prioritized mode value matches source");
    CHECK(CSB_V1_MODE_PLAY_ONE_TICK_LATER == 2, "one-tick-later mode value matches source");
}

static void test_rejections(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRequest request;
    csb_v1_audio_runtime_init(&runtime);

    request = req(CSB_V1_SOUND_SWITCH, CSB_V1_MODE_DO_NOT_PLAY, 3, 90);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 0,
          "do-not-play request rejected");
    CHECK(runtime.totalRejectedRequests == 1, "do-not-play counted as rejected");

    request = req(CSB_V1_SOUND_COUNT, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 3, 90);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 0,
          "out-of-range sound rejected");

    request = req(CSB_V1_SOUND_SWITCH, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 0, 90);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 0,
          "zero-volume sound rejected");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE, "rejections leave pending empty");
}

static void test_immediate_clears_pending(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRequest request;
    csb_v1_audio_runtime_init(&runtime);

    request = req(CSB_V1_SOUND_MOVE_SKELETON, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 2, 40);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "movement sound queues pending");

    request = req(CSB_V1_SOUND_SCREAM, CSB_V1_MODE_PLAY_IMMEDIATELY, 3, 95);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "immediate request plays");
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_SCREAM,
          "immediate sound becomes last played");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE,
          "immediate request clears stale pending sound");
    CHECK(runtime.totalImmediatePlays == 1, "immediate play counted");
}

static void test_pending_priority_arbitration(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRequest request;
    csb_v1_audio_runtime_init(&runtime);

    request = req(CSB_V1_SOUND_MOVE_SKELETON, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 2, 40);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "first prioritized request wins empty slot");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_MOVE_SKELETON,
          "skeleton movement pending");

    request = req(CSB_V1_SOUND_SWITCH, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 1, 90);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 0,
          "quieter higher-priority sound does not replace louder pending");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_MOVE_SKELETON,
          "louder pending preserved");

    request = req(CSB_V1_SOUND_COMBAT, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 2, 90);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "same-volume higher-priority sound replaces");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_COMBAT,
          "combat pending after priority replacement");

    request = req(CSB_V1_SOUND_ATTACK_SCORPION, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 3, 80);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "louder sound replaces regardless of priority");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_ATTACK_SCORPION,
          "scorpion attack pending after louder replacement");
}

static void test_flush_contract(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRequest request;
    csb_v1_audio_runtime_init(&runtime);

    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 0,
          "empty flush is a no-op");

    request = req(CSB_V1_SOUND_PARTY_DAMAGED, CSB_V1_MODE_PLAY_ONE_TICK_LATER, 2, 85);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "one-tick-later sound queues into pending slot");
    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 1,
          "pending flush plays one sound");
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_PARTY_DAMAGED,
          "flushed pending sound becomes last played");
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE,
          "flush clears pending index");
    CHECK(runtime.pendingVolume == 0, "flush clears pending volume");
    CHECK(runtime.totalPendingFlushes == 1, "flush count increments once");
}

static void test_save_snapshot_is_runtime_bounded(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRuntime restored;
    CsbV1AudioSaveSnapshot snapshot;
    CsbV1AudioRequest request;

    csb_v1_audio_runtime_init(&runtime);
    csb_v1_audio_runtime_record_creature_attack(&runtime, 12345);
    request = req(CSB_V1_SOUND_SPELL, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 3, 80);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1,
          "pre-save spell pending");

    memset(&snapshot, 0, sizeof(snapshot));
    csb_v1_audio_runtime_save_snapshot(&runtime, &snapshot);
    CHECK(snapshot.lastCreatureAttackTime == 12345,
          "save snapshot preserves LastCreatureAttackTime");

    csb_v1_audio_runtime_init(&restored);
    request = req(CSB_V1_SOUND_BUZZ, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 1, 75);
    CHECK(csb_v1_audio_runtime_request(&restored, &request) == 1,
          "restored runtime starts with local pending candidate");
    csb_v1_audio_runtime_load_snapshot(&restored, &snapshot);

    CHECK(restored.lastCreatureAttackTime == 12345,
          "load restores LastCreatureAttackTime");
    CHECK(restored.pendingSoundIndex == CSB_V1_SOUND_NONE,
          "load drops transient pending audio");
    CHECK(restored.lastPlayedSoundIndex == CSB_V1_SOUND_NONE,
          "load does not invent played audio");
}

static void test_source_evidence(void)
{
    const char* evidence = csb_v1_audio_runtime_source_evidence();
    CHECK(evidence != NULL, "source evidence string exists");
    CHECK(strstr(evidence, "DEFS.H:135-138") != NULL, "mode evidence cited");
    CHECK(strstr(evidence, "SOUND.C:1632-1638") != NULL, "request evidence cited");
    CHECK(strstr(evidence, "SOUND.C:1804-1865") != NULL, "flush evidence cited");
    CHECK(strstr(evidence, "GAMELOOP.C:114-115") != NULL, "game-loop flush evidence cited");
    CHECK(strstr(evidence, "LOADSAVE.C:1530/2739") != NULL, "save/load evidence cited");
}

int main(void)
{
    test_init_contract();
    test_rejections();
    test_immediate_clears_pending();
    test_pending_priority_arbitration();
    test_flush_contract();
    test_save_snapshot_is_runtime_bounded();
    test_source_evidence();

    printf("test_csb_v1_audio_runtime_pc34_compat: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail ? 1 : 0;
}
