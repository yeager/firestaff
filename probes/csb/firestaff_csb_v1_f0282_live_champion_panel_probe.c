/*
 * firestaff_csb_v1_f0282_live_champion_panel_probe.c
 *
 * CSB F0282 live champion-panel integration gate.
 *
 * Source-lock boundary:
 *   - ReDMCSB REVIVE.C F0280 lines 260-277 publishes
 *     G0299_ui_CandidateChampionOrdinal after a champion-mirror sensor
 *     materializes a candidate champion.
 *   - ReDMCSB PANEL.C F0346 lines 1619-1635 draws the C040
 *     resurrect/reincarnate panel while G0299 is live.
 *   - ReDMCSB COMMAND.C F0359 lines 1988-1990 dispatches C160/C161/C162
 *     panel commands to F0282.
 *   - ReDMCSB REVIVE.C F0282 lines 744-806 clears/cancels or confirms the
 *     candidate and disables the front champion-mirror sensor on confirm.
 *
 * The probe imports a user-staged CSB Utility Disk / CSBWin CSBGAME save,
 * uses the first real champion as the M11 mirror candidate, then drives a
 * synthetic one-square C127 front sensor through M11's live C040 select,
 * cancel, and confirm path.  Missing real save data is a clean SKIP.
 */

#include "m11_game_view.h"
#include "csb_v1_save_import_path_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#define FS_POPEN _popen
#define FS_PCLOSE _pclose
#else
#define FS_POPEN popen
#define FS_PCLOSE pclose
#endif

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];
    env = getenv("FIRESTAFF_CSBWIN_SAVE_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

static int find_candidate_file(const char *dir,
                               const char *const *candidates,
                               size_t candidate_count,
                               int max_depth,
                               char *out_path,
                               size_t out_path_cap)
{
    char cmd[2048];
    FILE *p;
    char line[1024];
    size_t i;

    if (!dir || !candidates || candidate_count == 0u ||
        !out_path || out_path_cap == 0u || max_depth < 0) {
        return 0;
    }

    snprintf(cmd, sizeof(cmd),
             "find '%s' -maxdepth %d -type f \\( ", dir, max_depth);
    for (i = 0u; i < candidate_count; ++i) {
        if (i > 0u) strncat(cmd, " -o ", sizeof(cmd) - strlen(cmd) - 1u);
        strncat(cmd, "-name ", sizeof(cmd) - strlen(cmd) - 1u);
        strncat(cmd, candidates[i], sizeof(cmd) - strlen(cmd) - 1u);
    }
    strncat(cmd, " \\) 2>/dev/null", sizeof(cmd) - strlen(cmd) - 1u);

    p = FS_POPEN(cmd, "r");
    if (!p) return 0;
    while (fgets(line, sizeof(line), p)) {
        size_t n = strlen(line);
        while (n > 0u && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) {
            line[--n] = '\0';
        }
        if (n == 0u) continue;
        for (i = 0u; i < candidate_count; ++i) {
            const char *base = strrchr(line, '/');
            base = base ? base + 1 : line;
            if (strcmp(base, candidates[i]) == 0) {
                strncpy(out_path, line, out_path_cap - 1u);
                out_path[out_path_cap - 1u] = '\0';
                FS_PCLOSE(p);
                return 1;
            }
        }
    }
    FS_PCLOSE(p);
    return 0;
}

static void copy_trimmed_ascii(unsigned char *dst, int dst_len, const char *src)
{
    int i;
    if (!dst || dst_len <= 0) return;
    memset(dst, 0, (size_t)dst_len);
    if (!src) return;
    for (i = 0; i < dst_len && src[i] != '\0'; ++i) {
        dst[i] = (unsigned char)src[i];
    }
}

static void map_csb_champion_to_m11_candidate(
    const CSB_V1_Champion *src,
    struct ChampionState_Compat *dst)
{
    int i;

    F0600_CHAMPION_InitEmpty_Compat(dst);
    dst->present = 1;
    dst->portraitIndex = 0;
    copy_trimmed_ascii(dst->name, CHAMPION_NAME_LENGTH,
                       (src && src->Name[0]) ? src->Name : "CSB");
    copy_trimmed_ascii(dst->title, CHAMPION_TITLE_LENGTH,
                       src ? src->Title : "");
    dst->sex = 'M';
    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        dst->attributes[i] = src
            ? src->Statistics[i][CSB_V1_STAT_CUR]
            : 40u;
        dst->attributeMaximums[i] = src
            ? src->Statistics[i][CSB_V1_STAT_MAX]
            : 40u;
    }
    for (i = 0; i < CHAMPION_SKILL_COUNT; ++i) {
        dst->skillLevels[i] = src ? src->Skills[i] : 0u;
        dst->skillExperience[i] = 0u;
    }
    dst->hp.current = (unsigned short)((src && src->CurrentHealth > 0)
        ? src->CurrentHealth : 1);
    dst->hp.maximum = (unsigned short)((src && src->MaximumHealth > 0)
        ? src->MaximumHealth : dst->hp.current);
    dst->hp.shifted = (unsigned short)(dst->hp.maximum << 1);
    dst->stamina.current = (unsigned short)((src && src->CurrentStamina > 0)
        ? src->CurrentStamina : 1);
    dst->stamina.maximum = (unsigned short)((src && src->MaximumStamina > 0)
        ? src->MaximumStamina : dst->stamina.current);
    dst->stamina.shifted = (unsigned short)(dst->stamina.maximum << 1);
    dst->mana.current = (unsigned short)((src && src->CurrentMana > 0)
        ? src->CurrentMana : 0);
    dst->mana.maximum = (unsigned short)((src && src->MaximumMana > 0)
        ? src->MaximumMana : dst->mana.current);
    dst->mana.shifted = (unsigned short)(dst->mana.maximum << 1);
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        dst->inventory[i] = src ? src->Slots[i] : THING_NONE;
    }
    dst->load = src ? src->Load : 0u;
    dst->maxLoad = 1u;
    dst->direction = src ? src->Direction : DIR_NORTH;
    dst->food = src ? src->Food : 1500;
    dst->water = src ? src->Water : 1500;
}

static int setup_synthetic_front_sensor_state(M11_GameViewState *state,
                                              const CSB_V1_Champion *candidate)
{
    struct DungeonDatState_Compat *dungeon;
    struct DungeonThings_Compat *things;
    unsigned short sensorThing;
    int i;

    if (!state) return 0;

    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    snprintf(state->sourceId, sizeof(state->sourceId), "csb");
    snprintf(state->title, sizeof(state->title), "CHAOS STRIKES BACK");
    state->showDebugHUD = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = DIR_NORTH;
    state->world.party.activeChampionIndex = -1;

    dungeon = (struct DungeonDatState_Compat *)calloc(1u, sizeof(*dungeon));
    things = (struct DungeonThings_Compat *)calloc(1u, sizeof(*things));
    if (!dungeon || !things) return 0;
    dungeon->maps = (struct DungeonMapDesc_Compat *)calloc(1u, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat *)calloc(1u, sizeof(*dungeon->tiles));
    things->squareFirstThings = (unsigned short *)calloc(9u, sizeof(unsigned short));
    things->sensors = (struct DungeonSensor_Compat *)calloc(1u, sizeof(*things->sensors));
    things->rawThingData[THING_TYPE_SENSOR] = (unsigned char *)calloc(1u, 8u);
    if (!dungeon->maps || !dungeon->tiles || !things->squareFirstThings ||
        !things->sensors || !things->rawThingData[THING_TYPE_SENSOR]) {
        return 0;
    }

    dungeon->header.mapCount = 1;
    dungeon->maps[0].width = 3;
    dungeon->maps[0].height = 3;
    dungeon->tiles[0].squareCount = 9;
    dungeon->tiles[0].squareData = (unsigned char *)calloc(9u, 1u);
    if (!dungeon->tiles[0].squareData) return 0;
    for (i = 0; i < 9; ++i) {
        dungeon->tiles[0].squareData[i] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        things->squareFirstThings[i] = THING_ENDOFLIST;
    }
    dungeon->tiles[0].squareData[1 * 3 + 1] =
        (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                        DUNGEON_SQUARE_MASK_THING_LIST);

    sensorThing = (unsigned short)((2u << 14) |
                                   (THING_TYPE_SENSOR << 10) |
                                   0u);
    things->squareFirstThings[1 * 3 + 1] = sensorThing;
    things->squareFirstThingCount = 9;
    things->sensorCount = 1;
    things->thingCounts[THING_TYPE_SENSOR] = 1;
    things->sensors[0].next = THING_ENDOFLIST;
    things->sensors[0].sensorType = SENSOR_WALL_TYPE_PORTRAIT;
    things->sensors[0].sensorData = 0;
    things->rawThingData[THING_TYPE_SENSOR][0] =
        (unsigned char)(THING_ENDOFLIST & 0xFFu);
    things->rawThingData[THING_TYPE_SENSOR][1] =
        (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    things->loaded = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.ownsDungeon = 0;

    state->mirrorCatalogAvailable = 1;
    state->mirrorCatalog.count = 1;
    state->mirrorCatalog.records[0].textStringIndex = 0;
    state->mirrorCatalog.records[0].mirrorOrdinal = 0;
    map_csb_champion_to_m11_candidate(
        candidate, &state->mirrorCatalog.records[0].champion);
    snprintf(state->mirrorCatalog.records[0].nameText,
             sizeof(state->mirrorCatalog.records[0].nameText),
             "%s", candidate && candidate->Name[0] ? candidate->Name : "CSB");
    snprintf(state->mirrorCatalog.records[0].titleText,
             sizeof(state->mirrorCatalog.records[0].titleText),
             "%s", candidate ? candidate->Title : "");
    return 1;
}

static void free_synthetic_front_sensor_state(M11_GameViewState *state)
{
    struct DungeonDatState_Compat *dungeon;
    struct DungeonThings_Compat *things;
    if (!state) return;
    dungeon = state->world.dungeon;
    things = state->world.things;
    state->world.dungeon = NULL;
    state->world.things = NULL;
    if (things) {
        free(things->squareFirstThings);
        free(things->sensors);
        free(things->rawThingData[THING_TYPE_SENSOR]);
        free(things);
    }
    if (dungeon) {
        if (dungeon->tiles) free(dungeon->tiles[0].squareData);
        free(dungeon->tiles);
        free(dungeon->maps);
        free(dungeon);
    }
    M11_GameView_Shutdown(state);
}

static void test_gate_null_and_non_csb(void)
{
    M11_GameViewState state;
    int front = 99;
    int candidate = 99;
    int partyIndex = 99;

    CHECK(M11_GameView_CsbF0282ChampionPanelGateActive(
              NULL, &front, &candidate, &partyIndex) == 0,
          "NULL CSB F0282 gate is inactive");
    CHECK(front == -1 && candidate == -1 && partyIndex == -1,
          "NULL gate clears all output ordinals");

    M11_GameView_Init(&state);
    state.active = 1;
    snprintf(state.sourceId, sizeof(state.sourceId), "dm1");
    CHECK(M11_GameView_CsbF0282ChampionPanelGateActive(
              &state, &front, &candidate, &partyIndex) == 0,
          "non-CSB source cannot open the CSB F0282 gate");
    M11_GameView_Shutdown(&state);
}

int main(int argc, char **argv)
{
    static const char *const candidates[] = {
        "csbgame.dat",
        "csbgame.bak",
        "CSBGAME.DAT",
        "CSBGAME.BAK"
    };
    char default_dir[1024];
    char found_path[1024];
    const char *dir;
    int found;
    int import_rc;
    CSB_V1_PartyState imported;
    M11_GameViewState state;
    int frontOrdinal;
    int candidateOrdinal;
    int candidatePartyIndex;
    int select_rc;
    int cancel_result;
    int confirm_rc;
    char mirrorName[CHAMPION_NAME_TEXT_CAPACITY];

    printf("=== CSB V1 F0282 live champion-panel probe ===\n\n");

    test_gate_null_and_non_csb();

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");
    found = (dir != NULL) && find_candidate_file(
        dir, candidates, sizeof(candidates) / sizeof(candidates[0]),
        6, found_path, sizeof(found_path));
    if (!found) {
        printf("SKIP: no user-staged CSB Utility Disk / CSBWin "
               "CSBGAME save found under data_dir; data-free gate "
               "sanity checks still ran.\n");
        return g_failures == 0 ? 0 : 1;
    }

    printf("real_save=%s\n", found_path);
    memset(&imported, 0, sizeof(imported));
    import_rc = csb_v1_import_csb_save_file(&imported, found_path);
    CHECK(import_rc > 0, "real CSBGAME save imports at least one champion");
    if (import_rc <= 0) {
        printf("SKIP: found save is not an importable CSBGAME v2.0/v2.1 "
               "roster for this probe (rc=%d).\n", import_rc);
        return g_failures == 0 ? 0 : 1;
    }

    CHECK(imported.ChampionCount >= 1 &&
          imported.ChampionCount <= CSB_V1_MAX_CHAMPIONS,
          "imported CSB party champion count is in 1..4");
    CHECK(setup_synthetic_front_sensor_state(&state,
                                             &imported.Champions[0]) == 1,
          "M11 CSB state has synthetic front C127 mirror sensor");
    if (g_failures != 0) {
        printf("ABORT: could not create the synthetic M11 mirror state.\n");
        return 1;
    }

    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    CHECK(frontOrdinal == 0, "front champion-mirror sensor reports ordinal 0");
    CHECK(M11_GameView_CsbF0282ChampionPanelGateActive(
              &state, &frontOrdinal, &candidateOrdinal,
              &candidatePartyIndex) == 0,
          "CSB F0282 gate is closed before selecting the mirror");

    mirrorName[0] = '\0';
    CHECK(M11_GameView_GetMirrorNameByOrdinal(
              &state, 0, mirrorName, (int)sizeof(mirrorName)) > 0,
          "mirror catalog exposes imported CSB champion candidate name");

    select_rc = M11_GameView_SelectFrontMirrorCandidate(&state);
    CHECK(select_rc == 1, "SelectFrontMirrorCandidate opens C040 panel");
    CHECK(state.candidateMirrorPanelActive == 1 &&
          state.inventoryPanelActive == 1 &&
          state.candidateMirrorOrdinal == 0 &&
          state.candidateMirrorPartyIndex == 0 &&
          state.world.party.championCount == 1 &&
          state.world.party.champions[0].present == 1,
          "C040 panel-live state appends imported champion candidate");
    CHECK(M11_GameView_CsbF0282ChampionPanelGateActive(
              &state, &frontOrdinal, &candidateOrdinal,
              &candidatePartyIndex) == 1,
          "CSB F0282 gate is active only while C040 panel is live");
    CHECK(frontOrdinal == 0 && candidateOrdinal == 0 &&
          candidatePartyIndex == 0,
          "CSB F0282 gate reports matching front/candidate ordinals");

    cancel_result = (int)M11_GameView_HandleInput(&state,
                                                  M12_MENU_INPUT_BACK);
    CHECK(cancel_result == M11_GAME_INPUT_REDRAW,
          "CSB M11 BACK input routes to F0282 C162 cancel while C040 live");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.inventoryPanelActive == 0 &&
          state.candidateMirrorOrdinal == -1 &&
          state.candidateMirrorPartyIndex == -1 &&
          state.world.party.championCount == 0,
          "C162 cancel clears candidate and leaves panel idle");
    CHECK(state.world.things->sensors[0].sensorType ==
          SENSOR_WALL_TYPE_PORTRAIT,
          "cancel keeps champion-mirror C127 sensor enabled");
    CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == 0,
          "front mirror remains selectable after cancel");

    CHECK(M11_GameView_SelectFrontMirrorCandidate(&state) == 1,
          "front mirror can reopen C040 after cancel");
    confirm_rc = M11_GameView_ConfirmMirrorCandidate(&state, 0);
    CHECK(confirm_rc == 1, "F0282 C160 confirm succeeds on imported candidate");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.inventoryPanelActive == 0 &&
          state.world.party.championCount == 1 &&
          state.world.party.champions[0].present == 1,
          "confirm leaves imported champion in the M11 party");
    CHECK(state.world.things->sensors[0].sensorType == 0,
          "confirm disables the champion-mirror sensor");
    CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == -1,
          "disabled mirror sensor no longer reports a front ordinal");
    CHECK(M11_GameView_CsbF0282ChampionPanelGateActive(
              &state, &frontOrdinal, &candidateOrdinal,
              &candidatePartyIndex) == 0,
          "CSB F0282 gate closes after confirmed C160 route");

    free_synthetic_front_sensor_state(&state);

    printf("\n=== Summary: %d checks, %d failures ===\n",
           g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
