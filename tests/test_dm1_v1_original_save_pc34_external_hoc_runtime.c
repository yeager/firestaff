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

static int receipt_is_runtime_candidate(
    const DM1OriginalSavePC34CorpusReceipt *receipt)
{
    return receipt && receipt->external_original &&
           receipt->source_byte_count > 0u && receipt->source_hash != 0u;
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
#define PC34_EXTERNAL_RUNTIME_TRACE_MAX_STEPS 256

/* Operator-provided trace rows are deliberately bound to an admitted save
 * snapshot: `<source_crc32_hex> <IDLE|FORWARD|BACKWARD|TURN_LEFT|TURN_RIGHT|
 * STRAFE_LEFT|STRAFE_RIGHT>`.  This target owns neither a replacement save
 * nor a made-up command stream.  ReDMCSB GAMELOOP.C hands normal movement
 * commands to the restored world after F0435; these are the M11 inputs that
 * map one-to-one to the M10 movement/timeline boundary. */
typedef struct PC34ExternalRuntimeTraceStep {
    uint32_t source_hash;
    M12_MenuInput input;
} PC34ExternalRuntimeTraceStep;

typedef struct PC34ExternalSaveSnapshot {
    uint8_t *bytes;
    size_t size;
    uint32_t hash;
} PC34ExternalSaveSnapshot;

static uint32_t snapshot_hash(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int read_external_save_snapshot(const char *path,
                                       PC34ExternalSaveSnapshot *snapshot)
{
    FILE *file;
    long length;

    if (!path || !path[0] || !snapshot) return 0;
    memset(snapshot, 0, sizeof(*snapshot));
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    snapshot->size = (size_t)length;
    snapshot->bytes = (uint8_t *)malloc(snapshot->size);
    if (!snapshot->bytes ||
        fread(snapshot->bytes, 1u, snapshot->size, file) != snapshot->size) {
        free(snapshot->bytes);
        memset(snapshot, 0, sizeof(*snapshot));
        fclose(file);
        return 0;
    }
    fclose(file);
    snapshot->hash = snapshot_hash(snapshot->bytes, snapshot->size);
    return snapshot->hash != 0u;
}

static int trace_input_from_name(const char *name, M12_MenuInput *out_input)
{
    if (!name || !out_input) return 0;
    if (strcmp(name, "IDLE") == 0) *out_input = M12_MENU_INPUT_NONE;
    else if (strcmp(name, "FORWARD") == 0) *out_input = M12_MENU_INPUT_UP;
    else if (strcmp(name, "BACKWARD") == 0) *out_input = M12_MENU_INPUT_DOWN;
    else if (strcmp(name, "TURN_LEFT") == 0) *out_input = M12_MENU_INPUT_TURN_LEFT;
    else if (strcmp(name, "TURN_RIGHT") == 0) *out_input = M12_MENU_INPUT_TURN_RIGHT;
    else if (strcmp(name, "STRAFE_LEFT") == 0) *out_input = M12_MENU_INPUT_STRAFE_LEFT;
    else if (strcmp(name, "STRAFE_RIGHT") == 0) *out_input = M12_MENU_INPUT_STRAFE_RIGHT;
    else return 0;
    return 1;
}

static int load_external_trace(const char *path,
                               PC34ExternalRuntimeTraceStep *out_steps,
                               int out_capacity,
                               int *out_count)
{
    FILE *file;
    char line[128];
    int count = 0;

    if (!path || !path[0] || !out_steps || out_capacity <= 0 || !out_count) {
        return 0;
    }
    file = fopen(path, "r");
    if (!file) return 0;
    while (fgets(line, sizeof(line), file)) {
        char hash_text[16];
        char input_text[32];
        char trailing[2];
        char *end;
        unsigned long hash;
        M12_MenuInput input;

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (count >= out_capacity ||
            sscanf(line, "%15s %31s %1s", hash_text, input_text, trailing) != 2) {
            fclose(file);
            return 0;
        }
        hash = strtoul(hash_text, &end, 16);
        if (strlen(hash_text) != 8u || *end != '\0' || hash > 0xfffffffful ||
            !trace_input_from_name(input_text, &input)) {
            fclose(file);
            return 0;
        }
        out_steps[count].source_hash = (uint32_t)hash;
        out_steps[count].input = input;
        ++count;
    }
    fclose(file);
    *out_count = count;
    return count > 0;
}

static uint8_t trace_command_for_input(M12_MenuInput input, int direction)
{
    switch (input) {
        case M12_MENU_INPUT_NONE: return CMD_NONE;
        case M12_MENU_INPUT_TURN_LEFT: return CMD_TURN_LEFT;
        case M12_MENU_INPUT_TURN_RIGHT: return CMD_TURN_RIGHT;
        case M12_MENU_INPUT_UP:
            return (uint8_t[]){CMD_MOVE_NORTH, CMD_MOVE_EAST,
                               CMD_MOVE_SOUTH, CMD_MOVE_WEST}[direction & 3];
        case M12_MENU_INPUT_DOWN:
            return (uint8_t[]){CMD_MOVE_SOUTH, CMD_MOVE_WEST,
                               CMD_MOVE_NORTH, CMD_MOVE_EAST}[direction & 3];
        case M12_MENU_INPUT_STRAFE_LEFT:
            return (uint8_t[]){CMD_MOVE_WEST, CMD_MOVE_NORTH,
                               CMD_MOVE_EAST, CMD_MOVE_SOUTH}[direction & 3];
        case M12_MENU_INPUT_STRAFE_RIGHT:
            return (uint8_t[]){CMD_MOVE_EAST, CMD_MOVE_SOUTH,
                               CMD_MOVE_WEST, CMD_MOVE_NORTH}[direction & 3];
        default: return CMD_NONE;
    }
}

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

static int region_has_nonzero_pixel(const unsigned char *pixels,
                                    int framebuffer_width,
                                    int x, int y, int width, int height)
{
    int row;
    int col;

    if (!pixels || framebuffer_width <= 0 || width <= 0 || height <= 0) {
        return 0;
    }
    for (row = 0; row < height; ++row) {
        const unsigned char *line = pixels + (y + row) * framebuffer_width + x;
        for (col = 0; col < width; ++col) {
            if (line[col] != 0u) return 1;
        }
    }
    return 0;
}

static int find_pending_poison_expiry(const struct GameWorld_Compat *world,
                                      uint32_t now,
                                      uint32_t *out_fire_at_tick,
                                      int *out_champion_index)
{
    int i;
    int found = 0;

    if (!world || !out_fire_at_tick || !out_champion_index) return 0;
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat *event = &world->timeline.events[i];

        if (event->kind != TIMELINE_EVENT_STATUS_TIMEOUT ||
            event->aux0 != DM1_EVENT_POISON_CHAMPION ||
            event->aux4 < 0 || event->aux4 >= DM1_MAX_CHAMPIONS ||
            (event->fireAtTick > now &&
             event->fireAtTick - now >= PC34_EXTERNAL_RUNTIME_EVENT_HORIZON)) {
            continue;
        }
        if (!found || event->fireAtTick < *out_fire_at_tick) {
            *out_fire_at_tick = event->fireAtTick;
            *out_champion_index = event->aux4;
            found = 1;
        }
    }
    return found;
}

static int tick_result_has_champion_damage(const struct TickResult_Compat *result,
                                            int champion_index,
                                            int damage)
{
    int i;

    if (!result) return 0;
    for (i = 0; i < result->emissionCount; ++i) {
        const struct TickEmission_Compat *emission = &result->emissions[i];

        if (emission->kind == EMIT_CHAMPION_DAMAGED &&
            emission->payload[0] == champion_index &&
            emission->payload[2] == damage) {
            return 1;
        }
    }
    return 0;
}

/* ReDMCSB LOADSAVE.C F0435:2803-2816 restores the four packed portrait
 * buffers before returning to the live game. The M11 inventory portrait is
 * at the C017 viewport origin plus its source-owned five/four pixel inset. */
static int saved_portrait_matches_inventory_frame(
    const struct ChampionState_Compat *champion,
    const unsigned char *pixels,
    int framebuffer_width)
{
    int y;

    if (!champion || !champion->portraitBitmapValid || !pixels ||
        framebuffer_width < 32) {
        return 0;
    }
    for (y = 0; y < CHAMPION_PORTRAIT_BITMAP_HEIGHT; ++y) {
        const unsigned char *source = champion->portraitBitmap +
            y * (CHAMPION_PORTRAIT_BITMAP_WIDTH / 2);
        const unsigned char *rendered = pixels +
            (PC34_VIEWPORT_Y + 4 + y) * framebuffer_width +
            PC34_VIEWPORT_X + 5;
        int x;

        for (x = 0; x < CHAMPION_PORTRAIT_BITMAP_WIDTH; x += 2) {
            if (rendered[x] != (unsigned char)(source[x / 2] >> 4) ||
                rendered[x + 1] != (unsigned char)(source[x / 2] & 0x0fU)) {
                return 0;
            }
        }
    }
    return 1;
}

int main(void)
{
    const char *corpus_root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    const char *data_dir = getenv("FIRESTAFF_DM1_PC_DATA");
    const char *trace_path = getenv("FIRESTAFF_DM1_PC34_HOC_COMMAND_TRACE");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    PC34ExternalRuntimeTraceStep trace_steps[PC34_EXTERNAL_RUNTIME_TRACE_MAX_STEPS];
    int trace_step_count = 0;
    int i;

    /* This is intentionally fixture-free. It only drives files classified as
     * external PC34 saves through the production F0435 M11 boundary. */
    if (!corpus_root || !corpus_root[0] || !data_dir || !data_dir[0]) {
        puts("SKIP external PC34 HoC runtime: FIRESTAFF_DM1_PC34_SAVE_CORPUS or FIRESTAFF_DM1_PC_DATA unset");
        return 0;
    }
    if (trace_path && trace_path[0] &&
        !load_external_trace(trace_path, trace_steps,
                             PC34_EXTERNAL_RUNTIME_TRACE_MAX_STEPS,
                             &trace_step_count)) {
        fprintf(stderr, "FAIL external PC34 HoC command trace is malformed or empty: %s\n",
                trace_path);
        return 1;
    }

    memset(&report, 0, sizeof(report));
    {
        int result = dm1_v1_original_save_pc34_roundtrip_corpus_root(
            corpus_root, &report);

        /* This long-running probe compares every intermediate F0435 tick
         * against the legacy self-contained stage receipt. Tail-less PC34
         * media instead follows M11's DUNGEON.DAT-backed route and is covered
         * by the dedicated backed-corpus roundtrip target. */
        for (i = 0; i < report.receipt_count; ++i) {
            if (!report.receipts[i].source_runtime_stage_committed) {
                puts("SKIP external PC34 HoC runtime: candidate requires DUNGEON.DAT backing; covered by dm1_v1_original_save_pc34_backed_corpus_roundtrip");
                return 0;
            }
        }
        CHECK(result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "external corpus passes F0435/F0433 preflight");
    }
    CHECK(report.pc34_candidate_count > 0 &&
              report.receipt_count == report.pc34_candidate_count,
          "external corpus has classified PC34 candidates only");

    for (i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];
        PC34ExternalSaveSnapshot snapshot;
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
        uint32_t poison_fire_at_tick = 0;
        int poison_champion_index = -1;
        int poison_runtime_checked = 0;
        int traced_steps_for_save = 0;

        /* The generic receipt authenticates the source bytes. The runtime
         * proof below deliberately uses M11's DUNGEON.DAT-backed F0435 path,
         * which is required by original saves that do not own a dungeon tail. */
        CHECK(receipt_is_runtime_candidate(receipt),
              "external save is a classified PC34 runtime candidate");
        CHECK(read_external_save_snapshot(receipt->path, &snapshot) &&
                  snapshot.size == receipt->source_byte_count &&
                  snapshot.hash == receipt->source_hash,
              "external runtime consumes the corpus-certified source snapshot");
        memset(&expected_world, 0, sizeof(expected_world));
        M11_GameView_Init(&state);
        CHECK(M11_GameView_StartDm1(&state, data_dir),
              "M11 starts against original PC34 data");
        CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
                  snapshot.bytes, snapshot.size, &state.world, &expected_world,
                  NULL, NULL) ==
                  DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "F0435 materializes the certified expected world with its start dungeon");
        CHECK(F0891_ORCH_WorldHash_Compat(&expected_world,
                                           &expected_world_hash),
              "F0435 materializes a serializable expected runtime world");
        CHECK(M11_GameView_LoadDm1OriginalPc34SaveBytes(
                  &state, snapshot.bytes, snapshot.size, receipt->path),
              "M11 adopts the same certified F0435 runtime");
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
        CHECK(state.world.party.activeChampionIndex >= 0 &&
                  state.world.party.activeChampionIndex < CHAMPION_MAX_PARTY &&
                  state.world.party.champions[
                      state.world.party.activeChampionIndex].present &&
                  state.world.party.champions[
                      state.world.party.activeChampionIndex].portraitBitmapValid,
              "F0435 retains the active champion's saved portrait payload");
        CHECK(M11_GameView_ToggleInventoryPanel(&state),
              "M11 opens the source inventory panel for the adopted runtime");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        CHECK(saved_portrait_matches_inventory_frame(
                  &state.world.party.champions[state.world.party.activeChampionIndex],
                  framebuffer, 320),
              "M11 inventory consumes the F0435 saved portrait pixels");
        /* PANEL.C F0349 command 70 dispatches F0345 through C545, the
         * source-owned mouth zone.  The restored save supplies the champion
         * state; no fixture may manufacture its food/water panel. */
        CHECK(M11_GameView_HandlePointer(&state, 64, 54, 1) ==
                  M11_GAME_INPUT_REDRAW,
              "C545 mouth click reaches the F0345 source panel route");
        CHECK(state.v1FoodWaterPanelActive,
              "C545 activates the original food/water panel state");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        CHECK(region_has_nonzero_pixel(framebuffer, 320,
                                       PC34_VIEWPORT_X + 112,
                                       PC34_VIEWPORT_Y + 60,
                                       46, 32),
              "F0345 materializes original C020/C030/C031 food-water panel pixels");
        CHECK(!M11_GameView_ToggleInventoryPanel(&state),
              "M11 closes the source inventory panel after the portrait receipt");
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
        /* The HUD interactions above deliberately exercise the restored UI,
         * but can mutate the live party panel state. Reload the same external
         * bytes before comparing an idle GAMELOOP tick with the independently
         * staged F0435 world. */
        CHECK(M11_GameView_LoadDm1OriginalPc34SaveBytes(
                  &state, snapshot.bytes, snapshot.size, receipt->path),
              "M11 reloads the original PC34 runtime for the idle tick proof");
        CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
                  snapshot.bytes, snapshot.size, &state.world, &expected_world,
                  NULL, NULL) == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "F0435 restages the reloaded original runtime for the idle tick proof");
        pre_tick = (uint32_t)state.world.gameTick;
        (void)find_pending_poison_expiry(&expected_world,
                                         pre_tick,
                                         &poison_fire_at_tick,
                                         &poison_champion_index);
        {
            int poison_hp_before = -1;

            if (poison_champion_index >= 0 &&
                poison_fire_at_tick <= pre_tick + 1u) {
                poison_hp_before = expected_world.party.champions[
                    poison_champion_index].hp.current;
            }
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
        /* M11 consumes the same canonical M10 receipt above, then applies
         * host-owned presentation state (damage overlays, status/banner
         * routing) to its live world. Its post-presentation hash is not the
         * F0435 world hash; the equality contract is lastTickResult and
         * lastWorldHash, both checked immediately above. */
        CHECK(F0891_ORCH_WorldHash_Compat(&state.world,
                                           &actual_post_tick_world_hash) &&
                  actual_post_tick_world_hash != 0u,
              "M11 post-presentation world remains serializable");
        CHECK(state.world.timeline.count == expected_world.timeline.count &&
                  state.lastTickResult.emissionCount == expected_tick.emissionCount &&
                  memcmp(state.lastTickResult.emissions, expected_tick.emissions,
                         sizeof(expected_tick.emissions)) == 0,
              "M11 preserves the staged F0435 timeline and emission receipt");
            if (poison_hp_before >= 0) {
                int poison_damage = poison_hp_before - expected_world.party.champions[
                    poison_champion_index].hp.current;

                if (poison_damage > 0) {
                    CHECK(tick_result_has_champion_damage(&expected_tick,
                                                           poison_champion_index,
                                                           poison_damage),
                          "C75 poison expiry emits its source damage receipt");
                    CHECK(state.championDamageTimer[poison_champion_index] > 0 &&
                              state.championDamageAmount[poison_champion_index] ==
                                  poison_damage,
                          "C75 poison expiry materializes an M11 damage overlay");
                }
                poison_runtime_checked = 1;
            }
        }
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
            uint32_t replay_until_tick;
            uint32_t step;

            CHECK(next_event.fireAtTick >= (uint32_t)expected_world.gameTick,
                  "restored C4 queue has no overdue event after the idle tick");
            replay_until_tick = next_event.fireAtTick;
            if (!poison_runtime_checked && poison_champion_index >= 0 &&
                poison_fire_at_tick > replay_until_tick) {
                replay_until_tick = poison_fire_at_tick;
            }
            ticks_until_event = replay_until_tick -
                                (uint32_t)expected_world.gameTick;
            if (ticks_until_event < PC34_EXTERNAL_RUNTIME_EVENT_HORIZON) {
                for (step = 0; step <= ticks_until_event; ++step) {
                    int poison_hp_before = -1;

                    if (!poison_runtime_checked && poison_champion_index >= 0 &&
                        poison_fire_at_tick <=
                            (uint32_t)expected_world.gameTick + 1u) {
                        poison_hp_before = expected_world.party.champions[
                            poison_champion_index].hp.current;
                    }
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
                    if (poison_hp_before >= 0) {
                        int poison_damage = poison_hp_before -
                            expected_world.party.champions[
                                poison_champion_index].hp.current;

                        if (poison_damage > 0) {
                            CHECK(tick_result_has_champion_damage(
                                      &expected_tick,
                                      poison_champion_index,
                                      poison_damage),
                                  "C75 poison expiry emits its source damage receipt");
                            CHECK(state.championDamageTimer[poison_champion_index] > 0 &&
                                      state.championDamageAmount[poison_champion_index] ==
                                          poison_damage,
                                  "C75 poison expiry materializes an M11 damage overlay");
                        }
                        poison_runtime_checked = 1;
                    }
                }
                CHECK((uint32_t)state.world.gameTick ==
                          replay_until_tick + 1u,
                      "M11 reaches the requested saved queue tick");
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
        if (trace_step_count > 0) {
            int trace_index;

            for (trace_index = 0; trace_index < trace_step_count; ++trace_index) {
                const PC34ExternalRuntimeTraceStep *step =
                    &trace_steps[trace_index];
                uint32_t trace_world_hash;
                uint32_t live_trace_world_hash;
                unsigned int trace_viewport_hash;

                if (step->source_hash != receipt->source_hash) continue;
                memset(&expected_input, 0, sizeof(expected_input));
                memset(&expected_tick, 0, sizeof(expected_tick));
                expected_input.tick = (uint32_t)expected_world.gameTick;
                expected_input.command = trace_command_for_input(
                    step->input, expected_world.party.direction);
                CHECK(F0884_ORCH_AdvanceOneTick_Compat(&expected_world,
                                                        &expected_input,
                                                        &expected_tick) != ORCH_FAIL,
                      "staged F0435 runtime accepts the traced source command");
                CHECK(M11_GameView_HandleInput(&state, step->input) ==
                          M11_GAME_INPUT_REDRAW,
                      "M11 accepts the traced source command");
                CHECK(state.lastTickResult.preTick == expected_tick.preTick &&
                          state.lastTickResult.postTick == expected_tick.postTick &&
                          state.lastTickResult.worldHashPost ==
                              expected_tick.worldHashPost &&
                          state.lastWorldHash == expected_tick.worldHashPost,
                      "M11 traced command receipt matches staged F0435 tick");
                CHECK(F0891_ORCH_WorldHash_Compat(&expected_world,
                                                   &trace_world_hash) &&
                          F0891_ORCH_WorldHash_Compat(&state.world,
                                                       &live_trace_world_hash) &&
                          trace_world_hash == expected_tick.worldHashPost &&
                          live_trace_world_hash == trace_world_hash,
                      "M11 traced command world matches staged F0435 world");
                CHECK(state.world.timeline.count == expected_world.timeline.count &&
                          state.lastTickResult.emissionCount ==
                              expected_tick.emissionCount &&
                          memcmp(state.lastTickResult.emissions,
                                 expected_tick.emissions,
                                 sizeof(expected_tick.emissions)) == 0,
                      "M11 traced command preserves timeline and emissions");
                memset(framebuffer, 0, sizeof(framebuffer));
                M11_GameView_Draw(&state, framebuffer, 320, 200);
                CHECK(viewport_is_nonblank(framebuffer, 320),
                      "traced source command produces a nonblank PC34 viewport");
                trace_viewport_hash = viewport_fingerprint(framebuffer, 320);
                memset(repeated_framebuffer, 0, sizeof(repeated_framebuffer));
                M11_GameView_Draw(&state, repeated_framebuffer, 320, 200);
                CHECK(memcmp(framebuffer, repeated_framebuffer,
                             sizeof(framebuffer)) == 0 &&
                          trace_viewport_hash ==
                              viewport_fingerprint(repeated_framebuffer, 320),
                      "traced source command preserves a stable viewport capture");
                ++traced_steps_for_save;
            }
            CHECK(traced_steps_for_save > 0,
                  "external command trace binds at least one row to every admitted save");
            printf("ADMITTED_HOC_RUNTIME_TRACE path=%s source_hash=%08x steps=%d\\n",
                   receipt->path, receipt->source_hash, traced_steps_for_save);
        }
        printf("ADMITTED_HOC_RUNTIME path=%s map=%d,%d,%d dir=%d tick=%u champions=%d viewport_hash=%08x\\n",
               receipt->path, state.world.party.mapIndex, state.world.party.mapX,
               state.world.party.mapY, state.world.party.direction,
               (unsigned int)state.world.gameTick,
               state.world.party.championCount, post_tick_viewport_hash);
        {
            PC34ExternalSaveSnapshot final_snapshot;
            CHECK(read_external_save_snapshot(receipt->path, &final_snapshot) &&
                      final_snapshot.size == snapshot.size &&
                      final_snapshot.hash == snapshot.hash,
                  "external save remains the certified snapshot through runtime replay");
            free(final_snapshot.bytes);
        }
        free(snapshot.bytes);
        F0883_WORLD_Free_Compat(&expected_world);
        M11_GameView_Shutdown(&state);
    }

    printf("ADMITTED_HOC_RUNTIME_SUMMARY root=%s candidates=%d trace_steps=%d\\n",
           corpus_root, report.receipt_count, trace_step_count);
    return 0;
}
