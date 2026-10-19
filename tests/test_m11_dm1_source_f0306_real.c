/* Authentic Atari ST / FM Towns F0306 -> F0309 runtime regression. */
#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_source_profile(const char *platform, const char *archive,
                                 int expectedFirstOperandFirst)
{
    M11_GameViewState state;
    struct ChampionState_Compat *live;
    unsigned short expectedMaxLoad;
    unsigned short secondOperandMaxLoad;
    unsigned short originalStamina;
    unsigned short originalWounds;
    unsigned short originalMaxLoad;
    unsigned short originalLoad;
    int firstProfileBaseline = -1;
    int secondProfileBaseline = -1;
    int feetIcon;
    int weight;
    int tick;
    int started;
    int isFmtowns = strcmp(platform, "FM Towns") == 0;

    if (!archive || !archive[0]) {
        printf("SKIP: %s archive environment variable is not set\n", platform);
        return 77;
    }
    {
        FILE *media = fopen(archive, "rb");
        if (!media) {
            printf("SKIP: %s original archive is unavailable: %s\n",
                   platform, archive);
            return 77;
        }
        fclose(media);
    }

    M11_GameView_Init(&state);
    if (isFmtowns) {
        M11_GameLaunchSpec spec = {0};
        spec.title = "Dungeon Master";
        spec.gameId = "dm1";
        spec.sourceId = "dm1";
        spec.dataDir = archive;
        spec.dm1Fmtowns = 1;
        spec.dm1FmtownsJapanese = 1;
        started = M11_GameView_Start(&state, &spec);
    } else {
        started = M11_GameView_StartDm1(&state, archive);
    }
    if (!started || !state.world.dungeon ||
        !state.world.things || !state.world.things->loaded ||
        !state.mirrorCatalogAvailable || state.mirrorCatalog.count <= 0 ||
        state.world.pc34F0306FirstOperandFirst != expectedFirstOperandFirst) {
        fprintf(stderr,
                "FAIL: %s original archive did not load with its source profile (start=%d dungeon=%d things=%d loaded=%d mirrors=%d catalog=%d profile=%d expected=%d)\n",
                platform, started, state.world.dungeon != NULL,
                state.world.things != NULL,
                state.world.things && state.world.things->loaded,
                state.mirrorCatalogAvailable, state.mirrorCatalog.count,
                state.world.pc34F0306FirstOperandFirst,
                expectedFirstOperandFirst);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* F0671 recruits ordinal zero from the archive's real mirror catalog.
     * The native runtime uses this path after selecting a mirror; it makes
     * the original parsed champion a live party member for F0309 refresh. */
    if (!F0671_CHAMPION_MirrorCatalogRecruitOrdinal_Compat(
            &state.mirrorCatalog, 0, &state.world.party) ||
        state.world.party.championCount != 1 ||
        state.world.party.champions[0].stamina.maximum < 4u ||
        state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] == 0u) {
        fprintf(stderr, "FAIL: %s archive has no usable original champion record\n",
                platform);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    live = &state.world.party.champions[0];
    originalStamina = live->stamina.current;
    originalWounds = live->wounds;
    originalMaxLoad = live->maxLoad;
    originalLoad = live->load;

    /* Keep the authentic champion stats, footwear and object tables, while
     * applying only the runtime state needed to enter F0309's low-stamina
     * branch and exercise the source compiler's operand-order difference. */
    live->stamina.current = (unsigned short)(live->stamina.maximum / 4u);
    live->wounds = (unsigned short)(live->wounds | DM1_WOUND_LEGS);
    feetIcon = dm1_v1_dungeon_get_object_icon_index_pc34(
        state.world.things, live->inventory[CHAMPION_SLOT_FEET],
        state.world.party.direction);
    expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
        live, feetIcon, 1);
    secondOperandMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
        live, feetIcon, 0);
    if (expectedMaxLoad == secondOperandMaxLoad) {
        fprintf(stderr,
                "FAIL: %s authentic champion state does not distinguish the F0306 profiles\n",
                platform);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* F0312 asks F0309 for the loaded source world. Find an object weight at
     * which the two platform profiles have observably different baselines. */
    for (weight = 0; weight <= 512; ++weight) {
        state.world.pc34F0306FirstOperandFirst = 1;
        firstProfileBaseline = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, live, 0, weight);
        state.world.pc34F0306FirstOperandFirst = 0;
        secondProfileBaseline = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, live, 0, weight);
        if (firstProfileBaseline != secondProfileBaseline) break;
    }
    state.world.pc34F0306FirstOperandFirst = expectedFirstOperandFirst;
    if (weight > 512) {
        fprintf(stderr, "FAIL: %s F0312 did not expose the F0306 profile difference\n",
                platform);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    live->maxLoad = 1u;
    (void)M11_GameView_AdvanceIdleTick(&state);
    for (tick = 0; tick < 3; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
        live, dm1_v1_dungeon_get_object_icon_index_pc34(
            state.world.things, live->inventory[CHAMPION_SLOT_FEET],
            state.world.party.direction), expectedFirstOperandFirst);
    secondOperandMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
        live, dm1_v1_dungeon_get_object_icon_index_pc34(
            state.world.things, live->inventory[CHAMPION_SLOT_FEET],
            state.world.party.direction), 0);
    if ((int)live->maxLoad != (int)expectedMaxLoad ||
        live->maxLoad == secondOperandMaxLoad) {
        fprintf(stderr,
                "FAIL: %s idle runtime did not cache first-operand-first F0309 (got %u expected %u, second %u)\n",
                platform, (unsigned)live->maxLoad,
                (unsigned)expectedMaxLoad, (unsigned)secondOperandMaxLoad);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Restore source champion fields before destroying the runtime. */
    live->stamina.current = originalStamina;
    live->wounds = originalWounds;
    live->maxLoad = originalMaxLoad;
    live->load = originalLoad;
    M11_GameView_Shutdown(&state);
    printf("PASS: %s authentic F0306 profile drives depleted-stamina F0309 and F0312 runtime\n",
           platform);
    return 0;
}

int main(void)
{
    const char *atari = getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
    const char *fmtowns = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    int result;

    result = verify_source_profile("Atari ST", atari, 1);
    if (result != 0) return result;
    return verify_source_profile("FM Towns", fmtowns, 1);
}
