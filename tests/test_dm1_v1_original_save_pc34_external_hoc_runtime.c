#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s (line %d): %s\\n", \
                    message, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

static int receipt_is_runtime_admitted(
    const DM1OriginalSavePC34CorpusReceipt *receipt)
{
    return receipt && receipt->external_original &&
           receipt->roundtrip_receipts_committed &&
           receipt->roundtrip_result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
           receipt->source_runtime_stage_committed &&
           receipt->source_runtime_stage_owns_dungeon &&
           receipt->source_runtime_adopted &&
           receipt->source_runtime_adopt_owns_dungeon &&
           receipt->source_runtime_adopt_queue_committed;
}

/* ReDMCSB DUNVIEW.C's PC viewport is the 224x136 region below the top bar.
 * HUD chrome alone must not certify an adopted F0435 runtime as rendered. */
#define PC34_VIEWPORT_X 0
#define PC34_VIEWPORT_Y 33
#define PC34_VIEWPORT_WIDTH 224
#define PC34_VIEWPORT_HEIGHT 136

/* This is a test-run bound, not a game-time policy.  The event time comes
 * exclusively from the admitted F0435 C4 queue; a far-future or malformed
 * timestamp must not make an opt-in corpus probe run indefinitely. */
#define PC34_EXTERNAL_RUNTIME_EVENT_HORIZON 1024u

static int viewport_is_nonblank(const unsigned char *pixels,
                                int framebuffer_width)
{
    int y;
    int x;

    for (y = 0; y < PC34_VIEWPORT_HEIGHT; ++y) {
        const unsigned char *row = pixels +
            ((PC34_VIEWPORT_Y + y) * framebuffer_width) + PC34_VIEWPORT_X;
        for (x = 0; x < PC34_VIEWPORT_WIDTH; ++x) {
            if (row[x] != 0u) {
                return 1;
            }
        }
    }
    return 0;
}

static unsigned int viewport_fingerprint(const unsigned char *pixels,
                                         int framebuffer_width)
{
    unsigned int hash = 2166136261u;
    int y;
    int x;

    for (y = 0; y < PC34_VIEWPORT_HEIGHT; ++y) {
        const unsigned char *row = pixels +
            ((PC34_VIEWPORT_Y + y) * framebuffer_width) + PC34_VIEWPORT_X;
        for (x = 0; x < PC34_VIEWPORT_WIDTH; ++x) {
            hash ^= row[x];
            hash *= 16777619u;
        }
    }
    return hash;
}

int main(void)
{
    const char *corpus_root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    const char *data_dir = getenv("FIRESTAFF_DM1_PC_DATA");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int i;

    /* This is intentionally fixture-free. It only drives files classified as
     * external PC34 saves through the production F0435 M11 boundary. */
    if (!corpus_root || !corpus_root[0] || !data_dir || !data_dir[0]) {
        puts("SKIP external PC34 HoC runtime: FIRESTAFF_DM1_PC34_SAVE_CORPUS or FIRESTAFF_DM1_PC_DATA unset");
        return 0;
    }

    memset(&report, 0, sizeof(report));
    CHECK(dm1_v1_original_save_pc34_roundtrip_corpus_root(corpus_root, &report) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "external corpus passes F0435/F0433 preflight");
    CHECK(report.pc34_candidate_count > 0 &&
              report.receipt_count == report.pc34_candidate_count,
          "external corpus has classified PC34 candidates only");

    for (i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];
        struct GameWorld_Compat expected_world;
        M11_GameViewState state;
        unsigned char framebuffer[320 * 200];
        unsigned char repeated_framebuffer[320 * 200];
        uint32_t expected_world_hash;
        unsigned int viewport_hash;
        unsigned int post_tick_viewport_hash;
        struct TickInput_Compat expected_input;
        struct TickResult_Compat expected_tick;
        uint32_t expected_post_tick_world_hash;
        uint32_t actual_post_tick_world_hash;
        uint32_t pre_tick;
        struct TimelineEvent_Compat next_event;

        CHECK(receipt_is_runtime_admitted(receipt),
              "external save owns an admitted F0435 runtime");
        memset(&expected_world, 0, sizeof(expected_world));
        M11_GameView_Init(&state);
        CHECK(M11_GameView_StartDm1(&state, data_dir),
              "M11 starts against original PC34 data");
        CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
                  receipt->path, &state.world, &expected_world, NULL, NULL) ==
                  DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "F0435 materializes an owned expected world");
        CHECK(F0891_ORCH_WorldHash_Compat(&expected_world,
                                           &expected_world_hash),
              "F0435 materializes a serializable expected runtime world");
        CHECK(M11_GameView_LoadDm1SavePath(&state, receipt->path, NULL),
              "M11 adopts the same external F0435 runtime");
        CHECK(state.dm1ViewportRuntimeOrigin ==
                  DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34,
              "M11 marks the viewport as an original-save runtime");
        CHECK(state.world.party.mapIndex == expected_world.party.mapIndex &&
                  state.world.party.mapX == expected_world.party.mapX &&
                  state.world.party.mapY == expected_world.party.mapY &&
                  state.world.party.direction == expected_world.party.direction &&
                  state.world.party.championCount == expected_world.party.championCount &&
                  state.world.gameTick == expected_world.gameTick,
              "M11 runtime preserves source party pose and tick");
        CHECK(state.lastWorldHash == expected_world_hash,
              "M11 adopts the complete F0435 runtime world without mutation");
        CHECK(state.world.dungeon != NULL && state.world.things != NULL &&
                  state.world.ownsDungeon == 1,
              "M11 retains the source-owned dungeon");
        CHECK(!state.candidateMirrorPanelActive &&
                  !state.candidateMirrorRenameActive &&
                  !state.inventoryPanelActive,
              "F0435 does not invent a transient C040 HoC panel");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        CHECK(viewport_is_nonblank(framebuffer, 320),
              "adopted original runtime produces a nonblank PC34 viewport");
        viewport_hash = viewport_fingerprint(framebuffer, 320);
        memset(repeated_framebuffer, 0, sizeof(repeated_framebuffer));
        M11_GameView_Draw(&state, repeated_framebuffer, 320, 200);
        CHECK(memcmp(framebuffer, repeated_framebuffer, sizeof(framebuffer)) == 0,
              "unchanged adopted runtime produces a byte-stable M11 frame");
        CHECK(viewport_hash == viewport_fingerprint(repeated_framebuffer, 320),
              "unchanged adopted runtime preserves the PC34 viewport receipt");
        /* ReDMCSB GAMELOOP.C advances the restored F0435 world through its
         * normal idle command path. Advance the separately staged F0435
         * world first, so M11 must publish the same M10 tick receipt rather
         * than merely incrementing a host-side counter. Do not manufacture an
         * input route or a replacement world here: every exercised event
         * remains owned by the external PC34 save's restored C3/C4 timeline.
         */
        pre_tick = (uint32_t)state.world.gameTick;
        memset(&expected_input, 0, sizeof(expected_input));
        memset(&expected_tick, 0, sizeof(expected_tick));
        expected_input.tick = (uint32_t)expected_world.gameTick;
        expected_input.command = CMD_NONE;
        CHECK(F0884_ORCH_AdvanceOneTick_Compat(&expected_world,
                                                &expected_input,
                                                &expected_tick) != ORCH_FAIL,
              "staged F0435 runtime accepts the source idle tick");
        CHECK(expected_tick.preTick == pre_tick &&
                  expected_tick.postTick == pre_tick + 1u,
              "staged F0435 runtime advances exactly one source tick");
        CHECK(F0891_ORCH_WorldHash_Compat(&expected_world,
                                           &expected_post_tick_world_hash) &&
                  expected_post_tick_world_hash == expected_tick.worldHashPost,
              "staged F0435 idle tick publishes its canonical world hash");
        CHECK(M11_GameView_HandleInput(&state, M12_MENU_INPUT_NONE) ==
                  M11_GAME_INPUT_REDRAW,
              "adopted original runtime accepts an idle source tick");
        CHECK((uint32_t)state.world.gameTick == pre_tick + 1u,
              "adopted original runtime advances exactly one source tick");
        CHECK(state.dm1ViewportRuntimeOrigin ==
                  DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34,
              "idle source tick retains the original-save viewport origin");
        CHECK(state.lastTickResult.worldHashPost == state.lastWorldHash &&
                  state.lastWorldHash != 0u,
              "idle source tick publishes the M10 runtime receipt");
        CHECK(state.lastTickResult.preTick == expected_tick.preTick &&
                  state.lastTickResult.postTick == expected_tick.postTick &&
                  state.lastTickResult.worldHashPost == expected_tick.worldHashPost &&
                  state.lastWorldHash == expected_post_tick_world_hash,
              "M11 idle receipt matches the independently staged F0435 tick");
        CHECK(F0891_ORCH_WorldHash_Compat(&state.world,
                                           &actual_post_tick_world_hash) &&
                  actual_post_tick_world_hash == expected_post_tick_world_hash,
              "M11 live world matches the staged F0435 idle world");
        CHECK(state.world.timeline.count == expected_world.timeline.count &&
                  state.lastTickResult.emissionCount == expected_tick.emissionCount &&
                  memcmp(state.lastTickResult.emissions, expected_tick.emissions,
                         sizeof(expected_tick.emissions)) == 0,
              "M11 preserves the staged F0435 timeline and emission receipt");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        CHECK(viewport_is_nonblank(framebuffer, 320),
              "ticked original runtime produces a nonblank PC34 viewport");
        post_tick_viewport_hash = viewport_fingerprint(framebuffer, 320);
        memset(repeated_framebuffer, 0, sizeof(repeated_framebuffer));
        M11_GameView_Draw(&state, repeated_framebuffer, 320, 200);
        CHECK(memcmp(framebuffer, repeated_framebuffer, sizeof(framebuffer)) == 0,
              "unchanged ticked runtime produces a byte-stable M11 frame");
        CHECK(post_tick_viewport_hash ==
                  viewport_fingerprint(repeated_framebuffer, 320),
              "ticked original runtime preserves the PC34 viewport receipt");

        /* The first tick proves the immediate idle route.  When the restored
         * C4 queue supplies a near pending event, continue with CMD_NONE until
         * that exact source-owned event is due.  Each host tick must retain
         * the independently materialized F0435 world's M10 receipt. */
        memset(&next_event, 0, sizeof(next_event));
        if (F0722_TIMELINE_Peek_Compat(&expected_world.timeline, &next_event)) {
            uint32_t ticks_until_event;
            uint32_t step;

            CHECK(next_event.fireAtTick >= (uint32_t)expected_world.gameTick,
                  "restored C4 queue has no overdue event after the idle tick");
            ticks_until_event = next_event.fireAtTick -
                                (uint32_t)expected_world.gameTick;
            if (ticks_until_event < PC34_EXTERNAL_RUNTIME_EVENT_HORIZON) {
                for (step = 0; step <= ticks_until_event; ++step) {
                    memset(&expected_input, 0, sizeof(expected_input));
                    memset(&expected_tick, 0, sizeof(expected_tick));
                    expected_input.tick = (uint32_t)expected_world.gameTick;
                    expected_input.command = CMD_NONE;
                    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&expected_world,
                                                            &expected_input,
                                                            &expected_tick) != ORCH_FAIL,
                          "staged F0435 runtime accepts the queued-event idle tick");
                    CHECK(M11_GameView_HandleInput(&state, M12_MENU_INPUT_NONE) ==
                              M11_GAME_INPUT_REDRAW,
                          "M11 accepts the queued-event idle tick");
                    CHECK(state.lastTickResult.preTick == expected_tick.preTick &&
                              state.lastTickResult.postTick == expected_tick.postTick &&
                              state.lastTickResult.worldHashPost == expected_tick.worldHashPost &&
                              state.lastWorldHash == expected_tick.worldHashPost,
                          "M11 queued-event idle receipt matches staged F0435 tick");
                    CHECK(F0891_ORCH_WorldHash_Compat(
                              &state.world, &actual_post_tick_world_hash) &&
                              actual_post_tick_world_hash == expected_tick.worldHashPost,
                          "M11 live world matches every staged F0435 queue tick");
                    CHECK(state.world.timeline.count == expected_world.timeline.count &&
                              state.lastTickResult.emissionCount == expected_tick.emissionCount &&
                              memcmp(state.lastTickResult.emissions, expected_tick.emissions,
                                     sizeof(expected_tick.emissions)) == 0,
                          "M11 preserves queued-event timeline and emissions");
                }
                CHECK((uint32_t)state.world.gameTick ==
                          next_event.fireAtTick + 1u,
                      "M11 reaches the saved queue event on its source tick");
                printf("ADMITTED_HOC_RUNTIME_EVENT path=%s kind=%d fire_at=%u steps=%u\\n",
                       receipt->path, next_event.kind,
                       (unsigned int)next_event.fireAtTick,
                       (unsigned int)(ticks_until_event + 1u));
            } else {
                printf("ADMITTED_HOC_RUNTIME_EVENT_DEFERRED path=%s kind=%d fire_at=%u horizon=%u\\n",
                       receipt->path, next_event.kind,
                       (unsigned int)next_event.fireAtTick,
                       (unsigned int)PC34_EXTERNAL_RUNTIME_EVENT_HORIZON);
            }
        } else {
            puts("ADMITTED_HOC_RUNTIME_EVENT_NONE");
        }
        printf("ADMITTED_HOC_RUNTIME path=%s map=%d,%d,%d dir=%d tick=%u champions=%d viewport_hash=%08x\\n",
               receipt->path, state.world.party.mapIndex, state.world.party.mapX,
               state.world.party.mapY, state.world.party.direction,
               (unsigned int)state.world.gameTick,
               state.world.party.championCount, post_tick_viewport_hash);
        F0883_WORLD_Free_Compat(&expected_world);
        M11_GameView_Shutdown(&state);
    }

    printf("ADMITTED_HOC_RUNTIME_SUMMARY root=%s candidates=%d\\n",
           corpus_root, report.receipt_count);
    return 0;
}
