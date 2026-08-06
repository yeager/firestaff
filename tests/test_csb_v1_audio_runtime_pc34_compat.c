#include "csb_v1_audio_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
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

static void test_pc34_source_sound_table(void)
{
    const CsbV1Pc34SoundSpec* spec;

    spec = csb_v1_audio_runtime_pc34_sound_spec(CSB_V1_SOUND_SWITCH);
    CHECK(spec != NULL, "PC3.4 switch source row exists");
    CHECK(spec && spec->graphicIndex == 672u, "switch points to graphic 672");
    CHECK(spec && spec->period == 112u && spec->priority == 15u,
          "switch period and priority match ReDMCSB DATA.C");
    CHECK(spec && spec->loudDistance == 0u && spec->softDistance == 3u,
          "switch distance envelope matches ReDMCSB DATA.C");

    spec = csb_v1_audio_runtime_pc34_sound_spec(CSB_V1_SOUND_DOOR_RATTLE);
    CHECK(spec && spec->graphicIndex == 673u && spec->period == 112u,
          "door rattle resolves the source graphic and period");
    spec = csb_v1_audio_runtime_pc34_sound_spec(CSB_V1_SOUND_MOVE_SKELETON);
    CHECK(spec && spec->graphicIndex == 712u && spec->period == 150u,
          "skeleton movement resolves the final source row");
    CHECK(csb_v1_audio_runtime_pc34_sound_spec(-1) == NULL,
          "negative source sound index is rejected");
    CHECK(csb_v1_audio_runtime_pc34_sound_spec(CSB_V1_SOUND_COUNT) == NULL,
          "out-of-range source sound index is rejected");
}

static void test_amiga_source_sound_record(void)
{
    const uint8_t record[] = { 0x00, 0x01, 0x80, 0x00, 0x7f, 0xff };
    CsbV1AmigaSoundPayloadView view;
    CHECK(csb_v1_audio_runtime_amiga_sound_payload_view(record, sizeof(record), &view) == 1,
          "Amiga F1051 sound record admits table-sized data");
    CHECK(view.byteCount == 4u && view.samples == record + 2u,
          "Amiga payload skips its two source header bytes");
    CHECK(csb_v1_audio_runtime_amiga_sound_payload_view(record, 1u, &view) == 0,
          "Amiga sound record rejects an absent source header");
}

static void test_amiga_graphics_sound_view(void)
{
    uint8_t graphics[400000];
    const size_t headerSize = 4u + 749u * 8u;
    CsbV1AmigaSoundPayloadView view;

    memset(graphics, 0, sizeof(graphics));
    graphics[0] = 0x80;
    graphics[1] = 0x01;
    graphics[2] = 0x02;
    graphics[3] = 0xed;
    /* Item zero has six direct-loaded bytes. */
    graphics[4] = 0x00;
    graphics[5] = 0x06;
    graphics[4u + 749u * 2u] = 0x00;
    graphics[4u + 749u * 2u + 1u] = 0x06;
    graphics[headerSize + 0u] = 0x00;
    graphics[headerSize + 1u] = 0x01;
    graphics[headerSize + 2u] = 0x80;
    graphics[headerSize + 3u] = 0x00;
    graphics[headerSize + 4u] = 0x7f;
    graphics[headerSize + 5u] = 0xff;
    CHECK(csb_v1_audio_runtime_amiga_graphics_sound_view(
              graphics, sizeof(graphics), 0u, &view) == 1,
          "Amiga graphics item reaches F1051 sound view");
    CHECK(view.byteCount == 4u && view.samples == graphics + headerSize + 2u,
          "Amiga graphics sound view preserves source bytes");
    graphics[4u + 749u * 2u + 1u] = 0x07;
    CHECK(csb_v1_audio_runtime_amiga_graphics_sound_view(
              graphics, sizeof(graphics), 0u, &view) == 0,
          "Amiga graphics sound view rejects non-direct source item");
}

static void test_amiga_original_graphics_sound_view(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_GRAPHICS_DAT");
    FILE *file;
    long byteCount;
    uint8_t *bytes;
    CsbV1AmigaSoundPayloadView view;

    if (!path || !*path) {
        CHECK(1, "Amiga original graphics test skipped without local original media");
        return;
    }
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (byteCount = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        CHECK(0, "Amiga original graphics file opens");
        return;
    }
    bytes = (uint8_t *)malloc((size_t)byteCount);
    if (!bytes || fread(bytes, 1u, (size_t)byteCount, file) != (size_t)byteCount) {
        free(bytes);
        fclose(file);
        CHECK(0, "Amiga original graphics file reads");
        return;
    }
    fclose(file);
    CHECK(csb_v1_audio_runtime_amiga_graphics_sound_view(
              bytes, (size_t)byteCount, 672u, &view) == 1,
          "original Amiga switch record reaches F1051 sound view");
    CHECK(view.byteCount == 130u,
          "original Amiga switch preserves F1051 table-derived byte count");
    CHECK(view.samples != NULL && view.samples[0] == 0x00u,
          "original Amiga switch begins after its two source bytes");
    free(bytes);
}

static void test_pc34_source_sound_payload(void)
{
    const char *path = getenv("FIRESTAFF_CSB_PC34_GRAPHICS_DAT");
    CsbV1Pc34SoundPayload payload;

    if (!path || !*path) {
        CHECK(1, "PC3.4 payload test skipped without local original media");
        return;
    }
    memset(&payload, 0, sizeof(payload));
    CHECK(csb_v1_audio_runtime_load_pc34_sound_payload(
              path, CSB_V1_SOUND_SWITCH, &payload) == 1,
          "original PC3.4 switch payload admits through F0060 framing");
    CHECK(payload.byteCount == 128u, "switch F0060 payload count is source-owned");
    CHECK(payload.spec.graphicIndex == 672u, "payload retains the DATA.C route");
    CHECK(payload.bytes != NULL && payload.bytes[0] == 0u,
          "payload begins after the original big-endian byte count");
    csb_v1_audio_runtime_pc34_sound_payload_free(&payload);
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

static void test_completed_play_history(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioRequest first = req(CSB_V1_SOUND_SWITCH,
                                  CSB_V1_MODE_PLAY_IMMEDIATELY, 32, 1);
    CsbV1AudioRequest second = req(CSB_V1_SOUND_COMBAT,
                                   CSB_V1_MODE_PLAY_IMMEDIATELY, 32, 1);
    CsbV1AudioRequest pending = req(CSB_V1_SOUND_SPELL,
                                    CSB_V1_MODE_PLAY_ONE_TICK_LATER, 32, 1);
    int16_t sound = CSB_V1_SOUND_NONE;
    int16_t volume = 0;

    csb_v1_audio_runtime_init(&runtime);
    CHECK(csb_v1_audio_runtime_request(&runtime, &first) == 1,
          "first immediate sound completes");
    CHECK(csb_v1_audio_runtime_request(&runtime, &second) == 1,
          "second immediate sound completes");
    CHECK(csb_v1_audio_runtime_request(&runtime, &pending) == 1,
          "pending sound is accepted");
    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 1,
          "pending sound completes on source tick");
    CHECK(runtime.totalCompletedPlays == 3u,
          "completed history counts every F0064/F0065 result");
    CHECK(csb_v1_audio_runtime_completed_play_details_at(
              &runtime, 1u, &sound, &volume) &&
              sound == CSB_V1_SOUND_SWITCH,
          "completed history retains first immediate source sound");
    CHECK(volume == 32,
          "completed history retains first immediate source volume");
    CHECK(csb_v1_audio_runtime_completed_play_at(&runtime, 2u, &sound) &&
              sound == CSB_V1_SOUND_COMBAT,
          "completed history retains second immediate source sound");
    CHECK(csb_v1_audio_runtime_completed_play_at(&runtime, 3u, &sound) &&
              sound == CSB_V1_SOUND_SPELL,
          "completed history retains F0065 pending flush sound");
    CHECK(!csb_v1_audio_runtime_completed_play_at(&runtime, 4u, &sound),
          "future completed-history sequence rejects");
    CHECK(!csb_v1_audio_runtime_completed_play_at(&runtime, 0u, &sound),
          "zero completed-history sequence rejects");
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
    test_pc34_source_sound_table();
    test_amiga_source_sound_record();
    test_amiga_graphics_sound_view();
    test_amiga_original_graphics_sound_view();
    test_pc34_source_sound_payload();
    test_rejections();
    test_completed_play_history();
    test_immediate_clears_pending();
    test_pending_priority_arbitration();
    test_flush_contract();
    test_save_snapshot_is_runtime_bounded();
    test_source_evidence();

    printf("test_csb_v1_audio_runtime_pc34_compat: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail ? 1 : 0;
}
