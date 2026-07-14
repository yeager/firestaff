#include "dm1_v1_original_save_pc34_handoff.h"
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

static int framebuffer_is_nonblank(const unsigned char *pixels, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0u) {
            return 1;
        }
    }
    return 0;
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
        CHECK(M11_GameView_LoadDm1SavePath(&state, receipt->path, NULL),
              "M11 adopts the same external F0435 runtime");
        CHECK(state.world.party.mapIndex == expected_world.party.mapIndex &&
                  state.world.party.mapX == expected_world.party.mapX &&
                  state.world.party.mapY == expected_world.party.mapY &&
                  state.world.party.direction == expected_world.party.direction &&
                  state.world.party.championCount == expected_world.party.championCount &&
                  state.world.gameTick == expected_world.gameTick,
              "M11 runtime preserves source party pose and tick");
        CHECK(state.world.dungeon != NULL && state.world.things != NULL &&
                  state.world.ownsDungeon == 1,
              "M11 retains the source-owned dungeon");
        CHECK(!state.candidateMirrorPanelActive &&
                  !state.candidateMirrorRenameActive &&
                  !state.inventoryPanelActive,
              "F0435 does not invent a transient C040 HoC panel");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        CHECK(framebuffer_is_nonblank(framebuffer, sizeof(framebuffer)),
              "adopted original runtime produces a real M11 frame");
        printf("ADMITTED_HOC_RUNTIME path=%s map=%d,%d,%d dir=%d tick=%u champions=%d\\n",
               receipt->path, state.world.party.mapIndex, state.world.party.mapX,
               state.world.party.mapY, state.world.party.direction,
               (unsigned int)state.world.gameTick,
               state.world.party.championCount);
        F0883_WORLD_Free_Compat(&expected_world);
        M11_GameView_Shutdown(&state);
    }

    printf("ADMITTED_HOC_RUNTIME_SUMMARY root=%s candidates=%d\\n",
           corpus_root, report.receipt_count);
    return 0;
}
